#pragma once
#include <vector>
#include <optional>
#include <dots/HostTransceiver.h>
#include "DotsDaemonStatus.dots.h"

namespace dots
{
    struct Server
    {
        using listeners_t = std::vector<io::listener_ptr_t>;

        Server(std::string name, listeners_t listeners, boost::asio::io_context& ioContext = dots::io::global_io_context());
        Server(const Server& other) = delete;
        Server(Server&& other) = delete;
        ~Server() = default;

        Server& operator = (const Server& rhs) = delete;
        Server& operator = (Server&& rhs) = delete;

        const boost::asio::io_context& ioContext() const;
        boost::asio::io_context& ioContext();

    private:

        inline static uint32_t M_nextTypeId = 0;

        void handleTransition(const Connection& connection);
        void handleNewStructType(const type::StructDescriptor<>& descriptor);
        void cleanUpClients();

        void updateServerStatus();

        DotsStatistics receiveStatistics() const;
        DotsCacheStatus cacheStatus() const;

        static std::string flags2String(const dots::type::StructDescriptor<>* td);

        HostTransceiver m_hostTransceiver;
        DotsDaemonStatus m_daemonStatus;
        std::optional<Subscription> m_descriptorSubscription;
    };
}
