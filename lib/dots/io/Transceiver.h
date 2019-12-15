#pragma once

#include <string_view>
#include <map>
#include "dots/cpp_config.h"
#include "Dispatcher.h"
#include "TD_Traversal.h"
#include "Transmitter.h"
#include "ServerConnection.h"
#include "Subscription.h"
#include <dots/io/services/Channel.h>
#include "Publisher.h"
#include <dots/io/Registry.h>

namespace dots::type
{
	class StructDescriptorSet;
}

namespace dots
{
	extern Publisher* onPublishObject;

	struct Transceiver : Publisher
	{
		template <typename T = type::Struct>
		using receive_handler_t = Dispatcher::receive_handler_t<T>;
		template <typename T = type::Struct>
		using event_handler_t = Dispatcher::event_handler_t<T>;

		using descriptor_map_t = std::map<std::string_view, type::StructDescriptor<>*>;

		Transceiver();

		bool start(const std::string& name, channel_ptr_t channel, descriptor_map_t preloadPublishTypes, descriptor_map_t preloadSubscribeTypes);
		void stop();

		const io::Registry& registry() const;
		io::Registry& registry();

		const ContainerPool& pool() const;
		const Container<>& container(const type::StructDescriptor<>& descriptor);

		Subscription subscribe(const type::StructDescriptor<>& descriptor, receive_handler_t<>&& handler);
		Subscription subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler);

		Subscription subscribe(const std::string_view& name, receive_handler_t<>&& handler);
		Subscription subscribe(const std::string_view& name, event_handler_t<>&& handler);

		ServerConnection& connection();

		bool connected() const;

		void publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t includedProperties, bool remove) override;
		void publish(const type::Struct& instance, types::property_set_t what = types::property_set_t::All, bool remove = false);

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

	private:

		void joinGroup(const std::string_view& name);
		void leaveGroup(const std::string_view& name);
		
		void onConnect();
		void onEarlySubscribe();

		ServerConnection m_serverConnection;

		bool m_connected = false;

		io::Registry m_registry;
		Dispatcher m_dispatcher;
		descriptor_map_t m_preloadPublishTypes;
		descriptor_map_t m_preloadSubscribeTypes;
	};
}