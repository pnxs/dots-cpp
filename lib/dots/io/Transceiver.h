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
#include <dots/io/Registry.h>

namespace dots::type
{
	class StructDescriptorSet;
}

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

	const io::Registry& registry() const;
	io::Registry& registry();

	const ContainerPool& pool() const;
	const Container<>& container(const type::StructDescriptor<>& descriptor);

	Subscription subscribe(const type::StructDescriptor<>& descriptor, receive_handler_t<>&& handler);
    Subscription subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler);

	Subscription subscribe(const std::string_view& name, receive_handler_t<>&& handler);
	Subscription subscribe(const std::string_view& name, event_handler_t<>&& handler);

    ServerConnection& connection();

    type::StructDescriptorSet getPublishedDescriptors() const;
    type::StructDescriptorSet getSubscribedDescriptors() const;
    type::StructDescriptorSet getDescriptors() const;

    void subscribeDescriptors();

    bool connected() const;

    void publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t what, bool remove) override;

	template <typename T>
	const Container<T>& container()
	{
		return m_dispatcher.container<T>();
	}

	template<typename T>
	Subscription subscribe(receive_handler_t<T>&& handler)
	{
		static_assert(!T::_SubstructOnly, "it is not allowed to subscribe to a struct that is marked with 'substruct_only'!");
		connection().joinGroup(T::_Descriptor().name());
		return m_dispatcher.subscribe<T>(std::move(handler));
	}

	template<typename T>
	Subscription subscribe(event_handler_t<T>&& handler)
	{
		static_assert(!T::_SubstructOnly, "it is not allowed to subscribe to a struct that is marked with 'substruct_only'!");
		connection().joinGroup(T::_Descriptor().name());

		return m_dispatcher.subscribe<T>(std::move(handler));
	}

private:
    void onConnect();
    void onEarlySubscribe();
	const type::StructDescriptor<>& getDescriptorFromName(const std::string_view& name) const;

    ServerConnection m_serverConnection;

    bool m_connected = false;

	io::Registry m_registry;
    Dispatcher m_dispatcher;
    //Receiver m_receiver;
    //Transmitter m_transmitter;

};

}