#undef DOTS_NO_GLOBAL_TRANSCEIVER
#include <dots/Application.h>
#include <boost/program_options.hpp>
#include <dots/tools/logging.h>
#include <DotsClient.dots.h>

namespace dots
{
    Application::Application(const std::string& name, int argc, char* argv[], std::optional<GuestTransceiver> transceiver/* = std::nullopt*/, bool handleExitSignals/* = true*/) :
        m_exitCode(EXIT_SUCCESS),
        m_transceiver(nullptr),
        m_transceiverStorage{ std::move(transceiver) }
    {
        parseProgramOptions(argc, argv);

        if (m_transceiverStorage == std::nullopt || &*m_transceiverStorage == &dots::transceiver())
        {
            m_transceiver = &set_transceiver(m_openEndpoint->userName().empty() ? name : m_openEndpoint->userName());
            m_transceiver->open(io::global_publish_types(), io::global_subscribe_types(), *m_openEndpoint);
        }
        else
        {
            m_transceiver = &*m_transceiverStorage;
            m_transceiver->open(*m_openEndpoint);
        }

        if (handleExitSignals)
        {
            m_signals.emplace(ioContext(), SIGINT, SIGTERM);
            m_signals->async_wait([this](boost::system::error_code/* error*/, int/* signalNumber*/){ exit(); });
        }

        while (!m_transceiver->connected())
        {
            ioContext().run_one();
        }

        m_transceiver->publish(DotsClient{ DotsClient::id_i{ m_transceiver->connection().selfId() }, DotsClient::running_i{ true } });
    }

    Application::~Application()
    {
        ioContext().stop();
    }

    const Transceiver& Application::transceiver() const
    {
        return *m_transceiver;
    }

    Transceiver& Application::transceiver()
    {
        return *m_transceiver;
    }

    int Application::exec()
    {
        m_exitCode = EXIT_SUCCESS;
        ioContext().run();

        return m_exitCode;
    }

    int Application::execOne(const std::chrono::milliseconds& timeout)
    {
        m_exitCode = EXIT_SUCCESS;
        ioContext().run_one_for(timeout);

        return m_exitCode;
    }

    void Application::exit(int exitCode)
    {
        m_exitCode = exitCode;
        ioContext().stop();
    }

    const asio::io_context& Application::ioContext() const
    {
        return m_transceiver->ioContext();
    }

    asio::io_context& Application::ioContext()
    {
        return m_transceiver->ioContext();
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
