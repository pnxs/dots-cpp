#undef DOTS_NO_GLOBAL_TRANSCEIVER 
#include <dots/Application.h>
#include <boost/program_options.hpp>
#include <dots/io/Io.h>
#include <dots/io/channels/TcpChannel.h>
#include <dots/io/channels/WebSocketChannel.h>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <dots/io/channels/UdsChannel.h>
#endif
#include <dots/io/Registry.h>
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

        GuestTransceiver& globalGuestTransceiver = dots::transceiver(m_openEndpoint->userName().empty() ? name : m_openEndpoint->userName());
        const io::Connection& connection = [&]() -> auto&
        {
            if (m_openEndpoint->scheme() == "tcp")
            {
                return globalGuestTransceiver.open<io::TcpChannel>(io::global_publish_types(), io::global_subscribe_types(), std::move(m_authSecret), *m_openEndpoint);
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
            ("dots-address", po::value<std::string>(), "address to bind to")
            ("dots-port", po::value<std::string>(), "port to bind to")
            ("auth-secret", po::value<std::string>(), "secret used during authentication")
            ("open,o", po::value<std::string>()->default_value("tcp://127.0.0.1:11234"), "remote endpoint URI to open for host connection (e.g. tcp://127.0.0.1:11234, ws://127.0.0.1, uds:/tmp/dots_uds.socket")
            ;

        po::variables_map vm;
        po::store(po::basic_command_line_parser<char>(argc, argv).options(desc).allow_unregistered().run(), vm);
        po::notify(vm);

        const po::variable_value& openEndpoint = vm["open"];
        m_openEndpoint.emplace(openEndpoint.as<std::string>());

        if (openEndpoint.defaulted())
        {
            if (auto it = vm.find("dots-address"); it != vm.end())
            {
                m_openEndpoint->setHost(it->second.as<std::string>());
            }
            else if (const char* dotsSeverAddress = ::getenv("DOTS_SERVER_ADDRESS"); dotsSeverAddress != nullptr)
            {
                m_openEndpoint->setHost(dotsSeverAddress);
            }

            if (auto it = vm.find("dots-port"); it != vm.end())
            {
                m_openEndpoint->setPort(it->second.as<std::string>());
            }
            else if (const char* dotsSeverPort = ::getenv("DOTS_SERVER_PORT"); dotsSeverPort != nullptr)
            {
                m_openEndpoint->setPort(dotsSeverPort);
            }
        }
        else
        {
            auto warn_about_argument_ignore = [](std::string argName, std::string argValue)
            {
                LOG_WARN_S("ignoring legacy argument '" << argName << "=" << argValue << "' because an endpoint argument was specified");
            };

            if (auto it = vm.find("dots-address"); it != vm.end())
            {
                warn_about_argument_ignore("dots-address", it->second.as<std::string>());
            }

            if (const char* dotsSeverAddress = ::getenv("DOTS_SERVER_ADDRESS"); dotsSeverAddress != nullptr)
            {
                warn_about_argument_ignore("DOTS_SERVER_ADDRESS", dotsSeverAddress);
            }

            if (auto it = vm.find("dots-port"); it != vm.end())
            {
                warn_about_argument_ignore("dots-port", it->second.as<std::string>());
            }

            if (const char* dotsSeverPort = ::getenv("DOTS_SERVER_PORT"); dotsSeverPort != nullptr)
            {
                warn_about_argument_ignore("DOTS_SERVER_PORT", dotsSeverPort);
            }
        }

        if (m_openEndpoint->scheme() == "tcp" && m_openEndpoint->port().empty())
        {
            m_openEndpoint->setPort("11234");
        }

        if (auto it = vm.find("auth-secret"); it != vm.end())
        {
            m_authSecret = it->second.as<std::string>();
        }
        else if (const char* dotsAuthSecret = ::getenv("DOTS_AUTH_SECRET"); dotsAuthSecret != nullptr)
        {
            m_authSecret = dotsAuthSecret;
        }
    }
}