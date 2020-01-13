#include "ConnectionManager.h"
#include "dots/io/TD_Traversal.h"
#include "dots/io/Registry.h"
#include <dots/dots.h>

#include "DotsCacheInfo.dots.h"
#include "DotsClient.dots.h"

namespace dots
{
    ConnectionManager::ConnectionManager(std::unique_ptr<Listener>&& listener, const std::string& name) :
        m_name(name),
        m_listener(std::move(listener))
    {
        m_dispatcher.pool().get<DotsClient>();
        m_onNewStruct = transceiver().registry().onNewStruct.connect(FUN(*this, onNewType));

        asyncAccept();
    }

    void ConnectionManager::init()
    {
        m_distributedTypeId = std::make_unique<DistributedTypeId>(true);

        for (auto& t : transceiver().registry().getTypes())
        {
            m_distributedTypeId->createTypeId(t.second.get());
        }

        add_timer(1, FUN(*this, clientCleanup));
    }

    const ContainerPool& ConnectionManager::pool() const
	{
		return m_dispatcher.pool();
	}

    void ConnectionManager::publishNs(const string& nameSpace,
                                      const type::StructDescriptor<>* td,
                                      const type::Struct& instance,
                                      type::PropertySet properties,
                                      bool remove, bool processLocal)
    {
        DotsTransportHeader header;
        m_transmitter.prepareHeader(header, td, properties, remove);
        header.dotsHeader->serverSentTime(pnxs::SystemNow());
        header.dotsHeader->sender(io::Connection::ServerIdDeprecated);
        if (!nameSpace.empty())
        header.nameSpace(nameSpace);

        // TODO: avoid local copy
        Transmission transmission{ type::AnyStruct{ instance } };

        // Send to peer or group
        if (processLocal)
        {
            m_dispatcher.dispatch(header.dotsHeader, transmission.instance(), true);
        }

        Group* grp = getGroup({ header.destinationGroup });
        if (grp) grp->deliver(header, std::move(transmission));
    }

    void ConnectionManager::publish(const type::StructDescriptor<>* td, const type::Struct& instance, type::PropertySet properties, bool remove)
    {
        publishNs("SYS", td, instance, properties, remove, true);
    }

    void ConnectionManager::publish(const type::Struct& instance, types::property_set_t what, bool remove)
    {
        publishNs({}, &instance._descriptor(), instance, what, remove, true);
    }

    void ConnectionManager::remove(const type::Struct& instance)
    {
        publish(instance, instance._keyProperties(), true);
    }

    void ConnectionManager::clientCleanup()
    {
        m_cleanupConnections.clear();

        std::set<io::Connection::id_t> obsoleteClients;

        for (auto& element : m_dispatcher.container<DotsClient>())
        {
            const auto& client = element.first.to<DotsClient>();

            if (client.connectionState == DotsConnectionState::closed)
            {
                for (const auto& [descriptor, container] : m_dispatcher.pool())
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

                obsoleteClients.emplace(client.id);
            }
        }

        for (io::Connection::id_t id : obsoleteClients)
        {
            remove(DotsClient{ DotsClient::id_i{ id } });
        }

        add_timer(1, FUN(*this, clientCleanup));
    }

    void ConnectionManager::onNewType(const dots::type::StructDescriptor<>* td)
    {
        LOG_DEBUG_S("onNewType name=" << td->name() << " flags:" << flags2String(td));
        LOG_INFO_S("register type " << td->name() << " published by " << m_name);
        
        if (!td->cached())
        {
            return;
        }

        const Container<>& container = m_dispatcher.container(*td);

        if (td->cleanup())
        {
            m_cleanupContainer.push_back(&container);
        }

        m_dispatcher.subscribe(*td, [](const Event<>&)
        {
        }).discard();
    }

    void ConnectionManager::asyncAccept()
    {
        Listener::accept_handler_t acceptHandler = [this](channel_ptr_t channel)
        {
            auto connection = std::make_shared<io::Connection>(std::move(channel), true);
            m_connections.insert({ connection->id(), connection });
            connection->asyncReceive(transceiver().registry(), m_name,
                [this](io::Connection& connection, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself) { return handleReceive(connection, header, std::move(transmission), isFromMyself); },
                [this](io::Connection& connection, const std::exception* e) { handleClose(connection, e); }
            );

            return true;
        };

        Listener::error_handler_t errorHandler = [](const std::exception& e)
        {
            LOG_ERROR_S("error while listening for incoming channels -> " << e.what());
        };

        m_listener->asyncAccept(std::move(acceptHandler), std::move(errorHandler));
    }

    bool ConnectionManager::handleReceive(io::Connection& connection, const DotsTransportHeader& transportHeader, Transmission&& transmission, bool isFromMyself)
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

        Group* grp = getGroup({ transportHeader.destinationGroup });
        if (grp) grp->deliver(transportHeader, transmission);

