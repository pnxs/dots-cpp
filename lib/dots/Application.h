#pragma once
#include <chrono>
#include <optional>
#include <dots/dots.h>
#include <dots/io/GuestTransceiver.h>

namespace dots
{
	struct Application
	{
		Application(const std::string& name, int& argc, char* argv[]);
		virtual ~Application();

		virtual int exec();
		virtual int execOne(const std::chrono::milliseconds& timeout);
		virtual void exit(int exitCode = 0);

		static Application* instance();

	private:

		void parseProgramOptions(int argc, char* argv[]);

		GuestTransceiver::descriptor_map_t getPreloadPublishTypes() const;
		GuestTransceiver::descriptor_map_t getPreloadSubscribeTypes() const;
		
		inline static Application* m_instance = nullptr;		
		int m_exitCode;
		std::string m_serverAddress;
		std::string m_serverPort;
		std::optional<std::string> m_authSecret;
	};
}