#pragma once
#include <string_view>
#include <optional>
#include <set>
#include <dots/type/DescriptorMap.h>
#include <dots/Transceiver.h>
#include <dots/Connection.h>

namespace dots
{
    struct GuestTransceiver : Transceiver
    {
        GuestTransceiver(std::string selfName, boost::asio::io_context& ioContext = io::global_io_context(), bool staticUserTypes = true);
        GuestTransceiver(const GuestTransceiver& other) = delete;
        GuestTransceiver(GuestTransceiver&& other) = default;
        ~GuestTransceiver() override = default;

        GuestTransceiver& operator = (const GuestTransceiver& rhs) = delete;
        GuestTransceiver& operator = (GuestTransceiver&& rhs) = default;

        const Connection& open(type::DescriptorMap preloadPublishTypes, type::DescriptorMap preloadSubscribeTypes, std::optional<std::string> authSecret, io::channel_ptr_t channel);
        const Connection& open(io::channel_ptr_t channel);

        template <typename TChannel, typename... Args>
        const Connection& open(type::DescriptorMap preloadPublishTypes, type::DescriptorMap preloadSubscribeTypes, std::optional<std::string> authSecret, Args&&... args)
        {
            return open(std::move(preloadPublishTypes), std::move(preloadSubscribeTypes), std::move(authSecret), io::make_channel<TChannel>(ioContext(), std::forward<Args>(args)...));
        }

        template <typename TChannel, typename... Args>
        const Connection& open(Args&&... args)
        {
            return open(io::make_channel<TChannel>(ioContext(), std::forward<Args>(args)...));
        }

        void publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false) override;

    private:

        void joinGroup(std::string_view name) override;
        void leaveGroup(std::string_view name) override;

        bool handleTransmission(Connection& connection, io::Transmission transmission);
        void handleTransition(Connection& connection, std::exception_ptr e) noexcept;

        std::optional<Connection> m_hostConnection;
        type::DescriptorMap m_preloadPublishTypes;
        type::DescriptorMap m_preloadSubscribeTypes;
        std::set<std::string> m_joinedGroups;
    };
}