#pragma once
#include <vector>
#include "dots/cpp_config.h"
#include "ConnectionManager.h"
#include "DotsDaemonStatus.dots.h"

namespace dots
{
    struct Server
    {
        using listeners_t = std::vector<listener_ptr_t>;

        Server(std::string name, listeners_t listeners);
		Server(const Server& other) = delete;
		Server(Server&& other) = delete;
		~Server() = default;

		Server& operator = (const Server& rhs) = delete;
		Server& operator = (Server&& rhs) = delete;

    private:

        void updateServerStatus();

        DotsStatistics receiveStatistics() const;
        DotsCacheStatus cacheStatus() const;

        ConnectionManager m_connectionManager;
        DotsDaemonStatus m_daemonStatus;
    };
}
