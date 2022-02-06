#undef DOTS_NO_GLOBAL_TRANSCEIVER
#include <dots/Application.h>
#include <boost/program_options.hpp>
#include <dots/io/Io.h>
#include <dots/tools/logging.h>
#include <DotsClient.dots.h>

namespace dots
{
    Application::Application(const std::string& name, int argc, char* argv[], bool handleExitSignals/* = true*/) :
        m_exitCode(EXIT_SUCCESS)
    {
        if (handleExitSignals)
        {
            m_signals.emplace(io::global_io_context(), SIGINT, SIGTERM);
            m_signals->async_wait([this](boost::system::error_code/* error*/, int/* signalNumber*/){ exit(); });
        }

        parseProgramOptions(argc, argv);

        GuestTransceiver& globalGuestTransceiver = set_transceiver(m_openEndpoint->userName().empty() ? name : m_openEndpoint->userName());
        const Connection& connection = globalGuestTransceiver.open(io::global_publish_types(), io::global_subscribe_types(), *m_openEndpoint);

        while (!connection.connected())
        {
            io::global_io_context().run_one();
        }

        globalGuestTransceiver.publish(DotsClient{ DotsClient::id_i{ connection.selfId() }, DotsClient::running_i{ true } });
    }

    Application::~Application()
    {
        io::global_io_context().stop();
    }

    int Application::exec()
    {
        m_exitCode = EXIT_SUCCESS;
        io::global_io_context().run();

        return m_exitCode;
    }

    int Application::execOne(const std::chrono::milliseconds& timeout)
    {
        m_exitCode = EXIT_SUCCESS;
        io::global_io_context().run_one_for(timeout);

        return m_exitCode;
    }

    void Application::exit(int exitCode)
    {
        m_exitCode = exitCode;
        io::global_io_context().stop();
    }

    void Application::parseProgramOptions(int argc, char* argv[])
    {
        namespace po = boost::program_options;
        
        po::options_description options("Allowed options");
        options.add_options()
            ("dots-auth-secret", po::value<std::string>(), "secret used during authentication (this can also be given as part of the --dots-open endpoint)")
            ("dots-open", po::value<std::string>()->default_value("tcp://127.0.0.1:11234"), "remote endpoint URI to open for host connection (e.g. tcp://127.0.0.1:11234, ws://127.0.0.1, uds:/tmp/dots.socket")
        ;

        po::variables_map args;
        po::store(po::basic_command_line_parser<char>(argc, argv).options(options).allow_unregistered().run(), args);
        po::notify(args);

        m_openEndpoint.emplace(args["dots-open"].as<std::string>());

        if (m_openEndpoint->scheme() == "tcp" && m_openEndpoint->port().empty())
        {
            m_openEndpoint->setPort("11234");
        }

        if (auto it = args.find("dots-auth-secret"); it != args.end())
        {
            m_openEndpoint->setUserPassword(it->second.as<std::string>());
        }
        else if (const char* dotsAuthSecret = ::getenv("DOTS_AUTH_SECRET"); dotsAuthSecret != nullptr)
        {
            m_openEndpoint->setUserPassword(dotsAuthSecret);
        }
    }
}
