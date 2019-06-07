#pragma once
#include <functional>
#include <dots/common/Chrono.h>
#include <dots/io/services/Timer.h>
#include <dots/io/Transceiver.h>

namespace dots
{
	Timer::id_t add_timer(const pnxs::chrono::Duration& timeout, const std::function<void()>& handler, bool periodic = false);
	void remove_timer(Timer::id_t id);

	void add_fd_handler(int fileDescriptor, const std::function<void()>& handler);
	void remove_fd_handler(int fileDescriptor);

	Transceiver& transceiver();

	void publish(const type::StructDescriptor* td, const type::Struct& instance, property_set what, bool remove);

	template<class T>
	void publish(const T& data, typename T::PropSet what)
	{
	    registerTypeUsage<T, PublishedType>();

	    static_assert(!data.isSubstructOnly(), "It is not allowed to publish a struct, that is marked with 'substruct_only'!");

	    onPublishObject->publish(T::_td(), &data, what, false);
	}

	template<class T>
	void publish(const T& data)
	{
	    registerTypeUsage<T, PublishedType>();

	    static_assert(!data.isSubstructOnly(), "It is not allowed to publish a struct, that is marked with 'substruct_only'!");

	    publish(data, data.valatt());
	}

	template<class T>
	void remove(const T& data)
	{
	    registerTypeUsage<T, PublishedType>();

	    static_assert(!data.isSubstructOnly(), "It is not allowed to remove a struct, that is marked with 'substruct_only'!");

	    onPublishObject->publish(T::_td(), &data, data.validProperties(), true);
	}

	Subscription subscribe(const type::StructDescriptor& descriptor, Dispatcher::receive_handler_t<>&& handler);

	Subscription subscribe(const type::StructDescriptor& descriptor, Dispatcher::event_handler_t<>&& handler);

	template<class T>
	Subscription subscribe(Dispatcher::receive_handler_t<T>&& handler)
	{
	    return transceiver().subscribe<T>(std::move(handler));
	}

	template<class T>
	Subscription subscribe(Dispatcher::event_handler_t<T>&& handler)
	{
	    return transceiver().subscribe<T>(std::move(handler));
	}

	const ContainerPool& pool();

	const Container<>& container(const type::StructDescriptor& descriptor);

	template <typename T>
	const Container<T>& container()
	{
		return transceiver().container<T>();
	}
}