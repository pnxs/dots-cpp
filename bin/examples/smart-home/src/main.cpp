// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <csignal>
#include <dots/Application.h>
#include <dots/tools/logging.h>
#include <Basement.h>
#include <LivingRoom.h>
#include <Stairwell.h>

int main(int argc, char* argv[])
{
    using namespace dots::type::literals;
    const std::string AppName = "smart-home";

    try
    {
        // create application
        dots::Application app(AppName, argc, argv);

        LOG_INFO_S("started " << AppName);

        examples::Basement basement{ 30s };
        examples::LivingRoom livingRoom;
        examples::Stairwell stairwell;

        // execute event loop of application
        return app.exec();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR_S("ERROR running " << AppName << " -> " << e.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        LOG_ERROR_S("ERROR running " << AppName << " -> <unknown exception>");
        return EXIT_FAILURE;
    }
}
