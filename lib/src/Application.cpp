#undef DOTS_NO_GLOBAL_TRANSCEIVER
#include <dots/Application.h>
#include <boost/program_options.hpp>
#include <dots/tools/logging.h>
#include <DotsClient.dots.h>

namespace dots
{
    Application::Application(const std::string& name, int argc, char* argv[], std::optional<GuestTransceiver> guestTransceiver/* = std::nullopt*/, bool handleExitSignals/* = true*/) :
        m_exitCode(EXIT_SUCCESS),
        m_transceiver(nullptr),
        m_guestTransceiverStorage{ std::move(guestTransceiver) }
    {
        parseGuestTransceiverArgs(argc, argv);
        GuestTransceiver* transceiver;

        if (m_guestTransceiverStorage == std::nullopt || &*m_guestTransceiverStorage == &dots::transceiver())
        {
            transceiver = &set_transceiver(m_openEndpoint->userName().empty() ? name : m_openEndpoint->userName());
            transceiver->open(io::global_publish_types(), io::global_subscribe_types(), *m_openEndpoint);
        }
        else
        {
            transceiver = &*m_guestTransceiverStorage;
            transceiver->open(*m_openEndpoint);
        }

        m_transceiver = transceiver;

        if (handleExitSignals)
        {
            m_signals.emplace(ioContext(), SIGINT, SIGTERM);
            m_signals->async_wait([this](boost::system::error_code/* error*/, int/* signalNumber*/){ exit(); });
        }

        while (!transceiver->connected())
        {
            ioContext().run_one();
        }

        transceiver->publish(DotsClient{ DotsClient::id_i{ transceiver->connection().selfId() }, DotsClient::running_i{ true } });
    }

    Application::Application(int argc, char* argv[], HostTransceiver hostTransceiver, bool handleExitSignals) :
        m_exitCode(EXIT_SUCCESS),
        m_transceiver(nullptr),
        m_hostTransceiverStorage{ std::move(hostTransceiver) }
    {
        parseHostTransceiverArgs(argc, argv);
        m_transceiver = &*m_hostTransceiverStorage;
        m_hostTransceiverStorage->listen(m_listenEndpoints);

        if (handleExitSignals)
        {
            m_signals.emplace(ioContext(), SIGINT, SIGTERM);
            m_signals->async_wait([this](boost::system::error_code/* error*/, int/* signalNumber*/){ exit(); });
        }
    }

    Application::~Application()
    {
        ioContext().stop();

        if (m_hostTransceiverStorage != std::nullopt)
        {
            m_hostTransceiverStorage = std::nullopt;
        }
        else if (m_guestTransceiverStorage != std::nullopt)
        {
            m_guestTransceiverStorage = std::nullopt;
        }
        else
        {
            set_transceiver();
        }

        ioContext().restart();
        ioContext().poll();
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

    void Application::parseGuestTransceiverArgs(int argc, char* argv[])
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

    void Application::parseHostTransceiverArgs(int argc, char* argv[])
    {
        namespace po = boost::program_options;
        
        po::options_description options{ "Allowed options" };
        options.add_options()
            ("dots-listen", po::value<std::vector<std::string>>(), "local endpoint URI to listen on for incoming guest connections (e.g. tcp://127.0.0.1:11234, ws://127.0.0.1, uds:/tmp/dots_uds.socket")
        ;

        po::variables_map args;
        po::store(po::basic_command_line_parser<char>(argc, argv).options(options).allow_unregistered().run(), args);
        po::notify(args);
        
        m_listenEndpoints = [&args]
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
                return std::vector{ io::Endpoint{ "tcp://127.0.0.1" } };
            }
        }();
    }
}
