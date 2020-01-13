#pragma once

#include "dots/cpp_config.h"
#include "ConnectionManager.h"
#include "DotsDaemonStatus.dots.h"

namespace dots
{
    class Server
    {
    public:
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;

        explicit Server(std::unique_ptr<Listener>&& listener, const string& name);

    private:
        void handleCleanupTimer();
        void updateServerStatus();

        std::string m_name;
        ConnectionManager m_connectionManager;
        DotsDaemonStatus m_daemonStatus;
    };
}
