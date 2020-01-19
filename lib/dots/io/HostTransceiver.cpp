#include "HostTransceiver.h"
#include <dots/dots.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>

namespace dots
{
    HostTransceiver::HostTransceiver(std::string selfName, new_struct_type_handler_t newStructTypeHandler, transition_handler_t transitionHandler) :
        m_registry{ std::move(newStructTypeHandler) },
        m_selfName{ std::move(selfName) },
        m_transitionHandler{ std::move(transitionHandler) }
    {
        /* do nothing */
    }

    void HostTransceiver::listen(listener_ptr_t&& listener)
    {
        Listener* listenerPtr = listener.get();
        m_listeners.emplace(listenerPtr, std::move(listener));

        listenerPtr->asyncAccept(
            [this](Listener& listener, channel_ptr_t channel){ return handleListenAccept(listener, std::move(channel)); },
            [this](Listener& listener, const std::exception& e){ handleListenError(listener, e); }
        );
    }

    const std::string& HostTransceiver::selfName() const
    {
        return m_selfName;
    }

    const ContainerPool& HostTransceiver::pool() const
	{
		return m_dispatcher.pool();
	}

    void HostTransceiver::publish(const type::Struct& instance, types::property_set_t includedProperties, bool remove)
    {
        const type::StructDescriptor<>& descriptor = instance._descriptor();

        DotsTransportHeader header{
            DotsTransportHeader::destinationGroup_i{ descriptor.name() },
            DotsTransportHeader::dotsHeader_i{
                DotsHeader::typeName_i{ descriptor.name() },
                DotsHeader::sentTime_i{ pnxs::SystemNow() },
                DotsHeader::serverSentTime_i{ pnxs::SystemNow() },
                DotsHeader::attributes_i{ includedProperties ==  types::property_set_t::All ? instance._validProperties() : includedProperties },
				DotsHeader::sender_i{ io::Connection::HostId },
                DotsHeader::removeObj_i{ remove }
            }
        };

        if (descriptor.internal() && !instance._is<DotsClient>() && !instance._is<DotsDescriptorRequest>())
        {
            header.nameSpace("SYS");
        }

        // TODO: avoid local copy
        Transmission transmission{ type::AnyStruct{ instance } };

        m_dispatcher.dispatch(header.dotsHeader, transmission.instance(), true);

        for (io::Connection* destinationConnection : m_groups[header.destinationGroup])
        {
            LOG_DEBUG_S("deliver message group:" << this << "(" << *header.destinationGroup << ")");

            if (destinationConnection->state() != DotsConnectionState::closed)
            {
                destinationConnection->transmit(header, transmission);
            }
        }
    }

    void HostTransceiver::remove(const type::Struct& instance)
    {
        publish(instance, instance._keyProperties(), true);
    }

    void HostTransceiver::publish(const type::StructDescriptor<>*/* td*/, const type::Struct& instance, type::PropertySet properties, bool remove)
    {
        publish(instance, properties, remove);
    }

    bool HostTransceiver::handleListenAccept(Listener&/* listener*/, channel_ptr_t channel)
    {
        auto connection = std::make_shared<io::Connection>(std::move(channel), true);
        connection->asyncReceive(m_registry, m_selfName,
            [this](io::Connection& connection, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself) { return handleReceive(connection, header, std::move(transmission), isFromMyself); },
            [this](io::Connection& connection, const std::exception* e) { handleTransition(connection, e); }
        );
        m_openConnections.emplace(connection.get(), connection);

        return true;
    }

    void HostTransceiver::handleListenError(Listener& listener, const std::exception& e)
    {
        LOG_ERROR_S("error while listening for incoming channels -> " << e.what());
        m_listeners.erase(&listener);
    }

    bool HostTransceiver::handleReceive(io::Connection& connection, const DotsTransportHeader& transportHeader, Transmission&& transmission, bool isFromMyself)
    {
        if (const type::Struct& instance = transmission.instance(); instance._descriptor().internal())
        {
            if (auto* member = instance._as<DotsMember>())
            {
                handleMemberMessage(connection, *member);
            }
            else if (auto* descriptorRequest = instance._as<DotsDescriptorRequest>())
            {
                handleDescriptorRequest(connection, *descriptorRequest);
            }
            else if (auto* clearCache = instance._as<DotsClearCache>())
            {
                handleClearCache(connection, *clearCache);
            }
        }

        m_dispatcher.dispatch(transportHeader.dotsHeader, transmission.instance(), isFromMyself);

        for (io::Connection* destinationConnection : m_groups[transportHeader.destinationGroup])
        {
            LOG_DEBUG_S("deliver message group:" << this << "(" << transportHeader.destinationGroup << ")");

            if (destinationConnection->state() != DotsConnectionState::closed)
            {
                destinationConnection->transmit(transportHeader, transmission);
            }
        }

        return true;
    }

