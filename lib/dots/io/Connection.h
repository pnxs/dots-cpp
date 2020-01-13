#pragma once
#include <string>
#include <map>
#include <set>
#include <tuple>
#include <dots/io/services/Channel.h>
#include <DotsConnectionState.dots.h>
#include <DotsTransportHeader.dots.h>
#include <DotsMsgHello.dots.h>
#include <DotsMsgConnectResponse.dots.h>
#include <DotsMsgConnect.dots.h>
#include <DotsMsgError.dots.h>

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

		using receive_handler_t = std::function<bool(Connection&, const DotsTransportHeader&, Transmission&&, bool)>;
		using close_handler_t = std::function<void(Connection&, const std::exception*)>;
		using descriptor_map_t = std::map<std::string_view, type::StructDescriptor<>*>;
		
		Connection(channel_ptr_t channel, bool server, descriptor_map_t preloadPublishTypes = {}, descriptor_map_t preloadSubscribeTypes = {});
		Connection(const Connection& other) = delete;
		Connection(Connection&& other) = default;
		~Connection();

		Connection& operator = (const Connection& rhs) = delete;
		Connection& operator = (Connection&& rhs) = default;

		DotsConnectionState state() const;
        id_t id() const;
		const std::string& name() const;
		bool connected() const;

		void asyncReceive(Registry& registry, const std::string& name, receive_handler_t&& receiveHandler, close_handler_t&& closeHandler);
		void transmit(const type::Struct& instance, types::property_set_t includedProperties = types::property_set_t::All, bool remove = false);
		void transmit(const DotsTransportHeader& header, const type::Struct& instance);
        void transmit(const DotsTransportHeader& header, const Transmission& transmission);

		void joinGroup(const std::string_view& name);
		void leaveGroup(const std::string_view& name);

	private:

		using system_type_t = std::tuple<const type::StructDescriptor<>*, types::property_set_t, std::function<void(const type::Struct&)>>;

		static constexpr id_t UninitializedId = 0;
        static constexpr id_t ServerId = 1;
        static constexpr id_t FirstClientId = 2;

		bool handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission);
		void handleError(const std::exception& e);
		void handleClose(const std::exception* e);

        void handleHello(const DotsMsgHello& hello);
        void handleAuthorizationRequest(const DotsMsgConnectResponse& connectResponse);
        void handlePreloadFinished(const DotsMsgConnectResponse& connectResponse);

        void handleConnect(const DotsMsgConnect& connect);
        void handlePreloadClientFinished(const DotsMsgConnect& connect);

		void handlePeerError(const DotsMsgError& error);

		void setConnectionState(DotsConnectionState state);

		void importType(const type::Struct& instance);
		void exportType(const type::Descriptor<>& descriptor);

		template <typename T>
		void expectSystemType(const types::property_set_t& expectedAttributes, void(Connection::* handler)(const T&));

        inline static id_t M_nextClientId = FirstClientId;

		system_type_t m_expectedSystemType;
		DotsConnectionState m_connectionState;
        id_t m_id;
        std::string m_name;

		channel_ptr_t m_channel;
		bool m_server;
		descriptor_map_t m_preloadPublishTypes;
		descriptor_map_t m_preloadSubscribeTypes;

		Registry* m_registry;
		receive_handler_t m_receiveHandler;
		close_handler_t m_closeHandler;
		
		std::set<std::string> m_sharedTypes;
	};

    using connection_ptr_t = std::shared_ptr<Connection>;
}