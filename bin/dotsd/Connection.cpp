#include <dots/type/EnumDescriptor.h>
#include "Connection.h"
#include "ConnectionManager.h"
#include "AuthManager.h"
#include "dots/io/Registry.h"

#include "DotsMsgConnectResponse.dots.h"
#include "DotsMsgError.dots.h"
#include "DotsMsgHello.dots.h"
#include "DotsClient.dots.h"
#include "DotsCacheInfo.dots.h"

#include "EnumDescriptorData.dots.h"
#include "StructDescriptorData.dots.h"

#include <dots/dots.h>

#define CEXPANSION "Connection[" << clientName() << "," << id() << "]: "
#define PEXPSTR_PRE "Connection[%p,%s]: "
#define PEXPARGS_PRE ,this, name().c_str(),
#include "dots/common/ext_logging.h"

namespace dots {

using namespace std::placeholders;

Connection::Connection(channel_ptr_t channel, ConnectionManager &manager)
:m_channel(std::move(channel)), m_connectionManager(manager)
{
    DotsMsgConnect::_Descriptor();
    DotsMember::_Descriptor();
    EnumDescriptorData::_Descriptor();
    StructDescriptorData::_Descriptor();

    // Create connection-name
    m_id = m_connectionManager.getUniqueClientId();

    LOG_INFO_S("connected");

	m_channel->asyncReceive(transceiver().registry(), FUN(*this, onReceivedMessage), FUN(*this, onChannelError));
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
bool Connection::onReceivedMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
{
    LOG_DEBUG_S("onReceivedMessage:");
    bool handled = false;

    auto modifiedHeader = transportHeader;
    // Overwrite sender to known client peerAddress
    auto& dotsHeader = *modifiedHeader.dotsHeader;
    dotsHeader.sender = id();

    dotsHeader.serverSentTime = pnxs::SystemNow();

    if (not dotsHeader.sentTime.isValid()) {
        dotsHeader.sentTime = dotsHeader.serverSentTime;
    }

    logRxTx(RxTx::rx, modifiedHeader);

    try
    {
        // Check for DOTS control message-types
        if (transportHeader.nameSpace.isValid() && *transportHeader.nameSpace == "SYS")
        {
            handled = onControlMessage(modifiedHeader, std::move(transmission));
        }
        else
        {
            handled = onRegularMessage(modifiedHeader, std::move(transmission));
        }

        if (not handled)
        {
            string objName;
            if (transportHeader.nameSpace.isValid()) objName = "::" + *transportHeader.nameSpace + "::";
            objName += *transportHeader.destinationGroup;
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
        errorReport += "dstGrp=" + *transportHeader.destinationGroup;
        errorReport += ",state=" + to_string(m_connectionState);
        errorReport += string("]:") + e.what();

        LOG_ERROR_S(errorReport);

        DotsMsgError error;
        error.errorCode(2);
        error.errorText(errorReport);
        send(error);

        stop();
    }

    return m_connectionState != DotsConnectionState::closed;
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
bool Connection::onControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
{
    bool handled = false;

    switch(m_connectionState)
    {
        case DotsConnectionState::connecting:
            // Only accept DotsMsgConnect messages (MsgType connect)
            if (auto* dotsMsgConnect = transmission.instance()->_as<DotsMsgConnect>())
            {    // Check authentication and authorization;
                 processConnectRequest(*dotsMsgConnect);
                handled = true;
            }
            break;
        case DotsConnectionState::early_subscribe:
            if (auto* dotsMsgConnect = transmission.instance()->_as<DotsMsgConnect>())
            {    // Check authentication and authorization;
                processConnectPreloadClientFinished(*dotsMsgConnect);
                handled = true;
            }
            //No break here: Falltrough
			[[fallthrough]];
        case DotsConnectionState::connected:
            if (auto* dotsMember = transmission.instance()->_as<DotsMember>())
            {
                processMemberMessage(transportHeader, *dotsMember, this);
                handled = true;
            }
            else if (auto* enumDescriptorData = transmission.instance()->_as<EnumDescriptorData>())
            {
                //enumDescriptorData->publisherId = id();
                type::EnumDescriptor<>::createFromEnumDescriptorData(*enumDescriptorData);
                m_connectionManager.deliver(transportHeader, std::move(transmission));
                handled = true;
            }
            else if (auto* structDescriptorData = transmission.instance()->_as<StructDescriptorData>())
            {
                //structDescriptorData->publisherId = id();
                LOG_DEBUG_S("received struct descriptor: " << structDescriptorData->name);
            	const type::StructDescriptor<>* descriptor = type::StructDescriptor<>::createFromStructDescriptorData(*structDescriptorData);
            	LOG_INFO_S("register type " << descriptor->name() << " published by " << m_clientName);
            	m_connectionManager.onNewType(descriptor);
                m_connectionManager.deliver(transportHeader, std::move(transmission));
                handled = true;
            }
            else if (transmission.instance()->_is<DotsClearCache>())
            {
                m_connectionManager.deliver(transportHeader, std::move(transmission));
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

bool Connection::onRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
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
            m_connectionManager.deliver(transportHeader, std::move(transmission));
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

	DotsClient{ DotsClient::id_i{ id() }, DotsClient::connectionState_i{ state } }._publish();
}

void Connection::send(const DotsTransportHeader& header, const type::Struct& instance)
{
    try
    {
        logRxTx(RxTx::tx, header);
        m_channel->transmit(header, instance);
    }
    catch(const std::exception& e)
    {
        LOG_WARN_S("exception: " << e.what())
        kill();
    }
}

void Connection::send(const DotsTransportHeader& header, const Transmission& transmission)
{
    try
    {
        logRxTx(RxTx::tx, header);
        m_channel->transmit(header, transmission);
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

void Connection::onChannelError(const std::exception& e)
{
    LOG_ERROR_S("channel error: " << e.what());
    kill();
}

void Connection::sendNs(const string &nameSpace,
                        const type::StructDescriptor<> *td,
                        const type::Struct& instance,
                        type::PropertySet properties,
                        bool remove)
{
    DotsTransportHeader header;
    m_transmitter.prepareHeader(header, td, properties, remove);
    if (not nameSpace.empty()) header.nameSpace(nameSpace);
    header.dotsHeader->sender(m_connectionManager.serverInfo().id());

    // Send to peer or group
    send(header, instance);
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

void Connection::sendContainerContent(const Container<>& container)
{
    const auto& td = container.descriptor();

    LOG_DEBUG_S("send cache for " << td.name() << " size=" << container.size());
    uint32_t remainingCacheObjects = container.size();
    for (const auto& [instance, cloneInfo] : container)
    {
        const char* lop = "";
        switch(cloneInfo.lastOperation) {
            case DotsMt::create: lop = "C"; break;
            case DotsMt::update: lop = "U"; break;
            case DotsMt::remove: lop = "R"; break;
        }

        LOG_DATA_S("clone-info: lastOp=" << lop << ", lastUpdateFrom=" << cloneInfo.lastUpdateFrom
                                         << ", created=" << cloneInfo.created->toString() << ", creator=" << cloneInfo.createdFrom
                                         << ", modified=" << cloneInfo.modified->toString() << ", localUpdateTime=" << cloneInfo.localUpdateTime->toString());


        DotsTransportHeader thead;
        m_transmitter.prepareHeader(thead, &td, instance->_validProperties(), false);

        auto& dotsHeader = *thead.dotsHeader;
        dotsHeader.sentTime = cloneInfo.modified.isValid() ? *cloneInfo.modified : *cloneInfo.created;
        dotsHeader.serverSentTime =pnxs::SystemNow();
        dotsHeader.sender = cloneInfo.lastUpdateFrom;
        dotsHeader.fromCache = --remainingCacheObjects;

        // Send to peer or group
        send(thead, instance);
    }

    sendCacheEnd(td.name());
}

void Connection::sendCacheEnd(const std::string& typeName)
{
    DotsCacheInfo dotsCacheInfo {
        DotsCacheInfo::typeName_i{typeName},
        DotsCacheInfo::endTransmission_i{true}
    };
    sendNs("SYS", dotsCacheInfo);
}

}

