#include "Application.h"
#include <boost/program_options.hpp>
#include <dots/io/IoContext.h>
#include <dots/io/services/TimerService.h>
#include <dots/io/services/TcpService.h>
#include <dots/type/Registry.h>
#include <DotsClient.dots.h>

namespace dots
{
	Application::Application(const string& name, int& argc, char* argv[])
	{
		m_instance = this;
		parseProgramOptions(argc, argv);

		// Start Transceiver
		// Connect to dotsd

		auto dotsSocket = asio::use_service<TcpService>(static_cast<asio::execution_context&>(ioContext())).connect(m_serverAddress, m_serverPort);
		
		if (not transceiver().start(name, dotsSocket))
		{
			throw std::runtime_error("unable to start transceiver");
		}

		LOG_DEBUG_S("run until state connected...");
		while (not transceiver().connected())
		{
			ioContext().run_one();
		}
		LOG_DEBUG_S("run one done");

		DotsClient{ DotsClient::id_t_i{ transceiver().connection().clientId() }, DotsClient::running_t_i{ true } }._publish();
	}

	Application::~Application()
	{
		ioContext().stop();
	}

	int Application::exec()
	{
		m_exitCode = 0;
		ioContext().run();

		return m_exitCode;
	}

	int Application::execOne(const std::chrono::milliseconds& timeout)
	{
		m_exitCode = 0;
		ioContext().run_one_for(std::chrono::milliseconds{ 10 });

		return m_exitCode;
	}

	void Application::exit(int exitCode)
	{
		m_exitCode = exitCode;
		ioContext().stop();
	}

	IoContext& Application::ioContext() const
	{
		return IoContext::Instance();
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
			("dots-address", po::value<string>()->default_value("127.0.0.1"), "address to bind to")
			("dots-port", po::value<int>()->default_value(11234), "port to bind to")
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

		m_serverAddress = vm["dots-address"].as<string>();
		m_serverPort = vm["dots-port"].as<int>();
	}
}