    void HostTransceiver::handleTransition(io::Connection& connection, const std::exception* e)
    {
        if (e != nullptr)
        {
            LOG_ERROR_S("connection error: " << e->what());
        }

        if (connection.state() == DotsConnectionState::closed)
        {
            add_timer(1, [&]()
            {
                for (auto& [groupName, group] : m_groups)
                {
                    group.erase(&connection);
                }
                m_openConnections.erase(&connection);
            });

            std::vector<const type::Struct*> cleanupInstances;

            for (const auto& [descriptor, container] : m_dispatcher.pool())
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

            LOG_INFO_S("connection closed -> peerId: " << connection.peerId() << ", name: " << connection.peerName());
        }

        if (m_transitionHandler)
        {
            m_transitionHandler(connection);
        }
    }

    void HostTransceiver::handleMemberMessage(io::Connection& connection, const DotsMember& member)
    {
        member._assertHasProperties(DotsMember::groupName_p + DotsMember::event_p);
        LOG_DEBUG_S(*member.event << " " << member.groupName);
        const std::string& groupName = member.groupName;

        if (member.event == DotsMemberEvent::kill)
        {
            LOG_WARN_S("kill event not supported");
        }
        else if (member.event == DotsMemberEvent::leave)
        {
            if (size_t removed = m_groups[groupName].erase(&connection); removed == 0)
            {
                LOG_WARN_S("invalid group leave: connection " << connection.peerName() << " is not a member of group " << groupName);
            }
        }
        else if (member.event == DotsMemberEvent::join)
        {
            if (auto [it, emplaced] = m_groups[groupName].emplace(&connection); emplaced)
            {
                if (const Container<>* container = m_dispatcher.pool().find(groupName); container != nullptr)
                {
                    if (container->descriptor().cached())
                    {
                        transmitContainer(connection, *container);
                    }

                    connection.transmit(DotsCacheInfo{
                        DotsCacheInfo::typeName_i{ member.groupName },
                        DotsCacheInfo::endTransmission_i{ true }
                    });
                }
            }
            else
            {
                LOG_WARN_S("invalid group join: connection " << connection.peerName() << " is already member of group " << groupName);
            }
        }
    }

    void HostTransceiver::handleDescriptorRequest(io::Connection& connection, const DotsDescriptorRequest& descriptorRequest)
    {
        const types::vector_t<types::string_t>& whiteList = descriptorRequest.whitelist.isValid() ? *descriptorRequest.whitelist : types::vector_t<types::string_t>{};
        const types::vector_t<types::string_t>& blacklist = descriptorRequest.blacklist.isValid() ? *descriptorRequest.blacklist : types::vector_t<types::string_t>{};

        LOG_INFO_S("received DescriptorRequest from " << connection.peerName() << "(" << connection.peerId() << ")");

        for (const auto& [descriptor, container] : m_dispatcher.pool())
        {
            if (descriptor->internal())
            {
                continue;
            }

            if (!whiteList.empty() && std::find(whiteList.begin(), whiteList.end(), descriptor->name()) == whiteList.end())
            {
                continue;
            }

            if (!blacklist.empty() && std::find(blacklist.begin(), blacklist.end(), descriptor->name()) != blacklist.end())
            {
                continue;
            }

            LOG_DEBUG_S("sending descriptor for type '" << descriptor->name() << "' to " << connection.peerId());

            connection.transmit(*descriptor);
        }

        connection.transmit(DotsCacheInfo{ DotsCacheInfo::endDescriptorRequest_i{ true } });
    }

    void HostTransceiver::handleClearCache(io::Connection&/* connection*/, const DotsClearCache& clearCache)
    {
        clearCache._assertHasProperties(DotsClearCache::typeNames_p);
        const types::vector_t<types::string_t>& typeNames = clearCache.typeNames;

        for (auto& [descriptor, container] : m_dispatcher.pool())
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

        DotsTransportHeader header{
            DotsTransportHeader::destinationGroup_i{ container.descriptor().name() },
            DotsTransportHeader::dotsHeader_i{
                DotsHeader::typeName_i{ container.descriptor().name() },
                DotsHeader::fromCache_i{ container.size() },
                DotsHeader::removeObj_i{ false }
            }
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

            DotsHeader& dotsHeader = header.dotsHeader;
            dotsHeader.sentTime = *cloneInfo.modified;
            dotsHeader.serverSentTime = pnxs::SystemNow();
            dotsHeader.attributes = instance->_validProperties();
            dotsHeader.sender = *cloneInfo.lastUpdateFrom;
			--*dotsHeader.fromCache;

            connection.transmit(header, instance);
        }
    }
}