        return true;
    }

    void ConnectionManager::handleClose(io::Connection& connection, const std::exception* e)
    {
        if (e != nullptr)
        {
            LOG_ERROR_S("connection error: " << e->what());
        }

        for (auto& i : m_allGroups)
        {
            auto& group = i.second;
            group->handleKill(&connection);
        }
        
        if (auto it = m_connections.find(connection.id()); it != m_connections.end())
        {
            m_cleanupConnections.insert(it->second);
            m_connections.erase(it);
            return;
        }

        cleanupObjects(&connection);

        LOG_INFO_S("connection closed -> id: " << connection.id() << ", name: " << connection.name());
    }

    io::connection_ptr_t ConnectionManager::findConnection(const io::Connection::id_t& id)
    {
        auto it = m_connections.find(id);
        if (it != m_connections.end())
        {
            return it->second;
        }
        return {};
    }

    void ConnectionManager::handleMemberMessage(io::Connection& connection, const DotsMember& member)
    {
        member._assertHasProperties(DotsMember::groupName_p + DotsMember::event_p);
        LOG_DEBUG_S(*member.event << " " << member.groupName);

        if (member.event == DotsMemberEvent::kill)
        {
            handleClose(connection, nullptr);
        }
        else if (member.event == DotsMemberEvent::leave)
        {
            auto group = getGroup(member.groupName);
            if (group == nullptr)
            {
                LOG_ERROR_S("group does not exist");
                return;
            }

            group->handleLeave(&connection);
        }
        else if (member.event == DotsMemberEvent::join)
        {
            auto group = getGroup(member.groupName);
            if (group == nullptr)
            {
                group = new Group(member.groupName);
                m_allGroups.insert({ member.groupName, group });
            }

            group->handleJoin(&connection);

            if (const Container<>* container = m_dispatcher.pool().find(*member.groupName); container != nullptr)
            {
                if (container->descriptor().cached())
                {
                    sendContainerContent(connection, *container);
                }

                connection.transmit(DotsCacheInfo{
                    DotsCacheInfo::typeName_i{ member.groupName },
                    DotsCacheInfo::endTransmission_i{ true }
                });
            }
        }
    }

    void ConnectionManager::handleDescriptorRequest(io::Connection& connection, const DotsDescriptorRequest& descriptorRequest)
    {
        auto& wl = descriptorRequest.whitelist.isValid() ? *descriptorRequest.whitelist : dots::type::Vector<string>();

        dots::TD_Traversal traversal;

        LOG_INFO_S("received DescriptorRequest from " << connection.name() << "(" << connection.id() << ")");

        for (const auto& cpItem : m_dispatcher.pool())
        {
            const auto& container = cpItem.second;
            const auto& td = container.descriptor();

            if (!wl.empty() && std::find(wl.begin(), wl.end(), td.name()) == wl.end())
            {
                // when whitelist is set, skip all types, that are not on the list.
                continue;
            }

            if (descriptorRequest.blacklist.isValid())
            {
                auto& bl = *descriptorRequest.blacklist;
                if (std::find(bl.begin(), bl.end(), td.name()) != wl.end())
                {
                    // if blacklist is set and the type was found on the list, skip it.
                    continue;
                }
            }

            if (td.internal()) continue; // skip internal types

            LOG_DEBUG_S("sending descriptor for type '" << td.name() << "' to " << connection.id());
            traversal.traverseDescriptorData(&td, [&](auto td, auto body)
            {
                DotsTransportHeader thead;
                m_transmitter.prepareHeader(thead, td, td->validProperties(body), false);
                thead.dotsHeader->sentTime = pnxs::SystemNow();
                thead.dotsHeader->sender(io::Connection::ServerIdDeprecated);

                // Send to peer or group
                connection.transmit(thead, *reinterpret_cast<const type::Struct*>(body));
            });
        }

        DotsCacheInfo dotsCacheInfo{
            DotsCacheInfo::endDescriptorRequest_i{ true }
        };
        connection.transmit(dotsCacheInfo);
    }

    void ConnectionManager::handleClearCache(io::Connection&/* connection*/, const DotsClearCache& clearCache)
    {
        auto& whitelist = clearCache.typeNames.isValid() ? *clearCache.typeNames : dots::type::Vector<string>();

        for (auto& cpItem : m_dispatcher.pool())
        {
            auto& container = cpItem.second;

            if (std::find(whitelist.begin(), whitelist.end(), container.descriptor().name()) == whitelist.end())
            {
                continue; // not found
            }

            // clear container content
            LOG_INFO_S("clear container '" << container.descriptor().name() << "' (" << container.size() << " elements)");

            // publish remove for every element of the container
            for (auto& element : container)
            {
                publishNs({}, &container.descriptor(), element.first, container.descriptor().keys(), true, false);
            }

            container.clear();
        }
    }

    void ConnectionManager::sendContainerContent(io::Connection& connection, const Container<>& container)
    {
        const auto& td = container.descriptor();

        LOG_DEBUG_S("send cache for " << td.name() << " size=" << container.size());
        uint32_t remainingCacheObjects = container.size();
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

            DotsTransportHeader thead;
            m_transmitter.prepareHeader(thead, &td, instance->_validProperties(), false);

            auto& dotsHeader = *thead.dotsHeader;
            dotsHeader.sentTime = cloneInfo.modified.isValid() ? *cloneInfo.modified : *cloneInfo.created;
            dotsHeader.serverSentTime = pnxs::SystemNow();
            dotsHeader.sender = cloneInfo.lastUpdateFrom;
            dotsHeader.fromCache = --remainingCacheObjects;

            connection.transmit(thead, instance);
        }
    }

    void ConnectionManager::cleanupObjects(io::Connection* connection)
    {
        for (const auto& container : m_cleanupContainer)
        {
            vector<const type::Struct*> remove;

            // Search for objects which where sent by this killed Connection.
            for (const auto& [instance, cloneInfo] : *container)
            {
                if (connection->id() == cloneInfo.lastUpdateFrom)
                {
                    remove.push_back(&*instance);
                }
            }

            for (auto item : remove)
            {
                publishNs({}, &container->descriptor(), *reinterpret_cast<const type::Struct*>(item), container->descriptor().keys(), true);
            }
        }
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
    std::string ConnectionManager::flags2String(const dots::type::StructDescriptor<>* td)
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
