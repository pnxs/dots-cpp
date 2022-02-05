#pragma once
#include <unordered_map>
#include <unordered_set>
#include <dots/tools/Handler.h>
#include <dots/Connection.h>
#include <dots/Transceiver.h>
#include <dots/io/Listener.h>
#include <dots/io/auth/AuthManager.h>
#include <DotsClearCache.dots.h>
#include <DotsDescriptorRequest.dots.h>
#include <DotsMember.dots.h>
#include <DotsEcho.dots.h>

namespace dots
{
    /*!
     * @class HostTransceiver HostTransceiver.h <dots/HostTransceiver.h>
     *
     * @brief Transceiver for hosting DOTS spaces.
     *
     * The HostTransceiver class implements the "DOTS host" concept. Hosts
     * create "DOTS spaces" that can be attended by "DOTS guests".
     *
     * A space is an environment in which attendees (including the host)
     * can subscribe to DOTS struct types and publish instances of types
     * into.
     *
     * The host is responsible for distributing the transmissions of
     * published DOTS instances to all attendees that are subscribed to the
     * corresponding types.
     *
     * Even though a HostTransceiver often technically acts as a server, it
     * is agnostic about how a connection is established. A HostTransceiver
     * can asynchronously accept incoming connections from provided
     * io::Listener instances.
     */
    struct HostTransceiver : Transceiver
    {
        using transition_handler_t = tools::Handler<void(const Connection&)>;

        /*!
         * @brief Construct a new HostTransceiver object.
         *
         * After construction, the transceiver will be inactive until listeners
         * are added via HostTransceiver::listen().
         *
         * @param selfName The name the transceiver will use to identify
         * itself.
         *
         * @param ioContext The ASIO IO context (i.e. the "event loop") to use.
         *
         * @param staticTypePolicy Specifies the static type policy of the
         * transceiver's registry.
         *
         * @param transitionHandler The handler to invoke every time the a
         * Connection transitions to a different connection state.
         */
        HostTransceiver(std::string selfName = "DotsHostTransceiver",
                        asio::io_context& ioContext = io::global_io_context(),
                        type::Registry::StaticTypePolicy staticTypePolicy = type::Registry::StaticTypePolicy::All,
                        std::optional<transition_handler_t> transitionHandler = std::nullopt);
        HostTransceiver(const HostTransceiver& other) = delete;
        HostTransceiver(HostTransceiver&& other) = default;
        ~HostTransceiver() override = default;

        HostTransceiver& operator = (const HostTransceiver& rhs) = delete;
        HostTransceiver& operator = (HostTransceiver&& rhs) = default;

        /*!
         * @brief Asynchronously accept incoming connections on a specific
         * listener.
         *
         * @param listener The listener to asynchronously accept connections
         * from.
         *
         * @return io::Listener& A reference to the listener after asynchronous
         * accepting has been started.
         */
        io::Listener& listen(io::listener_ptr_t&& listener);

        /*!
         * @brief Construct a specific listener and asynchronously accept
         * incoming connections.
         *
         * @tparam TListener The type of the listener to construct.
         *
         * @tparam Args The types of the arguments to forward to the compatible
         * constructor of @p TListener .
         *
         * @param args The arguments to forward to the compatible constructor
         * of @p TListener .
         *
         * @return TListener& A reference to the listener after asynchronous
         * accepting has been started.
         */
        template <typename TListener, typename... Args>
        TListener& listen(Args&&... args)
        {
            return static_cast<TListener&>(listen(std::make_unique<TListener>(ioContext(), std::forward<Args>(args)...)));
        }

        /*!
         * @brief Publish an instance of a DOTS struct type.
         *
         * This will create a corresponding io::Transmission for the publish
         * and dispatch it to all subscribers. For local subscribers, this is
         * done synchronously before the function returns, while for remote
         * subscribers the instance will be transmitted asynchronously.
         *
         * If the type of instance is "cached", the cache (i.e. the Container)
         * for that type is updated before any event subscriptions are
         * processed.
         *
         * @param instance The instance to publish.
         *
         * @param includedProperties The properties to publish in addition to
         * the key properties. If no set is given, the valid property set of
         * @p instance will be used.
         *
         * @param remove Specifies whether the publish is a remove.
         *
         * @exception std::logic_error Thrown if @p instance is of a
         * 'substruct-only' type.
         *
         * @exception std::runtime_error Thrown if a key property of the
         * instance is invalid.
         */
        void publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false) override;

        /*!
         * @brief Set the io::AuthManager instance to use for accepted
         * connections.
         *
         * This sets the type of authentication that is used for every
         * Connection that is accepted after this function returns.
         *
         * Note that it has no effect on connections that are already
         * established when the function is called.
         *
         * @tparam T The authentication manager type. Must be derived from
         * io::AuthManager.
         *
         * @tparam Args The types of the arguments to forward to the compatible
         * constructor of @p T .
         *
         * @param args The arguments to forward to the compatible constructor
         * of @p T .
         */
        template <typename T, typename... Args>
        void setAuthManager(Args&&... args)
        {
            static_assert(std::is_base_of_v<io::AuthManager, T>, "T must be derived from AuthManager");
            m_authManager = std::make_unique<T>(*this, std::forward<Args>(args)...);
        }

    private:

        using listener_map_t = std::unordered_map<io::Listener*, io::listener_ptr_t>;
        using connection_map_t = std::unordered_map<Connection*, connection_ptr_t>;
        using group_t = std::unordered_set<Connection*>;
        using group_map_t = std::unordered_map<std::string, group_t>;

        void joinGroup(std::string_view name) override;
        void leaveGroup(std::string_view name) override;

        void transmit(const io::Transmission& transmission);

        bool handleListenAccept(io::Listener& listener, io::channel_ptr_t channel);
        void handleListenError(io::Listener& listener, std::exception_ptr ePtr);

        bool handleTransmission(Connection& connection, io::Transmission transmission);
        void handleTransition(Connection& connection, std::exception_ptr ePtr) noexcept;

        void handleMemberMessage(Connection& connection, const DotsMember& member);
        void handleDescriptorRequest(Connection& connection, const DotsDescriptorRequest& descriptorRequest);
        void handleClearCache(Connection& connection, const DotsClearCache& clearCache);
        void handleEchoRequest(Connection& connection, const DotsEcho& echoRequest);

        void transmitContainer(Connection& connection, const Container<>& container);

        std::optional<transition_handler_t> m_transitionHandler;
        listener_map_t m_listeners;
        connection_map_t m_guestConnections;
        group_map_t m_groups;
        std::unique_ptr<io::AuthManager> m_authManager;
    };
}
