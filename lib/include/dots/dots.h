#pragma once

/*!
 * @file dots.h
 *
 * @brief Global DOTS API.
 *
 * This header provides access to all functions of the global DOTS API.
 * It is intended for simple use cases in which it is not necessary to
 * use specific DOTS transceivers or IO contexts (i.e. event loops).
 * The header is usually used alongside a dots::Application.
 *
 * The interface is similar to that of the GuestTransceiver, but
 * usually results in less verbose implementations, because it avoids
 * having to make a specific transceiver available to all components.
 *
 * However, beware that the global API introduces a global state, which
 * might make it undesirable for more complex applications.
 *
 * To avoid unintentional usage of the global transceiver, the
 * corresponding functions can be hidden by adding the definition
 * DOTS_NO_GLOBAL_TRANSCEIVER to the preprocessor.
 */

#include <string_view>
#include <dots/tools/Handler.h>
#include <dots/type/Chrono.h>
#include <dots/Timer.h>
#include <dots/GuestTransceiver.h>
#include <dots/io/GlobalType.h>
#include <dots/serialization/StringSerializer.h>

namespace dots
{
    /*!
     * @brief Add a timer to the global timer service.
     *
     * This will create and add a dots::Timer to the dots::io::TimerService
     * associated with the global IO context (see
     * dots::io::global_io_context()).
     *
     * The timer will be processed asynchronously when the global IO
     * context is running and invoke the given handler after the given
     * duration has passed.
     *
     * It is recommended to use the (DOTS) chrono literals to specify the
     * duration:
     *
     * @code{.cpp}
     * using namespace dots::literals;
     *
     * dots::add_timer(500ms, []
     * {
     *     // ...
     * });
     * @endcode
     *
     * @param timeout The duration of the timer (e.g. 500ms).
     *
     * @param handler The handler to invoke asynchronously after the timer
     * runs out.
     *
     * @param periodic Specifies whether the timer will be restarted after
     * it ran out and @p handler was invoked.
     *
     * @return Timer::id_t The unique id that identifies the timer. This
     * can be used to cancel the timer prematurely (see
     * dots::remove_timer()).
     */
    [[deprecated("superseded by managed timers (see dots::create_timer())")]]
    Timer::id_t add_timer(type::Duration timeout, tools::Handler<void()> handler, bool periodic = false);

    /*!
     * @brief Remove an active timer from the global timer service.
     *
     * This will attempt to remove an active timer from the
     * dots::io::TimerService associated with the global IO context (see
     * dots::io::global_io_context()).
     *
     * Note that calling this function has no effect if no active timer
     * with the given id exists.
     *
     * @param id The id of the timer to remove.
     */
    [[deprecated("superseded by managed timers (see dots::create_timer())")]]
    void remove_timer(Timer::id_t id);

    /*!
     * @brief Create a global timer.
     *
     * This will create a dots::Timer associated with the global IO context
     * (see dots::io::global_io_context()).
     *
     * The timer will be processed asynchronously when the global IO
     * context is running and invoke the given handler after the given
     * duration has passed.
     *
     * It is recommended to use the (DOTS) chrono literals to specify the
     * duration:
     *
     * @code{.cpp} using namespace dots::literals;
     *
     * dots::Timer timer = dots::create_timer(500ms, []
     * {
     *     // ...
     * });
     * @endcode
     *
     * @param timeout The duration of the timer (e.g. 500ms).
     *
     * @param handler The handler to invoke asynchronously after the timer
     * runs out.
     *
     * @param periodic Specifies whether the timer will be restarted after
     * it ran out and @p handler was invoked.
     *
     * @return Timer The Timer object that manages the state of the timer.
     * The timer will stay active until the object is destroyed or the
     * timer runs out.
     */
    Timer create_timer(type::Duration timeout, tools::Handler<void()> handler, bool periodic = false);

