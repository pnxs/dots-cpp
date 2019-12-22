#pragma once
#include <string_view>
#include <map>
#include <set>
#include <dots/io/Dispatcher.h>
#include <dots/io/Subscription.h>
#include <dots/io/services/Channel.h>
#include <dots/io/Publisher.h>
#include <dots/io/Registry.h>
#include <DotsConnectionState.dots.h>
#include <DotsTransportHeader.dots.h>
#include <DotsMsgHello.dots.h>
#include <DotsMsgConnectResponse.dots.h>

namespace dots
{
	struct Transceiver : Publisher
	{
		template <typename T = type::Struct>
		using receive_handler_t = Dispatcher::receive_handler_t<T>;
		template <typename T = type::Struct>
		using event_handler_t = Dispatcher::event_handler_t<T>;

		using descriptor_map_t = std::map<std::string_view, type::StructDescriptor<>*>;

		Transceiver();
		Transceiver(const Transceiver& other) = delete;
		Transceiver(Transceiver&& other) = default;
		virtual ~Transceiver() = default;

		Transceiver& operator = (const Transceiver& rhs) = delete;
		Transceiver& operator = (Transceiver&& rhs) = default;

		bool openChannel(channel_ptr_t channel, std::string name, descriptor_map_t preloadPublishTypes = {}, descriptor_map_t preloadSubscribeTypes = {});
		void closeChannel();

		const io::Registry& registry() const;
		io::Registry& registry();

		const ContainerPool& pool() const;
		const Container<>& container(const type::StructDescriptor<>& descriptor);

		Subscription subscribe(const type::StructDescriptor<>& descriptor, receive_handler_t<>&& handler);
		Subscription subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler);

		Subscription subscribe(const std::string_view& name, receive_handler_t<>&& handler);
		Subscription subscribe(const std::string_view& name, event_handler_t<>&& handler);

		DotsConnectionState connectionState() const;
        uint32_t id() const;
		bool connected() const;

		void publish(const type::Struct& instance, types::property_set_t what = types::property_set_t::All, bool remove = false);
		void remove(const type::Struct& instance);

		template <typename T>
		const Container<T>& container()
		{
			return m_dispatcher.container<T>();
		}

		template<typename T>
		Subscription subscribe(receive_handler_t<T>&& handler)
		{
			static_assert(!T::_SubstructOnly, "it is not allowed to subscribe to a struct that is marked with 'substruct_only'!");
			joinGroup(T::_Descriptor().name());
			return m_dispatcher.subscribe<T>(std::move(handler));
		}

		template<typename T>
		Subscription subscribe(event_handler_t<T>&& handler)
		{
			static_assert(!T::_SubstructOnly, "it is not allowed to subscribe to a struct that is marked with 'substruct_only'!");
			joinGroup(T::_Descriptor().name());

			return m_dispatcher.subscribe<T>(std::move(handler));
		}

		[[deprecated("only available for backwards compatibility")]]
		void publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t includedProperties, bool remove) override;

	private:

		void joinGroup(const std::string_view& name);
		void leaveGroup(const std::string_view& name);
		
		void onEarlySubscribe();

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

		io::Registry m_registry;
		Dispatcher m_dispatcher;
		
		DotsConnectionState m_connectionState;
        uint32_t m_id;
        std::string m_name;
		
        channel_ptr_t m_channel;
		descriptor_map_t m_preloadPublishTypes;
		descriptor_map_t m_preloadSubscribeTypes;

		std::set<std::string> m_sharedTypes;
	};
}