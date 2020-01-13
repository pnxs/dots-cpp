#pragma once

#include "dots/cpp_config.h"
#include "ConnectionManager.h"
#include "DotsDaemonStatus.dots.h"

namespace dots
{
    /*!
     * This class provides the server functionality of the DOTS system.
     */
    class Server
    {
    public:
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;

        /*!
         * Create a DOTS server, listening on the given address and port
         * @param io_context Boost-ASIO io-context object
         * @param address Address to bind to
         * @param port Port to bind to
         * @param name Servername
         */
        explicit Server(std::unique_ptr<Listener>&& listener, const string& name);

        /*!
         * Stops the DOTS server
         */
        void stop();

    private:
        void handleCleanupTimer();
        void updateServerStatus();

        std::string m_name;
        ConnectionManager m_connectionManager;
        DotsDaemonStatus m_daemonStatus;
    };
}
