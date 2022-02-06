
#include <boost/program_options.hpp>
#include <iostream>
#include <optional>
#include <dots/tools/logging.h>
#include <dots/io/Endpoint.h>
#include <Server.h>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    po::options_description options("Allowed options");
    options.add_options()
            ("help", "display help message")
            ("server-name,n", po::value<std::string>()->default_value("dotsd"), "set servername")
            ("dots-listen", po::value<std::vector<std::string>>(), "local endpoint URI to listen on for incoming guest connections (e.g. tcp://127.0.0.1:11234, ws://127.0.0.1, uds:/tmp/dots_uds.socket")
            #ifdef __linux__
            ("daemon,d", "daemonize")
            #endif
    ;

    po::variables_map args;
    po::store(po::parse_command_line(argc, argv, options), args);
    po::notify(args);

    if(args.count("help")) 
    {
        std::cout << options << "\n";
        return EXIT_SUCCESS;
    }

    LOG_NOTICE_S("starting dotsd...");

    std::vector<dots::io::Endpoint> listenEndpoints = [&args]
    {
        if (args.count("dots-listen"))
        {
            std::vector<dots::io::Endpoint> listenEndpoints;

            for (const std::string& listenEndpointUri : args["dots-listen"].as<std::vector<std::string>>())
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

    boost::asio::io_context& io_context = dots::io::global_io_context();
    std::optional<dots::Server> server{ std::in_place, args["server-name"].as<std::string>(), io_context, std::move(listenEndpoints) };

    boost::asio::signal_set signals{ io_context, SIGINT, SIGTERM };
    signals.async_wait([&](auto /*ec*/, int /*signo*/) {
        dots::io::global_io_context().stop();
        server.reset();
    });

    #ifdef __linux__
    if (args.count("daemon") && daemon(0, 0) == -1)
    {
        LOG_CRIT_S("could not daemonize dotsd application: " << errno);
        return EXIT_FAILURE;
    }
    #endif

    io_context.run();
    return EXIT_SUCCESS;
}
