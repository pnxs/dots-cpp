#pragma once

#include "dots/cpp_config.h"
#include "dots/io/DotsAsioSocket.h"

#include <memory>
#include <dots/io/AnyContainer.h>
#include "dots/io/Transmitter.h"
#include "DotsConnectionState.dots.h"
#include "DotsMsgConnect.dots.h"
#include "DotsMember.dots.h"
#include "dots/io/Message.h"

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
     * Create a Connection from a TcpSocket.
     * @param socket TcpSocket, that is moved into this Connection.
     * @param manager
     */
    explicit Connection(boost::asio::ip::tcp::socket socket, ConnectionManager &manager);
    ~Connection();

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
    virtual void send(const Message& msg);

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
    void sendNs(const string& nameSpace, const T& data)
    {
        sendNs(nameSpace, &data._Descriptor(), &data, data._validProperties(), false);
    }

    /*!
     * Send a DOTS-object on 'global' name-space to the client.
     * @tparam T The DOTS-object type
     * @param data A reference to the DOTS-object
     */
    template<class T>
    void send(const T& data)
    {
        sendNs({}, &T::_Descriptor(), &data, data._validProperties(), false);
    }

    /*!
     * Sends a DOTS-object using a StructDescriptor
     * @param nameSpace used name-space
     * @param td pointer to StructDescriptor
     * @param data pointer to DOTS-object
     * @param properties which properties should be send?
     * @param remove send normal or remove object?
     */
    void sendNs(const string& nameSpace, const type::StructDescriptor* td, const void* data, property_set properties, bool remove);

    void sendContainerContent(const AnyContainer &container);

protected:
    Connection(ConnectionManager &manager);

private:
    void processConnectRequest(const DotsMsgConnect& msg);
    void processConnectPreloadClientFinished(const DotsMsgConnect& msg);
    void processMemberMessage(const DotsTransportHeader& header, const DotsMember &member, Connection* connection);

    enum class RxTx { rx, tx };
    void logRxTx(RxTx, const DotsTransportHeader& header);

    void onSocketError(int ec);
    void onReceivedMessage(const Message &msg);
    bool onControlMessage(const Message &msg);
    bool onRegularMessage(const Message &msg);

    void setConnectionState(const DotsConnectionState& state);

    dots::Transmitter m_transmitter;

    dots::DotsAsioSocket m_dotsSocket;
    ConnectionManager& m_connectionManager;
    DotsConnectionState  m_connectionState = DotsConnectionState::connecting;
    bool m_wantMemberMessages = false;
    ConnectionId m_id;
    string m_clientName = "<not_set>";
};

typedef std::shared_ptr<Connection> connection_ptr;

}