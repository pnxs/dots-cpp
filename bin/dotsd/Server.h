#pragma once
#include <vector>
#include <dots/io/HostTransceiver.h>
#include "DotsDaemonStatus.dots.h"

namespace dots
{
    struct Server
    {
        using listeners_t = std::vector<io::listener_ptr_t>;

        Server(std::string name, listeners_t listeners);
		Server(const Server& other) = delete;
		Server(Server&& other) = delete;
		~Server() = default;

		Server& operator = (const Server& rhs) = delete;
		Server& operator = (Server&& rhs) = delete;

    private:

        inline static uint32_t M_nextTypeId = 0;

        void handleTransition(const io::Connection& connection);
        void handleNewStructType(const type::StructDescriptor<>& descriptor);
        void cleanUpClients();

        void updateServerStatus();

        DotsStatistics receiveStatistics() const;
        DotsCacheStatus cacheStatus() const;

        static std::string flags2String(const dots::type::StructDescriptor<>* td);

        io::HostTransceiver m_hostTransceiver;
        DotsDaemonStatus m_daemonStatus;
    };
}
