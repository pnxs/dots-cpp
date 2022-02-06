
#include <dots/tools/logging.h>
#include <dots/io/Endpoint.h>
#include "Server.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <optional>

namespace po = boost::program_options;
using std::string;

int main(int argc, char* argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "display help message")
            ("server-name,n", po::value<string>()->default_value("dotsd"), "set servername")
            ("dots-listen", po::value<std::vector<string>>(), "local endpoint URI to listen on for incoming guest connections (e.g. tcp://127.0.0.1:11234, ws://127.0.0.1, uds:/tmp/dots_uds.socket")
            #ifdef __linux__
            ("daemon,d", "daemonize")
            #endif
            ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    auto serverName = vm["server-name"].as<string>();

    boost::asio::io_context& io_context = dots::io::global_io_context();

    LOG_NOTICE_S("dotsd server");

    boost::asio::signal_set signals(io_context);

    signals.add(SIGINT);
    signals.add(SIGTERM);

    std::vector<dots::io::Endpoint> listenEndpoints = [&vm]
    {
        if (vm.count("dots-listen"))
        {
            std::vector<dots::io::Endpoint> listenEndpoints;

            for (const std::string& listenEndpointUri : vm["dots-listen"].as<std::vector<std::string>>())
            {
                listenEndpoints.emplace_back(listenEndpointUri);
            }

            return listenEndpoints;
        }
        else
        {
            return std::vector{ dots::io::Endpoint{ "tcp://127.0.0.1" } };
        }
    }();
    
    std::optional<dots::Server> server{ std::in_place, std::move(serverName), io_context, std::move(listenEndpoints) };

    signals.async_wait([&](auto /*ec*/, int /*signo*/) {
        LOG_NOTICE_S("stopping server");
        dots::io::global_io_context().stop();
        server.reset();
    });

    #ifdef __linux__
    if (vm.count("daemon"))
    {
        if (daemon(0, 0) == -1)
        {
            LOG_CRIT_S("could not start daemon: " << errno);
            return EXIT_FAILURE;
        }
    }
    #endif

    LOG_DEBUG_S("run mainloop");
    io_context.run();
    return 0;
}
