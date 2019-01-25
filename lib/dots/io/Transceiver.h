#pragma once

#include "dots/cpp_config.h"
#include "Dispatcher.h"
#include "TD_Traversal.h"
#include "Transmitter.h"
#include "ServerConnection.h"
#include "Subscription.h"
#include "DotsSocket.h"
#include "Publisher.h"


namespace dots
{

extern Publisher* onPublishObject;

class Transceiver: public Publisher
{
public:
    Transceiver();

    bool start(const string &name, const string &host, int port, DotsSocketPtr dotsSocket);
    void stop();

    Subscription addReceiver(const type::StructDescriptor* td, ContainerBase* cb, const Dispatcher::callback_type& f)
    {
        connection().joinGroup(td->name());
        return dispatcher().addReceiver(td, cb, f);
    }

    Subscription addTypelessReceiver(const type::StructDescriptor* td, const Dispatcher::typeless_callback_type& f)
    {
        connection().joinGroup(td->name());
        return dispatcher().addTypelessReceiver(td, f);
    }

    Dispatcher& dispatcher();
    ServerConnection& connection();

    type::StructDescriptorSet getPublishedDescriptors() const;
    type::StructDescriptorSet getSubscribedDescriptors() const;
    type::StructDescriptorSet getDescriptors() const;

    void subscribeDescriptors();

    bool connected() const;

    void publish(const type::StructDescriptor* td, CTypeless data, property_set what, bool remove) override;

private:
    void onConnect();
    void onEarlySubscribe();

    ServerConnection m_serverConnection;

    bool m_connected = false;

    Dispatcher m_dispatcher;

    //Receiver m_receiver;
    //Transmitter m_transmitter;

};

Transceiver& transceiver();

void publish(const type::StructDescriptor* td, CTypeless data, property_set what, bool remove);

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

template<class T>
Subscription subscribe(const function<void (const Cbd<T>&)>& callback)
{
    return transceiver().dispatcher().addReceiver(callback);
}

}