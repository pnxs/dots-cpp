#undef DOTS_NO_GLOBAL_TRANSCEIVER 
#include <dots/Application.h>
#include <boost/program_options.hpp>
#include <dots/io/Io.h>
#include <dots/io/channels/TcpChannel.h>
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

        GuestTransceiver& globalGuestTransceiver = dots::transceiver(name);
        const io::Connection& connection = globalGuestTransceiver.open<io::TcpChannel>(io::global_publish_types(), io::global_subscribe_types(), m_authSecret, m_serverAddress, m_serverPort);

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
            ("dots-address", po::value<std::string>()->default_value("127.0.0.1"), "address to bind to")
            ("dots-port", po::value<std::string>()->default_value("11234"), "port to bind to")
            ("auth-secret", po::value<std::string>(), "secret used during authentication")
            ;

        po::variables_map vm;
        po::store(po::basic_command_line_parser<char>(argc, argv).options(desc).allow_unregistered().run(), vm);
        po::notify(vm);

        // parse environment options
        if (::getenv("DOTS_SERVER_ADDRESS"))
        {
            m_serverAddress = getenv("DOTS_SERVER_ADDRESS");
        }

        if (::getenv("DOTS_SERVER_PORT"))
        {
            m_serverPort = atoi("DOTS_SERVER_PORT");
        }

        if (getenv("DOTS_AUTH_SECRET"))
        {
            m_authSecret = getenv("DOTS_AUTH_SECRET");
        }

        m_serverAddress = vm["dots-address"].as<std::string>();
        m_serverPort = vm["dots-port"].as<std::string>();

        if (vm.count("auth-secret") > 0)
        {
            m_authSecret = vm["auth-secret"].as<std::string>();
        }
    }
}