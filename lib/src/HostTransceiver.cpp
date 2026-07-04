// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/HostTransceiver.h>
#include <vector>
#include <dots/Filter.h>
#include <dots/fmt/logging_fmt.h>
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

        // Capture the pre-merge cache entry before dispatcher().dispatch() merges
        // the delta — filtered subs need the prior pointer to compute wasVisible.
        const type::Struct* preMergeEntry = nullptr;
        if (instance._descriptor().cached())
        {
            if (const Container<>* container = pool().find(instance._descriptor()))
            {
                preMergeEntry = container->find(instance);
            }
        }

        io::Transmission transmission{ std::move(header), instance };
        dispatcher().dispatch(transmission);
        transmit(transmission, preMergeEntry);
    }

    void HostTransceiver::joinGroup(std::string_view/* name*/)
    {
        /* do nothing */
    }

    void HostTransceiver::leaveGroup(std::string_view/* name*/)
    {
        /* do nothing */
    }

    void HostTransceiver::transmit(const io::Transmission& transmission, const type::Struct* preMergeEntry)
    {
        using dirty_connection_t = std::pair<Connection*, std::exception_ptr>;
        std::vector<dirty_connection_t> dirtyConnections;

        Group& group = m_groups[*transmission.header().typeName];

        // ---- Hot path: unfiltered subscribers — byte-identical to legacy behavior. ----
        for (Connection* destinationConnection : group.unfilteredSubs)
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

        // ---- Cold path: filtered subscribers (skipped entirely when none exist). ----
        if (!group.filteredSubs.empty())
        {
            const type::StructDescriptor& descriptor = transmission.instance()->_descriptor();
            const bool cached = descriptor.cached();
            const bool isRemove = (transmission.header().removeObj == true);

            // Resolve the post-merge state used to evaluate the filter:
            //   * cached non-remove publish: the merged cache entry.
            //   * uncached publish: the in-flight instance (no cache to consult).
            //   * remove publish: no current state (treated as "not matching").
            const type::Struct* current = nullptr;
            if (!isRemove)
            {
                if (cached)
                {
                    if (const Container<>* c = pool().find(descriptor))
                    {
                        current = c->find(*transmission.instance());
                    }
                }
                else
                {
                    current = &*transmission.instance();
                }
            }

            const property_set_t keyProps = descriptor.keyProperties();

            for (auto& [connection, subsByConn] : group.filteredSubs)
            {
                if (connection->state() == DotsConnectionState::closed) continue;

                for (auto& [subId, sub] : subsByConn)
                {
                    const bool nowMatches = current != nullptr &&
                                            sub.compiledPredicate.matches(*current);

                    const property_set_t effMask = sub.filter.propertyMask.isValid()
                        ? (*sub.filter.propertyMask + keyProps)
                        : property_set_t::All;

                    try
                    {
                        if (!cached)
                        {
                            // Uncached types have no persistent instances, so
                            // there is no enter/leave concept and no 'visible'
                            // bookkeeping (the in-flight instance is transient
                            // and must not be stored) — forward matching
                            // publishes as plain deltas.
                            if (nowMatches)
                            {
                                DotsHeader h = transmission.header();
                                h.attributes = transmission.header().attributes->intersection(effMask);
                                h.subscriptionId = subId;
                                connection->transmit(h, *transmission.instance());
                            }
                            continue;
                        }

                        // On a remove, the dispatcher has already extracted and
                        // freed the cache entry that preMergeEntry points to —
                        // it is only used as an opaque set key from here on,
                        // never dereferenced.
                        const bool wasVisible = preMergeEntry != nullptr &&
                                                sub.visible.count(preMergeEntry) > 0;

                        if (nowMatches && wasVisible)
                        {
                            // Update — forward delta with projected attributes.
                            DotsHeader h = transmission.header();
                            h.attributes = transmission.header().attributes->intersection(effMask);
                            h.subscriptionId = subId;
                            connection->transmit(h, *transmission.instance());
                        }
                        else if (nowMatches && !wasVisible)
                        {
                            // Enter view — must send full merged state, not the delta.
                            sub.visible.insert(current);
                            DotsHeader h = transmission.header();
                            h.attributes = current->_validProperties().intersection(effMask);
                            h.serverSentTime = timepoint_t::Now();
                            h.removeObj = false;
                            h.subscriptionId = subId;
                            connection->transmit(h, *current);
                        }
                        else if (!nowMatches && wasVisible)
                        {
                            // Leave view — synthesize key-only remove. The
                            // in-flight instance carries the same key values
                            // that located the (possibly already freed)
                            // pre-merge entry, so transmit it instead.
                            sub.visible.erase(preMergeEntry);
                            DotsHeader h = transmission.header();
                            h.attributes = keyProps;
                            h.serverSentTime = timepoint_t::Now();
                            h.removeObj = true;
                            h.subscriptionId = subId;
                            connection->transmit(h, *transmission.instance());
                        }
                        // else !nowMatches && !wasVisible: nothing to send.
                    }
                    catch (...)
                    {
                        dirtyConnections.emplace_back(connection, std::current_exception());
                        break; // skip remaining subs for this dirty connection
                    }
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
            LOG_ERROR_F("error while listening for incoming channels -> {}", e.what());
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

        // Capture pre-merge cache pointer for filtered dispatch.
        const type::StructDescriptor& descriptor = instance->_descriptor();
        const type::Struct* preMergeEntry = nullptr;
        if (descriptor.cached())
        {
            if (const Container<>* container = pool().find(descriptor))
            {
                preMergeEntry = container->find(*instance);
            }
        }

        dispatcher().dispatch(transmission);
        transmit(transmission, preMergeEntry);

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
                    group.unfilteredSubs.erase(&connection);
                    group.filteredSubs.erase(&connection);
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
            LOG_ERROR_F("error while handling transition for connection {} -> {}", connection.peerDescription(), e.what());
        }
    }

    void HostTransceiver::handleMemberMessage(Connection& connection, const DotsMember& member)
    {
        member._assertHasProperties(DotsMember::groupName_p + DotsMember::event_p);
        const std::string& groupName = *member.groupName;

        const bool isFiltered = member.filter.isValid();
        const uint32_t subId  = member.subscriptionId.isValid() ? *member.subscriptionId : 0;

        if (member.event == DotsMemberEvent::kill)
        {
            LOG_WARN_F("{} requested unsupported kill event", connection.peerDescription());
            return;
        }

        if (member.event == DotsMemberEvent::leave)
        {
            Group& group = m_groups[groupName];
            if (isFiltered || subId != 0)
            {
                if (auto it = group.filteredSubs.find(&connection); it != group.filteredSubs.end())
                {
                    if (it->second.erase(subId) == 0)
                    {
                        LOG_WARN_F("{} has no filtered subscription {} on group '{}'",
                                   connection.peerDescription(), subId, groupName);
                    }
                    if (it->second.empty()) group.filteredSubs.erase(it);
                }
                else
                {
                    LOG_WARN_F("{} has no filtered subscriptions on group '{}'",
                               connection.peerDescription(), groupName);
                }
            }
            else
            {
                if (group.unfilteredSubs.erase(&connection) == 0)
                {
                    LOG_WARN_F("{} is not a member of group '{}'", connection.peerDescription(), groupName);
                }
            }
            return;
        }

        if (member.event != DotsMemberEvent::join) return;

        // ---- join ----
        auto structDescriptor = registry().findStructType(groupName);

        if (isFiltered)
        {
            if (structDescriptor == nullptr)
            {
                LOG_ERROR_F("{} requested filtered subscription on unknown type '{}'",
                            connection.peerDescription(), groupName);
                return;
            }

            // Compile the predicate against the target descriptor. Compilation
            // performs the same validation as filter::validate() and additionally
            // resolves leaves + narrows rhs values so per-publish eval is cheap.
            // A failure here is a programming error on the guest side — we log
            // and drop the join (the guest will time out waiting for preload).
            filter::CompiledPredicate compiledPredicate;
            if (member.filter->predicate.isValid())
            {
                try
                {
                    compiledPredicate = filter::CompiledPredicate{
                        *member.filter->predicate, *structDescriptor };
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR_F("{} sent invalid filter on '{}' (subId={}): {}",
                                connection.peerDescription(), groupName, subId, e.what());
                    return;
                }
            }

            Group& group = m_groups[groupName];
            auto& subsByConn = group.filteredSubs[&connection];
            auto [subIt, inserted] = subsByConn.try_emplace(subId,
                FilteredSub{ subId, *member.filter, std::move(compiledPredicate), {} });

            if (!inserted)
            {
                LOG_WARN_F("{} already has filtered subscription {} on '{}'",
                           connection.peerDescription(), subId, groupName);
                return;
            }

            LOG_DEBUG_F("{} created filtered subscription {} on group '{}'",
                        connection.peerDescription(), subId, groupName);

            if (structDescriptor->cached())
            {
                if (const Container<>* container = pool().find(*structDescriptor))
                {
                    transmitFilteredContainer(connection, *container, subIt->second);
                }
                connection.transmit(DotsCacheInfo{
                    .typeName = member.groupName,
                    .endTransmission = true
                });
            }
            return;
        }

        // Unfiltered join — existing behavior.
        if (auto [it, emplaced] = m_groups[groupName].unfilteredSubs.emplace(&connection); emplaced)
        {
            LOG_DEBUG_F("{} is now a member of group '{}'", connection.peerDescription(), groupName);
        }
        else
        {
            LOG_WARN_F("{} is already member of group '{}'", connection.peerDescription(), groupName);
        }

        // note: transmitting the container content even when the guest has already joined the group is currently
        // necessary to retain backwards compatibility
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

    void HostTransceiver::transmitFilteredContainer(Connection& connection, const Container<>& container, FilteredSub& sub)
    {
        if (container.empty()) return;

        const auto& descriptor = container.descriptor();
        const property_set_t keyProps = descriptor.keyProperties();
        const property_set_t effMask = sub.filter.propertyMask.isValid()
            ? (*sub.filter.propertyMask + keyProps)
            : property_set_t::All;

        // Pre-pass: count matches so DotsHeader.fromCache reports the actual
        // number of instances that will be transmitted, not the unfiltered total.
        uint32_t matchCount = 0;
        for (const auto& [instance, cloneInfo] : container)
        {
            (void)cloneInfo;
            if (sub.compiledPredicate.matches(*instance))
            {
                ++matchCount;
            }
        }
        if (matchCount == 0) return;

        DotsHeader header{
            .typeName = descriptor.name(),
            .fromCache = matchCount,
            .removeObj = false,
            .subscriptionId = sub.subscriptionId
        };

        for (const auto& [instance, cloneInfo] : container)
        {
            if (!sub.compiledPredicate.matches(*instance)) continue;

            header.sentTime = *cloneInfo.modified;
            header.serverSentTime = timepoint_t::Now();
            header.attributes = instance->_validProperties().intersection(effMask);
            header.sender = *cloneInfo.lastUpdateFrom;
            --*header.fromCache;

            connection.transmit(header, instance);
            sub.visible.insert(&instance.get());
        }
    }
}

#include <boost/program_options.hpp>
#include <dots/io/channels/TcpListener.h>
#if defined(ENABLE_CHANNEL_WEBSOCKET)
#include <dots/io/channels/WebSocketListener.h>
#endif
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

                // workaround for including default port in log output below
                if (listenEndpoint.port().empty())
                    listenEndpoint.setPort(std::string{ io::TcpListener::DefaultPort });
            }
            else if (scheme == "tcp-v2")
            {
                listen<io::v2::TcpListener>(listenEndpoint);

                // workaround for including default port in log output below
                if (listenEndpoint.port().empty())
                    listenEndpoint.setPort(std::string{ io::v2::TcpListener::DefaultPort });
            }
            else if (scheme == "tcp-v1")
            {
                listen<io::v1::TcpListener>(listenEndpoint);

                // workaround for including default port in log output below
                if (listenEndpoint.port().empty())
                    listenEndpoint.setPort(std::string{ io::v1::TcpListener::DefaultPort });
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
#if defined(ENABLE_CHANNEL_WEBSOCKET)
            else if (scheme == "ws")
            {
                listen<io::WebSocketListener>(listenEndpoint);
            }
#endif
            else
            {
                throw std::runtime_error{ "unknown or unsupported endpoint scheme: '" + std::string{ scheme } + "'" };
            }

            LOG_NOTICE_F("listening on endpoint '{}'", listenEndpoint.uriStr());
        }
    }
}
