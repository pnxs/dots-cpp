#include "Server.h"
#include <sys/resource.h>
#include <dots/dots.h>
#include <dots/tools/logging.h>
#include <dots/type/PosixTime.h>
#include <dots_legacy/io/auth/LegacyAuthManager.h>
#include "DotsClient.dots.h"
#include <StructDescriptorData.dots.h>
#include <DotsTypes.dots.h>
#include <DotsStatistics.dots.h>
#include <DotsCacheStatus.dots.h>

using namespace dots::types::literals;

namespace dots
{
    Server::Server(std::string name, listeners_t listeners) :
        m_hostTransceiver{ std::move(name), [&](const io::Connection& connection){ handleTransition(connection); } },
        m_daemonStatus{ DotsDaemonStatus::serverName_i{ m_hostTransceiver.selfName() }, DotsDaemonStatus::startTime_i{ types::timepoint_t::Now() } }
    {
        add_timer(1s, [&](){ updateServerStatus(); }, true);
        add_timer(10s, [&](){ cleanUpClients(); }, true);

        for (io::listener_ptr_t& listener : listeners)
        {
            m_hostTransceiver.listen(std::move(listener));
        }

        m_hostTransceiver.subscribe<type::Type::Struct>([&](const type::StructDescriptor<>& descriptor){ handleNewStructType(descriptor); });
        m_hostTransceiver.setAuthManager<io::LegacyAuthManager>();
    }

    void Server::handleTransition(const io::Connection& connection)
    {
        m_hostTransceiver.publish(DotsClient{
            DotsClient::id_i{ connection.peerId() },
            DotsClient::name_i{ connection.peerName() },
            DotsClient::connectionState_i{ connection.state() }
        });
    }

    void Server::handleNewStructType(const type::StructDescriptor<>& descriptor)
    {
        LOG_DEBUG_S("onNewType name=" << descriptor.name() << " flags:" << flags2String(&descriptor));

        m_hostTransceiver.publish(DotsTypes{
            DotsTypes::id_i{ M_nextTypeId++ },
            DotsTypes::name_i{ descriptor.name() }
	    });
    }

    void Server::cleanUpClients()
    {
        std::set<io::Connection::id_t> expiredClients;

        for (auto& element : m_hostTransceiver.pool().get<DotsClient>())
        {
            const auto& client = element.first.to<DotsClient>();

            if (client.connectionState == DotsConnectionState::closed)
            {
                for (const auto& [descriptor, container] : m_hostTransceiver.pool())
                {
                    (void)descriptor;

                    for (const auto& [instance, cloneInformation] : container)
                    {
                        (void)instance;

                        if (cloneInformation.createdFrom == client.id || cloneInformation.lastUpdateFrom == client.id)
                        {
                            break;
                        }
                    }
                }

                expiredClients.emplace(client.id);
            }
        }

        for (io::Connection::id_t id : expiredClients)
        {
            m_hostTransceiver.remove(DotsClient{ DotsClient::id_i{ id } });
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

                struct rusage usage;
                ::getrusage(RUSAGE_SELF, &usage);

                ds.resourceUsage = DotsResourceUsage{
                    DotsResourceUsage::userCpuTime_i{ type::posix::Timeval{ usage.ru_utime } },
                    DotsResourceUsage::systemCpuTime_i{ type::posix::Timeval{ usage.ru_stime } },
                    DotsResourceUsage::maxRss_i{ usage.ru_maxrss },
                    DotsResourceUsage::minorFaults_i{ usage.ru_minflt },
                    DotsResourceUsage::majorFaults_i{ usage.ru_majflt },
                    DotsResourceUsage::nrSwaps_i{ usage.ru_nswap },
                    DotsResourceUsage::inBlock_i{ usage.ru_inblock },
                    DotsResourceUsage::outBlock_i{ usage.ru_oublock },
                    DotsResourceUsage::nrSignals_i{ usage.ru_nsignals },
                    DotsResourceUsage::nrVoluntaryContextSwitches_i{ usage.ru_nvcsw },
                    DotsResourceUsage::nrInvoluntaryContextSwitches_i{ usage.ru_nivcsw }
                };
                ds.cache = cacheStatus();

                m_hostTransceiver.publish(ds);
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

        auto& pool = m_hostTransceiver.pool();

        cs.nrTypes(pool.size());
        cs.size(pool.totalMemoryUsage());
        return cs;
    }

    /*!
     * Returns a short string-representation of the DotsStructFlags.
     * The String consists of 5 chars (5 flags). Every flag has a static place in
     * this string:
     * @code
     * "....." No flags are set.
     * Flags:
     * "CIPcL"
     *  ||||\- local (L)
     *  |||\-- cleanup (c)
     *  ||\--- persistent (P)
     *  |\---- internal (I)
     *  \----- cached (C)
     * @endcode
     *
     * @param td the structdescriptor from which the flags should be processed.
     * @return short string containing the flags.
     */
    std::string Server::flags2String(const dots::type::StructDescriptor<>* td)
    {
        std::string ret = ".....";
        if (td->cached()) ret[0] = 'C';
        if (td->internal()) ret[1] = 'I';
        if (td->persistent()) ret[2] = 'P';
        if (td->cleanup()) ret[3] = 'c';
        if (td->local()) ret[4] = 'L';
        return ret;
    } 
}
