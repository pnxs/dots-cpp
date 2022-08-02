// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <iostream>
#include <dots/Application.h>
#include <RoundtripData.dots.h>

int main(int argc, char* argv[])
{
    const std::string AppName = "roundtrip";
    constexpr uint32_t NumInstances = 20;

    try
    {
        // create application
        dots::Application app{ AppName, argc, argv };

        // create subscription to 'RoundtripData' type with a corresponding event handler
        dots::Subscription subscription = dots::subscribe<RoundtripData>([&](const dots::Event<RoundtripData>& event)
        {
            if (const RoundtripData& roundtripData = event(); event.isFromMyself())
            {
                std::cout << dots::to_string(roundtripData) << "\n";

                if (uint32_t i = *roundtripData.id + 1; i < NumInstances)
                {

                    dots::publish(RoundtripData{
                        RoundtripData::id_i{ i },
                        RoundtripData::someString_i{ "foobar" },
                        RoundtripData::someFloat_i{ 3.1415f }
                    });
                }
                else
                {
                    std::cout << "finished roundtrip of '" << NumInstances << "' instances" << "\n";
                    app.exit();
                }
            }
        });

        std::cout << "starting roundtrip of '" << NumInstances << "' instances" << "\n";

        // publish initial 'RoundtripData' instance
        dots::publish(RoundtripData{
            RoundtripData::id_i{ 0 },
            RoundtripData::someString_i{ "foobar" },
            RoundtripData::someFloat_i{ 3.1415f }
        });

        // execute event loop of application 
        return app.exec();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR running " << AppName << " -> " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "ERROR running " << AppName << " -> <unknown exception>" << "\n";
        return EXIT_FAILURE;
    }
}
