// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <DotsDaemon.h>
#ifdef __unix__
#include <sys/resource.h>
#include <dots/type/PosixTime.h>
#endif
#include <dots/dots.h>
#include <dots/tools/logging.h>
#include <dots/io/Io.h>
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
    DotsDaemon::DotsDaemon(std::string name, int argc, char* argv[]) :
        Application(argc, argv, HostTransceiver{ std::move(name), io::global_io_context(), type::Registry::StaticTypePolicy::InternalOnly, HostTransceiver::transition_handler_t{&DotsDaemon::handleTransition, this}}),
        m_daemonStatus{ .serverName = transceiver().selfName(), .startTime = timepoint_t::Now() },
        m_updateServerStatusTimer{ io::global_io_context(), 1s, { &DotsDaemon::updateServerStatus, this }, true },
        m_cleanUpClientsTimer{ io::global_io_context(), 10s, { &DotsDaemon::cleanUpClients, this }, true }
    {
        // For backward compatibility: in the legacy version of DOTS,
        // DotsContinuousRecorderStatus and DotsDumpContinuousRecorder where internal-types.
        // The clients do not publish the StructDescriptors for internal-types.
        // So legacy clients, like dots record, will not function without those registered types.
        type::Descriptor<DotsContinuousRecorderStatus>::Instance();
        type::Descriptor<DotsDumpContinuousRecorder>::Instance();

        static_cast<HostTransceiver&>(transceiver()).setAuthManager<io::LegacyAuthManager>();
    }

    void DotsDaemon::handleTransition(const Connection& connection, std::exception_ptr/* ePtr*/)
    {
        transceiver().publish(DotsClient{
            .id = connection.peerId(),
            .name = connection.peerName(),
            .connectionState = connection.state()
        });
    }

    void DotsDaemon::cleanUpClients()
    {
        std::set<Connection::id_t> expiredClients;

        for (auto& element : transceiver().pool().get<DotsClient>())
        {
            const auto& client = element.first.to<DotsClient>();

            if (client.connectionState == DotsConnectionState::closed)
            {
                bool isStale = [&]
                {
                    for (const auto& [descriptor, container] : transceiver().pool())
                    {
                        (void)descriptor;

                        for (const auto& [instance, cloneInformation] : container)
                        {
                            (void)instance;

                            if (cloneInformation.createdFrom == client.id || cloneInformation.lastUpdateFrom == client.id)
                            {
                                return false;
                            }
                        }
                    }

                    return true;
                }();

                if (isStale)
                {
                    expiredClients.emplace(*client.id);
                }
            }
        }

        for (Connection::id_t id : expiredClients)
        {
            transceiver().remove(DotsClient{ .id = id });
        }
    }

    void DotsDaemon::updateServerStatus()
    {
        try
        {
            DotsDaemonStatus ds{ m_daemonStatus };

            if (m_daemonStatus._diffProperties(ds))
            {
                #ifdef __unix__
                struct rusage usage;
                ::getrusage(RUSAGE_SELF, &usage);

                ds.resourceUsage = DotsResourceUsage{
                    .minorFaults = static_cast<int32_t>(usage.ru_minflt),
                    .majorFaults = static_cast<int32_t>(usage.ru_majflt),
                    .inBlock = static_cast<int32_t>(usage.ru_inblock),
                    .outBlock = static_cast<int32_t>(usage.ru_oublock),
                    .nrSignals = static_cast<int32_t>(usage.ru_nsignals),
                    .nrSwaps = static_cast<int32_t>(usage.ru_nswap),
                    .nrVoluntaryContextSwitches = static_cast<int32_t>(usage.ru_nvcsw),
                    .nrInvoluntaryContextSwitches = static_cast<int32_t>(usage.ru_nivcsw),
                    .maxRss = static_cast<int32_t>(usage.ru_maxrss),
                    .userCpuTime = type::posix::Timeval{ usage.ru_utime },
                    .systemCpuTime = type::posix::Timeval{ usage.ru_stime }
                };
                #endif
                ds.cache = DotsCacheStatus{
                    .nrTypes = static_cast<uint32_t>(transceiver().pool().size()),
                    .size = static_cast<uint32_t>(transceiver().pool().totalMemoryUsage())
                };

                transceiver().publish(ds);
                m_daemonStatus = ds;
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("exception in updateServerStatus: " << e.what());
        }
    }
}
