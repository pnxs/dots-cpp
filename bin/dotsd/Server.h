#pragma once

#include <dots/io/TcpAcceptor.h>
#include <dots/io/TcpSocket.h>
#include "dots/cpp_config.h"
#include "boost/asio.hpp"
#include "ConnectionManager.h"
#include "AuthManager.h"
#include "ServerInfo.h"

#include "DotsDaemonStatus.dots.h"

namespace dots
{
namespace ASIO = boost::asio;

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
     * @param io_service Boost-ASIO io-service object
     * @param address Address to bind to
     * @param port Port to bind to
     * @param name Servername
     */
    explicit Server(boost::asio::io_service& io_service, const string& address, const string& port, const string& name);

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

    void processAccept(boost::system::error_code ec);

    void updateServerStatus();

    boost::asio::io_service& m_ioservice;
    dots::TcpAcceptor m_acceptor;
    dots::TcpSocket m_socket;

	string m_name;

    GroupManager m_groupManager;
    ConnectionManager m_connectionManager;
    AuthManager m_authManager;    

    DotsDaemonStatus m_daemonStatus;
    ClientId m_serverId = 1;
    int m_minimumSendBufferSize = 1024*1024; // 1MB
};

}

