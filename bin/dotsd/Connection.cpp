#include <dots/type/EnumDescriptor.h>
#include "Connection.h"
#include "ConnectionManager.h"
#include "AuthManager.h"
#include "dots/type/Registry.h"
#include "dots/io/serialization/CborNativeSerialization.h"

#include "DotsMsgConnectResponse.dots.h"
#include "DotsMsgError.dots.h"
#include "DotsMsgHello.dots.h"
#include "DotsClient.dots.h"
#include "DotsCacheInfo.dots.h"

#include "EnumDescriptorData.dots.h"
#include "StructDescriptorData.dots.h"


#define CEXPANSION "Connection[" << clientName() << "," << id() << "]: "
#define PEXPSTR_PRE "Connection[%p,%s]: "
#define PEXPARGS_PRE ,this, name().c_str(),
#include "dots/common/ext_logging.h"

namespace dots {

using namespace std::placeholders;

Connection::Connection(dots::TcpSocket socket, ConnectionManager &manager)
:m_dotsSocket(std::move(socket)), m_connectionManager(manager)
{
    // Create connection-name
    m_id = m_connectionManager.getUniqueClientId();

    LOG_INFO_S("connected");

    m_dotsSocket.setReceiveCallback(FUN(*this, onReceivedMessage));
    m_dotsSocket.setErrorCallback(FUN(*this, onSocketError));
}

void Connection::start()
{
    auto& server = m_connectionManager.serverInfo();

    DotsMsgHello hello;
    hello.serverName(server.name());
    hello.authChallenge(server.authManager().newChallenge()); // Random-Number
    sendNs("SYS", hello);
}

void Connection::stop()
{
    LOG_INFO_S("stopped");
    setConnectionState(DotsConnectionState::closed);
    m_dotsSocket.disconnect();
}

void Connection::kill()
{
    LOG_INFO_S("killed");
    m_connectionManager.handleKill(this);
}

Connection::~Connection()
{
    LOG_DEBUG_S("DTOR");
}

void Connection::processConnectRequest(const DotsMsgConnect &msg)
{
    m_clientName = msg.clientName;

    LOG_INFO_S("authorized");
    connectionManager().addClient(this);

    DotsMsgConnectResponse cr;
    cr.serverName(m_connectionManager.serverInfo().name());
    cr.accepted(true);
    cr.clientId(id());
    if (msg.preloadCache.isValid() and msg.preloadCache.isValid())
    {
        cr.preload(true);
    }
    sendNs("SYS", cr);

    if (msg.preloadCache.isValid() and msg.preloadCache.isValid()) {
        setConnectionState(DotsConnectionState::early_subscribe);
    }
    else {
        setConnectionState(DotsConnectionState::connected);
    }
}

void Connection::processConnectPreloadClientFinished(const DotsMsgConnect& msg)
{
    // Check authentication and authorization;
    if (not msg.preloadClientFinished.isValid() || not msg.preloadClientFinished.isValid())
    {
        LOG_WARN_S("invalid DotsMsgConnect in state early_connect");
        return;
    }

    setConnectionState(DotsConnectionState::connected);

    // When all cache items are sent to client, send fin-message
    DotsMsgConnectResponse cr;
    cr.preloadFinished(true);
    sendNs("SYS", cr);
}


/**
 * Central method to process received messages
 * @param msg
 * @param data
 *
 * @code
 * Table of accepted messages
 *
 * State      | Dots    | Dots   | Other |
 *            | Connect | Member |       |
 * -----------+---------+--------+-------+
 * Connecting |   X                      |
 * EarlySub   |             X            |
 * Connected  |             X        X   |
 * Suspended  |                          |
 * Closed     |                          |
 * -----------+--------------------------+
 * @endcode
 */
void Connection::onReceivedMessage(const Message &msg)
{
    LOG_DEBUG_S("onReceivedMessage:");
    bool handled = false;

    auto modifiedHeader = msg.header();
    // Overwrite sender to known client peerAddress
    auto& dotsHeader = *modifiedHeader.dotsHeader;
    dotsHeader.sender = id();

    dotsHeader.serverSentTime = pnxs::SystemNow();

    if (not dotsHeader.sentTime.isValid()) {
        dotsHeader.sentTime = dotsHeader.serverSentTime;
    }

    Message modifiedMessage(modifiedHeader, msg.data());

    logRxTx(RxTx::rx, modifiedMessage.header());

    try
    {
        // Check for DOTS control message-types
        if (msg.header().nameSpace.isValid() && *msg.header().nameSpace == "SYS")
        {
            handled = onControlMessage(modifiedMessage);
        }
        else
        {
            handled = onRegularMessage(modifiedMessage);
        }

        if (not handled)
        {
            string objName;
            if (msg.header().nameSpace.isValid()) objName = "::" + *msg.header().nameSpace + "::";
            objName += *msg.header().destinationGroup;
            string errorText = "invalid message received while in state " + to_string(m_connectionState) + ": " + objName;
            LOG_WARN_S(errorText);
            // send false response
            DotsMsgError error;
            error.errorCode(1);
            error.errorText(errorText);

            send(error);

        }
    }
    catch(const std::exception& e)
    {
        string errorReport = "exception in receive [";
        errorReport += "dstGrp=" + *msg.header().destinationGroup;
        errorReport += ",state=" + to_string(m_connectionState);
        errorReport += string("]:") + e.what();

        LOG_ERROR_S(errorReport);

        DotsMsgError error;
        error.errorCode(2);
        error.errorText(errorReport);
        send(error);

        stop();
    }
}

/**
 *
 * @code
 * Receive-Table:
 * *State                    | connecting | early_subscribe | connected | suspended | closed
 * --------------------------+------------+-----------------+-----------+-----------+---------
 * SYS::DotsMsgConnect       |     X      |                 |           |           |
 * SYS::DotsMember           |            |        X        |     X     |           |
 * SYS::EnumDescriptorData   |            |        X        |     X     |           |
 * SYS::StructDescriptorData |            |        X        |     X     |           |
 *                           |            |                 |           |           |
 *                           |            |                 |           |           |
 *                           |            |                 |           |           |
 *                           |            |                 |           |           |
 *                           |            |                 |           |           |
 *                           |            |                 |           |           |
 * *Other*                   |            |                 |     X     |           |
 *                           |            |                 |           |           |
 *                           |            |                 |           |           |
 * @endcode
 */
bool Connection::onControlMessage(const Message &msg)
{
    const auto& typeName = *msg.header().dotsHeader->typeName;
    const auto& data = msg.data();
    bool handled = false;

    switch(m_connectionState)
    {
        case DotsConnectionState::connecting:
            // Only accept DotsMsgConnect messages (MsgType connect)
            if (typeName == "DotsMsgConnect")
            {    // Check authentication and authorization;
                 processConnectRequest(dots::decodeInto_cbor<DotsMsgConnect>(data));
                handled = true;
            }
            break;
        case DotsConnectionState::early_subscribe:
            if (typeName == "DotsMsgConnect")
            {    // Check authentication and authorization;
                processConnectPreloadClientFinished(dots::decodeInto_cbor<DotsMsgConnect>(data));
                handled = true;
            }
            //No break here: Falltrough
			[[fallthrough]];
        case DotsConnectionState::connected:
            if (typeName == "DotsMember")
            {
                processMemberMessage(msg.header(), dots::decodeInto_cbor<DotsMember>(data), this);
                handled = true;
            }
            else if (typeName == "EnumDescriptorData")
            {
                auto enumDescriptorData = dots::decodeInto_cbor<EnumDescriptorData>(data);
                enumDescriptorData.publisherId = id();
                type::EnumDescriptor::createFromEnumDescriptorData(enumDescriptorData);
                m_connectionManager.deliverMessage(msg);
                handled = true;
            }
            else if (typeName == "StructDescriptorData")
            {
                auto structDescriptorData = dots::decodeInto_cbor<StructDescriptorData>(data);
                structDescriptorData.publisherId = id();
                LOG_DEBUG_S("received struct descriptor: " << structDescriptorData.name);
                type::StructDescriptor::createFromStructDescriptorData(structDescriptorData);
                m_connectionManager.deliverMessage(msg);
                handled = true;
            }
            else if (typeName == "DotsClearCache")
            {
                m_connectionManager.deliverMessage(msg);
                handled = true;
            }
            break;
        case DotsConnectionState::suspended:
            LOG_WARN_S("state suspended not implemented");
            // Connection is temporarly not available
            break;

        case DotsConnectionState::closed:
            LOG_WARN_S("state closed not implemented");
            // Connection is closed and will never be open again
            break;
    }

    return handled;
}

bool Connection::onRegularMessage(const Message &msg)
{
    bool handled = false;
    switch (m_connectionState)
    {
        case DotsConnectionState::connecting:
        case DotsConnectionState::early_subscribe: // Only accept DotsMsgConnect messages (MsgType connect)
            break;

        case DotsConnectionState::connected:
        {
            // Normal operation
            m_connectionManager.deliverMessage(msg);
            handled = true;
        }
            break;

        case DotsConnectionState::suspended:
            LOG_WARN_S("state suspended not implemented");
            // Connection is temporarly not available
            break;

        case DotsConnectionState::closed:
            LOG_WARN_S("state closed not implemented");
            // Connection is closed and will never be open again
            break;

    }
    return handled;
}

DotsConnectionState Connection::state() const
{
    return m_connectionState;
}

void Connection::setConnectionState(const DotsConnectionState& state)
{
    LOG_DEBUG_S("change connection state to " << state);
    m_connectionState = state;

	DotsClient{ DotsClient::id_t_i{ id() }, DotsClient::connectionState_t_i{ state } }._publish();
}

void Connection::send(const Message &msg)
{
    try
    {
        logRxTx(RxTx::tx, msg.header());
        m_dotsSocket.send(msg.header(), msg.data());
    }
    catch(const std::exception& e)
    {
        LOG_WARN_S("exception: " << e.what())
        kill();
    }
}

bool Connection::wantMemberMessages() const
{
    return m_wantMemberMessages;
}

ConnectionManager &Connection::connectionManager() const
{
    return m_connectionManager;
}

void Connection::processMemberMessage(const DotsTransportHeader& header, const DotsMember &member, Connection* connection)
{
    DotsMember memberMod = member;
    memberMod.client(connection->id());
    if (not member.event.isValid()) {
        LOG_WARN_S("member message without event");
    }
    LOG_DEBUG_S(*member.event << " " << member.groupName);
    connectionManager().processMemberMessage(header, member, connection);
}

const Connection::ConnectionId& Connection::id() const
{
    return m_id;
}

Connection::Connection(ConnectionManager &manager)
:m_connectionManager(manager)
{

}

void Connection::onSocketError(int ec)
{
    if (ec != 2) {
        LOG_ERROR_S("socket error: " << ec);
    }
    kill();
}

void Connection::sendNs(const string &nameSpace,
                        const type::StructDescriptor *td,
                        const void *data,
                        property_set properties,
                        bool remove)
{
    DotsTransportHeader header;
    m_transmitter.prepareHeader(header, td, properties, remove);
    if (not nameSpace.empty()) header.nameSpace(nameSpace);
    header.dotsHeader->sender(m_connectionManager.serverInfo().id());

    // prepareBuffer
    m_transmitter.prepareBuffer(td, data, header, properties);

    // Send to peer or group
    send({header, m_transmitter.buffer()});
}

void Connection::logRxTx(Connection::RxTx rxtx, const DotsTransportHeader &header)
{
    const char* rxtxColor = "\33[1;31m";
    const char* msg = "";
    const char* allOff = "\33[0m";

    string ns = header.nameSpace.isValid() ? *header.nameSpace + "::" : "";

    switch (rxtx) {
        case RxTx::rx: rxtxColor = "\33[1;32m"; msg = "rx "; break;
        case RxTx::tx: rxtxColor = "\33[1;31m"; msg = "tx "; break;
    }
    LOG_DEBUG_S(rxtxColor << msg << ns << header.destinationGroup << allOff);

}

const string &Connection::clientName() const
{
    return m_clientName;
}

void Connection::sendContainerContent(const AnyContainer &container)
{
    auto td = container.td();

    LOG_DEBUG_S("send cache for " << td->name() << " size=" << container.size());
    uint32_t remainingCacheObjects = container.size();
    for (const auto& e : container)
    {
        const char* lop = "";
        switch(e.information.lastOperation) {
            case Mt::create: lop = "C"; break;
            case Mt::update: lop = "U"; break;
            case Mt::remove: lop = "R"; break;
        }

        LOG_DATA_S("clone-info: lastOp=" << lop << ", lastUpdateFrom=" << e.information.lastUpdateFrom
                                         << ", created=" << e.information.created->toString() << ", creator=" << e.information.createdFrom
                                         << ", modified=" << e.information.modified->toString() << ", localUpdateTime=" << e.information.localUpdateTime->toString());


        DotsTransportHeader thead;
        m_transmitter.prepareHeader(thead, td, td->validProperties(e.data), false);

        auto& dotsHeader = *thead.dotsHeader;
        dotsHeader.sentTime = e.information.modified;
        dotsHeader.serverSentTime =pnxs::SystemNow();
        dotsHeader.sender = e.information.lastUpdateFrom;
        dotsHeader.fromCache = --remainingCacheObjects;

        // prepareBuffer
        m_transmitter.prepareBuffer(td, e.data, thead, td->validProperties(e.data));

        // Send to peer or group
        send({thead, m_transmitter.buffer()});
    }

    sendCacheEnd(td->name());
}

void Connection::sendCacheEnd(const std::string& typeName)
{
    DotsCacheInfo dotsCacheInfo {
        DotsCacheInfo::typeName_t_i{typeName},
        DotsCacheInfo::endTransmission_t_i{true}
    };
    sendNs("SYS", dotsCacheInfo);
}

}

