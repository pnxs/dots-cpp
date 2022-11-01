// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/HostTransceiver.h>
#include <vector>
#include <dots/tools/logging.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>

namespace dots
{
    HostTransceiver::HostTransceiver(std::string selfName,
                                     asio::io_context& ioContext,
                                     type::Registry::StaticTypePolicy staticTypePolicy /*= type::Registry::StaticTypePolicy::All*/,
                                     std::optional<transition_handler_t> transitionHandler/* = std::nullopt*/) :
        Transceiver(std::move(selfName), ioContext, staticTypePolicy, std::move(transitionHandler))
    {
        /* do nothing */
    }

    HostTransceiver::~HostTransceiver()
    {
        m_groups.clear();
        connection_map_t guestConnections = std::move(m_guestConnections);
        guestConnections.clear();
    }

    io::Listener& HostTransceiver::listen(io::listener_ptr_t&& listener)
    {
        io::Listener* listenerPtr = listener.get();
        m_listeners.emplace(listenerPtr, std::move(listener));

        listenerPtr->asyncAccept(
            { &HostTransceiver::handleListenAccept, this },
            { &HostTransceiver::handleListenError, this }
        );

        return *listenerPtr;
    }

    void HostTransceiver::publish(const type::Struct& instance, std::optional<property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        if (const type::StructDescriptor& descriptor = instance._descriptor(); descriptor.substructOnly())
        {
            throw std::logic_error{ "attempt to publish substruct-only type '" + descriptor.name() + "'" };
        }

        if (!(instance._keyProperties() <= instance._validProperties()))
        {
            throw std::runtime_error("attempt to publish instance with missing key properties '" + (instance._keyProperties() - instance._validProperties()).toString() + "'");
        }

        if (includedProperties == std::nullopt)
        {
            includedProperties = instance._validProperties();
        }
        else
        {
            *includedProperties += instance._keyProperties();
            *includedProperties ^= instance._properties();
        }

        DotsHeader header{
            .typeName = instance._descriptor().name(),
            .sentTime = timepoint_t::Now(),
            .serverSentTime = timepoint_t::Now(),
            .attributes = *includedProperties,
            .sender = Connection::HostId,
            .removeObj = remove,
            .isFromMyself = true
        };

        io::Transmission transmission{ std::move(header), instance };
        dispatcher().dispatch(transmission);
        transmit(transmission);
    }

    void HostTransceiver::joinGroup(std::string_view/* name*/)
    {
        /* do nothing */
    }

    void HostTransceiver::leaveGroup(std::string_view/* name*/)
    {
        /* do nothing */
    }

    void HostTransceiver::transmit(const io::Transmission& transmission)
    {
        using dirty_connection_t = std::pair<Connection*, std::exception_ptr>;
        std::vector<dirty_connection_t> dirtyConnections;

        for (Connection* destinationConnection : m_groups[*transmission.header().typeName])
        {
            if (destinationConnection->state() != DotsConnectionState::closed)
            {
                try
                {
                    destinationConnection->transmit(transmission);
                }
                catch (...)
                {
                    dirtyConnections.emplace_back(destinationConnection, std::current_exception());
                }
            }
        }

        if (!dirtyConnections.empty())
        {
            for (const auto& [connection, e] : dirtyConnections)
            {
                connection->handleError(e);
            }
        }
    }

    bool HostTransceiver::handleListenAccept(io::Listener&/* listener*/, io::channel_ptr_t channel)
    {
        auto connection = std::make_shared<Connection>(std::move(channel), true);
        connection->asyncReceive(registry(), m_authManager.get(), selfName(),
            { &HostTransceiver::handleTransmission, this },
            { &HostTransceiver::handleTransition, this }
        );
        m_guestConnections.emplace(connection.get(), connection);

        return true;
    }

    void HostTransceiver::handleListenError(io::Listener& listener, std::exception_ptr ePtr)
    {
        try
        {
            std::rethrow_exception(ePtr);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("error while listening for incoming channels -> " << e.what());
        }

        m_listeners.erase(&listener);
    }

