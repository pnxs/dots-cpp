#pragma once
#include <vector>
#include <optional>
#include <dots/Application.h>
#include <DotsDaemonStatus.dots.h>

namespace dots
{
    struct DotsDaemon : Application
    {
        DotsDaemon(std::string name, int argc, char* argv[]);
        DotsDaemon(const DotsDaemon& other) = delete;
        DotsDaemon(DotsDaemon&& other) = delete;
        ~DotsDaemon() = default;

        DotsDaemon& operator = (const DotsDaemon& rhs) = delete;
        DotsDaemon& operator = (DotsDaemon&& rhs) = delete;

    private:

        void handleTransition(const Connection& connection, std::exception_ptr ePtr);
        void cleanUpClients();

        void updateServerStatus();

        DotsDaemonStatus m_daemonStatus;
        Timer m_updateServerStatusTimer;
        Timer m_cleanUpClientsTimer;
    };
}
