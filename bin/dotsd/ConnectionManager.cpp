#include "ConnectionManager.h"
#include "dots/io/TD_Traversal.h"
#include "dots/type/Registry.h"

#include "DotsCacheInfo.dots.h"
#include "DotsClient.dots.h"

namespace dots {


ConnectionManager::ConnectionManager(GroupManager &groupManager, ServerInfo &server)
        :m_groupManager(groupManager), m_serverInfo(server)
{
	m_dispatcher.pool().get<DotsClient>();
    dots::type::Descriptor::registry().onNewStruct.connect(FUN(*this, onNewType));
    m_dispatcher.subscribe<DotsDescriptorRequest>(FUN(*this, handleDescriptorRequest)).discard();
    m_dispatcher.subscribe<DotsClearCache>(FUN(*this, handleClearCache)).discard();
}

void ConnectionManager::init()
{
    m_distributedTypeId = std::make_unique<DistributedTypeId>(true);

    // Register all types, that are registered before this instance was created
    for (auto& t : type::Descriptor::registry().getTypes())
    {
        m_distributedTypeId->createTypeId(t.second);
    }
}

void ConnectionManager::start(connection_ptr c)
{
    m_connections.insert({c->id(), c});
    c->start();
}

/*
void ConnectionManager::stop(connection_ptr c)
{
    m_connections.erase(c);
    c->stop();
}
 */

void ConnectionManager::stop_all()
{
    for (auto c : m_connections)
    {
        c.second->stop();
    }
    m_connections.clear();

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
static std::string flags2String(const dots::type::StructDescriptor* td)
{
    std::string ret = ".....";
    if (td->cached())       ret[0] = 'C';
    if (td->internal())     ret[1] = 'I';
    if (td->persistent())   ret[2] = 'P';
    if (td->cleanup())      ret[3] = 'c';
    if (td->local())        ret[4] = 'L';
    return ret;
}

/*!
 * Called for newly registered DOTS types.
 * @param td typedescriptor of the new type.
 */
void ConnectionManager::onNewType(const dots::type::StructDescriptor* td)
{
    LOG_INFO_S("register type " << td->name() << " published by " << clientId2Name(td->publisherId()));

    LOG_DEBUG_S("onNewType name=" << td->name() << " flags:" << flags2String(td));

    // Only continue, if type is a cached type
    if(not td->cached())
    {
        return;
    }

	const Container<>& container = m_dispatcher.container(*td);

    // Cached types can be marked as "cleanup"
    if(td->cleanup())
    {
        m_cleanupContainer.push_back(&container);
    }

	m_dispatcher.subscribe(*td, [](const Event<>&){}).discard();
}

void ConnectionManager::deliver(const DotsTransportHeader& transportHeader, Transmission&& transmission)
{
    if(transportHeader.destinationGroup.isValid())
    {
        if(m_CacheEnabled)
        {
            DotsHeader dotsHeader = transportHeader.dotsHeader;
            dotsHeader.isFromMyself(dotsHeader.sender == 1u);
            m_dispatcher.dispatch(dotsHeader, transmission.instance());
        }

        Group *grp = m_groupManager.getGroup({ transportHeader.destinationGroup });
        if (grp) grp->deliver(transportHeader, transmission);

        return;
    }

    // Send to a specific client (unicast)
    if (transportHeader.destinationClientId.isValid())
    {
        auto dstConnection = findConnection(transportHeader.destinationClientId);
        if (dstConnection)
        {
            dstConnection->send(transportHeader, transmission);
        }
    }
}

void ConnectionManager::processMemberMessage(const DotsTransportHeader& /*header*/, const DotsMember &member, Connection *connection)
{
    if (member.event == DotsMemberEvent::kill) {
        m_groupManager.handleKill(connection);

        if (connection) {
            cleanupObjects(connection);
            m_connections.erase(connection->id());
        }
    }
    else if (member.event == DotsMemberEvent::leave)
    {
        m_groupManager.handleLeave(member.groupName, connection);
    }
    else if (member.event == DotsMemberEvent::join)
    {
        m_groupManager.handleJoin(member.groupName, connection);

        if (not m_CacheEnabled) return;
        // Check for system_group?

        auto& typeName = member.groupName;

        const Container<>* container = m_dispatcher.pool().find(*typeName);
        if (container == nullptr) return;

        if (container->descriptor().cached())
        {
            connection->sendContainerContent(*container);
        }
        else
        {
            connection->sendCacheEnd(typeName);
        }
    }
}

connection_ptr ConnectionManager::findConnection(const Connection::ConnectionId &id)
{
    auto it = m_connections.find(id);
    if (it != m_connections.end())
    {
        return it->second;
    }
    return {};
}

#if 0
connection_ptr ConnectionManager::findConnection(const DotsPeerAddress &pa)
{
    for (auto& c : m_connections)
    {
        if (c.second->peerAddress() == pa)
            return c.second;
    }
    return {};
}
#endif

void ConnectionManager::handleKill(Connection *connection)
{
    m_groupManager.handleKill(connection);

    auto connPtr = findConnection(connection->id());
    if (connPtr)
    {
        // Move connection to m_cleanupConnection for later deletion.
        m_cleanupConnections.insert(connPtr);
        connPtr->stop();
        removeConnection(connPtr);

        // Look if objects has to be cleaned up
        cleanupObjects(connection);
    }
}

void ConnectionManager::removeConnection(connection_ptr c)
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
    for (auto& c : m_cleanupConnections) {
        c->stop();
    }
    m_cleanupConnections.clear();

    const auto& container = m_dispatcher.container<DotsClient>();
    std::vector<ClientId> clientsToRemove;

    for (auto &element : container) {
        const auto& client = static_cast<const DotsClient&>(element.first);
        if (client.connectionState == DotsConnectionState::closed) {
            // Search for a ClientId reference in all containers
            //LOG_DATA_S("check closed conn state of " << client->name());
            if (isClientIdInContainers(client.id)) {
                continue;
            }
            else {
                clientsToRemove.push_back(client.id);
            }
        }
    }

    for (auto& id : clientsToRemove) {
		DotsClient client(DotsClient::id_t_i{ id });
        client._remove();
    }
}

/**
 * Sends all registered descriptors directly to the requester
 * @param cbd
 */
void ConnectionManager::handleDescriptorRequest(const DotsDescriptorRequest::Cbd &cbd)
{
    if (cbd.isOwnUpdate()) return;

    auto& wl = cbd().whitelist.IsPartOf(cbd.updatedProperties()) ? *cbd().whitelist : dots::Vector<string>();

    dots::TD_Traversal traversal;

    auto connection = findConnection(cbd.header().sender);

    if (not connection)
    {
        LOG_WARN_S("no connection found");
        return;
    }

    LOG_INFO_S("received DescriptorRequest from " << connection->clientName() << "(" << connection->id() << ")");

    for (const auto& cpItem : m_dispatcher.pool())
    {
        const auto& container = cpItem.second;
        const auto& td = container.descriptor();

        if (not wl.empty() && std::find(wl.begin(), wl.end(), td.name()) == wl.end())
        {
            // when whitelist is set, skip all types, that are not on the list.
            continue;
        }

        if (cbd().blacklist.isValid()) {
            auto& bl = *cbd().blacklist;
            if (std::find(bl.begin(), bl.end(), td.name()) != wl.end()) {
                // if blacklist is set and the type was found on the list, skip it.
                continue;
            }
        }

        if (td.internal()) continue; // skip internal types

        LOG_DEBUG_S("sending descriptor for type '" << td.name() << "' to " << cbd.header().sender);
        traversal.traverseDescriptorData(&td, [&](auto td, auto body) {
            DotsTransportHeader thead;
            m_transmitter.prepareHeader(thead, td, td->validProperties(body), false);
            thead.dotsHeader->sentTime = pnxs::SystemNow();
            thead.dotsHeader->sender(this->serverInfo().id());

            // Send to peer or group
            connection->send(thead, *reinterpret_cast<const type::Struct*>(body));
        });
    }

    DotsCacheInfo dotsCacheInfo {
        DotsCacheInfo::endDescriptorRequest_t_i{true}
    };
    connection->sendNs("SYS", dotsCacheInfo);
}

void ConnectionManager::handleClearCache(const DotsClearCache::Cbd& cbd)
{
    auto& whitelist = cbd().typeNames.isValid() ? *cbd().typeNames : dots::Vector<string>();

    for (auto& cpItem : m_dispatcher.pool())
    {
        auto &container = cpItem.second;

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

/**
 * Removes all objects, marked as "cleanup", when the client, that created that
 * object is disconnected.
 * @param connection - Connection of the client, that is disconnected
 */
void ConnectionManager::cleanupObjects(Connection *connection)
{
    for (const auto& container : m_cleanupContainer)
    {
        vector<const type::Struct*> remove;

        // Search for objects which where sent by this killed Connection.
        for (const auto& [instance, cloneInfo] : *container)
        {
            if  (connection->id() == cloneInfo.lastUpdateFrom)
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

void ConnectionManager::publishNs(const string &nameSpace,
                                  const type::StructDescriptor *td,
                                  const type::Struct& instance,
                                  property_set properties,
                                  bool remove, bool processLocal)
{
    DotsTransportHeader header;
    m_transmitter.prepareHeader(header, td, properties, remove);
    header.dotsHeader->serverSentTime(pnxs::SystemNow());
    header.dotsHeader->sender(serverInfo().id());
    if (not nameSpace.empty()) header.nameSpace(nameSpace);

    // TODO: avoid local copy
    Transmission transmission{ type::AnyStruct{ instance } };

    // Send to peer or group
    if (processLocal)
    {
        deliver(header, std::move(transmission));
    }
    else {
        if(header.destinationGroup.isValid())
        {
            Group *grp = m_groupManager.getGroup({header.destinationGroup});
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

DotsStatistics ConnectionManager::sendStatistics() const
{
    return DotsStatistics();
}

DotsCacheStatus ConnectionManager::cacheStatus() const
{
    DotsCacheStatus cs;

    auto& pool = m_dispatcher.pool();

    cs.nrTypes(pool.size());
    cs.size(pool.totalMemoryUsage());
    return cs;
}

Connection::ConnectionId ConnectionManager::getUniqueClientId()
{
    return ++m_lastConnectionId;
}

void ConnectionManager::addClient(Connection* connection)
{
    // Send DotsClient when Client is added to network.
    DotsClient client(DotsClient::id_t_i{ connection->id() });
    client.name(connection->clientName());
    client.connectionState(connection->state());
    client._publish();
}

void
ConnectionManager::publish(const type::StructDescriptor *td, const type::Struct& instance, property_set properties, bool remove)
{
    publishNs("SYS", td, instance, properties, remove, true);
}

string ConnectionManager::clientId2Name(ClientId id) const
{
    const auto& container = m_dispatcher.container<DotsClient>();

    const DotsClient* client = container.find(DotsClient{ 
		DotsClient::id_t_i{ id }
	});

    if (client != nullptr)
    {
        if (client->name.isValid())
            return client->name;
    }

    if (id == 0)
    {
        return m_serverInfo.name();
    }

    return to_string(id);
}

const DistributedTypeId &ConnectionManager::distributedTypeId() const
{
    if (m_distributedTypeId)
    {
        return *m_distributedTypeId.get();
    }
    throw std::runtime_error("distributedTypeId not initialized");
}

}
