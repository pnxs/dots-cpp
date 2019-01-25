#pragma once

#include "Connection.h"
#include <set>
#include <dots/io/Dispatcher.h>
#include "dots/io/AnyContainer.h"
#include "dots/io/Publisher.h"
#include "dots/io/DistributedTypeId.h"
#include "GroupManager.h"
#include "ServerInfo.h"

#include "DotsClearCache.dots.h"
#include "DotsDescriptorRequest.dots.h"
#include "DotsStatistics.dots.h"
#include "DotsCacheStatus.dots.h"

namespace dots
{

/*!
 * Manages connections to DOTS clients.
 */
class ConnectionManager: public Publisher
{
    dots::AnyContainerPool m_containerPool;
public:
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager&operator=(const ConnectionManager&) = delete;

    ConnectionManager(GroupManager &groupManager, ServerInfo &server);

    void init();

    /*!
     * Register and start a connection.
     * @param c shared-ptr to Connection-object
     */
    void start(connection_ptr c);
   // void stop(connection_ptr c);

    /*!
     * Stops all connections.
     */
    void stop_all();

    /*!
     * Find a Connection-object by it's name.
     * @param name of the connection-object.
     * @return shared-ptr to Connection object. Null if none was found.
     */
    connection_ptr findConnection(const Connection::ConnectionId &id);

    /*!
     * Find a Connection-object by it's peer address.
     * @param pa PeerAddress of the connection-object.
     * @return shared-ptr to Connection object. Null if none was found.
     */
    //connection_ptr findConnection(const DotsPeerAddress& pa);

    /*!
     * @return ServerInfo reference
     */
    ServerInfo & serverInfo() const { return m_serverInfo; }

    // Space things:
    /*!
     * Deliver a message to all subscribed connections.
     * @param message
     */
    void deliverMessage(const Message &message);

    /*!
     * Publishes a Object with a namespace
     * @param nameSpace when not empty, publish into this namespace
     * @param td typedescriptor of data
     * @param data typeless data pointer
     * @param properties properties, that should be send
     * @param remove if object should be removed
     */
    void publishNs(const string& nameSpace, const type::StructDescriptor* td, const void* data, property_set properties, bool remove, bool processLocal = true);

    // Need for Publisher-Interface
    void publish(const type::StructDescriptor* td, const void* data, property_set properties, bool remove) override;

    /*!
     * Process DotsMember-message. Do Join or Leave from Groups.
     * @param member DotsMember-Object
     */
    void processMemberMessage(const DotsTransportHeader&, const DotsMember& member, Connection*);

    /*!
     * Stops and remove all connections contained in m_cleanupConnections.
     */
    void cleanup();

    /*!
     * Handle kill()-Method from a Connection-Object. Mark the connection for cleanup.
     */
    void handleKill(Connection* );

    /*!
     * Send DotsClient information about the connected client/connection.
     */
    void addClient(Connection*);

    /*!
     * Creates a new unique clientId
     * @return new clientId
     */
    Connection::ConnectionId getUniqueClientId();

    DotsStatistics receiveStatistics() const;
    DotsStatistics sendStatistics() const;
    DotsCacheStatus cacheStatus() const;

    void onNewType(const type::StructDescriptor*);

    const DistributedTypeId& distributedTypeId() const;


private:
    void onReceivedMessage(const dots::TypelessCbd* cbd, dots::AnyContainer& container);

    void removeConnection(connection_ptr c);

    void handleDescriptorRequest(const DotsDescriptorRequest::Cbd& cbd);
    void handleClearCache(const DotsClearCache::Cbd& cbd);
    void cleanupObjects(Connection *connection);
    bool isClientIdInContainers(ClientId id);
    string clientId2Name(ClientId id) const;


    std::map<Connection::ConnectionId, connection_ptr> m_connections;
    vector<AnyContainer*> m_cleanupContainer; ///< all containers with cleanup-flag.

    std::set<connection_ptr> m_cleanupConnections; ///< old connection-object.

    bool m_CacheEnabled = true;
    GroupManager& m_groupManager;
    ServerInfo& m_serverInfo;
    dots::Dispatcher m_dispatcher;
    dots::Transmitter m_transmitter;
    Connection::ConnectionId m_lastConnectionId = 1; // 0 is used for unitialized, 1 is used for the server.
    std::unique_ptr<DistributedTypeId> m_distributedTypeId;
};

}
