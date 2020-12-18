#pragma once
#include <string_view>
#include <optional>
#include <set>
#include <dots/io/Transceiver.h>
#include <dots/io/Connection.h>

namespace dots::io
{
    struct GuestTransceiver : Transceiver
    {
        GuestTransceiver(std::string selfName);
        GuestTransceiver(const GuestTransceiver& other) = delete;
        GuestTransceiver(GuestTransceiver&& other) = default;
        virtual ~GuestTransceiver() = default;

        GuestTransceiver& operator = (const GuestTransceiver& rhs) = delete;
        GuestTransceiver& operator = (GuestTransceiver&& rhs) = default;

        const io::Connection& open(channel_ptr_t channel, descriptor_map_t preloadPublishTypes = {}, descriptor_map_t preloadSubscribeTypes = {}, std::optional<std::string> authSecret = std::nullopt);
        void publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false) override;

    private:

        void joinGroup(const std::string_view& name) override;
        void leaveGroup(const std::string_view& name) override;

        bool handleTransmission(io::Connection& connection, Transmission transmission);
        void handleTransition(io::Connection& connection, const std::exception_ptr& e) noexcept;

        std::optional<io::Connection> m_hostConnection;
        descriptor_map_t m_preloadPublishTypes;
        descriptor_map_t m_preloadSubscribeTypes;
        std::set<std::string> m_joinedGroups;
    };
}