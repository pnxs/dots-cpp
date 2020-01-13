#include "Server.h"
#include <dots/dots.h>
#include <dots/io/ResourceUsage.h>
#include <dots/io/Transceiver.h>
#include "DotsClient.dots.h"
#include <StructDescriptorData.dots.h>
#include "EnumDescriptorData.dots.h"

namespace dots
{
    Server::Server(listener_ptr_t&& listener, const string& name) :
        m_name(name),
        m_connectionManager(name)
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

        m_daemonStatus.serverName = m_name;
        m_daemonStatus.startTime = pnxs::SystemNow();

        m_connectionManager.init();
        m_connectionManager.listen(std::move(listener));

        add_timer(1, FUN(*this, updateServerStatus));
    }

    void Server::updateServerStatus()
    {
        try
        {
            DotsDaemonStatus ds(m_daemonStatus);

            ds.received = receiveStatistics();

            if (m_daemonStatus._diffProperties(ds))
            {
                LOG_DEBUG_S("updateServerStatus");

                ds.resourceUsage = static_cast<DotsResourceUsage&&>(dots::ResourceUsage());
                ds.cache = cacheStatus();

                ds._publish();
                m_daemonStatus = ds;
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("exception in updateServerStatus: " << e.what());
        }

        add_timer(1, FUN(*this, updateServerStatus));
    }

    DotsStatistics Server::receiveStatistics() const
    {
        //return m_dispatcher.statistics();
        // TODO: determine if still necessary
        return DotsStatistics{};
    }

    DotsCacheStatus Server::cacheStatus() const
    {
        DotsCacheStatus cs;

        auto& pool = m_connectionManager.pool();

        cs.nrTypes(pool.size());
        cs.size(pool.totalMemoryUsage());
        return cs;
    }
}
