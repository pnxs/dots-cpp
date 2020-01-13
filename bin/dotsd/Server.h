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

        explicit Server(listener_ptr_t&& listener, const string& name);

    private:
        void updateServerStatus();

        DotsStatistics receiveStatistics() const;
        DotsCacheStatus cacheStatus() const;

        std::string m_name;
        ConnectionManager m_connectionManager;
        DotsDaemonStatus m_daemonStatus;
    };
}
