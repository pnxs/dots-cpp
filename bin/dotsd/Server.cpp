#include "Server.h"
#include <dots/io/Io.h>
#include <dots/io/ResourceUsage.h>
#include <dots/io/Transceiver.h>

#include "DotsClient.dots.h"
#include "EnumDescriptorData.dots.h"

namespace dots
{

Server::Server(asio::io_context& io_context, const string& address, const string& port, const string& name)
        :m_ioContext(io_context)
        ,m_acceptor(io_context)
        ,m_socket(io_context)
        ,m_name(name)
,m_connectionManager(m_groupManager, *this)
{
    asio::ip::tcp::resolver resolver(io_context);
	asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});

    onPublishObject = &m_connectionManager;

    for (const auto& e : dots::PublishedType::allChained())
    {
        LOG_DEBUG_S("Published type: " << e->td->name());
        m_connectionManager.onNewType(e->td);
    }

    {
        StructDescriptorData::_Descriptor();
        EnumDescriptorData::_Descriptor();
        DotsTransportHeader::_Descriptor();
        DotsMsgConnect::_Descriptor();
        DotsMsgConnectResponse::_Descriptor();
        DotsMsgHello::_Descriptor();
    }

    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

    asio::error_code ec;
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

    m_daemonStatus.serverName = m_name;
    m_daemonStatus.startTime = pnxs::SystemNow();

    m_connectionManager.init();

    // Start cleanup-timer
    add_timer(1, FUN(*this, handleCleanupTimer));
    add_timer(1, FUN(*this, updateServerStatus));
}

void Server::stop()
{
    m_acceptor.close();
    m_connectionManager.stop_all();
    m_ioContext.stop();
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
void Server::processAccept(asio::error_code ec)
{
    using asio::socket_base;

    if (not m_acceptor.is_open())
    {
        return;
    }

    if (not ec)
    {
        asio::error_code ec;
        m_socket.set_option(asio::ip::tcp::no_delay(true), ec);
        m_socket.set_option(asio::ip::tcp::socket::keep_alive(true), ec);

        socket_base::receive_buffer_size receiveBufferSize;
        socket_base::send_buffer_size sendBufferSize;
        socket_base::receive_low_watermark receiveLowWatermark;
        socket_base::send_low_watermark sendLowWatermark;

        m_socket.get_option(receiveBufferSize);
        m_socket.get_option(sendBufferSize);
        /*m_socket.get_option(receiveLowWatermark);
        m_socket.get_option(sendLowWatermark);*/

        m_socket.non_blocking(true);

        if (sendBufferSize.value() < m_minimumSendBufferSize) {
            LOG_DEBUG_S("try to set send-buffer-size to " << m_minimumSendBufferSize);
            m_socket.set_option(socket_base::send_buffer_size(m_minimumSendBufferSize), ec);
            m_socket.get_option(sendBufferSize);
            if (sendBufferSize.value() < m_minimumSendBufferSize) {
                LOG_ERROR_S("unable to set send-buffer-size to " << m_minimumSendBufferSize);
            }
        }

        LOG_INFO_S("network-buffers: receive:" << receiveBufferSize.value() <<
                   " send:" << sendBufferSize.value()/* <<
                   " receiveLowWatermark:" << receiveLowWatermark.value() <<
                   " sendLowWatermark:" << sendLowWatermark.value()*/);

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

    if (not m_ioContext.stopped())
    {
        add_timer(1, FUN(*this, handleCleanupTimer));
    }
}

void Server::updateServerStatus()
{
    try
    {
        DotsDaemonStatus ds(m_daemonStatus);

        ds.received = m_connectionManager.receiveStatistics();

        if (m_daemonStatus._diffProperties(ds))
        {
            LOG_DEBUG_S("updateServerStatus");

            ds.resourceUsage = static_cast<DotsResourceUsage&&>(dots::ResourceUsage());
            ds.cache = m_connectionManager.cacheStatus();

            ds._publish();
            m_daemonStatus = ds;
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERROR_S("exception in updateServerStatus: " << e.what());
    }

    if (not m_ioContext.stopped())
    {
        add_timer(1, FUN(*this, updateServerStatus));
    }
}

}
