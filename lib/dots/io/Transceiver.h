#pragma once

#include <string_view>
#include "dots/cpp_config.h"
#include "Dispatcher.h"
#include "TD_Traversal.h"
#include "Transmitter.h"
#include "ServerConnection.h"
#include "Subscription.h"
#include <dots/io/services/Channel.h>
#include "Publisher.h"


namespace dots
{

extern Publisher* onPublishObject;

class Transceiver: public Publisher
{
public:

	template <typename T = type::Struct>
	using receive_handler_t = Dispatcher::receive_handler_t<T>;
	template <typename T = type::Struct>
	using event_handler_t = Dispatcher::event_handler_t<T>;

    Transceiver();

    bool start(const string &name, channel_ptr_t channel);
    void stop();

	const ContainerPool& pool() const;
	const Container<>& container(const type::StructDescriptor& descriptor);

	Subscription subscribe(const type::StructDescriptor& descriptor, receive_handler_t<>&& handler);
    Subscription subscribe(const type::StructDescriptor& descriptor, event_handler_t<>&& handler);

	Subscription subscribe(const std::string_view& name, receive_handler_t<>&& handler);
	Subscription subscribe(const std::string_view& name, event_handler_t<>&& handler);

    ServerConnection& connection();

    type::StructDescriptorSet getPublishedDescriptors() const;
    type::StructDescriptorSet getSubscribedDescriptors() const;
    type::StructDescriptorSet getDescriptors() const;

    void subscribeDescriptors();

    bool connected() const;

    void publish(const type::StructDescriptor* td, const type::Struct& instance, property_set what, bool remove) override;

	template <typename T>
	const Container<T>& container()
	{
		return m_dispatcher.container<T>();
	}

	template<typename T>
	Subscription subscribe(receive_handler_t<T>&& handler)
	{
		static_assert(!T::_IsSubstructOnly(), "it is not allowed to subscribe to a struct that is marked with 'substruct_only'!");
		
		registerTypeUsage<T, SubscribedType>();
		connection().joinGroup(T::_Descriptor().name());

		return m_dispatcher.subscribe<T>(std::move(handler));
	}

	template<typename T>
	Subscription subscribe(event_handler_t<T>&& handler)
	{
		static_assert(!T::_IsSubstructOnly(), "it is not allowed to subscribe to a struct that is marked with 'substruct_only'!");

		registerTypeUsage<T, SubscribedType>();
		connection().joinGroup(T::_Descriptor().name());

		return m_dispatcher.subscribe<T>(std::move(handler));
	}

private:
    void onConnect();
    void onEarlySubscribe();
	const type::StructDescriptor& getDescriptorFromName(const std::string_view& name) const;

    ServerConnection m_serverConnection;

    bool m_connected = false;

    Dispatcher m_dispatcher;

    //Receiver m_receiver;
    //Transmitter m_transmitter;

};

Transceiver& transceiver();

void publish(const type::StructDescriptor* td, const type::Struct& instance, property_set what, bool remove);

template<class T>
void publish(const T& data, typename T::PropSet what)
{
    registerTypeUsage<T, PublishedType>();

    static_assert(not data.isSubstructOnly(), "It is not allowed to publish a struct, that is marked with 'substruct_only'!");

    onPublishObject->publish(T::_td(), &data, what, false);
}

template<class T>
void publish(const T& data)
{
    registerTypeUsage<T, PublishedType>();

    static_assert(not data.isSubstructOnly(), "It is not allowed to publish a struct, that is marked with 'substruct_only'!");

    publish(data, data.valatt());
}

template<class T>
void remove(const T& data)
{
    registerTypeUsage<T, PublishedType>();

    static_assert(not data.isSubstructOnly(), "It is not allowed to remove a struct, that is marked with 'substruct_only'!");

    onPublishObject->publish(T::_td(), &data, data.validProperties(), true);
}

inline Subscription subscribe(const type::StructDescriptor& descriptor, Dispatcher::receive_handler_t<>&& handler)
{
    return transceiver().subscribe(descriptor, std::move(handler));
}

inline Subscription subscribe(const type::StructDescriptor& descriptor, Dispatcher::event_handler_t<>&& handler)
{
    return transceiver().subscribe(descriptor, std::move(handler));
}

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

inline const ContainerPool& pool()
{
	return transceiver().pool();
}

inline const Container<>& container(const type::StructDescriptor& descriptor)
{
	return transceiver().container(descriptor);
}

template <typename T>
const Container<T>& container()
{
	return transceiver().container<T>();
}

}