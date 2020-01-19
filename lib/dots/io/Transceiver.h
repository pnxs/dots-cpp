#pragma once
#include <string_view>
#include <optional>
#include <dots/io/Connection.h>
#include <dots/io/Dispatcher.h>
#include <dots/io/Subscription.h>
#include <dots/io/Publisher.h>
#include <dots/io/Registry.h>

namespace dots
{
	struct Transceiver : Publisher
	{
		using descriptor_map_t = std::map<std::string_view, type::StructDescriptor<>*>;
		using new_struct_type_handler_t = io::Registry::new_struct_type_handler_t;

		template <typename T = type::Struct>
		using receive_handler_t = Dispatcher::receive_handler_t<T>;
		template <typename T = type::Struct>
		using event_handler_t = Dispatcher::event_handler_t<T>;

		Transceiver(std::string selfName, new_struct_type_handler_t newStructTypeHandler);
		Transceiver(const Transceiver& other) = delete;
		Transceiver(Transceiver&& other) = default;
		virtual ~Transceiver() = default;

		Transceiver& operator = (const Transceiver& rhs) = delete;
		Transceiver& operator = (Transceiver&& rhs) = default;

		const std::string& selfName() const;

		const io::Registry& registry() const;
		io::Registry& registry();

		const ContainerPool& pool() const;
		const Container<>& container(const type::StructDescriptor<>& descriptor);

		Subscription subscribe(const type::StructDescriptor<>& descriptor, receive_handler_t<>&& handler);
		Subscription subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler);

		Subscription subscribe(const std::string_view& name, receive_handler_t<>&& handler);
		Subscription subscribe(const std::string_view& name, event_handler_t<>&& handler);

		virtual void publish(const type::Struct& instance, types::property_set_t what = types::property_set_t::All, bool remove = false) = 0;
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

	protected:

		Dispatcher& dispatcher();

	private:

        virtual void joinGroup(const std::string_view& name) = 0;
		virtual void leaveGroup(const std::string_view& name) = 0;

		io::Registry m_registry;
		Dispatcher m_dispatcher;
		std::string m_selfName;
	};
}