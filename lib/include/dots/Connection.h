#pragma once
#include <string_view>
#include <tuple>
#include <optional>
#include <dots/io/Channel.h>
#include <dots/io/auth/AuthManager.h>
#include <dots/serialization/StringSerializer.h>
#include <DotsConnectionState.dots.h>
#include <DotsHeader.dots.h>
#include <DotsMsgHello.dots.h>
#include <DotsMsgConnectResponse.dots.h>
#include <DotsMsgConnect.dots.h>
#include <DotsMsgError.dots.h>

namespace dots::type
{
    struct Registry;
}

namespace dots
{
    /*!
     * @class Connection Connection.h <dots/Connection.h>
     *
     * @brief Transmit and receive DOTS transmissions via a specific
     * channel.
     *
     * This class implements the logical DOTS protocol to handle
     * communication between a DOTS host and guest. This includes the
     * initial handshake (possibly with authentication), as well as the
     * handling of connection errors.
     *
     * Note that this class is only responsible for the logical part of the
     * communication and is agnostic to the technical details of how
     * transmissions are transported, which is implemented by the given
     * io::Channel.
     *
     * @attention Outside of advanced use cases, a regular user is never
     * required to create or manage Connection objects themselves.
     */
    struct Connection
    {
        using id_t = uint32_t;
        static constexpr id_t UninitializedId = 0;
        static constexpr id_t HostId = 1;
        static constexpr id_t FirstGuestId = 2;

        using receive_handler_t = std::function<bool(Connection&, io::Transmission)>;
        using transition_handler_t = std::function<void(Connection&, std::exception_ptr)>;

        /*!
         * @brief Construct a new Connection object.
         *
         * This will initialize the connection to a suspended (i.e inactive)
         * state. No processing of incoming transmissions will take place until
         * Connection::asyncReceive() is called.
         *
         * Note that even though a connection might have already been
         * established by the channel on a technical level (e.g. via a TCP
         * connection), using Connection::transmit() is invalid before the DOTS
         * connection has been established logically.
         *
         * @param channel The channel to use for communication.
         *
         * @param host Specifies whether the connection should act as a host or
         * guest.
         *
         * @param authSecret The secret to use for authentication as a guest.
         * If no secret is given, the connection will close with an error if
         * authentication is requested. Note that this parameter will have no
         * effect if @p host is given as true.
         */
        Connection(io::channel_ptr_t channel, bool host, std::optional<std::string> authSecret = std::nullopt);
        Connection(const Connection& other) = delete;
        Connection(Connection&& other) = default;
        ~Connection() noexcept;

        Connection& operator = (const Connection& rhs) = delete;
        Connection& operator = (Connection&& rhs) = default;

        /*!
         * @brief Get the local endpoint of the channel used by this
         * connection.
         *
         * @return const io::Endpoint& A reference to the local endpoint of the
         * channel used by this connection.
         */
        const io::Endpoint& localEndpoint() const;

        /*!
         * @brief Get the remote endpoint of the channel used by this
         * connection.
         *
         * @return const io::Endpoint& A reference to the remote endpoint of
         * the channel used by this connection.
         */
        const io::Endpoint& remoteEndpoint() const;

        /*!
         * @brief Get the current state of the connection.
         *
         * @return DotsConnectionState The current state of the connection.
         */
        DotsConnectionState state() const;

        /*!
         * @brief Get the local id of the connection.
         *
         * This is the unique id of this connection in the current DOTS space.
         * It is used in the header when transmitting instances and can be used
         * to identify which transmissions originated from this location.
         *
         * If the connection was initialized to act as a host, the local id
         * will always be Connection::HostId.
         *
         * If the connection was initialized to act as a guest, the local id
         * will initially be Connection::UninitializedId until the handshake
         * was performed and an id was assigned by the remote host.
         *
         * @return id_t The local id of the connection.
         */
        id_t selfId() const;

        /*!
         * @brief Get the id of the remote peer.
         *
         * This is the unique id of the remote peer in the current DOTS space.
         *
         * If the connection was initialized to act as a host, this will return
         * the unique id that was or will be assigned to the remote peer
         * (depending on whether the handshake was completed or not).
         *
         * If the connection was initialized to act as a guest, the remote id
         * will always be Connection::HostId.
         *
         * @return id_t The id of the remote peer.
         */
        id_t peerId() const;

        /*!
         * @brief Get the name of the remote peer.
         *
         * This is the self-assigned name of the remote peer.
         *
         * If the handshake has not been completed, the connection might not
         * yet have received a peer name, in which case this function will
         * return "<not_set>".
         *
         * @return const std::string& A reference to the name of the remote
         * peer.
         */
        const std::string& peerName() const;

        /*!
         * @brief Indicates whether the Connection is in the 'connected' state.
         *
         * This is equivalent to comparing Connection::state() against
         * DotsConnectionState::connected.
         *
         * @return true If the connection is connected (i.e.
         * Connection::state() == DotsConnectionState::connected is true).
         * @return false Else.
         */
        bool connected() const;

        /*!
         * @brief Indicates whether the Connection is in the 'closed' state.
         *
         * This is equivalent to comparing Connection::state() against
         * DotsConnectionState::closed.
         *
         * @return true If the connection is closed (i.e. Connection::state()
         * == DotsConnectionState::closed is true).
         * @return false Else.
         */
        bool closed() const;

        /*!
         * @brief Get a description of the remote peer of this connection.
         *
         * This will construct a textual representation of the current peer
         * information with the following format: "<host-or-guest>
         * '<peer-name>' [<peer-id>]".
         *
         * For example: "guest 'foo' [42]" or "host 'bar' [1]".
         *
         * @return std::string A string representation of the remote peer.
         */
        std::string peerDescription() const;

