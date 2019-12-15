#pragma once

#include "dots/cpp_config.h"
#include <dots/dots_base.h>
#include <dots/functional/signal.h>
#include <dots/io/services/Channel.h>
#include "Transmitter.h"

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

		Transmitter& transmitter();

		// Server actions BEGIN
		typedef string GroupName;
		typedef string ClientName;

		enum class ConnectMode { direct, preload };

		typedef std::vector<string> DescriptorList;

		void joinGroup(const GroupName&);
		void leaveGroup(const GroupName&);

		void requestConnection(const ClientName&, ConnectMode);

		void publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t what = types::property_set_t::All, bool remove = false);
		void publishNs(const string& nameSpace, const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t what = types::property_set_t::All, bool remove = false);
		// Server actions END

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

		void processConnectResponse(const DotsMsgConnectResponse& cr);
		void processEarlySubscribe(const DotsMsgConnectResponse& cr);
		void processHello(const DotsMsgHello&);

		void setConnectionState(DotsConnectionState state);

		void disconnect();

		bool m_running = false;
		channel_ptr_t m_channel;
		DotsConnectionState m_connectionState = DotsConnectionState::connecting;
		Transmitter m_transmitter;
		string m_clientName;
		ClientId m_serversideClientname;
	};
}
