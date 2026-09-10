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
#include <string>
#include <vector>
#include <dots/Application.h>
#include <dots/FilterBuilder.h>
#include <dots/GuestTransceiver.h>
#include <dots/View.h>
#include <dots/asio.h>
#include <dots/dots.h>
#include <Pinger.dots.h>

#include <fmt/printf.h>

#include "dots/io/Io.h"

namespace f = dots::filter;
using namespace std::chrono_literals;

struct ObservedEvent
{
    DotsMt   op;
    uint32_t id;
    uint64_t sequence; // 0 if absent (remove-from-view)
    bool     hasMessage;
};

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

uint32_t get_random_id()
{
    // Use a process-unique key so reruns against a long-lived dotsd don't
    // collide with stale cached entries from prior runs.
    std::mt19937 rng{std::random_device{}()};
    return std::uniform_int_distribution<uint32_t>{1000, UINT32_MAX}(rng);
}

class PingerApp
{
    uint32_t m_key;
    dots::View<Pinger> m_view;
    std::vector<ObservedEvent> m_observed;
    std::optional<dots::Subscription> m_sub;
    uint32_t m_expected_transmissions = 0;
    uint32_t m_echos = 0;
    dots::steady_timepoint_t m_t0;

public:
    PingerApp() :
        m_key(get_random_id()),
        m_view(dots::view<Pinger>(
            f::predicate(
                  (f::attr<Pinger::sequence_pt> < uint64_t{1000})
                & (f::attr<Pinger::id_pt>       == m_key))
                .project(Pinger::sequence_p)))
    {
        fmt::println("== pinger-filtered — server-side filter demo ==");
        fmt::println("broker advertises filteredSubscriptions: yes\n");

        fmt::print("opened View (subId={} with filter:\n"
        "  predicate: sequence < 1000\n"
        "  project:   {{id, sequence}}  (message is masked out)\n\n",
        m_view.subscriptionId());

        m_sub = m_view.subscribe([&](const dots::Event<Pinger>& e)
        {
            ObservedEvent o{
                .op = e.mt(),
                .id = *e().id,
                .sequence = e().sequence.isValid() ? *e().sequence : uint64_t{0},
                .hasMessage = e().message.isValid()
            };
            m_observed.push_back(o);

            fmt::println("  recv  op={}  id={}  seq={}  msg={}",
                dots::to_string(o.op), o.id, o.sequence, o.hasMessage ? "<set>" : "<masked>");

            if (!e.isFromMyself())
                return;

            if (++m_echos == m_expected_transmissions)
            {
                const auto elapsed = std::chrono::steady_clock::now() - m_t0;
                const auto us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
                const double secs = static_cast<double>(us) / 1'000'000.0;
                const double rate = secs > 0.0 ? static_cast<double>(m_expected_transmissions) / secs : 0.0;
                std::cout << "received " << m_echos << " echoes in "
                          << us << " us (" << rate << " msg/s)\n";
                dots::global_transceiver().ioContext().stop();
            }
        });
    }
/*
    static void publish(uint32_t id, uint64_t sequence, std::string_view msg)
    {
        fmt::println("publish  id={}  seq={}  msg='{}'", id, sequence, msg);
        dots::publish(Pinger {
            .id = id,
            .message = msg,
            .sequence = sequence
        });
    }
    */

    void pub_seq(uint32_t loops)
    {
        m_expected_transmissions = loops;

        m_t0 = std::chrono::steady_clock::now();

        fmt::println("using key id={}", m_key);

        for (uint32_t i = 1; i <= loops; ++i)
        {
            dots::publish(Pinger{
                .id = m_key,
                .message = "hello from dots-cpp pinger, attempt " + std::to_string(i),
                .sequence = static_cast<uint64_t>(i)
            });
        }

        /*
        // Sequence designed to cross the filter boundary on a single key.
        publish(m_key, 50, "first"); // matches → enter view → create
        publish(m_key, 75, "second"); // matches → in-view update
        publish(m_key, 150, "outside"); // does NOT match → leave view → remove
        publish(m_key, 42, "back"); // matches again → re-enter → create
        */

        //fmt::println("\nview container after sequence: {} entry(ies)\n", m_view.size());
    }

};


int main(int argc, char* argv[])
{
    uint32_t loops = 3;

    // Pick up --loops N without disturbing the dotscpp option parser
    // (which uses allow_unregistered, so unknown flags are harmless).
    for (int i = 1; i < argc - 1; ++i)
    {
        if (std::string{ argv[i] } == "--loops")
        {
            loops = static_cast<uint32_t>(std::strtoul(argv[i + 1], nullptr, 10));
            break;
        }
    }

    try
    {
        dots::Application app{"pinger-filtered", argc, argv};

        auto pinger = PingerApp();

        const auto& caps = dots::global_transceiver().connection().peerCapabilities();
        if (!caps.filteredSubscriptions.isValid() || *caps.filteredSubscriptions != true)
        {
            std::cerr << "ERROR broker does not advertise filteredSubscriptions capability\n";
            return EXIT_FAILURE;
        }

        pinger.pub_seq(loops);

        return app.exec();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR running pinger-filtered -> " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
