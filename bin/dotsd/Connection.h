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

        Connection(const Connection&) = delete;
        Connection& operator = (const Connection&) = delete;

        /*!
         * Create a Connection from a Channel.
         * @param channel Channel, that is moved into this Connection.
         * @param manager
         */
        explicit Connection(channel_ptr_t channel, ConnectionManager& manager);
        ~Connection();

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
        void send(const DotsTransportHeader& header, const type::Struct& instance);

        void send(const DotsTransportHeader& header, const Transmission& transmission);

        void send(const type::Struct& instance, types::property_set_t includedProperties = types::property_set_t::All, bool remove = false);

        void sendContainerContent(const Container<>& container);
        void sendCacheEnd(const std::string& typeName);

    private:
        void processConnectRequest(const DotsMsgConnect& msg);
        void processConnectPreloadClientFinished(const DotsMsgConnect& msg);
        void processMemberMessage(const DotsTransportHeader& header, const DotsMember& member, Connection* connection);

        enum class RxTx { rx, tx };

        void logRxTx(RxTx, const DotsTransportHeader& header);

        void onChannelError(const std::exception& e);
        bool onReceivedMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        bool onControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        bool onRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);

        void setConnectionState(const DotsConnectionState& state);

        inline static ConnectionId m_lastConnectionId = 1; // 0 is used for unitialized, 1 is used for the server.

        dots::Transmitter m_transmitter;

        channel_ptr_t m_channel;
        ConnectionManager& m_connectionManager;
        DotsConnectionState m_connectionState = DotsConnectionState::connecting;
        ConnectionId m_id;
        string m_clientName = "<not_set>";
    };

    typedef std::shared_ptr<Connection> connection_ptr;
}
