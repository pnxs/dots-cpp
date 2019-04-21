#pragma once
#include <chrono>
#include <dots/io/Io.h>
#include <dots/cpp_config.h>
#include <dots/io/Transceiver.h>

namespace dots
{
	struct Application
	{
		Application(const string& name, int& argc, char* argv[]);
		virtual ~Application();

		virtual int exec();
		virtual int execOne(const std::chrono::milliseconds& timeout);
		virtual void exit(int exitCode = 0);

		static Application* instance();

	private:

		void parseProgramOptions(int argc, char* argv[]);
		
		inline static Application* m_instance = nullptr;		
		int m_exitCode;
		std::string m_serverAddress;
		int m_serverPort;
	};
}