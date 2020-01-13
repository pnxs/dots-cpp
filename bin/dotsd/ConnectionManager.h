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
    class ConnectionManager : public Publisher
    {
    public:
        ConnectionManager(std::unique_ptr<Listener>&& listener, const std::string& name);
        ConnectionManager(const ConnectionManager&) = delete;
        ConnectionManager& operator=(const ConnectionManager&) = delete;

        void init();

        const ContainerPool& pool() const;

        void publishNs(const string& nameSpace, const type::StructDescriptor<>* td, const type::Struct& instance, type::PropertySet properties, bool remove, bool processLocal = true);
        void publish(const type::StructDescriptor<>* td, const type::Struct& instance, type::PropertySet properties, bool remove) override;
        
        void cleanup();
        void onNewType(const type::StructDescriptor<>*);

    private:

        void asyncAccept();

        bool handleReceive(io::Connection& connection, const DotsTransportHeader& transportHeader, Transmission&& transmission, bool isFromMyself);
        void handleClose(io::Connection& connection, const std::exception* e);

        io::connection_ptr_t findConnection(const io::Connection::id_t& id);

        void handleMemberMessage(const DotsMember::Cbd& cbd);
        void handleDescriptorRequest(const DotsDescriptorRequest::Cbd& cbd);
        void handleClearCache(const DotsClearCache::Cbd& cbd);

        Group* getGroup(const GroupKey& groupKey)
        {
            auto it = m_allGroups.find(groupKey);
            return it != m_allGroups.end() ? it->second : NULL;
        }

        void sendContainerContent(io::Connection& connection, const Container<>& container);
        void sendCacheEnd(io::Connection& connection, const std::string& typeName);

        void cleanupObjects(io::Connection* connection);

        bool isClientIdInContainers(ClientId id);
        string clientId2Name(ClientId id) const;

        static std::string flags2String(const dots::type::StructDescriptor<>* td);

        std::map<io::Connection::id_t, io::connection_ptr_t> m_connections;
        std::vector<const Container<>*> m_cleanupContainer;

        std::set<io::connection_ptr_t> m_cleanupConnections;

        string m_name;
        std::unique_ptr<Listener> m_listener;
        std::unordered_map<GroupKey, Group*> m_allGroups;
        dots::Dispatcher m_dispatcher;
        dots::Transmitter m_transmitter;
        std::unique_ptr<DistributedTypeId> m_distributedTypeId;
        pnxs::SignalConnection m_onNewStruct;
    };
}