    bool HostTransceiver::handleTransmission(Connection& connection, io::Transmission transmission)
    {
        // ensure that the connection is alive until the handler returns,
        // because it might get removed during processing of the transmission
        // if it gets dirty
        connection_ptr_t connectionPtr = m_guestConnections.find(&connection)->second;
        (void)connectionPtr;

        const auto& [header, instance] = transmission;

        if (instance->_descriptor().internal())
        {
            if (auto* member = instance.as<DotsMember>())
            {
                handleMemberMessage(connection, *member);
                return !connection.closed();
            }
            else if (auto* descriptorRequest = instance.as<DotsDescriptorRequest>())
            {
                handleDescriptorRequest(connection, *descriptorRequest);
                return !connection.closed();
            }
            else if (auto* clearCache = instance.as<DotsClearCache>())
            {
                handleClearCache(connection, *clearCache);
            }
            else if (auto* echoRequest = instance.as<DotsEcho>())
            {
                handleEchoRequest(connection, *echoRequest);
                return !connection.closed();
            }
        }

        dispatcher().dispatch(transmission);
        transmit(transmission);

        return !connection.closed();
    }

    void HostTransceiver::handleTransitionImpl(Connection& connection, std::exception_ptr/* e*/) noexcept
    {
        try
        {
            if (connection.state() == DotsConnectionState::closed)
            {
                for (auto& [groupName, group] : m_groups)
                {
                    group.erase(&connection);
                }

                std::vector<const type::Struct*> cleanupInstances;

                for (const auto& [descriptor, container] : pool())
                {
                    if (descriptor->cleanup())
                    {
                        for (const auto& [instance, cloneInfo] : container)
                        {
                            if (connection.peerId() == cloneInfo.lastUpdateFrom)
                            {
                                cleanupInstances.emplace_back(&*instance);
                            }
                        }
                    }
                }

                for (const type::Struct* instance : cleanupInstances)
                {
                    remove(*instance);
                }

                m_guestConnections.erase(&connection);
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("error while handling transition for connection " << connection.peerDescription() << " -> " << e.what());
        }
    }

    void HostTransceiver::handleMemberMessage(Connection& connection, const DotsMember& member)
    {
        member._assertHasProperties(DotsMember::groupName_p + DotsMember::event_p);
        const std::string& groupName = *member.groupName;

        if (member.event == DotsMemberEvent::kill)
        {
            LOG_WARN_S(connection.peerDescription() << " requested unsupported kill event");
        }
        else if (member.event == DotsMemberEvent::leave)
        {
            if (size_t removed = m_groups[groupName].erase(&connection); removed == 0)
            {
                LOG_WARN_S(connection.peerDescription() << " is not a member of group '" << groupName << "'");
            }
        }
        else if (member.event == DotsMemberEvent::join)
        {
            if (auto [it, emplaced] = m_groups[groupName].emplace(&connection); emplaced)
            {
                LOG_DEBUG_S(connection.peerDescription() << " is now a member of group '" << groupName << "'");
            }
            else
            {
                LOG_WARN_S(connection.peerDescription() << " is already member of group '" << groupName << "'");
            }

            // note: transmitting the container content even when the guest has already joined the group is currently
            // necessary to retain backwards compatibility
            auto structDescriptor = registry().findStructType(groupName);
            if (structDescriptor && structDescriptor->cached())
            {
                if (const Container<> *container = pool().find(*structDescriptor); container != nullptr)
                {
                    transmitContainer(connection, *container);
                }

                connection.transmit(DotsCacheInfo{
                    .typeName = member.groupName,
                    .endTransmission = true
                });
            }
        }
    }

    void HostTransceiver::handleDescriptorRequest(Connection& connection, const DotsDescriptorRequest& descriptorRequest)
    {
        const vector_t<string_t>& whiteList = descriptorRequest.whitelist.isValid() ? *descriptorRequest.whitelist : vector_t<string_t>{};
        const vector_t<string_t>& blacklist = descriptorRequest.blacklist.isValid() ? *descriptorRequest.blacklist : vector_t<string_t>{};

        registry().forEach<type::StructDescriptor>([&](const auto& descriptor) 
        {
            if (descriptor.internal())
            {
                return;
            }

            if (!whiteList.empty() && std::find(whiteList.begin(), whiteList.end(), descriptor.name()) == whiteList.end())
            {
                return;
            }

            if (!blacklist.empty() && std::find(blacklist.begin(), blacklist.end(), descriptor.name()) != blacklist.end())
            {
                return;
            }

            connection.transmit(descriptor);
        });

        connection.transmit(DotsCacheInfo{ .endDescriptorRequest = true });
    }

    void HostTransceiver::handleClearCache(Connection&/* connection*/, const DotsClearCache& clearCache)
    {
        clearCache._assertHasProperties(DotsClearCache::typeNames_p);
        const vector_t<string_t>& typeNames = *clearCache.typeNames;

        for (auto& [descriptor, container] : pool())
        {
            if (std::find(typeNames.begin(), typeNames.end(), container.descriptor().name()) != typeNames.end())
            {
                std::vector<const type::Struct*> removeInstances;

                for (const auto& [instance, cloneInformation] : container)
                {
                    (void)cloneInformation;
                    removeInstances.emplace_back(&instance.get());
                }

                for (const type::Struct* instance : removeInstances)
                {
                    remove(*instance);
                }
            }
        }
    }

    void HostTransceiver::handleEchoRequest(Connection& connection, const DotsEcho& echoRequest)
    {
        if  (echoRequest.request == true)
        {
            DotsEcho echoReply(echoRequest);
            echoReply.request = false;
            connection.transmit(echoReply);
        }
    }

    void HostTransceiver::transmitContainer(Connection& connection, const Container<>& container)
    {
        if (container.empty())
        {
            return;
        }

        DotsHeader header{
            .typeName = container.descriptor().name(),
            .fromCache = static_cast<uint32_t>(container.size()),
            .removeObj = false
        };

        for (const auto& [instance, cloneInfo] : container)
        {
            header.sentTime = *cloneInfo.modified;
            header.serverSentTime = timepoint_t::Now();
            header.attributes = instance->_validProperties();
            header.sender = *cloneInfo.lastUpdateFrom;
            --*header.fromCache;

            connection.transmit(header, instance);
        }
    }
}

#include <boost/program_options.hpp>
#include <dots/io/channels/TcpListener.h>
#include <dots/io/channels/WebSocketListener.h>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <dots/io/channels/UdsListener.h>
#endif

namespace dots
{
    void HostTransceiver::listen(std::vector<io::Endpoint> listenEndpoints)
    {
        for (io::Endpoint& listenEndpoint : listenEndpoints)
        {
            std::string_view scheme = listenEndpoint.scheme();

            if (scheme == "tcp")
            {
                listen<io::TcpListener>(listenEndpoint);
            }
            else if (scheme == "tcp-v2")
            {
                listen<io::v2::TcpListener>(listenEndpoint);
            }
            else if (scheme == "tcp-v1")
            {
                listen<io::v1::TcpListener>(listenEndpoint);
            }
            #if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
            else if (scheme == "uds")
            {
                listen<io::posix::UdsListener>(listenEndpoint);
            }
            else if (scheme == "uds-v2")
            {
                listen<io::posix::v2::UdsListener>(listenEndpoint);
            }
            else if (scheme == "uds-v1")
            {
                listen<io::posix::v1::UdsListener>(listenEndpoint);
            }
            #endif
            else if (scheme == "ws")
            {
                listen<io::WebSocketListener>(listenEndpoint);
            }
            else
            {
                throw std::runtime_error{ "unknown or unsupported endpoint scheme: '" + std::string{ scheme } + "'" };
            }

            LOG_NOTICE_S("listening on endpoint '" << listenEndpoint.uriStr() << "'");
        }
    }
}
