#pragma once
#include <string>
#include <map>
#include <set>
#include <dots/io/services/Channel.h>
#include <DotsConnectionState.dots.h>
#include <DotsTransportHeader.dots.h>
#include <DotsMsgHello.dots.h>
#include <DotsMsgConnectResponse.dots.h>
#include <DotsMsgConnect.dots.h>

namespace dots::io
{
	struct Registry;
}

namespace dots::io
{
	struct Connection
	{
		using id_t = uint32_t;
        static constexpr id_t ServerIdDeprecated = 1;

		using receive_handler_t = std::function<bool(const DotsTransportHeader&, Transmission&&, bool)>;
		using error_handler_t = std::function<void(id_t, const std::exception&)>;
		using descriptor_map_t = std::map<std::string_view, type::StructDescriptor<>*>;
		
		Connection(channel_ptr_t channel, bool server, descriptor_map_t preloadPublishTypes = {}, descriptor_map_t preloadSubscribeTypes = {});
		Connection(const Connection& other) = delete;
		Connection(Connection&& other) = default;
		~Connection() = default;

		Connection& operator = (const Connection& rhs) = delete;
		Connection& operator = (Connection&& rhs) = default;

		DotsConnectionState state() const;
        id_t id() const;
		const std::string& name() const;
		bool connected() const;

		void asyncReceive(Registry& registry, const std::string& name, receive_handler_t&& receiveHandler, error_handler_t&& errorHandler);
		void transmit(const type::Struct& instance, types::property_set_t includedProperties = types::property_set_t::All, bool remove = false);
		void transmit(const DotsTransportHeader& header, const type::Struct& instance);
        void transmit(const DotsTransportHeader& header, const Transmission& transmission);

		void joinGroup(const std::string_view& name);
		void leaveGroup(const std::string_view& name);

	private:

		static constexpr id_t UninitializedId = 0;
        static constexpr id_t ServerId = 1;
        static constexpr id_t FirstClientId = 2;

		bool handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        void handleControlMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        void handleRegularMessage(const DotsTransportHeader& transportHeader, Transmission&& transmission);
		void handleError(const std::exception& e);

        void processHello(const DotsMsgHello& hello);
        void processConnectResponse(const DotsMsgConnectResponse& connectResponse);
        void processEarlySubscribe(const DotsMsgConnectResponse& connectResponse);

		bool handleReceiveServer(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        bool handleControlMessageServer(const DotsTransportHeader& transportHeader, Transmission&& transmission);
        bool handleRegularMessageServer(const DotsTransportHeader& transportHeader, Transmission&& transmission);

        void processConnectRequest(const DotsMsgConnect& msg);
        void processConnectPreloadClientFinished(const DotsMsgConnect& msg);

		void setConnectionState(DotsConnectionState state);

		void importType(const type::Struct& instance);
		void exportType(const type::Descriptor<>& descriptor);

        inline static id_t M_nextClientId = FirstClientId;

		bool m_server;
		DotsConnectionState m_connectionState;
        id_t m_id;
		
		channel_ptr_t m_channel;
		io::Registry* m_registry;
        std::string m_name;
		descriptor_map_t m_preloadPublishTypes;
		descriptor_map_t m_preloadSubscribeTypes;
		receive_handler_t m_receiveHandler;
		error_handler_t m_errorHandler;
		
		std::set<std::string> m_sharedTypes;
	};

	using channel_connection_ptr_t = std::shared_ptr<Connection>;
}