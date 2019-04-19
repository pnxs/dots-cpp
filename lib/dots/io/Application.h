#pragma once
#include <dots/cpp_config.h>
#include <dots/io/Transceiver.h>

namespace asio
{
	class io_context;
	typedef io_context io_service;
}

namespace dots
{
	struct AsioEventLoop;

	struct Application
	{
		Application(const string& name, int& argc, char* argv[]);
		virtual ~Application();

		virtual int exec();
		virtual void exit(int exitCode = 0);

		AsioEventLoop& eventLoop() const;
		asio::io_service& ioService() const;

		static Application* instance();

	private:

		void parseProgramOptions(int argc, char* argv[]);
		
		inline static Application* m_instance = nullptr;		
		int m_exitCode;
		std::string m_serverAddress;
		int m_serverPort;
	};
}