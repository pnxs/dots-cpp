#pragma once
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <dots/io/Registry.h>
#include <dots/io/Connection.h>
#include <dots/io/Dispatcher.h>
#include "dots/io/Publisher.h"
#include <dots/io/services/Listener.h>
#include <DotsClearCache.dots.h>
#include <DotsDescriptorRequest.dots.h>
#include <DotsMember.dots.h>

namespace dots
{
    struct HostTransceiver : Publisher
    {
        using new_struct_type_handler_t = io::Registry::new_struct_type_handler_t;
        using transition_handler_t = std::function<void(const io::Connection&)>;

        HostTransceiver(std::string selfName, new_struct_type_handler_t newStructTypeHandler, transition_handler_t transitionHandler);
		HostTransceiver(const HostTransceiver& other) = delete;
		HostTransceiver(HostTransceiver&& other) = default;
		virtual ~HostTransceiver() = default;

		HostTransceiver& operator = (const HostTransceiver& rhs) = delete;
		HostTransceiver& operator = (HostTransceiver&& rhs) = default;

        void listen(listener_ptr_t&& listener);

        const std::string& selfName() const;
        const ContainerPool& pool() const;

        void publish(const type::Struct& instance, types::property_set_t includedProperties = types::property_set_t::All, bool remove = false);
        void remove(const type::Struct& instance);

        [[deprecated("only available for backwards compatibility")]]
        void publish(const type::StructDescriptor<>* td, const type::Struct& instance, type::PropertySet properties, bool remove) override;

    private:

        using listener_map_t = std::unordered_map<Listener*, listener_ptr_t>;
        using connection_map_t = std::unordered_map<io::Connection*, io::connection_ptr_t>;
        using group_t = std::unordered_set<io::Connection*>;
        using group_map_t = std::unordered_map<std::string, group_t>;

        bool handleListenAccept(Listener& listener, channel_ptr_t channel);
        void handleListenError(Listener& listener, const std::exception& e);

        bool handleReceive(io::Connection& connection, const DotsTransportHeader& transportHeader, Transmission&& transmission, bool isFromMyself);
        void handleTransition(io::Connection& connection, const std::exception* e);

        void handleMemberMessage(io::Connection& connection, const DotsMember& member);
        void handleDescriptorRequest(io::Connection& connection, const DotsDescriptorRequest& descriptorRequest);
        void handleClearCache(io::Connection& connection, const DotsClearCache& clearCache);

        void transmitContainer(io::Connection& connection, const Container<>& container);

        Dispatcher m_dispatcher;
        io::Registry m_registry;
        std::string m_selfName;
        transition_handler_t m_transitionHandler;
        listener_map_t m_listeners;
        connection_map_t m_openConnections;
        group_map_t m_groups;
    };
}