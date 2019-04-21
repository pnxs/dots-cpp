#pragma once

#include "dots/cpp_config.h"
#include "ConnectionManager.h"
#include "AuthManager.h"
#include "ServerInfo.h"
#include <dots/io/services/TcpListener.h>

#include "DotsDaemonStatus.dots.h"

namespace dots
{
/*!
 * This class provides the server functionality of the DOTS system.
 */
class Server: public ServerInfo
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
    explicit Server(const string& address, const string& port, const string& name);

    /*!
     * Returns the AuthManager as reference
     * @return the Authentication Manager object
     */
    AuthManager& authManager() override { return m_authManager; }

    /*!
     * Returns the name of the DOTS server
     * @return servername as string
     */
    const string& name() const override { return m_name; }

    /*!
     * Stops the DOTS server
     */
    void stop();
    
    /*!
     * Returns the servers connection ID
     * @return connection id of the server
     */
    const ClientId& id() const override;

private:
    void asyncAccept();
	void handleCleanupTimer();
    void updateServerStatus();

	string m_name;

    GroupManager m_groupManager;
    ConnectionManager m_connectionManager;
    AuthManager m_authManager;  

	std::unique_ptr<Listener> m_listener;

    DotsDaemonStatus m_daemonStatus;
    ClientId m_serverId = 1;
};

}

