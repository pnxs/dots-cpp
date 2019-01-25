#include "Server.h"
#include <dots/eventloop/Timer.h>
#include <dots/io/ResourceUsage.h>
#include <boost/asio/socket_base.hpp>
#include <dots/io/Transceiver.h>

#include "DotsClient.dots.h"
#include "EnumDescriptorData.dots.h"

namespace dots
{

Server::Server(IoService& io_service, const string& address, const string& port, const string& name)
        :m_ioservice(io_service)
        ,m_acceptor(io_service)
        ,m_socket(io_service)
        ,m_name(name)
,m_connectionManager(m_groupManager, *this)
{
    ASIO::ip::tcp::resolver resolver(io_service);
    ASIO::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});

    onPublishObject = &m_connectionManager;

    for (const auto& e : dots::PublishedType::allChained())
    {
        LOG_DEBUG_S("Published type: " << e->td->name());
        m_connectionManager.onNewType(e->td);
    }

    {
        StructDescriptorData::_td();
        EnumDescriptorData::_td();
        DotsTransportHeader::_td();
        DotsMsgConnect::_td();
        DotsMsgConnectResponse::_td();
        DotsMsgHello::_td();
    }

    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

    boost::system::error_code ec;
    m_acceptor.bind(endpoint, ec);
    if (ec) {
        LOG_ERROR_S("error binding to port " << port);
        throw std::runtime_error("error binding to port " + port);
    }

    m_acceptor.listen(25, ec);
    if (ec) {
        LOG_ERROR_S("error on listeing:" << ec);
        throw std::runtime_error("error listening:" + ec.message());
    }

    asyncAccept();

    m_daemonStatus.setServerName(m_name);
    m_daemonStatus.setStartTime(pnxs::SystemNow());

    m_connectionManager.init();

    // Start cleanup-timer
    pnxs::addTimer(1, FUN(*this, handleCleanupTimer));
    pnxs::addTimer(1, FUN(*this, updateServerStatus));
}

void Server::stop()
{
    m_acceptor.close();
    m_connectionManager.stop_all();
    m_ioservice.stop();
}
    
const ClientId& Server::id() const
{
    return m_serverId;
}

/*!
 * Starts an asynchronous Accept. Calls processAccept when a new connection is accepted.
 */
void Server::asyncAccept()
{
    m_acceptor.async_accept(m_socket, FUN(*this, processAccept));
}

/*!
 * Creates a new Connection object for a new connection (when no error on accept has been occured).
 * @param ec error-code of accept-call
 */
void Server::processAccept(boost::system::error_code ec)
{
    namespace IP = boost::asio::ip;
    using boost::asio::socket_base;

    if (not m_acceptor.is_open())
    {
        return;
    }

    if (not ec)
    {
        boost::system::error_code ec;
        m_socket.set_option(IP::tcp::no_delay(true), ec);
        m_socket.set_option(IP::tcp::socket::keep_alive(true), ec);

        socket_base::receive_buffer_size receiveBufferSize;
        socket_base::send_buffer_size sendBufferSize;
        socket_base::receive_low_watermark receiveLowWatermark;
        socket_base::send_low_watermark sendLowWatermark;

        m_socket.get_option(receiveBufferSize);
        m_socket.get_option(sendBufferSize);
        m_socket.get_option(receiveLowWatermark);
        m_socket.get_option(sendLowWatermark);

        m_socket.nonBlocking(true);

        if (sendBufferSize.value() < m_minimumSendBufferSize) {
            LOG_DEBUG_S("try to set send-buffer-size to " << m_minimumSendBufferSize);
            m_socket.set_option(socket_base::send_buffer_size(m_minimumSendBufferSize), ec);
            m_socket.get_option(sendBufferSize);
            if (sendBufferSize.value() < m_minimumSendBufferSize) {
                LOG_ERROR_S("unable to set send-buffer-size to " << m_minimumSendBufferSize);
            }
        }

        LOG_INFO_S("network-buffers: receive:" << receiveBufferSize.value() <<
                   " send:" << sendBufferSize.value() <<
                   " receiveLowWatermark:" << receiveLowWatermark.value() <<
                   " sendLowWatermark:" << sendLowWatermark.value());

        auto connection = std::make_shared<Connection>(std::move(m_socket), m_connectionManager);
        m_connectionManager.start(connection);
    }

    this->asyncAccept();
}

/*!
 * Calls cleanup-method to cleanup old resources
 * @param error
 */
void Server::handleCleanupTimer()
{
    m_connectionManager.cleanup();

    if (not m_ioservice.stopped())
    {
        pnxs::addTimer(1, FUN(*this, handleCleanupTimer));
    }
}

void Server::updateServerStatus()
{
    try
    {
        DotsDaemonStatus ds(m_daemonStatus);

        ds.setReceived(m_connectionManager.receiveStatistics());

        if (m_daemonStatus.diff(ds))
        {
            LOG_DEBUG_S("updateServerStatus");

            ds.setResourceUsage(dots::ResourceUsage());
            ds.setCache(m_connectionManager.cacheStatus());

            ds.publish();
            m_daemonStatus = ds;
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERROR_S("exception in updateServerStatus: " << e.what());
    }

    if (not m_ioservice.stopped())
    {
        pnxs::addTimer(1, FUN(*this, updateServerStatus));
    }
}

}
