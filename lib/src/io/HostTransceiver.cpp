#include <dots/io/HostTransceiver.h>
#include <vector>
#include <dots/tools/logging.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>

namespace dots::io
{
    HostTransceiver::HostTransceiver(std::string selfName/* = "DotsHostTransceiver"*/, boost::asio::io_context& ioContext/* = global_io_context()*/, bool staticUserTypes/* = true*/, transition_handler_t transitionHandler/* = nullpt*/) :
        Transceiver(std::move(selfName), ioContext, staticUserTypes),
        m_transitionHandler{ std::move(transitionHandler) }
    {
        /* do nothing */
    }

    Listener& HostTransceiver::listen(listener_ptr_t&& listener)
    {
        Listener* listenerPtr = listener.get();
        m_listeners.emplace(listenerPtr, std::move(listener));

        listenerPtr->asyncAccept(
            [this](Listener& listener, channel_ptr_t channel){ return handleListenAccept(listener, std::move(channel)); },
            [this](Listener& listener, const std::exception_ptr& e){ handleListenError(listener, e); }
        );

        return *listenerPtr;
    }

    void HostTransceiver::publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        if (includedProperties == std::nullopt)
        {
            includedProperties = instance._validProperties();
        }
        else
        {
            *includedProperties ^= instance._descriptor().properties();
        }

        DotsHeader header{
            DotsHeader::typeName_i{ instance._descriptor().name() },
            DotsHeader::sentTime_i{ types::timepoint_t::Now() },
            DotsHeader::serverSentTime_i{ types::timepoint_t::Now() },
            DotsHeader::attributes_i{ *includedProperties },
            DotsHeader::sender_i{ io::Connection::HostId },
            DotsHeader::removeObj_i{ remove },
            DotsHeader::isFromMyself_i{ true }
        };

