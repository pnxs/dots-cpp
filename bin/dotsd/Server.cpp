#include "Server.h"
#include <dots/dots.h>
#include <dots/io/ResourceUsage.h>
#include <dots/io/Transceiver.h>
#include "DotsClient.dots.h"
#include "EnumDescriptorData.dots.h"

namespace dots
{

Server::Server(std::unique_ptr<Listener>&& listener, const string& name)
:m_name(name)
,m_connectionManager(m_groupManager, *this)
,m_listener(std::move(listener))
{
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
	m_listener.reset();
    m_connectionManager.stop_all();
}
    
const ClientId& Server::id() const
{
    return m_serverId;
}

/*!
 * Starts an asynchronous Accept.
 */
void Server::asyncAccept()
{
	m_listener->asyncAccept([&](channel_ptr_t channel)
	{
		auto connection = std::make_shared<Connection>(std::move(channel), m_connectionManager);
		m_connectionManager.start(connection);

		return true;
	});
}

/*!
 * Calls cleanup-method to cleanup old resources
 * @param error
 */
void Server::handleCleanupTimer()
{
    m_connectionManager.cleanup();

    if (m_listener != nullptr)
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

    if (m_listener != nullptr)
    {
        add_timer(1, FUN(*this, updateServerStatus));
    }
}

}
