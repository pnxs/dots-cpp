#include "Server.h"
#ifdef __unix__
#include <sys/resource.h>
#include <dots/type/PosixTime.h>
#endif
#include <dots/dots.h>
#include <dots/tools/logging.h>
#include <dots/io/auth/LegacyAuthManager.h>
#include <DotsClient.dots.h>
#include <DotsContinuousRecorderStatus.dots.h>
#include <DotsDumpContinuousRecorder.dots.h>
#include <StructDescriptorData.dots.h>
#include <DotsStatistics.dots.h>
#include <DotsCacheStatus.dots.h>

using namespace dots::literals;

namespace dots
{
    Server::Server(std::string name, asio::io_context& ioContext, std::vector<io::Endpoint> listenEndpoints) :
        m_hostTransceiver{ std::move(name), ioContext, type::Registry::StaticTypePolicy::InternalOnly, HostTransceiver::transition_handler_t{ &Server::handleTransition, this } },
        m_daemonStatus{ DotsDaemonStatus::serverName_i{ m_hostTransceiver.selfName() }, DotsDaemonStatus::startTime_i{ timepoint_t::Now() } }
    {
        add_timer(1s, { &Server::updateServerStatus, this }, true);
        add_timer(10s, { &Server::cleanUpClients, this }, true);

        // For backward compatibility: in the legacy version of DOTS,
        // DotsContinuousRecorderStatus and DotsDumpContinuousRecorder where internal-types.
        // The clients do not publish the StructDescriptors for internal-types.
        // So legacy clients, like dots record, will not function without those registered types.
        type::Descriptor<DotsContinuousRecorderStatus>::Instance();
        type::Descriptor<DotsDumpContinuousRecorder>::Instance();

        m_hostTransceiver.listen(std::move(listenEndpoints));
        m_hostTransceiver.setAuthManager<io::LegacyAuthManager>();
    }

    void Server::handleTransition(const Connection& connection, std::exception_ptr/* ePtr*/)
    {
        m_hostTransceiver.publish(DotsClient{
            DotsClient::id_i{ connection.peerId() },
            DotsClient::name_i{ connection.peerName() },
            DotsClient::connectionState_i{ connection.state() }
        });
    }

    void Server::cleanUpClients()
    {
        std::set<Connection::id_t> expiredClients;

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

        for (Connection::id_t id : expiredClients)
        {
            m_hostTransceiver.remove(DotsClient{ DotsClient::id_i{ id } });
        }
    }

    void Server::updateServerStatus()
    {
        try
        {
            DotsDaemonStatus ds{ m_daemonStatus };

            if (m_daemonStatus._diffProperties(ds))
            {
                LOG_DEBUG_S("updateServerStatus");

                #ifdef __unix__
                struct rusage usage;
                ::getrusage(RUSAGE_SELF, &usage);

                ds.resourceUsage = DotsResourceUsage{
                    DotsResourceUsage::userCpuTime_i{ type::posix::Timeval{ usage.ru_utime } },
                    DotsResourceUsage::systemCpuTime_i{ type::posix::Timeval{ usage.ru_stime } },
                    DotsResourceUsage::maxRss_i{ static_cast<int32_t>(usage.ru_maxrss) },
                    DotsResourceUsage::minorFaults_i{ static_cast<int32_t>(usage.ru_minflt) },
                    DotsResourceUsage::majorFaults_i{ static_cast<int32_t>(usage.ru_majflt) },
                    DotsResourceUsage::nrSwaps_i{ static_cast<int32_t>(usage.ru_nswap) },
                    DotsResourceUsage::inBlock_i{ static_cast<int32_t>(usage.ru_inblock) },
                    DotsResourceUsage::outBlock_i{ static_cast<int32_t>(usage.ru_oublock) },
                    DotsResourceUsage::nrSignals_i{ static_cast<int32_t>(usage.ru_nsignals) },
                    DotsResourceUsage::nrVoluntaryContextSwitches_i{ static_cast<int32_t>(usage.ru_nvcsw) },
                    DotsResourceUsage::nrInvoluntaryContextSwitches_i{ static_cast<int32_t>(usage.ru_nivcsw) }
                };
                #endif
                ds.cache = DotsCacheStatus{
                    DotsCacheStatus::nrTypes_i{ static_cast<uint32_t>(m_hostTransceiver.pool().size()) },
                    DotsCacheStatus::size_i{ static_cast<uint32_t>(m_hostTransceiver.pool().totalMemoryUsage()) }
                };

                m_hostTransceiver.publish(ds);
                m_daemonStatus = ds;
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("exception in updateServerStatus: " << e.what());
        }
    }
}
