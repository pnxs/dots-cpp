#include <dots/io/Io.h>
#include <dots/io/services/ChannelService.h>
#include <dots/io/channels/TcpListener.h>
#include <dots/common/logging.h>
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
            ("dots-address", po::value<string>()->default_value("127.0.0.1"), "address to bind to")
            ("dots-port", po::value<string>()->default_value("11234"), "port to bind to")
            ("server-name,n", po::value<string>()->default_value("dotsd"), "set servername")
            ("daemon,d", "daemonize")
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

    string host = vm["dots-address"].as<string>();
    string port = vm["dots-port"].as<string>();

    dots::Server::listeners_t listeners;
    listeners.emplace_back(dots::io::global_service<dots::io::ChannelService>().makeListener<dots::io::TcpListener>(host, port));
    std::optional<dots::Server> server{ std::in_place, std::move(serverName), std::move(listeners) };
    LOG_NOTICE_S("Listen to " << host << ":" << port);

    signals.async_wait([&](auto /*ec*/, int /*signo*/) {
        LOG_NOTICE_S("stopping server");
        dots::io::global_io_context().stop();
        server.reset();
    });

    if (vm.count("daemon"))
    {
        if (daemon(0, 0) == -1)
        {
			LOG_CRIT_S("could not start daemon: " << errno);
			return EXIT_FAILURE;
        }
    }

    LOG_DEBUG_S("run mainloop");
    io_context.run();
    return 0;
}
