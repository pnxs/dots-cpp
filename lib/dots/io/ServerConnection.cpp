#include "ServerConnection.h"

#include "DotsTransportHeader.dots.h"
#include "DotsMsgConnect.dots.h"
#include "DotsMember.dots.h"
#include "DotsCacheInfo.dots.h"
#include "DotsDescriptorRequest.dots.h"
#include "dots/io/serialization/AsciiSerialization.h"
#include "Transmitter.h"

namespace dots
{

bool ServerConnection::start(const string &name, channel_ptr_t channel)
{
    if (running())
    {
        LOG_WARN_S("already started");
        return true;
    }

    DotsMsgHello::_Descriptor();
    DotsMsgConnectResponse::_Descriptor();
    DotsCacheInfo::_Descriptor();

    m_channel = channel;
	m_channel->asyncReceive(FUN(*this, handleReceivedMessage), nullptr);

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
	m_channel.reset();
}

Channel& ServerConnection::channel()
{
    return *m_channel.get();
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

bool ServerConnection::handleReceivedMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
{
    try 
    {
        if (transportHeader.nameSpace.isValid() && transportHeader.nameSpace == "SYS")
        {
            onControlMessage(transportHeader, std::move(transmission));
        } 
        else
        {
            onRegularMessage(transportHeader, std::move(transmission));
        }

        return true;
    }
    catch (const std::exception& e) 
    {
        LOG_ERROR_S("exception in receive: " << e.what());
        stop();

        return false;
    }
}

void ServerConnection::onControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
{
    Transmission transmission_ = std::move(transmission);
    const auto& typeName = *transportHeader.dotsHeader->typeName;
    const auto& dotsHeader = *transportHeader.dotsHeader;

    switch(m_connectionState)
    {
        case DotsConnectionState::connecting:
            if (typeName == "DotsMsgHello")
            {
                processHello(static_cast<const DotsMsgHello&>(transmission_.instance().get()));
            }
            else if (typeName == "DotsMsgConnectResponse")
            {
                processConnectResponse(static_cast<const DotsMsgConnectResponse&>(transmission_.instance().get()));
            }
            break;
        case DotsConnectionState::early_subscribe:
            if (typeName == "DotsMsgConnectResponse")
            {
                processEarlySubscribe(static_cast<const DotsMsgConnectResponse&>(transmission_.instance().get()));
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
                    dotsHeader.sender,
                    transportHeader.destinationGroup,
                    dotsHeader.sentTime,
                    dotsHeader,
                    transmission.instance(),
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

void ServerConnection::onRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission)
{
    Transmission transmission_ = std::move(transmission);
    const auto& typeName = *transportHeader.dotsHeader->typeName;
    const auto& dotsHeader = *transportHeader.dotsHeader;

    switch(m_connectionState)
    {
        case DotsConnectionState::connecting:
            break;
        case DotsConnectionState::early_subscribe:
        case DotsConnectionState::connected:
        {
            LOG_DATA_S("dispatch message " << typeName);

            ReceiveMessageData rmd = {
                dotsHeader.sender,
                transportHeader.destinationGroup,
                dotsHeader.sentTime,
                dotsHeader,
                transmission_.instance(),
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

Transmitter &ServerConnection::transmitter()
{
    return m_transmitter;
}

void ServerConnection::publishNs(const string& nameSpace, const type::StructDescriptor* td, const type::Struct& instance, property_set what, bool remove)
{
    DotsTransportHeader header;
    transmitter().prepareHeader(header, td, what, remove); //< Modifies header and what
    if (not nameSpace.empty()) {
        header.nameSpace(nameSpace);
    }

    // Send to peer or group

    LOG_DEBUG_S("publish ns=" << nameSpace << " type=" << td->name());
    LOG_DATA_S("data:" << to_ascii(td, &instance, what));
    //LOG_INFO_S("publish data-size: " << b.size());
    //LOG_INFO_S("data: " << b.toString());
    //vector<uint8_t > v(b.data(), b.data() + b.size());
    //LOG_INFO_S("publish data: " <<cbor::hexlify(&v[0], v.size()));
    channel().transmit(header, instance);
}

void ServerConnection::publish(const type::StructDescriptor *td, const type::Struct& instance, property_set what, bool remove)
{
    publishNs(string(), td, instance, what, remove);
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
    publishNs("SYS", &member._Descriptor(), member);
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
    publishNs("SYS", &cm._Descriptor(), cm);
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

    publish(&req._Descriptor(), req);
}

void ServerConnection::leaveGroup(const ServerConnection::GroupName &groupName)
{
    DotsMember member;
    member.groupName(groupName);
    member.event(DotsMemberEvent::leave);

    LOG_INFO_S("send DotsMember (leave " << groupName << ")");
    publishNs("SYS", &member._Descriptor(), member);
}


}
