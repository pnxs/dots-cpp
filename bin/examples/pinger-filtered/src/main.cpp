// SPDX-License-Identifier: LGPL-3.0-only
// pinger-filtered — end-to-end demonstration of server-side filtered
// subscriptions. Publishes a deliberately-crafted sequence that crosses the
// filter boundary on the same key, exercising all four cases of the broker's
// dispatch state machine: enter view, in-view update, leave view, re-enter.
//
// Verifies the events delivered to the View match expectations and exits
// non-zero on mismatch — usable as a smoke test.
#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <dots/Application.h>
#include <dots/FilterBuilder.h>
#include <dots/GuestTransceiver.h>
#include <dots/View.h>
#include <dots/asio.h>
#include <dots/dots.h>
#include <Pinger.dots.h>

namespace
{
    struct ObservedEvent
    {
        DotsMt   op;
        uint32_t id;
        uint64_t sequence; // 0 if absent (remove-from-view)
        bool     hasMessage;
    };

    std::string opName(DotsMt mt)
    {
        switch (mt)
        {
            case DotsMt::create: return "create";
            case DotsMt::update: return "update";
            case DotsMt::remove: return "remove";
        }
        return "?";
    }

    // Pretty-print an expected vs. actual mismatch with line-by-line diff.
    bool verify(const std::vector<ObservedEvent>& expected,
                const std::vector<ObservedEvent>& actual)
    {
        bool ok = expected.size() == actual.size();
        const size_t n = std::min(expected.size(), actual.size());
        for (size_t i = 0; i < n; ++i)
        {
            const auto& e = expected[i];
            const auto& a = actual[i];
            const bool match = e.op == a.op && e.id == a.id && e.sequence == a.sequence;
            if (!match) ok = false;
        }
        return ok;
    }
}

int main(int argc, char* argv[])
{
    namespace f = dots::filter;
    using namespace std::chrono_literals;

    try
    {
        dots::Application app{ "pinger-filtered", argc, argv };

        const auto& caps = dots::global_transceiver().connection().peerCapabilities();
        if (!caps.filteredSubscriptions.isValid() || *caps.filteredSubscriptions != true)
        {
            std::cerr << "ERROR broker does not advertise filteredSubscriptions capability\n";
            return EXIT_FAILURE;
        }
        std::cout << "== pinger-filtered — server-side filter demo ==\n";
        std::cout << "broker advertises filteredSubscriptions: yes\n\n";

        // Filter: only Pingers with sequence < 100, project to {id, sequence}
        // (drop the 'message' field on the wire).
        auto view = dots::view<Pinger>(
            f::predicate(f::attr<Pinger::sequence_pt> < uint64_t{100})
             .project(Pinger::sequence_p));

        std::cout << "opened View (subId=" << view.subscriptionId() << ") with filter:\n"
                     "  predicate: sequence < 100\n"
                     "  project:   {id, sequence}  (message is masked out)\n\n";

        std::vector<ObservedEvent> observed;
        auto sub = view.subscribe([&](const dots::Event<Pinger>& e)
        {
            ObservedEvent o{
                .op         = e.mt(),
                .id         = *e().id,
                .sequence   = e().sequence.isValid() ? *e().sequence : uint64_t{0},
                .hasMessage = e().message.isValid()
            };
            observed.push_back(o);

            std::cout << "  recv  op=" << opName(o.op)
                      << "  id=" << o.id
                      << "  seq=" << o.sequence
                      << "  msg=" << (o.hasMessage ? "<set>" : "<masked>")
                      << "\n";
        });

        // Helper to publish a Pinger and run the io_context briefly so the
        // echo round-trips before the next publish.
        auto publish = [&](uint32_t id, uint64_t sequence, std::string_view msg)
        {
            std::cout << "publish  id=" << id << "  seq=" << sequence
                      << "  msg='" << msg << "'\n";
            Pinger p;
            p.id = id;
            p.sequence = sequence;
            p.message = std::string{ msg };
            dots::publish(p);

            // Spin the event loop to let the echo round-trip back. run_for
            // blocks until either work is exhausted or the timeout elapses,
            // which is what we want here (poll() is non-blocking and races
            // ahead of the TCP receive).
            auto& ctx = app.transceiver().ioContext();
            ctx.restart();
            ctx.run_for(100ms);
        };

        // Use a process-unique key so reruns against a long-lived dotsd don't
        // collide with stale cached entries from prior runs.
        std::mt19937 rng{ std::random_device{}() };
        const uint32_t key = std::uniform_int_distribution<uint32_t>{ 1000, UINT32_MAX }(rng);
        std::cout << "using key id=" << key << "\n";

        // Sequence designed to cross the filter boundary on a single key.
        publish(key,  50, "first");   // matches → enter view → create
        publish(key,  75, "second");  // matches → in-view update
        publish(key, 150, "outside"); // does NOT match → leave view → remove
        publish(key,  42, "back");    // matches again → re-enter → create

        std::cout << "\nview container after sequence: " << view.size() << " entry(ies)\n";

        // On a remove event, Event<T>::operator()() returns the cached "updated"
        // snapshot — i.e. the last in-view value, not the key-only wire payload.
        // (DotsMt::remove ⇒ "the instance with this key, which last looked like
        // <updated>, is now gone from the view.") The seq=75 below is that
        // last-known value, not a key fragment.
        std::vector<ObservedEvent> expected{
            { DotsMt::create, key,  50, false },
            { DotsMt::update, key,  75, false },
            { DotsMt::remove, key,  75, false }, // remove carries last cached state
            { DotsMt::create, key,  42, false },
        };

        const bool ok = verify(expected, observed);
        std::cout << "\n" << (ok ? "PASS" : "FAIL")
                  << ": expected " << expected.size()
                  << " events, observed " << observed.size() << "\n";

        if (!ok)
        {
            std::cout << "\n--- expected ---\n";
            for (auto& e : expected)
                std::cout << "  " << opName(e.op) << "  id=" << e.id << "  seq=" << e.sequence << "\n";
            std::cout << "--- observed ---\n";
            for (auto& a : observed)
                std::cout << "  " << opName(a.op) << "  id=" << a.id << "  seq=" << a.sequence << "\n";
        }

        return ok ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR running pinger-filtered -> " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
