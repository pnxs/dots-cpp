#pragma once

#include <dots/io/Connection.h>
#include <dots/io/Transmitter.h>
#include <set>
#include <unordered_map>
#include <dots/io/Dispatcher.h>
#include "dots/io/Publisher.h"
#include "dots/io/DistributedTypeId.h"
#include <dots/io/services/Listener.h>
#include <dots/functional/signal.h>

#include "Group.h"

#include "DotsClearCache.dots.h"
#include "DotsDescriptorRequest.dots.h"
#include "DotsStatistics.dots.h"
#include "DotsCacheStatus.dots.h"

namespace dots
{
    /*!
     * Manages connections to DOTS clients.
     */
    class ConnectionManager : public Publisher
    {
    public:
        ConnectionManager(const ConnectionManager&) = delete;
        ConnectionManager& operator=(const ConnectionManager&) = delete;

        ConnectionManager(std::unique_ptr<Listener>&& listener, const std::string& name);

        void init();

        /*!
         * Find a Connection-object by it's name.
         * @param name of the connection-object.
         * @return shared-ptr to Connection object. Null if none was found.
         */
        io::connection_ptr_t findConnection(const io::Connection::id_t& id);

        // Space things:
        /*!
         * Deliver a message to all subscribed connections.
         * @param message
         */
        bool handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission, bool isFromMyself);

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
        void handleClose(io::Connection::id_t id, const std::exception* e);

        DotsStatistics receiveStatistics() const;
        DotsCacheStatus cacheStatus() const;

        void onNewType(const type::StructDescriptor<>*);

    private:

        void asyncAccept();

        void removeConnection(io::connection_ptr_t c);

        void handleDescriptorRequest(const DotsDescriptorRequest::Cbd& cbd);
        void handleClearCache(const DotsClearCache::Cbd& cbd);
        void cleanupObjects(io::Connection* connection);
        bool isClientIdInContainers(ClientId id);
        string clientId2Name(ClientId id) const;

        void sendContainerContent(io::Connection& connection, const Container<>& container);
        void sendCacheEnd(io::Connection& connection, const std::string& typeName);

        /*!
         * Find Group from group-key
         * @param groupKey key of group to search for.
         * @return Group-Pointer. Null of group-key was not found.
         */
        Group* getGroup(const GroupKey& groupKey)
        {
            auto it = m_allGroups.find(groupKey);
            return it != m_allGroups.end() ? it->second : NULL;
        }

        /*!
         * Add Connection to group. Create group if it does not exist jet.
         * @param groupKey
         * @param connection
         */
        void handleJoin(const GroupKey& groupKey, io::Connection* connection);

        /*!
         * Remove a Connection from a group.
         * @param groupKey
         * @param connection
         */
        void handleLeave(const GroupKey& groupKey, io::Connection* connection);

        /*!
         * Removes a killed Connection from all groups.
         * @param connection
         */
        void handleKill(io::Connection* connection);

        std::map<io::Connection::id_t, io::connection_ptr_t> m_connections;
        std::vector<const Container<>*> m_cleanupContainer; ///< all containers with cleanup-flag.

        std::set<io::connection_ptr_t> m_cleanupConnections; ///< old connection-object.

        string m_name;
        std::unique_ptr<Listener> m_listener;
        std::unordered_map<GroupKey, Group*> m_allGroups;
        dots::Dispatcher m_dispatcher;
        dots::Transmitter m_transmitter;
        std::unique_ptr<DistributedTypeId> m_distributedTypeId;
        pnxs::SignalConnection m_onNewStruct;
    };
}
