#pragma once
#include <vector>
#include <optional>
#include <dots/HostTransceiver.h>
#include <DotsDaemonStatus.dots.h>

namespace dots
{
    struct Server
    {
        Server(std::string name, asio::io_context& ioContext, std::vector<io::Endpoint> listenEndpoints);
        Server(const Server& other) = delete;
        Server(Server&& other) = delete;
        ~Server() = default;

        Server& operator = (const Server& rhs) = delete;
        Server& operator = (Server&& rhs) = delete;

    private:

        void handleTransition(const Connection& connection, std::exception_ptr ePtr);
        void cleanUpClients();

        void updateServerStatus();

        HostTransceiver m_hostTransceiver;
        DotsDaemonStatus m_daemonStatus;
    };
}
