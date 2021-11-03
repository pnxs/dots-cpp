#pragma once
#include <functional>
#include <string_view>
#include <dots/type/Chrono.h>
#include <dots/Timer.h>
#include <dots/GuestTransceiver.h>
#include <dots/io/GlobalType.h>
#include <dots/serialization/StringSerializer.h>

namespace dots
{
    Timer::id_t add_timer(type::Duration timeout, std::function<void()> handler, bool periodic = false);
    void remove_timer(Timer::id_t id);

    #if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
    void add_fd_handler(int fileDescriptor, std::function<void()> handler);
    void remove_fd_handler(int fileDescriptor);
    #endif

    #ifndef DOTS_NO_GLOBAL_TRANSCEIVER

    GuestTransceiver& transceiver(std::string_view name = "dots-transceiver", bool reset = false);

    void publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false);
    void remove(const type::Struct& instance);

    template<typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int>>
    void publish(const T& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false);

    template<typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int>>
    void remove(const T& instance);

    Subscription subscribe(const type::StructDescriptor<>& descriptor, Transceiver::transmission_handler_t handler);
    Subscription subscribe(const type::StructDescriptor<>& descriptor, Transceiver::event_handler_t<> handler);

    template<typename T, typename EventHandler, typename... Args>
    Subscription subscribe(EventHandler&& handler, Args&&... args);

    const ContainerPool& pool();

    const Container<>& container(const type::StructDescriptor<>& descriptor);
    template <typename T>
    const Container<T>& container();

    template<typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    void publish(const T& instance, std::optional<types::property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        static_assert(!T::_SubstructOnly, "it is not allowed to publish to a struct that is marked with 'substruct_only'!");
        io::register_global_publish_type<T>();
        publish(static_cast<const type::Struct&>(instance), includedProperties, remove);
    }

    template<typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    void remove(const T& instance)
    {
        static_assert(!T::_SubstructOnly, "it is not allowed to remove to a struct that is marked with 'substruct_only'!");
        io::register_global_publish_type<T>();
        remove(static_cast<const type::Struct&>(instance));
    }

    template<typename T, typename EventHandler, typename... Args>
    Subscription subscribe(EventHandler&& handler, Args&&... args)
    {
        io::register_global_subscribe_type<T>();
        return transceiver().subscribe<T>(std::forward<EventHandler>(handler), std::forward<Args>(args)...);
    }

    template <typename T>
    const Container<T>& container()
    {
        io::register_global_subscribe_type<T>();
        return transceiver().container<T>();
    }

    #endif
}