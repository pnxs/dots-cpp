#pragma once

#include "dots/cpp_config.h"
#include <dots/io/services/Channel.h>

#include <memory>
#include <dots/io/Container.h>
#include "dots/io/Transmitter.h"
#include "DotsConnectionState.dots.h"
#include "DotsMsgConnect.dots.h"
#include "DotsMember.dots.h"

namespace dots {

class ConnectionManager;

/*!
 * Represents a connection to a DOTS client.
 */
class Connection: public std::enable_shared_from_this<Connection>
{
public:
    typedef uint32_t ConnectionId;

    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;

    /*!
     * Create a Connection from a Channel.
     * @param channel Channel, that is moved into this Connection.
     * @param manager
     */
    explicit Connection(channel_ptr_t channel, ConnectionManager &manager);
    virtual ~Connection();

    virtual DotsConnectionState state() const;
    virtual const ConnectionId& id() const; ///< return client-id
    virtual const string& clientName() const; ///< return client-supplied name
    virtual ConnectionManager& connectionManager() const;

    virtual void start();
    virtual void stop();
    virtual void kill();

    /*!
     * Directly send a Message to the client.
     * @param msg Message-object
     */
    virtual void send(const DotsTransportHeader& header, const type::Struct& instance);

    virtual void send(const DotsTransportHeader& header, const Transmission& transmission);

    /*!
     * @return true if the client said, the it is intrested in DotsMember-messages.
     */
    virtual bool wantMemberMessages() const;

    /*!
     * Send a DOTS-object with a specific name-space to the client.
     * @tparam T The DOTS-object type
     * @param nameSpace used name-space
     * @param data A reference to the DOTS-object
     */
    template<class T>
    void sendNs(const string& nameSpace, const T& instance)
    {
        sendNs(nameSpace, &instance._Descriptor(), instance, instance._validProperties(), false);
    }

    /*!
     * Send a DOTS-object on 'global' name-space to the client.
     * @tparam T The DOTS-object type
     * @param data A reference to the DOTS-object
     */
    template<class T>
    void send(const T& instance)
    {
        sendNs({}, &T::_Descriptor(), instance, instance._validProperties(), false);
    }

    /*!
     * Sends a DOTS-object using a StructDescriptor
     * @param nameSpace used name-space
     * @param td pointer to StructDescriptor
     * @param data pointer to DOTS-object
     * @param properties which properties should be send?
     * @param remove send normal or remove object?
     */
    void sendNs(const string& nameSpace, const type::StructDescriptor<>* td, const type::Struct& instance, type::PropertySet properties, bool remove);

    void sendContainerContent(const Container<>& container);
    void sendCacheEnd(const std::string& typeName);

protected:
    Connection(ConnectionManager &manager);

private:
    void processConnectRequest(const DotsMsgConnect& msg);
    void processConnectPreloadClientFinished(const DotsMsgConnect& msg);
    void processMemberMessage(const DotsTransportHeader& header, const DotsMember &member, Connection* connection);

    enum class RxTx { rx, tx };
    void logRxTx(RxTx, const DotsTransportHeader& header);

    void onChannelError(const std::exception& e);
    bool onReceivedMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
    bool onControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
    bool onRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);

    void setConnectionState(const DotsConnectionState& state);

    dots::Transmitter m_transmitter;

	channel_ptr_t m_channel;
    ConnectionManager& m_connectionManager;
    DotsConnectionState  m_connectionState = DotsConnectionState::connecting;
    bool m_wantMemberMessages = false;
    ConnectionId m_id;
    string m_clientName = "<not_set>";
};

typedef std::shared_ptr<Connection> connection_ptr;

}