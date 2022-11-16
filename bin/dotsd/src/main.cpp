// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>

#include <boost/program_options.hpp>
#include <iostream>
#include <optional>
#include <dots/tools/logging.h>
#include <dots/io/Endpoint.h>
#include <DotsDaemon.h>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    try
    {
        po::options_description options("Allowed options");
        options.add_options()
            ("help", "display help message")
            ("daemon-name,n", po::value<std::string>()->default_value("dotsd"), "the name hat will be used by the host transceiver to identify itself")
            #ifdef __linux__
            ("daemonize,d", "indicates whether to use the Linux 'daemon' syscall to detach the application from the controlling terminal")
            #endif
        ;

        po::variables_map args;
        po::store(po::basic_command_line_parser<char>(argc, argv).options(options).allow_unregistered().run(), args);
        po::notify(args);

        if (args.count("help"))
        {
            std::cout << options << "\n";
            return EXIT_SUCCESS;
        }

        LOG_NOTICE_S("starting dotsd...");

        dots::DotsDaemon dotsDaemon{ args["daemon-name"].as<std::string>(), argc, argv };

        #ifdef __linux__
        if (args.count("daemonize") && ::daemon(0, 0) == -1)
        {
            LOG_CRIT_S("could not daemonize dotsd application: " << strerror(errno));
            return EXIT_FAILURE;
        }
        #endif

        return dotsDaemon.exec();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR_S("ERROR running dotsd -> " << e.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        LOG_ERROR_S("ERROR running dotsd -> <unknown exception>");
        return EXIT_FAILURE;
    }
}
