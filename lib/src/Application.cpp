#undef DOTS_NO_GLOBAL_TRANSCEIVER 
#include <dots/Application.h>
#include <boost/program_options.hpp>
#include <dots/io/Io.h>
#include <dots/io/channels/TcpChannel.h>
#include <dots/io/channels/LegacyTcpChannel.h>
#include <dots/io/channels/WebSocketChannel.h>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <dots/io/channels/UdsChannel.h>
#endif
#include <dots/tools/logging.h>
#include <DotsClient.dots.h>

namespace dots
{
    Application::Application(const std::string& name, int& argc, char* argv[])
    {
        m_instance = this;
        parseProgramOptions(argc, argv);

        // Start Transceiver
        // Connect to dotsd

        GuestTransceiver& globalGuestTransceiver = set_transceiver(m_openEndpoint->userName().empty() ? name : m_openEndpoint->userName());
        const Connection& connection = [&]() -> auto&
        {
            if (m_openEndpoint->scheme() == "tcp")
            {
                return globalGuestTransceiver.open<io::TcpChannel>(io::global_publish_types(), io::global_subscribe_types(), std::move(m_authSecret), *m_openEndpoint);
            }
            else if (m_openEndpoint->scheme() == "tcp-legacy")
            {
                return globalGuestTransceiver.open<io::LegacyTcpChannel>(io::global_publish_types(), io::global_subscribe_types(), std::move(m_authSecret), *m_openEndpoint);
            }
            else if (m_openEndpoint->scheme() == "ws")
            {
                return globalGuestTransceiver.open<io::WebSocketChannel>(io::global_publish_types(), io::global_subscribe_types(), std::move(m_authSecret), *m_openEndpoint);
            }
            #if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
            else if (m_openEndpoint->scheme() == "uds")
            {
                return globalGuestTransceiver.open<io::posix::UdsChannel>(io::global_publish_types(), io::global_subscribe_types(), std::move(m_authSecret), *m_openEndpoint);
            }
            #endif
            else
            {
                throw std::runtime_error{ "unknown or unsupported URI scheme: '" + std::string{ m_openEndpoint->scheme() } + "'" };
            }
        }();


        LOG_DEBUG_S("run until state connected...");
        while (!connection.connected())
        {
            io::global_io_context().run_one();
        }
        LOG_DEBUG_S("run one done");

        globalGuestTransceiver.publish(DotsClient{ DotsClient::id_i{ connection.selfId() }, DotsClient::running_i{ true } });
    }

    Application::~Application()
    {
        io::global_io_context().stop();
    }

    int Application::exec()
    {
        m_exitCode = 0;
        io::global_io_context().run();

        return m_exitCode;
    }

    int Application::execOne(const std::chrono::milliseconds& timeout)
    {
        m_exitCode = 0;
        io::global_io_context().run_one_for(timeout);

        return m_exitCode;
    }

    void Application::exit(int exitCode)
    {
        m_exitCode = exitCode;
        io::global_io_context().stop();
    }

    Application* Application::instance()
    {
        return m_instance;
    }

    void Application::parseProgramOptions(int argc, char* argv[])
    {
        namespace po = boost::program_options;

        // define and parse command line options
        po::options_description desc("Allowed options");
        desc.add_options()
            ("dots-auth-secret", po::value<std::string>(), "secret used during authentication (this can also be given as part of the --dots-open endpoint)")
            ("dots-open", po::value<std::string>()->default_value("tcp://127.0.0.1:11234"), "remote endpoint URI to open for host connection (e.g. tcp://127.0.0.1:11234, ws://127.0.0.1, uds:/tmp/dots_uds.socket")
            ;

        po::variables_map vm;
        po::store(po::basic_command_line_parser<char>(argc, argv).options(desc).allow_unregistered().run(), vm);
        po::notify(vm);

        const po::variable_value& openEndpoint = vm["dots-open"];
        m_openEndpoint.emplace(openEndpoint.as<std::string>());

        if (m_openEndpoint->scheme() == "tcp" && m_openEndpoint->port().empty())
        {
            m_openEndpoint->setPort("11234");
        }

        if (auto it = vm.find("dots-auth-secret"); it != vm.end())
        {
            m_authSecret = it->second.as<std::string>();
        }
        else if (const char* dotsAuthSecret = ::getenv("DOTS_AUTH_SECRET"); dotsAuthSecret != nullptr)
        {
            m_authSecret = dotsAuthSecret;
        }
    }
}