        /*!
         * @brief Get a description of the endpoints of this connection.
         *
         * This will construct a textual representation of both the local end
         * remote endpoint of this connection in the following format: "from
         * '<local-endpoint-uri>' at '<remote-endpoint-uri>'".
         *
         * For example: "from 'tcp://127.0.0.1:60062' at
         * 'tcp://127.0.0.1:11234'"
         *
         * @return std::string A string representation the endpoints of this
         * connection.
         */
        std::string endpointDescription() const;

        /*!
         * @brief Start to asynchronously receive transmissions via the
         * underlying channel.
         *
         * This will effectively call Channel::asyncReceive and initialize the
         * handshake procedure.
         *
         * As a result, the given receive handler will be invoked
         * asynchronously every time a non-system transmission is received.
         *
         * @param registry The registry to be used by the underlying channel.
         *
         * @param authManager The authentication manager to use. Note that this
         * parameter is ignored when the Connection was initialized to act as a
         * guest.
         *
         * @param name The self-assigned name to use for identification with
         * the peer (see Connection::selfName()).
         *
         * @param receiveHandler The handler to invoke asynchronously when a
         * transmission is received.
         *
         * @param transitionHandler The handler to invoke when the Connection
         * transitions to another state (e.g. when starting to connect).
         *
         * @exception std::logic_error Thrown if another "async receive" is
         * already active on the connection.
         *
         * @exception std::logic_error Thrown if either @p receiveHandler or @p
         * transitionHandler are unset.
         */
        void asyncReceive(type::Registry& registry, io::AuthManager* authManager, std::string_view name, receive_handler_t receiveHandler, transition_handler_t transitionHandler);

        /*!
         * @brief Transmit a specific instance.
         *
         * This will automatically construct a default DotsHeader and transmit
         * the given instance via the underlying channel.
         *
         * Note that before the connection is established (i.e. the handshake
         * has been completed), using any of the Connection::transmit()
         * function is invalid and will indirectly result in closing the
         * connection.
         *
         * @param instance The instance to transmit.
         *
         * @param includedProperties The property set to include in the
         * transmit. If no set is given, the valid property set of
         * @p instance will be used.
         *
         * @param remove Specifies whether the transmit is a remove.
         */
        void transmit(const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false);

        /*!
         * @brief Transmit a specific header and instance.
         *
         * This constructs a transmission from the given header and instance
         * and transmits it via the underlying channel.
         *
         * Note that before the connection is established (i.e. the handshake
         * has been completed), using any of the Connection::transmit()
         * function is invalid and will indirectly result in closing the
         * connection.
         *
         * @param header The header to transmit.
         *
         * @param instance The instance to transmit.
         */
        void transmit(const DotsHeader& header, const type::Struct& instance);

        /*!
         * @brief Transmit a specific transmission.
         *
         * This will transmit the given transmission via the underlying
         * channel.
         *
         * Note that before the connection is established (i.e. the handshake
         * has been completed), using any of the Connection::transmit()
         * function is invalid and will indirectly result in closing the
         * connection.
         *
         * @param transmission The transmission to transmit.
         */
        void transmit(const io::Transmission& transmission);

        /*!
         * @brief Transmit a specific type.
         *
         * This will transmit the given type descriptor via the underlying
         * channel.
         *
         * Note that before the connection is established (i.e. the handshake
         * has been completed), using any of the Connection::transmit()
         * function is invalid and will indirectly result in closing the
         * connection.
         *
         * Also note that this might result in none or multiple transmissions,
         * depending on whether the given type and its dependencies are already
         * known to the peer.
         *
         * @param descriptor The descriptor of the type to transmit.
         */
        void transmit(const type::StructDescriptor<>& descriptor);

        /*!
         * @brief Handle a specific error.
         *
         * This will close the connection and invoke the transition handler
         * with the given exception accordingly.
         *
         * If the connection is currently connected, this function will also
         * attempt to relay the error to the peer by transmitting a
         * corresponding DotsMsgError instance before the connection is closed.
         *
         * @param ePtr The error to handle.
         */
        void handleError(std::exception_ptr ePtr);

    private:

        using system_type_t = std::tuple<const type::StructDescriptor<>*, types::property_set_t, std::function<void(const type::Struct&)>>;

        static constexpr serialization::StringSerializerOptions StringOptions = { serialization::StringSerializerOptions::MultiLine };

        bool handleReceive(io::Transmission transmission);
        void handleClose(std::exception_ptr ePtr);

        void handleHello(const DotsMsgHello& hello);
        void handleAuthorizationRequest(const DotsMsgConnectResponse& connectResponse);
        void handlePreloadFinished(const DotsMsgConnectResponse& connectResponse);

        void handleConnect(const DotsMsgConnect& connect);
        void handlePreloadClientFinished(const DotsMsgConnect& connect);

        void handlePeerError(const DotsMsgError& error);

        void setConnectionState(DotsConnectionState state, std::exception_ptr e = nullptr);

        template <typename T>
        void expectSystemType(types::property_set_t expectedAttributes, void(Connection::* handler)(const T&));

        inline static id_t M_nextGuestId = FirstGuestId;

        system_type_t m_expectedSystemType;
        DotsConnectionState m_connectionState;
        id_t m_selfId;
        id_t m_peerId;
        std::string m_selfName;
        std::string m_peerName;

        io::channel_ptr_t m_channel;
        std::optional<std::string> m_authSecret;
        std::optional<io::Nonce> m_nonce;

        type::Registry* m_registry;
        io::AuthManager* m_authManager;
        receive_handler_t m_receiveHandler;
        transition_handler_t m_transitionHandler;
    };

    using connection_ptr_t = std::shared_ptr<Connection>;
}