        Transmission transmission{ std::move(header), instance };
        dispatcher().dispatch(transmission);
        transmit(nullptr, std::move(transmission));
    }

    void HostTransceiver::joinGroup(const std::string_view&/* name*/)
    {
        /* do nothing */
    }

    void HostTransceiver::leaveGroup(const std::string_view&/* name*/)
    {
        /* do nothing */
    }

    void HostTransceiver::transmit(io::Connection* origin, const Transmission& transmission)
    {
        using dirty_connection_t = std::pair<io::Connection*, std::exception_ptr>;
        std::vector<dirty_connection_t> dirtyConnections;

        for (io::Connection* destinationConnection : m_groups[transmission.header().typeName])
        {
            LOG_DEBUG_S("deliver message group:" << this << "(" << *transmission.header().typeName << ")");

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
            std::exception_ptr originError;

            for (const auto& [connection, e] : dirtyConnections)
            {
                if (connection == origin)
                {
                    originError = e;
                }
                else
                {
                    connection->handleError(e);
                }
            }

            if (originError != nullptr)
            {
                std::rethrow_exception(originError);
            }
        }
    }

    bool HostTransceiver::handleListenAccept(Listener&/* listener*/, channel_ptr_t channel)
    {
        auto connection = std::make_shared<io::Connection>(std::move(channel), true);
        connection->asyncReceive(registry(), m_authManager.get(), selfName(),
            [this](io::Connection& connection, Transmission transmission) { handleTransmission(connection, std::move(transmission)); },
            [this](io::Connection& connection, const std::exception_ptr& e) { handleTransition(connection, e); }
        );
        m_guestConnections.emplace(connection.get(), connection);
        LOG_DEBUG_S("guest '" << connection->peerName() << "' emplaced")

        return true;
    }

    void HostTransceiver::handleListenError(Listener& listener, const std::exception_ptr& e)
    {
        try
        {
            std::rethrow_exception(e);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("error while listening for incoming channels -> " << e.what());
        }

        m_listeners.erase(&listener);
    }

    void HostTransceiver::handleTransmission(io::Connection& connection, Transmission transmission)
    {
        const auto& [header, instance] = transmission;

        if (instance->_descriptor().internal())
        {
            if (auto* member = instance.as<DotsMember>())
            {
                handleMemberMessage(connection, *member);
                return;
            }
            else if (auto* descriptorRequest = instance.as<DotsDescriptorRequest>())
            {
                handleDescriptorRequest(connection, *descriptorRequest);
                return;
            }
            else if (auto* clearCache = instance.as<DotsClearCache>())
            {
                handleClearCache(connection, *clearCache);
            }
        }

        dispatcher().dispatch(transmission);
        transmit(&connection, std::move(transmission));
    }

    void HostTransceiver::handleTransition(io::Connection& connection, const std::exception_ptr& e) noexcept
    {
        if (m_transitionHandler)
        {
            try
            {
                m_transitionHandler(connection);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR_S("error in connection transition handler -> " << e.what());
            }
        }

        try
        {
            if (connection.state() == DotsConnectionState::connected)
            {
                LOG_NOTICE_S("guest '" << connection.peerName() << "' opened connection at '" << connection.localEndpoint().uriStr() << "' from '" << connection.remoteEndpoint().uriStr() << "'");
            }
            else if (connection.state() == DotsConnectionState::closed)
            {
                if (e == nullptr)
                {
                    LOG_NOTICE_S("guest '" << connection.peerName() << "' gracefully closed connection");
                }
                else
                {
                    try
                    {
                        std::rethrow_exception(e);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR_S("guest '" << connection.peerName() << "' closed connection with error -> " << e.what());
                    }
                }

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

                LOG_DEBUG_S("guest '" << connection.peerName() << "' erased");
                m_guestConnections.erase(&connection);
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("error while handling connection transition -> peerId: " << connection.peerId() << ", name: " << connection.peerName() << " -> " << e.what());
        }
    }

    void HostTransceiver::handleMemberMessage(io::Connection& connection, const DotsMember& member)
    {
        member._assertHasProperties(DotsMember::groupName_p + DotsMember::event_p);
        const std::string& groupName = member.groupName;

        if (member.event == DotsMemberEvent::kill)
        {
            LOG_WARN_S("guest '" << connection.peerName() << "' requested unsupported kill event");
        }
        else if (member.event == DotsMemberEvent::leave)
        {
            if (size_t removed = m_groups[groupName].erase(&connection); removed == 0)
            {
                LOG_WARN_S("guest '" << connection.peerName() << "' is not a member of group '" << groupName << "'");
            }
        }
        else if (member.event == DotsMemberEvent::join)
        {
            if (auto [it, emplaced] = m_groups[groupName].emplace(&connection); emplaced)
            {
                LOG_INFO_S("guest '" << connection.peerName() << "' is now a member of group '" << groupName << "'");
            }
            else
            {
                LOG_WARN_S("guest '" << connection.peerName() << "' is already member of group '" << groupName << "'");
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
                    DotsCacheInfo::typeName_i{ member.groupName },
                    DotsCacheInfo::endTransmission_i{ true }
                });
            }
        }
    }

    void HostTransceiver::handleDescriptorRequest(io::Connection& connection, const DotsDescriptorRequest& descriptorRequest)
    {
        const types::vector_t<types::string_t>& whiteList = descriptorRequest.whitelist.isValid() ? *descriptorRequest.whitelist : types::vector_t<types::string_t>{};
        const types::vector_t<types::string_t>& blacklist = descriptorRequest.blacklist.isValid() ? *descriptorRequest.blacklist : types::vector_t<types::string_t>{};

        LOG_INFO_S("received DescriptorRequest from " << connection.peerName() << "(" << connection.peerId() << ")");

        registry().forEach<type::StructDescriptor<>>([&](const auto& descriptor) 
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

            LOG_DEBUG_S("sending structDescriptor for type '" << descriptor.name() << "' to " << connection.peerId());

            connection.transmit(descriptor);
        });

        connection.transmit(DotsCacheInfo{ DotsCacheInfo::endDescriptorRequest_i{ true } });
    }

    void HostTransceiver::handleClearCache(io::Connection&/* connection*/, const DotsClearCache& clearCache)
    {
        clearCache._assertHasProperties(DotsClearCache::typeNames_p);
        const types::vector_t<types::string_t>& typeNames = clearCache.typeNames;

        for (auto& [descriptor, container] : pool())
        {
            if (std::find(typeNames.begin(), typeNames.end(), container.descriptor().name()) != typeNames.end())
            {
                LOG_INFO_S("clear container '" << container.descriptor().name() << "' (" << container.size() << " elements)");
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

    void HostTransceiver::transmitContainer(io::Connection& connection, const Container<>& container)
    {
        const auto& td = container.descriptor();

        LOG_DEBUG_S("send cache for " << td.name() << " size=" << container.size());

        if (container.empty())
        {
            return;
        }

        DotsHeader header{
            DotsHeader::typeName_i{ container.descriptor().name() },
            DotsHeader::fromCache_i{ static_cast<uint32_t>(container.size()) },
            DotsHeader::removeObj_i{ false }
        };

        for (const auto& [instance, cloneInfo] : container)
        {
            const char* lop = "";
            switch (cloneInfo.lastOperation)
            {
                case DotsMt::create: lop = "C";
                    break;
                case DotsMt::update: lop = "U";
                    break;
                case DotsMt::remove: lop = "R";
                    break;
            }

            LOG_DATA_S("clone-info: lastOp=" << lop << ", lastUpdateFrom=" << cloneInfo.lastUpdateFrom
                << ", created=" << cloneInfo.created->toString() << ", creator=" << cloneInfo.createdFrom
                << ", modified=" << cloneInfo.modified->toString() << ", localUpdateTime=" << cloneInfo.localUpdateTime->toString());

            header.sentTime = *cloneInfo.modified;
            header.serverSentTime = types::timepoint_t::Now();
            header.attributes = instance->_validProperties();
            header.sender = *cloneInfo.lastUpdateFrom;
            --*header.fromCache;

            connection.transmit(header, instance);
        }
    }
}