    #ifndef DOTS_NO_GLOBAL_TRANSCEIVER

    /*!
     * @brief Get the global guest transceiver.
     *
     * This retrieves the current instance of the global GuestTransceiver.
     *
     * When using a dots::Application, the global transceiver will be
     * initialized automatically and must not be accessed before the
     * application's construction.
     *
     * When not using dots::Application, the global transceiver has to be
     * created manually.
     *
     * @warning For the global API to work as intended, the global
     * transceiver has to operate on the global IO context (see
     * dots::io::global_io_context()) and has to have the type policy
     * type::Registry::StaticTypePolicy::All. It is the user's
     * responsibility to ensure this.
     *
     * @return std::optional<GuestTransceiver>& A reference to the global
     * guest transceiver.
     */
    std::optional<GuestTransceiver>& global_transceiver();

    /*!
     * @brief Publish an instance of a DOTS struct type via the global
     * transceiver.
     *
     * This will effectively call GuestTransceiver::publish() on the global
     * transceiver returned by dots::global_transceiver().
     *
     * Note that this will neither directly invoke any callbacks of local
     * subscribers nor have any effect on the local container. If the
     * transceiver itself has a subscription to the instance type, the
     * instance will be dispatched once it is acknowledged (i.e. send back)
     * by the host.
     *
     * @param instance The instance to publish.
     *
     * @param includedProperties The properties to publish in addition to
     * the key properties. If no set is given, the valid property set of
     * @p instance will be used.
     *
     * @param remove Specifies whether the publish is a remove.
     *
     * @exception std::logic_error Thrown if @p instance is of a
     * 'substruct-only' type.
     *
     * @exception std::runtime_error Thrown if a key property of the
     * instance is invalid.
     *
     * @exception std::runtime_error Thrown if no host connection has been
     * established when the function is called.
     */
    void publish(const type::Struct& instance, std::optional<property_set_t> includedProperties = std::nullopt, bool remove = false);

    /*!
     * @brief Remove an instance of a DOTS struct type via the global
     * transceiver.
     *
     * This will effectively call dots::publish() with the remove flag set
     * to true and only including the key properties of the type.
     *
     * @param instance The instance to remove.
     */
    void remove(const type::Struct& instance);

    /*!
     * @brief Publish an instance of a DOTS struct type via the global
     * transceiver.
     *
     * This will effectively call dots::publish() with the given arguments.
     *
     * Instantiating this template will also register \p T as a global
     * publish type. When using the dots::Application, this will result in
     * the type's descriptor being transmitted while the connection is
     * being established.
     *
     * @tparam T The type of the DOTS struct to publish. Must not be
     * qualified as "substruct-only".
     *
     * @param instance The instance to publish.
     *
     * @param includedProperties The properties to publish in addition to
     * the key properties. If no set is given, the valid property set of
     * @p instance will be used.
     *
     * @param remove Specifies whether the publish is a remove.
     *
     * @exception std::runtime_error Thrown if a key property of the
     * instance is invalid.
     *
     * @exception std::runtime_error Thrown if no host connection has been
     * established when the function is called.
     */
    template<typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    void publish(const T& instance, std::optional<property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        static_assert(!T::_SubstructOnly, "it is not allowed to publish to a struct that is marked with 'substruct_only'!");
        io::register_global_publish_type<T>();
        publish(static_cast<const type::Struct&>(instance), includedProperties, remove);
    }

    /*!
     * @brief Remove an instance of a DOTS struct type via the global
     * transceiver.
     *
     * This will effectively call dots::remove() with the given argument.
     *
     * Instantiating this template will also register \p T as a global
     * publish type. When using the dots::Application, this will result in
     * the type's descriptor being transmitted while the connection is
     * being established.
     *
     * @tparam T The type of the DOTS struct to publish. Must not be
     * qualified as "substruct-only".
     *
     * @param instance The instance to remove.
     *
     * @exception std::runtime_error Thrown if a key property of the
     * instance is invalid.
     *
     * @exception std::runtime_error Thrown if no host connection has been
     * established when the function is called.
     */
    template<typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    void remove(const T& instance)
    {
        static_assert(!T::_SubstructOnly, "it is not allowed to remove to a struct that is marked with 'substruct_only'!");
        io::register_global_publish_type<T>();
        remove(static_cast<const type::Struct&>(instance));
    }

