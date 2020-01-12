#pragma once
#include <string>
#include <map>
#include <set>
#include <dots/io/services/Channel.h>
#include <DotsConnectionState.dots.h>
#include <DotsTransportHeader.dots.h>
#include <DotsMsgHello.dots.h>
#include <DotsMsgConnectResponse.dots.h>

namespace dots::io
{
	struct Registry;
}

namespace dots::io
{
	struct ChannelConnection
	{
		using receive_handler_t = std::function<bool(const DotsTransportHeader&, Transmission&&)>;
		using error_handler_t = std::function<void(const std::exception&)>;
		using descriptor_map_t = std::map<std::string_view, type::StructDescriptor<>*>;
		
		ChannelConnection(channel_ptr_t channel, descriptor_map_t preloadPublishTypes = {}, descriptor_map_t preloadSubscribeTypes = {});
		ChannelConnection(const ChannelConnection& other) = delete;
		ChannelConnection(ChannelConnection&& other) = default;
		~ChannelConnection() = default;

		ChannelConnection& operator = (const ChannelConnection& rhs) = delete;
		ChannelConnection& operator = (ChannelConnection&& rhs) = default;

		DotsConnectionState state() const;
        uint32_t id() const;
		const std::string& name() const;
		bool connected() const;

		void asyncReceive(Registry& registry, const std::string& name, receive_handler_t&& receiveHandler, error_handler_t&& errorHandler);
		void transmit(const type::Struct& instance, types::property_set_t includedProperties = types::property_set_t::All, bool remove = false);

		void joinGroup(const std::string_view& name);
		void leaveGroup(const std::string_view& name);

	private:

		bool handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        void handleControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        void handleRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
		void handleError(const std::exception& e);

        void processHello(const DotsMsgHello& hello);
        void processConnectResponse(const DotsMsgConnectResponse& connectResponse);
        void processEarlySubscribe(const DotsMsgConnectResponse& connectResponse);

		void setConnectionState(DotsConnectionState state);

		void importType(const type::Struct& instance);
		void exportType(const type::Descriptor<>& descriptor);

		DotsConnectionState m_connectionState;
        uint32_t m_id;
		
		channel_ptr_t m_channel;
		io::Registry* m_registry;
        std::string m_name;
		descriptor_map_t m_preloadPublishTypes;
		descriptor_map_t m_preloadSubscribeTypes;
		receive_handler_t m_receiveHandler;
		error_handler_t m_errorHandler;
		
		std::set<std::string> m_sharedTypes;
	};	
}