#pragma once

#include "dots/cpp_config.h"
#include <dots/io/services/Channel.h>

#include <memory>
#include <dots/io/Container.h>
#include "dots/io/Transmitter.h"
#include "DotsConnectionState.dots.h"
#include "DotsMsgConnect.dots.h"
#include "DotsMember.dots.h"

namespace dots::io
{
	struct Registry;
}

namespace dots
{
    class ConnectionManager;

    /*!
     * Represents a connection to a DOTS client.
     */
    class Connection
    {
    public:
        typedef uint32_t ConnectionId;
        static constexpr ConnectionId ServerId = 1;

        using receive_handler_t = std::function<bool(const DotsTransportHeader&, Transmission&&)>;
		using error_handler_t = std::function<void(const std::exception&)>;

        /*!
         * Create a Connection from a Channel.
         * @param channel Channel, that is moved into this Connection.
         * @param manager
         */
        explicit Connection(channel_ptr_t channel, std::string serverName, ConnectionManager& manager);
        ~Connection();
        Connection(const Connection&) = delete;
        Connection& operator = (const Connection&) = delete;

        DotsConnectionState state() const;
        const ConnectionId& id() const; ///< return client-id
        const string& clientName() const; ///< return client-supplied name

        void asyncReceive(io::Registry& registry, receive_handler_t&& receiveHandler, error_handler_t&& errorHandler);
        void stop();
        void kill();

        /*!
         * Directly send a Message to the client.
         * @param msg Message-object
         */
        void transmit(const type::Struct& instance, types::property_set_t includedProperties = types::property_set_t::All, bool remove = false);
        void transmit(const DotsTransportHeader& header, const type::Struct& instance);
        void transmit(const DotsTransportHeader& header, const Transmission& transmission);

        void sendContainerContent(const Container<>& container);
        void sendCacheEnd(const std::string& typeName);

    private:

        enum class RxTx { rx, tx };

        bool handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        bool handleControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        bool handleRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);

        void processConnectRequest(const DotsMsgConnect& msg);
        void processConnectPreloadClientFinished(const DotsMsgConnect& msg);
        void processMemberMessage(const DotsTransportHeader& header, const DotsMember& member, Connection* connection);

        void handleError(const std::exception& e);

        void logRxTx(RxTx, const DotsTransportHeader& header);
        void setConnectionState(const DotsConnectionState& state);

        inline static ConnectionId m_lastConnectionId = ServerId; // 0 is used for unitialized, 1 is used for the server.

        dots::Transmitter m_transmitter;
        channel_ptr_t m_channel;
        io::Registry* m_registry;
        receive_handler_t m_receiveHandler;
		error_handler_t m_errorHandler;
        string m_serverName;
        ConnectionManager& m_connectionManager;
        DotsConnectionState m_connectionState = DotsConnectionState::closed;
        ConnectionId m_clientId;
        string m_clientName = "<not_set>";
    };

    typedef std::shared_ptr<Connection> connection_ptr;
}
