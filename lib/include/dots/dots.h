#pragma once
#include <functional>
#include <string_view>
#include <boost/asio.hpp>
#include <dots/type/Chrono.h>
#include <dots/io/Timer.h>
#include <dots/io/GuestTransceiver.h>
#include <dots/io/Publisher.h>
#include <dots/io/GlobalType.h>

namespace dots
{
    using io::Timer;
    using io::Publisher;
    using io::Transceiver;
    using io::GuestTransceiver;
    using io::Subscription;
    using io::Container;
    using io::ContainerPool;
    using io::Event;

    Timer::id_t add_timer(const type::Duration& timeout, const std::function<void()>& handler, bool periodic = false);
    void remove_timer(Timer::id_t id);

    #if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
    void add_fd_handler(int fileDescriptor, const std::function<void()>& handler);
    void remove_fd_handler(int fileDescriptor);
    #endif

    #ifndef DOTS_NO_GLOBAL_TRANSCEIVER

    Publisher*& publisher();
    GuestTransceiver& transceiver(const std::string_view& name = "dots-transceiver");

    void publish(const type::Struct& instance, types::property_set_t includedProperties, bool remove);
    void remove(const type::Struct& instance);

    template<typename T>
    void publish(const T& instance, const types::property_set_t& includedProperties = types::property_set_t::All, bool remove = false);

    template<typename  T>
    void remove(const T& instance);

    Subscription subscribe(const type::StructDescriptor<>& descriptor, Transceiver::transmission_handler_t&& handler);
    Subscription subscribe(const type::StructDescriptor<>& descriptor, Transceiver::event_handler_t<>&& handler);

    template<typename T>
    Subscription subscribe(Transceiver::event_handler_t<T>&& handler);

    const ContainerPool& pool();

    const Container<>& container(const type::StructDescriptor<>& descriptor);
    template <typename T>
    const Container<T>& container();

    template<typename T>
    void publish(const T& instance, const types::property_set_t& includedProperties/* = types::property_set_t::All*/, bool remove/* = false*/)
    {
        static_assert(!T::_SubstructOnly, "it is not allowed to publish to a struct that is marked with 'substruct_only'!");
        io::register_global_publish_type<T>();
        publish(static_cast<const type::Struct&>(instance), includedProperties, remove);
    }

    template<typename T>
    void remove(const T& instance)
    {
        static_assert(!T::_SubstructOnly, "it is not allowed to remove to a struct that is marked with 'substruct_only'!");
        io::register_global_publish_type<T>();
        remove(static_cast<const type::Struct&>(instance));
    }

    template<typename T>
    Subscription subscribe(Transceiver::event_handler_t<T>&& handler)
    {
        io::register_global_subscribe_type<T>();
        return transceiver().subscribe<T>(std::move(handler));
    }

    template <typename T>
    const Container<T>& container()
    {
        io::register_global_subscribe_type<T>();
        return transceiver().container<T>();
    }

    [[deprecated("only available for backwards compatibility")]]
    void publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t what, bool remove);

    #endif
}