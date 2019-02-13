#include "ConnectionManager.h"
#include "dots/io/TD_Traversal.h"
#include "dots/type/Registry.h"

#include "DotsCacheInfo.dots.h"
#include "DotsClient.dots.h"

namespace dots {


ConnectionManager::ConnectionManager(GroupManager &groupManager, ServerInfo &server)
        :m_groupManager(groupManager), m_serverInfo(server)
{
    dots::type::Descriptor::registry().onNewStruct.connect(FUN(*this, onNewType));
    m_dispatcher.addReceiver<DotsDescriptorRequest>(FUN(*this, handleDescriptorRequest));
    m_dispatcher.addReceiver<DotsClearCache>(FUN(*this, handleClearCache));
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
 * Main receive method for new messages from groups.
 * @param cbd typless callbackdata.
 * @param container the container associated with the calling type.
 */
void ConnectionManager::onReceivedMessage(const dots::TypelessCbd* cbd, dots::AnyContainer &container)
{
    container.process(cbd->header, cbd->data);
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
    dots::AnyContainer* container = m_containerPool.getContainer(td->name());

    LOG_INFO_S("register type " << td->name() << " published by " << clientId2Name(td->publisherId()));

    LOG_DEBUG_S("onNewType name=" << td->name() << " flags:" << flags2String(td) << " container:" << container);

    if(not container) {
        LOG_WARN_S("container is NULL");
        return;
    }

    // Only continue, if type is a cached type
    if(not td->cached())
    {
        return;
    }

    // Cached types can be marked as "cleanup"
    if(td->cleanup())
    {
        m_cleanupContainer.push_back(container);
    }

    m_dispatcher.addTypelessReceiver(td, bind(&ConnectionManager::onReceivedMessage, this, _1, std::ref(*container)));
}

void ConnectionManager::deliverMessage(const Message &msg)
{
    DotsTransportHeader transportHeader(msg.header());
    const DotsHeader& dotsHeader = *transportHeader.dotsHeader;
    bool isFromMySelf = false;

    // Send to a group (fan-out)
    if(transportHeader.destinationGroup.isValid())
    {
        if(dotsHeader.sender.isValid() && *dotsHeader.sender == 1)
        {
            LOG_DEBUG_P("own message \n");
            isFromMySelf = true;
        }

        if(m_CacheEnabled)
        {
            dots::ReceiveMessageData rmd = {
                .data = msg.data().data(),
                .length = msg.data().size(),
                .sender = dotsHeader.sender.isValid() ? *dotsHeader.sender : 0,
                .group = transportHeader.destinationGroup,
                .sentTime = dotsHeader.sentTime,
                .header = dotsHeader,
                .isFromMyself = isFromMySelf
            };

            m_dispatcher.dispatchMessage(rmd);
        }

        Group *grp = m_groupManager.getGroup({ transportHeader.destinationGroup });
        if (grp) grp->deliverMessage(msg);

        return;
    }

    // Send to a specific client (unicast)
    if (transportHeader.destinationClientId.isValid())
    {
        auto dstConnection = findConnection(transportHeader.destinationClientId);
        if (dstConnection)
        {
            dstConnection->send(msg);
        }
    }
}

void ConnectionManager::processMemberMessage(const DotsTransportHeader& header, const DotsMember &member, Connection *connection)
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

        dots::AnyContainer* containerPtr = m_containerPool.getContainer(typeName);
        if (containerPtr == nullptr) return;

        if (containerPtr->td()->cached())
        {
            connection->sendContainerContent(*containerPtr);
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
    for (auto& poolIter : m_containerPool.getPool())
    {
        auto& container = poolIter.second;
        for (auto& element : container)
        {
            if (element.information.createdFrom == id) return true;
            if (element.information.lastUpdateFrom == id) return true;
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

    auto containerPtr = m_containerPool.getContainer("DotsClient");
    std::vector<ClientId> clientsToRemove;

    if (containerPtr) {
        for (auto &element : *containerPtr) {
            auto client = static_cast<DotsClient *>(element.data);
            if (client->connectionState.isValid() && client->connectionState == DotsConnectionState::closed) {
                // Search for a ClientId reference in all containers
                //LOG_DATA_S("check closed conn state of " << client->name());
                if (isClientIdInContainers(client->id)) {
                    continue;
                }
                else {
                    clientsToRemove.push_back(client->id);
                }
            }
        }
    }

    for (auto& id : clientsToRemove) {
		DotsClient client(DotsClient::id_t::init_t{ id });
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

    auto connection = findConnection(cbd.header.sender);

    if (not connection)
    {
        LOG_WARN_S("no connection found");
        return;
    }

    LOG_INFO_S("received DescriptorRequest from " << connection->clientName() << "(" << connection->id() << ")");

    for (const auto& cpItem : m_containerPool.getPool())
    {
        const auto& container = cpItem.second;
        auto td = container.td();

        if (not wl.empty() && std::find(wl.begin(), wl.end(), td->name()) == wl.end())
        {
            // when whitelist is set, skip all types, that are not on the list.
            continue;
        }

        if (cbd().blacklist.isValid()) {
            auto& bl = *cbd().blacklist;
            if (std::find(bl.begin(), bl.end(), td->name()) != wl.end()) {
                // if blacklist is set and the type was found on the list, skip it.
                continue;
            }
        }

        if (td->internal()) continue; // skip internal types

        LOG_DEBUG_S("sending descriptor for type '" << td->name() << "' to " << cbd.header.sender);
        traversal.traverseDescriptorData(td, [&](auto td, auto body) {
            DotsTransportHeader thead;
            thead.dotsHeader();
            m_transmitter.prepareHeader(thead, td, td->validProperties(body), false);
            thead.dotsHeader->sentTime(pnxs::SystemNow());
            thead.dotsHeader->sender(this->serverInfo().id());

            // prepareBuffer
            m_transmitter.prepareBuffer(td, body, thead, td->validProperties(body));

            // Send to peer or group
            connection->send({thead, m_transmitter.buffer()});
        });
    }

    DotsCacheInfo dotsCacheInfo;
    dotsCacheInfo.endDescriptorRequest(true);
    connection->sendNs("SYS", dotsCacheInfo);
}

void ConnectionManager::handleClearCache(const DotsClearCache::Cbd& cbd)
{
    auto& whitelist = cbd().typeNames.isValid() ? *cbd().typeNames : dots::Vector<string>();

    for (auto& cpItem : m_containerPool.getPool())
    {
        auto &container = cpItem.second;

        if (std::find(whitelist.begin(), whitelist.end(), container.td()->name()) == whitelist.end())
        {
            continue; // not found
        }

        // clear container content
        LOG_INFO_S("clear container '" << container.td()->name() << "' (" << container.size() << " elements)");

        // publish remove for every element of the container
        for (auto& element : container)
        {
            publishNs({}, container.td(), element.data, container.td()->keys(), true, false);
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
        vector<const void*> remove;

        // Search for objects which where sent by this killed Connection.
        for (const AnyElement& item : *container)
        {
            if  (connection->id() == item.information.lastUpdateFrom)
            {
                remove.push_back(item.data);
            }
        }

        for (auto item : remove)
        {
            publishNs({}, container->td(), item, container->td()->keys(), true);
        }
    }

}

void ConnectionManager::publishNs(const string &nameSpace,
                                  const type::StructDescriptor *td,
                                  const void *data,
                                  property_set properties,
                                  bool remove, bool processLocal)
{
    DotsTransportHeader header;
    m_transmitter.prepareHeader(header, td, properties, remove);
    header.dotsHeader->serverSentTime(pnxs::SystemNow());
    header.dotsHeader->sender(serverInfo().id());
    if (not nameSpace.empty()) header.nameSpace(nameSpace);

    // prepareBuffer
    m_transmitter.prepareBuffer(td, data, header, properties);

    // Send to peer or group
    if (processLocal)
    {
        deliverMessage({header, m_transmitter.buffer()});
    }
    else {
        if(header.destinationGroup.isValid())
        {
            Group *grp = m_groupManager.getGroup({header.destinationGroup});
            if (grp) grp->deliverMessage({header, m_transmitter.buffer()});
        }
    }
}

DotsStatistics ConnectionManager::receiveStatistics() const
{
    return m_dispatcher.statistics();
}

DotsStatistics ConnectionManager::sendStatistics() const
{
    return DotsStatistics();
}

DotsCacheStatus ConnectionManager::cacheStatus() const
{
    DotsCacheStatus cs;

    auto& pool = m_containerPool.getPool();

    cs.nrTypes(pool.size());

    uint64_t sizeOfContainers = 0;

    for (auto& container : pool)
    {
        auto nrElements = container.second.size();
        sizeOfContainers += sizeof(container.first) + container.first.size(); // Size of pool-key
        sizeOfContainers += sizeof(AnyElement) * nrElements; // Size of Container Metadata
        sizeOfContainers += container.second.td()->sizeOf() * nrElements; // Size of Container payload

    }
    cs.size(sizeOfContainers);
    return cs;
}

Connection::ConnectionId ConnectionManager::getUniqueClientId()
{
    return ++m_lastConnectionId;
}

void ConnectionManager::addClient(Connection* connection)
{
    // Send DotsClient when Client is added to network.
    DotsClient client(DotsClient::id_t::init_t{ connection->id() });
    client.name(connection->clientName());
    client.connectionState(connection->state());
    client._publish();
}

void
ConnectionManager::publish(const type::StructDescriptor *td, const void *data, property_set properties, bool remove)
{
    publishNs("SYS", td, data, properties, remove, true);
}

string ConnectionManager::clientId2Name(ClientId id) const
{
    auto containerPtr = m_containerPool.getConstContainer("DotsClient");
    if (containerPtr)
    {
		DotsClient searchKey(DotsClient::id_t::init_t{ id });

        auto iter = containerPtr->find({&searchKey, pnxs::TimePoint()});
        if (iter != containerPtr->end())
        {
            auto client = static_cast<DotsClient *>(iter->data);
            if (client->name.isValid())
                return client->name();
        }
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
