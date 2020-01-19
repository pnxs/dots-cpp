#pragma once
#include <string_view>
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
        static constexpr id_t UninitializedId = 0;
        static constexpr id_t HostId = 1;
        static constexpr id_t FirstGuestId = 2;

		using receive_handler_t = std::function<bool(Connection&, const DotsTransportHeader&, Transmission&&, bool)>;
		using transition_handler_t = std::function<void(Connection&, const std::exception*)>;
		
		Connection(channel_ptr_t channel, bool host);
		Connection(const Connection& other) = delete;
		Connection(Connection&& other) = default;
		~Connection();

		Connection& operator = (const Connection& rhs) = delete;
		Connection& operator = (Connection&& rhs) = default;

		DotsConnectionState state() const;
		id_t selfId() const;
        id_t peerId() const;
		const std::string& peerName() const;
		bool connected() const;

		void asyncReceive(Registry& registry, const std::string_view& name, receive_handler_t&& receiveHandler, transition_handler_t&& transitionHandler);
		void transmit(const type::Struct& instance, types::property_set_t includedProperties = types::property_set_t::All, bool remove = false);
		void transmit(const DotsTransportHeader& header, const type::Struct& instance);
        void transmit(const DotsTransportHeader& header, const Transmission& transmission);
		void transmit(const type::StructDescriptor<>& descriptor);

	private:

		using system_type_t = std::tuple<const type::StructDescriptor<>*, types::property_set_t, std::function<void(const type::Struct&)>>;

		bool handleReceive(const DotsTransportHeader& transportHeader, Transmission&& transmission);
		void handleError(const std::exception& e);
		void handleClose(const std::exception* e);

        void handleHello(const DotsMsgHello& hello);
        void handleAuthorizationRequest(const DotsMsgConnectResponse& connectResponse);
        void handlePreloadFinished(const DotsMsgConnectResponse& connectResponse);

        void handleConnect(const DotsMsgConnect& connect);
        void handlePreloadClientFinished(const DotsMsgConnect& connect);

		void handlePeerError(const DotsMsgError& error);

		void setConnectionState(DotsConnectionState state, const std::exception* e = nullptr);

		void importType(const type::Struct& instance);
		void exportType(const type::Descriptor<>& descriptor);

		template <typename T>
		void expectSystemType(const types::property_set_t& expectedAttributes, void(Connection::* handler)(const T&));

        inline static id_t M_nextGuestId = FirstGuestId;

		system_type_t m_expectedSystemType;
		DotsConnectionState m_connectionState;
		id_t m_selfId;
        id_t m_peerId;
        std::string m_peerName;

		channel_ptr_t m_channel;

		Registry* m_registry;
		receive_handler_t m_receiveHandler;
		transition_handler_t m_transitionHandler;
		
		std::set<std::string> m_sharedTypes;
	};

    using connection_ptr_t = std::shared_ptr<Connection>;
}