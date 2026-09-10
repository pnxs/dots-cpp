// SPDX-License-Identifier: LGPL-3.0-only
// Pinger - performance comparison counterpart to dots-go's examples/pinger.
//
// Same Pinger model, same publish loop (fixed key id=1, varied
// message/sequence), so end-to-end roundtrip cost can be compared
// directly against the Go client against the same dotsd broker.
//
// Lifecycle:
//   1. dots::Application constructor opens the connection and blocks
//      until preload finishes (analogous to Go's app.Open + Run).
//   2. Subscribe[Pinger] - handler counts echoes of our own publishes.
//   3. Publish `--loops` Pingers; exit once that many self-echoes
//      have come back.
//   4. Report wall-clock duration of the publish/drain phase.
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <dots/Application.h>
#include <Pinger.dots.h>

int main(int argc, char* argv[])
{
    const std::string AppName = "pinger";
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
        std::cout << "== pinger example - dots-cpp roundtrip benchmark ==\n";

        dots::Application app{ AppName, argc, argv };
        std::cout << "connected (preload finished)\n";

        uint32_t echoes = 0;
        const auto t0 = std::chrono::steady_clock::now();

        dots::Subscription subscription = dots::subscribe<Pinger>([&](const dots::Event<Pinger>& event)
        {
            if (!event.isFromMyself())
                return;

            if (++echoes == loops)
            {
                const auto elapsed = std::chrono::steady_clock::now() - t0;
                const auto us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
                const double secs = static_cast<double>(us) / 1'000'000.0;
                const double rate = secs > 0.0 ? static_cast<double>(loops) / secs : 0.0;
                std::cout << "received " << echoes << " echoes in "
                          << us << " us (" << rate << " msg/s)\n";
                app.exit();
            }
        });

        std::cout << "publishing " << loops << " Pinger(s)\n";
        for (uint32_t i = 1; i <= loops; ++i)
        {
            dots::publish(Pinger{
                .id = 1,
                .message = "hello from dots-cpp pinger, attempt " + std::to_string(i),
                .sequence = static_cast<uint64_t>(i)
            });
        }

        return app.exec();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR running " << AppName << " -> " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