    /*!
     * @brief Subscribe to events of a specific type via the global
     * transceiver.
     *
     * This will effectively call GuestTransceiver::subscribe() on the
     * global transceiver returned by dots::global_transceiver().
     *
     * Calling this function will create a subscription to a given type and
     * cause the given handler to be invoked asynchronously every time a
     * corresponding DOTS event occurs. For cached types, events are
     * created after the local Container has been updated.
     *
     * @attention Unless otherwise required, users are advised to use the
     * explicitly typed versions of Transceiver::subscribe() instead.
     *
     * @param descriptor The type to subscribe to.
     *
     * @param handler The handler to invoke asynchronously every time a
     * corresponding DOTS event occurs. If the given type is a cached type
     * and the corresponding Container is not empty, the given handler will
     * also be invoked synchronously with create events for each contained
     * instance before this function returns.
     *
     * @return Subscription The Subscription object that manages the state
     * of the subscription. The subscription will stay active until the
     * object is destroyed or Subscription::unsubscribe() is called
     * manually.
     *
     * @exception std::logic_error Thrown if @p descriptor is a sub-struct
     * only type.
     */
    Subscription subscribe(const type::StructDescriptor<>& descriptor, Transceiver::event_handler_t<> handler);

    /*!
     * @brief Subscribe to events of a specific type via the global
     * transceiver.
     *
     * This will effectively call GuestTransceiver::subscribe() on the
     * global transceiver returned by dots::global_transceiver().
     *
     * Calling this function will create a subscription to a given type and
     * cause the given handler to be invoked asynchronously every time a
     * corresponding DOTS event occurs. For cached types, events are
     * created after the local Container has been updated.
     *
     * Instantiating this template will also register \p T as a global
     * subscribe type. When using the dots::Application, this will result
     * in the type's cache being preloaded while the connection is being
     * established
     *
     * Note that the @p handler can be any compatible invocable object,
     * including lambdas and class member functions:
     *
     * @code{.cpp}
     * // subscribing to events of a DOTS struct type Foobar with lambda handler
     * dots::subscribe<Foobar>([](const dots::Event<Foobar>& event)
     * {
     *     // ...
     * });
     *
     * // subscribing to events of a DOTS struct type Foobar member function
     * dots::subscribe<Foobar>({ &SomeClass::handleFoobar, this });
     * @endcode
     *
     * @tparam T The type to subscribe to.
     *
     * @param handler The handler to invoke asynchronously every time a
     * corresponding DOTS event occurs. If the given type is a cached type
     * and the corresponding Container is not empty, the given handler will
     * also be invoked synchronously with create events for each contained
     * instance before this function returns.
     *
     * @return Subscription The Subscription object that manages the state
     * of the subscription. The subscription will stay active until the
     * object is destroyed or Subscription::unsubscribe() is called
     * manually.
     */
    template<typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    Subscription subscribe(Transceiver::event_handler_t<T> handler)
    {
        io::register_global_subscribe_type<T>();
        return global_transceiver()->subscribe<T>(std::move(handler));
    }

