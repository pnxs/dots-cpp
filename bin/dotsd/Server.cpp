#include "Server.h"
#include <dots/dots.h>
#include <dots/io/ResourceUsage.h>
#include <dots/io/Transceiver.h>
#include "DotsClient.dots.h"
#include <StructDescriptorData.dots.h>
#include "EnumDescriptorData.dots.h"

namespace dots
{
    Server::Server(std::unique_ptr<Listener>&& listener, const string& name):
        m_name(name),
        m_connectionManager(std::move(listener), name)
    {
        transceiver();
        publisher() = &m_connectionManager;

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
            DotsCloneInformation::_Descriptor();
        }

        m_daemonStatus.serverName = name;
        m_daemonStatus.startTime = pnxs::SystemNow();

        m_connectionManager.init();

        // Start cleanup-timer
        add_timer(1, FUN(*this, handleCleanupTimer));
        add_timer(1, FUN(*this, updateServerStatus));
    }

    void Server::stop()
    {
        m_connectionManager.stop_all();
    }

    /*!
     * Calls cleanup-method to cleanup old resources
     * @param error
     */
    void Server::handleCleanupTimer()
    {
        m_connectionManager.cleanup();

        if (m_connectionManager.running())
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
        catch (const std::exception& e)
        {
            LOG_ERROR_S("exception in updateServerStatus: " << e.what());
        }

        if (m_connectionManager.running())
        {
            add_timer(1, FUN(*this, updateServerStatus));
        }
    }
}
