#pragma once

#include "dots/cpp_config.h"
#include <dots/io/services/Channel.h>

#include <memory>
#include <dots/io/Container.h>
#include "dots/io/Transmitter.h"
#include "DotsConnectionState.dots.h"
#include "DotsMsgConnect.dots.h"
#include "DotsMember.dots.h"

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

        void start();
        void stop();
        void kill();

        /*!
         * Directly send a Message to the client.
         * @param msg Message-object
         */
        void send(const type::Struct& instance, types::property_set_t includedProperties = types::property_set_t::All, bool remove = false);
        void send(const DotsTransportHeader& header, const type::Struct& instance);
        void send(const DotsTransportHeader& header, const Transmission& transmission);

        void sendContainerContent(const Container<>& container);
        void sendCacheEnd(const std::string& typeName);

    private:

        enum class RxTx { rx, tx };

        bool onReceivedMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        bool onControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        bool onRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);

        void processConnectRequest(const DotsMsgConnect& msg);
        void processConnectPreloadClientFinished(const DotsMsgConnect& msg);
        void processMemberMessage(const DotsTransportHeader& header, const DotsMember& member, Connection* connection);

        void onChannelError(const std::exception& e);

        void logRxTx(RxTx, const DotsTransportHeader& header);
        void setConnectionState(const DotsConnectionState& state);

        inline static ConnectionId m_lastConnectionId = ServerId; // 0 is used for unitialized, 1 is used for the server.

        dots::Transmitter m_transmitter;
        channel_ptr_t m_channel;
        string m_serverName;
        ConnectionManager& m_connectionManager;
        DotsConnectionState m_connectionState = DotsConnectionState::connecting;
        ConnectionId m_clientId;
        string m_clientName = "<not_set>";
    };

    typedef std::shared_ptr<Connection> connection_ptr;
}
