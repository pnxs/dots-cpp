#pragma once

#include "dots/cpp_config.h"
#include <dots/functional/signal.h>
#include "DotsSocket.h"
#include "Transmitter.h"

#include "DotsConnectionState.dots.h"
#include "DotsMsgConnectResponse.dots.h"
#include "DotsTransportHeader.dots.h"
#include "DotsMsgHello.dots.h"

namespace dots
{

struct ReceiveMessageData
{
    const uint8_t* data;
    size_t length;
    ClientId sender;
    string group;
    TimePoint sentTime;
    const DotsHeader& header;
    bool isFromMyself;
};

typedef pnxs::Signal<void (const ReceiveMessageData& cbd)> ReceiveMessageSignal;

/**
 * This class is a proxy to a dotsd server
 * Is contains the actions, that can be done with the server
 */
class ServerConnection
{
public:

    bool start(const string &name, DotsSocketPtr dotsSocket);
    void stop();

    bool running();

    Transmitter& transmitter();

    // Server actions BEGIN
    typedef string GroupName;
    typedef string ClientName;
    enum class ConnectMode { direct, preload };
    typedef vector<string> DescriptorList;

    void joinGroup(const GroupName&);
    void leaveGroup(const GroupName&);
    void requestDescriptors(const DescriptorList &whiteList = {}, const DescriptorList &blackList = {});

    void requestConnection(const ClientName&, ConnectMode);

    void publish(const type::StructDescriptor* td, CTypeless data, property_set what = PROPERTY_SET_ALL, bool remove = false);
    void publishNs(const string& nameSpace, const type::StructDescriptor* td, CTypeless data, property_set what = PROPERTY_SET_ALL, bool remove = false);
    // Server actions END

    int send(const DotsTransportHeader& header, const vector<uint8_t>& data = {});
    const ClientId& clientId() const { return m_serversideClientname; }

    DotsSocket& socket();

    // Signals:
    ReceiveMessageSignal onReceiveMessage;
    pnxs::Signal<void()> onConnected;
    pnxs::Signal<void()> onEarlyConnect;

private:
    void handleConnected(const string &name);
    void handleDisconnected();
    void onControlMessage(const Message &);
    void onRegularMessage(const Message &msg);
    void handleReceivedMessage(const Message &);

    void processConnectResponse(const DotsMsgConnectResponse &cr);
    void processEarlySubscribe(const DotsMsgConnectResponse &cr);
    void processHello(const DotsMsgHello&);

    void setConnectionState(DotsConnectionState state);

    //bool connect(const string& host, int port, const string& name);
    void disconnect();

    //int receive();
    //int send(const DotsMessageHeader& header, const vector<uint8_t>& data = {});

    bool m_running = false;
    DotsSocketPtr m_dotsSocket;
    DotsConnectionState m_connectionState = DotsConnectionState::connecting;
    Transmitter m_transmitter;
    string m_clientName;
    ClientId m_serversideClientname;
};

}
