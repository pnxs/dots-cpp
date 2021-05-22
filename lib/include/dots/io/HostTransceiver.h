#pragma once
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <dots/io/Connection.h>
#include <dots/io/Transceiver.h>
#include <dots/io/Listener.h>
#include <dots/io/auth/AuthManager.h>
#include <DotsClearCache.dots.h>
#include <DotsDescriptorRequest.dots.h>
#include <DotsMember.dots.h>
#include <DotsEcho.dots.h>

namespace dots::io
{
    struct HostTransceiver : Transceiver
    {
        using transition_handler_t = std::function<void(const io::Connection&)>;

        HostTransceiver(std::string selfName = "DotsHostTransceiver", boost::asio::io_context& ioContext = global_io_context(), bool staticUserTypes = true, transition_handler_t transitionHandler = nullptr);
        HostTransceiver(const HostTransceiver& other) = delete;
        HostTransceiver(HostTransceiver&& other) = default;
        virtual ~HostTransceiver() = default;

        HostTransceiver& operator = (const HostTransceiver& rhs) = delete;
        HostTransceiver& operator = (HostTransceiver&& rhs) = default;

        Listener& listen(listener_ptr_t&& listener);

        template <typename TListener, typename... Args>
        TListener& listen(Args&&... args)
        {
            return static_cast<TListener&>(listen(std::make_unique<TListener>(ioContext(), std::forward<Args>(args)...)));
        }

        void publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false) override;

        template <typename T, typename... Args>
        void setAuthManager(Args&&... args)
        {
            static_assert(std::is_base_of_v<AuthManager, T>, "T must be derived from AuthManager");
            m_authManager = std::make_unique<T>(*this, std::forward<Args>(args)...);
        }

    private:

        using listener_map_t = std::unordered_map<Listener*, listener_ptr_t>;
        using connection_map_t = std::unordered_map<io::Connection*, io::connection_ptr_t>;
        using group_t = std::unordered_set<io::Connection*>;
        using group_map_t = std::unordered_map<std::string, group_t>;

        void joinGroup(const std::string_view& name) override;
        void leaveGroup(const std::string_view& name) override;

        void transmit(io::Connection* origin, const Transmission& transmission);

        bool handleListenAccept(Listener& listener, channel_ptr_t channel);
        void handleListenError(Listener& listener, const std::exception_ptr& e);

        void handleTransmission(io::Connection& connection, Transmission transmission);
        void handleTransition(io::Connection& connection, const std::exception_ptr& e) noexcept;

        void handleMemberMessage(io::Connection& connection, const DotsMember& member);
        void handleDescriptorRequest(io::Connection& connection, const DotsDescriptorRequest& descriptorRequest);
        void handleClearCache(io::Connection& connection, const DotsClearCache& clearCache);
        void handleEchoRequest(io::Connection& connection, const DotsEcho& echoRequest);

        void transmitContainer(io::Connection& connection, const Container<>& container);

        transition_handler_t m_transitionHandler;
        listener_map_t m_listeners;
        connection_map_t m_guestConnections;
        group_map_t m_groups;
        std::unique_ptr<AuthManager> m_authManager;
    };
}
