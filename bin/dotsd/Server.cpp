#include "Server.h"
#include <dots/dots.h>
#include <dots/io/ResourceUsage.h>
#include <dots/io/Transceiver.h>
#include "DotsClient.dots.h"
#include <StructDescriptorData.dots.h>
#include "DotsStatistics.dots.h"
#include "DotsCacheStatus.dots.h"

namespace dots
{
    Server::Server(std::string name, listeners_t listeners) :
        m_connectionManager(std::move(name)),
        m_daemonStatus{ DotsDaemonStatus::serverName_i{ m_connectionManager.selfName() }, DotsDaemonStatus::startTime_i{ pnxs::SystemNow() } }
    {
        add_timer(1, [&](){ updateServerStatus(); }, true);

        for (listener_ptr_t& listener : listeners)
        {
            m_connectionManager.listen(std::move(listener));
        }
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

                m_connectionManager.publish(ds);
                m_daemonStatus = ds;
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("exception in updateServerStatus: " << e.what());
        }
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
