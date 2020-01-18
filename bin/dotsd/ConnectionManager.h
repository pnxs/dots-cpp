#pragma once

#include <unordered_map>
#include <unordered_set>
#include <dots/io/Registry.h>
#include <dots/io/Connection.h>
#include <dots/io/Dispatcher.h>
#include "dots/io/Publisher.h"
#include <dots/io/services/Listener.h>

#include "DotsClearCache.dots.h"
#include "DotsDescriptorRequest.dots.h"
#include "DotsMember.dots.h"

namespace dots
{
    class ConnectionManager : public Publisher
    {
    public:
        ConnectionManager(std::string selfName);
        ConnectionManager(const ConnectionManager&) = delete;
        ConnectionManager& operator=(const ConnectionManager&) = delete;

        void listen(listener_ptr_t&& listener);

        const ContainerPool& pool() const;

        void publish(const type::Struct& instance, types::property_set_t includedProperties = types::property_set_t::All, bool remove = false);
        void remove(const type::Struct& instance);

        [[deprecated("only available for backwards compatibility")]]
        void publish(const type::StructDescriptor<>* td, const type::Struct& instance, type::PropertySet properties, bool remove) override;

    private:

        using connection_map_t = std::unordered_map<io::Connection*, io::connection_ptr_t>;
        using group_t = std::unordered_set<io::Connection*>;
        using group_map_t = std::unordered_map<std::string, group_t>;

        bool handleReceive(io::Connection& connection, const DotsTransportHeader& transportHeader, Transmission&& transmission, bool isFromMyself);
        void handleTransition(io::Connection& connection, const std::exception* e);

        void handleMemberMessage(io::Connection& connection, const DotsMember& member);
        void handleDescriptorRequest(io::Connection& connection, const DotsDescriptorRequest& descriptorRequest);
        void handleClearCache(io::Connection& connection, const DotsClearCache& clearCache);

        void handleNewStructType(const type::StructDescriptor<>& descriptor);

        void cleanUpClients();
        void transmitContainer(io::Connection& connection, const Container<>& container);

        static std::string flags2String(const dots::type::StructDescriptor<>* td);

        inline static uint32_t M_nextTypeId = 0;

        io::Registry m_registry;
        std::string m_selfName;
        connection_map_t m_openConnections;
        connection_map_t m_closedConnections;
        group_map_t m_groups;
        std::vector<const Container<>*> m_cleanupContainers;
        listener_ptr_t m_listener;
        Dispatcher m_dispatcher;
    };
}