    /*!
     * @brief Subscribe to new types of a specific category via the global
     * transceiver.
     *
     * This will effectively call GuestTransceiver::subscribe() on the
     * global transceiver returned by dots::global_transceiver().
     *
     * Calling this will create a subscription to new types of a given
     * category and cause the given handler to be invoked whenever a
     * corresponding type is added to the registry.
     *
     * The descriptor category can be specified as a DOTS descriptor type.
     * Additionally, @p handler can be any compatible invocable object.
     *
     * For example:
     *
     * @code{.cpp}
     * // subscribing to new struct types with lambda handler
     * dots::subscribe<dots::type::StructDescriptor<>>([](const auto& descriptor)
     * {
     *     // ...
     * });
     *
     * // subscribing to new vector types with member function
     * dots::subscribe<dots::type::VectorDescriptor>({ &SomeClass::handleNewVector, this });
     * @endcode
     *
     * @tparam TDescriptor The descriptor type (e.g.
     * dots::type::StructDescriptor<>).
     *
     * @param handler The handler to invoke asynchronously every time a
     * type of the given category is added to the registry. If the registry
     * already contains types of the given category, the given handler will
     * also be invoked synchronously with all such currently known types
     * before this function returns.
     *
     * @return Subscription The Subscription object that manages the state
     * of the subscription. The subscription will stay active until the
     * object is destroyed or Subscription::unsubscribe() is called
     * manually.
     */
    template <typename TDescriptor, std::enable_if_t<std::is_base_of_v<type::Descriptor<>, TDescriptor>, int> = 0>
    Subscription subscribe(Transceiver::new_type_handler_t<TDescriptor> handler)
    {
        return global_transceiver()->subscribe<TDescriptor>(std::move(handler));
    }

    /*!
     * @brief Subscribe to events of a specific type via the global
     * transceiver.
     *
     * This will effectively call GuestTransceiver::subscribe() on the
     * global transceiver returned by dots::global_transceiver().
     *
     * Calling this function will create a subscription to a given type and
     * cause the given handler to be invoked asynchronously every time a
     * corresponding DOTS event occurs. For cached types, events are
     * created after the local Container has been updated.
     *
     * Instantiating this template will also register \p T as a global
     * subscribe type. When using the dots::Application, this will result
     * in the type's cache being preloaded while the connection is being
     * established
     *
     * Note that the @p handler can be any compatible invocable object,
     * including lambdas and class member functions:
     *
     * @code{.cpp}
     * // subscribing to events of a DOTS struct type Foobar with lambda handler
     * dots::subscribe<Foobar>([](const dots::Event<Foobar>& event)
     * {
     *     // ...
     * });
     *
     * // subscribing to events of a DOTS struct type Foobar member function
     * dots::subscribe<Foobar>(&SomeClass::handleFoobar, this);
     * @endcode
     *
     * @tparam T The type to subscribe to.
     *
     * @tparam EventHandler The type of the handler. Must be be invocable
     * with a constant reference to Event<T> and optionally @p args if
     * given.
     *
     * @tparam Args The types of the optional arguments to the handler.
     *
     * @param handler The handler to invoke asynchronously every time a
     * corresponding DOTS event occurs. If the given type is a cached type
     * and the corresponding Container is not empty, the given handler will
     * also be invoked synchronously with create events for each contained
     * instance before this function returns.
     *
     * @param args Optional arguments that will be bound and passed to the
     * handler upon invocation (e.g. 'this' when specifying a class member
     * function as @p handler ).
     *
     * @return Subscription The Subscription object that manages the state
     * of the subscription. The subscription will stay active until the
     * object is destroyed or Subscription::unsubscribe() is called
     * manually.
     */
    template<typename T, typename EventHandler, typename... Args, std::enable_if_t<sizeof...(Args) >= 1 && std::is_base_of_v<type::Struct, T>, int> = 0>
    [[deprecated("superseded by dots::subscribe(Transceiver::event_handler_t<T>)")]]
    Subscription subscribe(EventHandler&& handler, Args&&... args)
    {
        return subscribe<T>(Transceiver::event_handler_t<T>{ std::forward<EventHandler>(handler), std::forward<Args>(args)... });
    }

