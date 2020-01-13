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
        m_dispatcher.subscribe<DotsMember>(FUN(*this, handleMemberMessage)).discard();
        m_dispatcher.subscribe<DotsDescriptorRequest>(FUN(*this, handleDescriptorRequest)).discard();
        m_dispatcher.subscribe<DotsClearCache>(FUN(*this, handleClearCache)).discard();
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
    static std::string flags2String(const dots::type::StructDescriptor<>* td)
    {
        std::string ret = ".....";
        if (td->cached()) ret[0] = 'C';
        if (td->internal()) ret[1] = 'I';
        if (td->persistent()) ret[2] = 'P';
        if (td->cleanup()) ret[3] = 'c';
        if (td->local()) ret[4] = 'L';
        return ret;
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

    bool ConnectionManager::handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission, bool isFromMyself)
    {
        m_dispatcher.dispatch(transportHeader.dotsHeader, transmission.instance(), isFromMyself);

        Group* grp = getGroup({ transportHeader.destinationGroup });
        if (grp) grp->deliver(transportHeader, transmission);

        return true;
    }

    void ConnectionManager::handleMemberMessage(const DotsMember::Cbd& cbd)
    {
        io::Connection* connection = m_connections.find(cbd.header().sender)->second.get();

        const DotsMember& member = cbd();
        DotsMember memberMod = member;
        memberMod.client(connection->id());
        if (!member.event.isValid())
        {
            LOG_WARN_S("member message without event");
        }
        LOG_DEBUG_S(*member.event << " " << member.groupName);

        if (member.event == DotsMemberEvent::kill)
        {
            handleKill(connection);

            if (connection)
            {
                cleanupObjects(connection);
                m_connections.erase(connection->id());
            }
        }
        else if (member.event == DotsMemberEvent::leave)
        {
           handleLeave(member.groupName, connection);
        }
        else if (member.event == DotsMemberEvent::join)
        {
            handleJoin(member.groupName, connection);

            auto& typeName = member.groupName;

            const Container<>* container = m_dispatcher.pool().find(*typeName);
            if (container == nullptr) return;

            if (container->descriptor().cached())
            {
                sendContainerContent(*connection, *container);
            }
            else
            {
                sendCacheEnd(*connection, typeName);
            }
        }
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

    void ConnectionManager::handleClose(io::Connection::id_t id, const std::exception* e)
    {
        if (e != nullptr)
        {
            LOG_ERROR_S("connection error: " << e->what());
        }

        const io::connection_ptr_t& connection = findConnection(id);

        if (connection == nullptr)
        {
            LOG_WARN_S("cannot close unknown connection -> id: " << id);
            return;
        }

        handleKill(connection.get());

        // move connection to m_cleanupConnection for later deletion.
        m_cleanupConnections.insert(connection);
        removeConnection(connection);

        // look if instances have to be cleaned up
        cleanupObjects(connection.get());

        LOG_INFO_S("connection closed -> id: " << connection->id() << ", name: " << connection->name());
    }

    void ConnectionManager::asyncAccept()
    {
        Listener::accept_handler_t acceptHandler = [this](channel_ptr_t channel)
        {
            auto connection = std::make_shared<io::Connection>(std::move(channel), true);
            m_connections.insert({ connection->id(), connection });
            connection->asyncReceive(transceiver().registry(), m_name,
                                     [this](const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself) { return handleReceive(header, std::move(transmission), isFromMyself); },
                                     [this](io::Connection::id_t id, const std::exception* e) { handleClose(id, e); }
            );

            return true;
        };

        Listener::error_handler_t errorHandler = [](const std::exception& e)
        {
            LOG_ERROR_S("error while listening for incoming channels -> " << e.what());
        };

        m_listener->asyncAccept(std::move(acceptHandler), std::move(errorHandler));
    }

    void ConnectionManager::removeConnection(io::connection_ptr_t c)
    {
        auto it = m_connections.find(c->id());
        if (it != m_connections.end())
        {
            m_connections.erase(it);
            return;
        }
    }

    bool ConnectionManager::isClientIdInContainers(ClientId id)
    {
        for (auto& poolIter : m_dispatcher.pool())
        {
            auto& container = poolIter.second;
            for (auto& element : container)
            {
                if (element.second.createdFrom == id) return true;
                if (element.second.lastUpdateFrom == id) return true;
            }
        }
        return false;
    }

    void ConnectionManager::cleanup()
    {
        m_cleanupConnections.clear();

        const auto& container = m_dispatcher.container<DotsClient>();
        std::vector<ClientId> clientsToRemove;

        for (auto& element : container)
        {
            const auto& client = static_cast<const DotsClient&>(element.first);
            if (client.connectionState == DotsConnectionState::closed)
            {
                // Search for a ClientId reference in all containers
                if (isClientIdInContainers(client.id))
                {
                    continue;
                }
                else
                {
                    clientsToRemove.push_back(client.id);
                }
            }
        }

        for (auto& id : clientsToRemove)
        {
            DotsClient client(DotsClient::id_i{ id });
            client._remove();
        }
    }

    void ConnectionManager::handleDescriptorRequest(const DotsDescriptorRequest::Cbd& cbd)
    {
        if (cbd.isOwnUpdate()) return;

        auto& wl = cbd().whitelist.IsPartOf(cbd.updatedProperties()) ? *cbd().whitelist : dots::type::Vector<string>();

        dots::TD_Traversal traversal;

        auto connection = findConnection(cbd.header().sender);

        if (!connection)
        {
            LOG_WARN_S("no connection found");
            return;
        }

        LOG_INFO_S("received DescriptorRequest from " << connection->name() << "(" << connection->id() << ")");

        for (const auto& cpItem : m_dispatcher.pool())
        {
            const auto& container = cpItem.second;
            const auto& td = container.descriptor();

            if (!wl.empty() && std::find(wl.begin(), wl.end(), td.name()) == wl.end())
            {
                // when whitelist is set, skip all types, that are not on the list.
                continue;
            }

            if (cbd().blacklist.isValid())
            {
                auto& bl = *cbd().blacklist;
                if (std::find(bl.begin(), bl.end(), td.name()) != wl.end())
                {
                    // if blacklist is set and the type was found on the list, skip it.
                    continue;
                }
            }

            if (td.internal()) continue; // skip internal types

            LOG_DEBUG_S("sending descriptor for type '" << td.name() << "' to " << cbd.header().sender);
            traversal.traverseDescriptorData(&td, [&](auto td, auto body)
            {
                DotsTransportHeader thead;
                m_transmitter.prepareHeader(thead, td, td->validProperties(body), false);
                thead.dotsHeader->sentTime = pnxs::SystemNow();
                thead.dotsHeader->sender(io::Connection::ServerIdDeprecated);

                // Send to peer or group
                connection->transmit(thead, *reinterpret_cast<const type::Struct*>(body));
            });
        }

        DotsCacheInfo dotsCacheInfo{
            DotsCacheInfo::endDescriptorRequest_i{ true }
        };
        connection->transmit(dotsCacheInfo);
    }

    void ConnectionManager::handleClearCache(const DotsClearCache::Cbd& cbd)
    {
        auto& whitelist = cbd().typeNames.isValid() ? *cbd().typeNames : dots::type::Vector<string>();

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
            handleReceive(header, std::move(transmission), true);
        }
        else
        {
            if (header.destinationGroup.isValid())
            {
                Group* grp = getGroup({ header.destinationGroup });
                if (grp) grp->deliver(header, std::move(transmission));
            }
        }
    }

    DotsStatistics ConnectionManager::receiveStatistics() const
    {
        //return m_dispatcher.statistics();
        // TODO: determine if still necessary
        return DotsStatistics{};
    }

    DotsCacheStatus ConnectionManager::cacheStatus() const
    {
        DotsCacheStatus cs;

        auto& pool = m_dispatcher.pool();

        cs.nrTypes(pool.size());
        cs.size(pool.totalMemoryUsage());
        return cs;
    }

    void
    ConnectionManager::publish(const type::StructDescriptor<>* td, const type::Struct& instance, type::PropertySet properties, bool remove)
    {
        publishNs("SYS", td, instance, properties, remove, true);
    }

    string ConnectionManager::clientId2Name(ClientId id) const
    {
        const auto& container = m_dispatcher.container<DotsClient>();

        const DotsClient* client = container.find(DotsClient{
            DotsClient::id_i{ id }
        });

        if (client != nullptr)
        {
            if (client->name.isValid())
                return client->name;
        }

        if (id == 0)
        {
            return m_name;
        }

        return std::to_string(id);
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

        sendCacheEnd(connection, td.name());
    }

    void ConnectionManager::sendCacheEnd(io::Connection& connection, const std::string& typeName)
    {
        DotsCacheInfo dotsCacheInfo{
            DotsCacheInfo::typeName_i{ typeName },
            DotsCacheInfo::endTransmission_i{ true }
        };
        connection.transmit(dotsCacheInfo);
    }

    void ConnectionManager::handleJoin(const string& groupKey, io::Connection* connection)
    {
        auto group = getGroup(groupKey);
        if (group == nullptr)
        {
            group = new Group(groupKey);
            m_allGroups.insert({ group->name(), group });
        }

        group->handleJoin(connection);
    }

    void ConnectionManager::handleLeave(const string& groupKey, io::Connection* connection)
    {
        auto group = getGroup(groupKey);
        if (group == nullptr)
        {
            LOG_ERROR_S("group does not exist");
            return;
        }

        group->handleLeave(connection);
    }

    void ConnectionManager::handleKill(io::Connection* connection)
    {
        for (auto& i : m_allGroups)
        {
            auto& group = i.second;
            group->handleKill(connection);
        }
    }
}
