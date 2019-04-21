#include "ServerConnection.h"

#include "DotsTransportHeader.dots.h"
#include "DotsMsgConnect.dots.h"
#include "DotsMember.dots.h"
#include "DotsDescriptorRequest.dots.h"
#include "dots/io/serialization/CborNativeSerialization.h"
#include "dots/io/serialization/AsciiSerialization.h"
#include "Transmitter.h"

namespace dots
{

bool ServerConnection::start(const string &name, ChannelPtr dotsSocket)
{
    if (running())
    {
        LOG_WARN_S("already started");
        return true;
    }

    m_dotsSocket = dotsSocket;

    socket().asyncReceive(FUN(*this, handleReceivedMessage), nullptr);

    m_running = true;
    m_clientName = name;

    handleConnected(name);

    return true;
}

void ServerConnection::stop()
{
    if (not running()) return;

    disconnect();

    m_running = false;
}

bool ServerConnection::running()
{
    return m_running;
}

void ServerConnection::disconnect()
{
	m_dotsSocket.reset();
}

Channel& ServerConnection::socket()
{
    return *m_dotsSocket.get();
}

void ServerConnection::handleConnected(const string &/*name*/)
{
    switch(m_connectionState)
    {
        case DotsConnectionState::connecting:
#if 0
            // Connection established, send ConnectReqest (with e.g. credentials)
            {
                DotsMsgConnect cm;
                cm.setClientName(name);
                cm.setPreloadCache(true);

                LOG_INFO_S("publish DotsMsgConnect");
                publishNs("SYS", cm._td(), &cm);
            }
#endif
            break;
        case DotsConnectionState::suspended:
            // When a connection is reestablished, continue operation
            break;
        case DotsConnectionState::early_subscribe:
        case DotsConnectionState::connected:
        case DotsConnectionState::closed:
            // Do nothing
            break;
    }

}


void ServerConnection::handleDisconnected()
{
    switch(m_connectionState)
    {
        case DotsConnectionState::connecting:
            // Connect failed
            setConnectionState(DotsConnectionState::closed);
            break;
        case DotsConnectionState::early_subscribe:
            // Connect failed
            setConnectionState(DotsConnectionState::closed);
            break;
        case DotsConnectionState::connected:
            // Connection interrupted, try to reestablish
            setConnectionState(DotsConnectionState::suspended);
            break;
        case DotsConnectionState::suspended:
        case DotsConnectionState::closed:
            // Do nothing
            break;
    }
}

void ServerConnection::handleReceivedMessage(const Message &msg)
{
    try {
        if (msg.header().nameSpace.isValid() && msg.header().nameSpace == "SYS")
        {
            onControlMessage(msg);
        } else
        {
            onRegularMessage(msg);
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR_S("exception in receive: " << e.what());
        stop();
    }
}

void ServerConnection::onControlMessage(const Message &msg)
{
    const auto& typeName = *msg.header().dotsHeader->typeName;
    const auto& data = msg.data();
    const auto& dotsHeader = *msg.header().dotsHeader;

    switch(m_connectionState)
    {
        case DotsConnectionState::connecting:
            if (typeName == "DotsMsgHello")
            {
                processHello(decodeInto_cbor<DotsMsgHello>(msg.data()));
            }
            else if (typeName == "DotsMsgConnectResponse")
            {
                processConnectResponse(decodeInto_cbor<DotsMsgConnectResponse>(data));
            }
            break;
        case DotsConnectionState::early_subscribe:
            if (typeName == "DotsMsgConnectResponse")
            {
                processEarlySubscribe(decodeInto_cbor<DotsMsgConnectResponse>(data));
            }
            // No break here: falltrough
            // process all messages, put non-cache messages into buffer
			[[fallthrough]];
        case DotsConnectionState::connected:
        {
            if (typeName == "DotsCacheInfo") {
                //TODO: implement handling of DotsCacheInfo
                //for now let trough like an normal object
                // return;
            }

            ReceiveMessageData rmd = {
                    &data[0],
                    data.size(),
                    dotsHeader.sender,
                    msg.header().destinationGroup,
                    dotsHeader.sentTime,
                    dotsHeader,
                    (dotsHeader.sender == m_serversideClientname)
            };

            onReceiveMessage(rmd);
        }

            break;
        case DotsConnectionState::suspended:
            // buffer outgoing messages
            break;
        case DotsConnectionState::closed:
            // Do nothing
            break;
    }

}

void ServerConnection::onRegularMessage(const Message &msg)
{
    const auto& typeName = *msg.header().dotsHeader->typeName;
    const auto& data = msg.data();
    const auto& dotsHeader = *msg.header().dotsHeader;

    switch(m_connectionState)
    {
        case DotsConnectionState::connecting:
            break;
        case DotsConnectionState::early_subscribe:
        case DotsConnectionState::connected:
        {
            LOG_DATA_S("dispatch message " << typeName);

            ReceiveMessageData rmd = {
                &data[0],
                data.size(),
                dotsHeader.sender,
                msg.header().destinationGroup,
                dotsHeader.sentTime,
                dotsHeader,
                (dotsHeader.sender == m_serversideClientname)
            };

            onReceiveMessage(rmd);
        }
            break;
        case DotsConnectionState::suspended:
            // buffer outgoing messages
            break;
        case DotsConnectionState::closed:
            // Do nothing
            break;
    }
}

void ServerConnection::setConnectionState(DotsConnectionState state)
{
    LOG_DEBUG_S("change connection state to " << to_string(state));
    m_connectionState = state;
    switch(m_connectionState)
    {
        case DotsConnectionState::connected:
            onConnected();
            break;
        case DotsConnectionState::early_subscribe:
            onEarlyConnect();
            break;
        default:
            break;
    }
}

int ServerConnection::send(const DotsTransportHeader &header, const vector<uint8_t> &data)
{
    return socket().transmit(header, data);
}

Transmitter &ServerConnection::transmitter()
{
    return m_transmitter;
}

void ServerConnection::publishNs(const string& nameSpace, const type::StructDescriptor* td, CTypeless data, property_set what, bool remove)
{
    DotsTransportHeader header;
    transmitter().prepareHeader(header, td, what, remove); //< Modifies header and what
    if (not nameSpace.empty()) {
        header.nameSpace(nameSpace);
    }

    // prepareBuffer
    transmitter().prepareBuffer(td, data, header, what);

    // Send to peer or group

    LOG_DEBUG_S("publish ns=" << nameSpace << " type=" << td->name());
    LOG_DATA_S("data:" << to_ascii(td, data, what));
    //LOG_INFO_S("publish data-size: " << b.size());
    //LOG_INFO_S("data: " << b.toString());
    //vector<uint8_t > v(b.data(), b.data() + b.size());
    //LOG_INFO_S("publish data: " <<cbor::hexlify(&v[0], v.size()));
    this->send(header, transmitter().buffer());
}

void ServerConnection::publish(const type::StructDescriptor *td, CTypeless data, property_set what, bool remove)
{
    publishNs(string(), td, data, what, remove);
}

void ServerConnection::processConnectResponse(const DotsMsgConnectResponse& cr)
{
    string serverName = cr.serverName.isValid() ? *cr.serverName : "<unknown>";
    LOG_DEBUG_S("connectResponse: serverName=" << serverName << " accepted=" << *cr.accepted);
    if (cr.clientId.isValid()) {
        m_serversideClientname = cr.clientId;
    }
    if (cr.preload.isValid() && cr.preload &&
            (not cr.preloadFinished.isValid() ||
            (cr.preloadFinished.isValid() && not cr.preloadFinished)))
    {
        setConnectionState(DotsConnectionState::early_subscribe);
    }
    else
    {
        setConnectionState(DotsConnectionState::connected);
    }
}

void ServerConnection::processEarlySubscribe(const DotsMsgConnectResponse &cr)
{
    if (cr.preloadFinished.isValid() and cr.preloadFinished)
    {
        setConnectionState(DotsConnectionState::connected);
    } else{
        LOG_ERROR_S("invalid DotsMsgConnectResponse");
    }
}


void ServerConnection::processHello(const DotsMsgHello &hello)
{
    if (hello.authChallenge.isValid() && hello.serverName.isValid())
    {
        LOG_DEBUG_S("received hello from '" << *hello.serverName << "' authChallenge=" << hello.authChallenge);
        LOG_DATA_S("send DotsMsgConnect");
        requestConnection(m_clientName, ConnectMode::preload);
    }
    else
    {
        LOG_WARN_S("Invalid hello from server valatt:" << hello._validProperties().to_string());
    }
}

void ServerConnection::joinGroup(const GroupName &groupName)
{
    DotsMember member;
    member.groupName(groupName);
    member.event(DotsMemberEvent::join);

    LOG_DEBUG_S("send DotsMember (join " << groupName << ")");
    publishNs("SYS", &member._Descriptor(), &member);
}

void ServerConnection::requestConnection(const ServerConnection::ClientName& name, ServerConnection::ConnectMode mode)
{
    DotsMsgConnect cm;
    cm.clientName(name);
    switch (mode)
    {
        case ConnectMode::direct: cm.preloadCache(false); break;
        case ConnectMode::preload: cm.preloadCache(true); break;
    }
    publishNs("SYS", &cm._Descriptor(), &cm);
}

void ServerConnection::requestDescriptors(const DescriptorList &whiteList, const DescriptorList &blackList)
{
    DotsDescriptorRequest req;

    if (not whiteList.empty())
    {
		req.whitelist();
        for (auto& e : whiteList)
            req.whitelist->push_back(e);
    }
    if (not blackList.empty())
    {
		req.blacklist();
        for (auto& e : blackList)
            req.blacklist->push_back(e);
    }

    publish(&req._Descriptor(), &req);
}

void ServerConnection::leaveGroup(const ServerConnection::GroupName &groupName)
{
    DotsMember member;
    member.groupName(groupName);
    member.event(DotsMemberEvent::leave);

    LOG_INFO_S("send DotsMember (leave " << groupName << ")");
    publishNs("SYS", &member._Descriptor(), &member);
}


}
