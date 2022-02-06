#pragma once
#include <vector>
#include <optional>
#include <dots/HostTransceiver.h>
#include <DotsDaemonStatus.dots.h>

namespace dots
{
    struct DotsDaemon
    {
        DotsDaemon(std::string name, asio::io_context& ioContext, std::vector<io::Endpoint> listenEndpoints);
        DotsDaemon(const DotsDaemon& other) = delete;
        DotsDaemon(DotsDaemon&& other) = delete;
        ~DotsDaemon() = default;

        DotsDaemon& operator = (const DotsDaemon& rhs) = delete;
        DotsDaemon& operator = (DotsDaemon&& rhs) = delete;

    private:

        void handleTransition(const Connection& connection, std::exception_ptr ePtr);
        void cleanUpClients();

        void updateServerStatus();

        HostTransceiver m_hostTransceiver;
        DotsDaemonStatus m_daemonStatus;
    };
}
