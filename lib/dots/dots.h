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

	void publish(const type::NewStructDescriptor<>* td, const type::NewStruct& instance, types::property_set_t what, bool remove);

	template<class T>
	void publish(const T& instance, const types::property_set_t& what = types::property_set_t::All, bool remove = false);
	template<class T>
	void publish(const T& data);

	template<class T>
	void remove(const T& data);

	Subscription subscribe(const type::NewStructDescriptor<>& descriptor, Transceiver::receive_handler_t<>&& handler);
	Subscription subscribe(const type::NewStructDescriptor<>& descriptor, Transceiver::event_handler_t<>&& handler);

	template<class T>
	Subscription subscribe(Dispatcher::receive_handler_t<T>&& handler);
	template<class T>
	Subscription subscribe(Dispatcher::event_handler_t<T>&& handler);

	const ContainerPool& pool();

	const Container<>& container(const type::NewStructDescriptor<>& descriptor);
	template <typename T>
	const Container<T>& container();

	template<class T>
	void publish(const T& instance, const types::property_set_t& what/* = types::property_set_t::All*/, bool remove/* = false*/)
	{
	    static_assert(!T::_IsSubstructOnly(), "it is not allowed to publish to a struct that is marked with 'substruct_only'!");
	    registerTypeUsage<T, PublishedType>();
	    onPublishObject->publish(T::_Descriptor(), &instance, what, remove);
	}

	template<class T>
	void remove(const T& instance)
	{
	    static_assert(!T::_IsSubstructOnly(), "it is not allowed to remove to a struct that is marked with 'substruct_only'!");
	    registerTypeUsage<T, PublishedType>();
	    onPublishObject->publish(T::_Descriptor(), &instance, instance.validProperties(), true);
	}

	template<class T>
	Subscription subscribe(Dispatcher::receive_handler_t<T>&& handler)
	{
		registerTypeUsage<T, SubscribedType>();
	    return transceiver().subscribe<T>(std::move(handler));
	}

	template<class T>
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
}