    /*!
     * @brief Subscribe to new types of specific categories via the global
     * transceiver.
     *
     * This will effectively call GuestTransceiver::subscribe() on the
     * global transceiver returned by dots::global_transceiver().
     *
     * Calling this function will create a subscription to new types of
     * given categories and cause the given handler to be invoked whenever
     * a corresponding type is added to the registry.
     *
     * The descriptor categories can be specified as an arbitrary
     * combination of DOTS descriptor types. Additionally, @p handler can
     * be any compatible invocable object.
     *
     * For example:
     *
     * @code{.cpp}
     * // subscribing to new struct and enum types with lambda handler
     * dots::subscribe<dots::type::StructDescriptor<>, dots::type::EnumDescriptor<>>([](const auto& descriptor)
     * {
     *     // ...
     * });
     *
     * // subscribing to new vector types with member function
     * dots::subscribe<dots::type::VectorDescriptor>(&SomeClass::handleNewVector, this);
     * @endcode
     *
     * @tparam TDescriptors The descriptor types (e.g.
     * dots::type::StructDescriptor<>).
     *
     * @tparam TypeHandler The type of the handler. Must be invocable with
     * references of all @p TDescriptor types and optionally @p args if
     * given.
     *
     * @tparam Args The types of the optional arguments to the handler.
     *
     * @param handler The handler to invoke asynchronously every time a
     * type of the given categories is added to the registry. If the
     * registry already contains types of the given categories, the given
     * handler will also be invoked synchronously with all such currently
     * known types before this function returns.
     *
     * @param args Optional arguments that will be bound and passed to the
     * handler upon invocation (e.g. 'this' when specifying a class member
     * function as @p handler ).
     *
     * @return Subscription The Subscription object that manages the state
     * of the subscription. The subscription will stay active until the
     * object is destroyed or Subscription::unsubscribe() is called
     * manually.
     */
    template <typename... TDescriptors, typename TypeHandler, typename... Args, std::enable_if_t<sizeof...(Args) >= 1 && sizeof...(TDescriptors) >= 1 && std::conjunction_v<std::is_base_of<type::Descriptor<>, TDescriptors>...>, int> = 0>
    [[deprecated("superseded by dots::subscribe(Transceiver::new_type_handler_t<TDescriptor>)")]]
    Subscription subscribe(TypeHandler&& handler, Args&&... args)
    {
        return global_transceiver()->subscribe<TDescriptors...>(std::forward<TypeHandler>(handler), std::forward<Args>(args)...);
    }

    /*!
     * @brief Get the container pool of the global transceiver.
     *
     * This will effectively call GuestTransceiver::pool() on the global
     * transceiver returned by dots::global_transceiver().
     *
     * @return const ContainerPool& A reference to the container pool.
     */
    const ContainerPool& pool();

    /*!
     * @brief Get a specific container of the global transceiver by type.
     *
     * This will effectively call GuestTransceiver::container() on the
     * global transceiver returned by dots::global_transceiver().
     *
     * @param descriptor The type descriptor of the Container to get.
     *
     * @return const Container<>& A reference to the Container.
     *
     * @exception std::runtime_error Thrown if no Container for
     * @p descriptor was found.
     */
    const Container<>& container(const type::StructDescriptor<>& descriptor);

    /*!
     * @brief Get a specific container of the global transceiver by type.
     *
     * This will effectively call GuestTransceiver::container() on the
     * global transceiver returned by dots::global_transceiver().
     *
     * Instantiating this template will also register \p T as a global
     * subscribe type. When using the dots::Application, this will result
     * in implicitly creating a subscription to \p T and the type's cache
     * being preloaded while the connection is being established.
     *
     * @tparam T The type of the Container to get.
     *
     * @return const Container<>& A reference to the Container.
     */
    template <typename T>
    const Container<T>& container()
    {
        io::register_global_subscribe_type<T>();
        return global_transceiver()->container<T>();
    }

    #endif
}
