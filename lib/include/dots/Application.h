#pragma once
#include <chrono>
#include <optional>
#include <dots/dots.h>
#include <dots/io/GuestTransceiver.h>
#include <dots/io/Endpoint.h>

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

        inline static Application* m_instance = nullptr;
        int m_exitCode;
        std::optional<io::Endpoint> m_openEndpoint;
        std::optional<std::string> m_authSecret;
    };
}