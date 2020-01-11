#pragma once

#include "Connection.h"
#include <set>
#include <dots/io/Dispatcher.h>
#include "dots/io/Publisher.h"
#include "dots/io/DistributedTypeId.h"
#include "GroupManager.h"
#include <dots/io/services/Listener.h>
#include <dots/functional/signal.h>

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
public:
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager&operator=(const ConnectionManager&) = delete;

    ConnectionManager(std::unique_ptr<Listener>&& listener, const std::string& name);

    void init();

    /*!
     * Register and start a connection.
     * @param c shared-ptr to Connection-object
     */
    void start(connection_ptr c);

    /*!
     * Stops all connections.
     */
    void stop_all();

    bool running() const;

    /*!
     * Find a Connection-object by it's name.
     * @param name of the connection-object.
     * @return shared-ptr to Connection object. Null if none was found.
     */
    connection_ptr findConnection(const Connection::id_t &id);

    // Space things:
    /*!
     * Deliver a message to all subscribed connections.
     * @param message
     */
    bool handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission);

    /*!
     * Publishes a Object with a namespace
     * @param nameSpace when not empty, publish into this namespace
     * @param td typedescriptor of data
     * @param data typeless data pointer
     * @param properties properties, that should be send
     * @param remove if object should be removed
     */
    void publishNs(const string& nameSpace, const type::StructDescriptor<>* td, const type::Struct& instance, type::PropertySet properties, bool remove, bool processLocal = true);

    // Need for Publisher-Interface
    void publish(const type::StructDescriptor<>* td, const type::Struct& instance, type::PropertySet properties, bool remove) override;

    /*!
     * Process DotsMember-message. Do Join or Leave from Groups.
     * @param member DotsMember-Object
     */
    void handleMemberMessage(const DotsMember::Cbd& cbd);

    /*!
     * Stops and remove all connections contained in m_cleanupConnections.
     */
    void cleanup();

    /*!
     * Handle kill()-Method from a Connection-Object. Mark the connection for cleanup.
     */
    void handleError(Connection::id_t id, const std::exception& e);

    DotsStatistics receiveStatistics() const;
    DotsStatistics sendStatistics() const;
    DotsCacheStatus cacheStatus() const;

    void onNewType(const type::StructDescriptor<>*);

    const DistributedTypeId& distributedTypeId() const;


private:

    void asyncAccept();

    void removeConnection(connection_ptr c);

    void handleDescriptorRequest(const DotsDescriptorRequest::Cbd& cbd);
    void handleClearCache(const DotsClearCache::Cbd& cbd);
    void cleanupObjects(Connection *connection);
    bool isClientIdInContainers(ClientId id);
    string clientId2Name(ClientId id) const;

    void sendContainerContent(Connection& connection, const Container<>& container);
    void sendCacheEnd(Connection& connection, const std::string& typeName);

    std::map<Connection::id_t, connection_ptr> m_connections;
    std::vector<const Container<>*> m_cleanupContainer; ///< all containers with cleanup-flag.

    std::set<connection_ptr> m_cleanupConnections; ///< old connection-object.

    bool m_running;
    string m_name;
    std::unique_ptr<Listener> m_listener;
    GroupManager m_groupManager;
    dots::Dispatcher m_dispatcher;
    dots::Transmitter m_transmitter;
    std::unique_ptr<DistributedTypeId> m_distributedTypeId;
    pnxs::SignalConnection m_onNewStruct;
};

}
