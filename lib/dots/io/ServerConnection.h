#pragma once

#include <string_view>
#include "dots/cpp_config.h"
#include <dots/dots_base.h>
#include <dots/functional/signal.h>
#include <dots/io/services/Channel.h>

#include "DotsConnectionState.dots.h"
#include "DotsMsgConnectResponse.dots.h"
#include "DotsTransportHeader.dots.h"
#include "DotsMsgHello.dots.h"

namespace dots
{
	typedef pnxs::Signal<void(const DotsHeader& header, const type::AnyStruct& instance)> ReceiveMessageSignal;

	/**
	 * This class is a proxy to a dotsd server
	 * Is contains the actions, that can be done with the server
	 */
	class ServerConnection
	{
	public:

		bool start(const string& name, channel_ptr_t channel);
		void stop();

		bool running();

		const ClientId& clientId() const { return m_serversideClientname; }

		Channel& channel();

		// Signals:
		ReceiveMessageSignal onReceiveMessage;
		pnxs::Signal<void()> onConnected;
		pnxs::Signal<void()> onEarlyConnect;

	private:
		void handleConnected(const string& name);
		void handleDisconnected();
		void onControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
		void onRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
		bool handleReceivedMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);

		void processConnectResponse(const DotsMsgConnectResponse& connectResponse);
		void processEarlySubscribe(const DotsMsgConnectResponse& cr);
		void processHello(const DotsMsgHello&);

		void setConnectionState(DotsConnectionState state);

		void disconnect();

		bool m_running = false;
		channel_ptr_t m_channel;
		DotsConnectionState m_connectionState = DotsConnectionState::connecting;
		string m_clientName;
		ClientId m_serversideClientname;
	};
}
