#pragma once
#include <functional>
#include <string_view>
#include <dots/common/Chrono.h>
#include <dots/io/services/Timer.h>
#include <dots/io/GuestTransceiver.h>
#include <dots/io/Publisher.h>

namespace dots
{
	Timer::id_t add_timer(const pnxs::chrono::Duration& timeout, const std::function<void()>& handler, bool periodic = false);
	void remove_timer(Timer::id_t id);

	void add_fd_handler(int fileDescriptor, const std::function<void()>& handler);
	void remove_fd_handler(int fileDescriptor);

	Publisher*& publisher();
	GuestTransceiver& transceiver(const std::string_view& name = "dots-transceiver");

	void publish(const type::Struct& instance, types::property_set_t includedProperties, bool remove);
	void remove(const type::Struct& instance);

	template<typename T>
	void publish(const T& instance, const types::property_set_t& includedProperties = types::property_set_t::All, bool remove = false);

	template<typename  T>
	void remove(const T& instance);

	Subscription subscribe(const type::StructDescriptor<>& descriptor, GuestTransceiver::receive_handler_t<>&& handler);
	Subscription subscribe(const type::StructDescriptor<>& descriptor, GuestTransceiver::event_handler_t<>&& handler);

	template<typename T>
	Subscription subscribe(Dispatcher::receive_handler_t<T>&& handler);
	template<typename T>
	Subscription subscribe(Dispatcher::event_handler_t<T>&& handler);

	const ContainerPool& pool();

	const Container<>& container(const type::StructDescriptor<>& descriptor);
	template <typename T>
	const Container<T>& container();

	template<typename T>
	void publish(const T& instance, const types::property_set_t& includedProperties/* = types::property_set_t::All*/, bool remove/* = false*/)
	{
	    static_assert(!T::_IsSubstructOnly(), "it is not allowed to publish to a struct that is marked with 'substruct_only'!");
	    registerTypeUsage<T, PublishedType>();
	    publish(T::_Descriptor(), &instance, includedProperties, remove);
	}

	template<typename T>
	void remove(const T& instance)
	{
	    static_assert(!T::_IsSubstructOnly(), "it is not allowed to remove to a struct that is marked with 'substruct_only'!");
	    registerTypeUsage<T, PublishedType>();
	    remove(instance);
	}

	template<typename T>
	Subscription subscribe(Dispatcher::receive_handler_t<T>&& handler)
	{
		registerTypeUsage<T, SubscribedType>();
	    return transceiver().subscribe<T>(std::move(handler));
	}

	template<typename T>
	Subscription subscribe(Dispatcher::event_handler_t<T>&& handler)
	{
		registerTypeUsage<T, SubscribedType>();
	    return transceiver().subscribe<T>(std::move(handler));
	}

	template <typename T>
	const Container<T>& container()
	{
		registerTypeUsage<T, SubscribedType>();
		return transceiver().container<T>();
	}

	[[deprecated("only available for backwards compatibility")]]
	void publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t what, bool remove);
}