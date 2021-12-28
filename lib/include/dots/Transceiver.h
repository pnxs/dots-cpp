#pragma once
#include <string_view>
#include <optional>
#include <functional>
#include <dots/io/Io.h>
#include <dots/Connection.h>
#include <dots/Dispatcher.h>
#include <dots/Subscription.h>
#include <dots/type/Registry.h>

namespace dots
{
    /*!
     * @class Transceiver Transceiver.h <dots/Transceiver.h>
     *
     * @brief Abstract base class for all DOTS transceivers.
     *
     * Transceivers are the central components within DOTS. They manage
     * connections to DOTS peers and provide the actual publish-subscribe
     * functionality.
     *
     * @remark Regular users usually do not have to implement transceivers
     * themselves. This class is intended to be used by the
     * dots::GuestTransceiver and dots::HostTransceiver implementations.
     */
    struct Transceiver
    {
        using transmission_handler_t = Dispatcher::transmission_handler_t;
        template <typename T = type::Struct>
        using event_handler_t = Dispatcher::event_handler_t<T>;

        using new_type_handler_t = type::Registry::new_type_handler_t;

        /*!
         * @brief Construct a new Transceiver object.
         *
         * @param selfName The name the transceiver will use to identify
         * itself.
         *
         * @param ioContext The ASIO IO context (i.e. the "event loop") to use.
         *
         * @param staticTypePolicy Specifies the static type policy of the
         * transceiver's registry.
         */
        Transceiver(std::string selfName, boost::asio::io_context& ioContext = io::global_io_context(), type::Registry::StaticTypePolicy staticTypePolicy = type::Registry::StaticTypePolicy::All);
        Transceiver(const Transceiver& other) = delete;
        Transceiver(Transceiver&& other) noexcept;
        virtual ~Transceiver() = default;

        Transceiver& operator = (const Transceiver& rhs) = delete;
        Transceiver& operator = (Transceiver&& rhs) noexcept;

        const std::string& selfName() const;

        /*!
         * @brief Get the currently used IO context.
         *
         * Note that this is the same IO context that was given in
         * Transceiver().
         *
         * @return const boost::asio::io_context& A reference to the currently
         * used IO context.
         */
        const boost::asio::io_context& ioContext() const;

        /*!
         * @brief Get the currently used IO context.
         *
         * Note that this is the same IO context that was given in
         * Transceiver().
         *
         * @return boost::asio::io_context& A reference to the currently used
         * IO context.
         */
        boost::asio::io_context& ioContext();

        /*!
         * @brief Get the type registry.
         *
         * @return const type::Registry& A reference to the type registry.
         */
        const type::Registry& registry() const;

        /*!
         * @brief Get the type registry.
         *
         * @return type::Registry& A reference to the type registry.
         */
        type::Registry& registry();

        /*!
         * @brief Get the container pool.
         *
         * @return const ContainerPool& A reference to the container pool.
         */
        const ContainerPool& pool() const;

        /*!
         * @brief Get a specific container by type.
         *
         * @remark This effectively calls dots::ContainerPool::get() on
         * Transceiver::pool() with the given descriptor.
         *
         * @param descriptor The type descriptor of the Container to get.
         *
         * @return const Container<>& A reference to the Container.
         *
         * @exception std::runtime_error Thrown if no Container for @p
         * descriptor was found.
         */
        const Container<>& container(const type::StructDescriptor<>& descriptor) const;

        /*!
         * @brief Subscribe to transmissions of a specific type.
         *
         * This will create a subscription to a given type and cause the given
         * handler to be invoked asynchronously every time a corresponding
         * transmission is received.
         *
         * Note that contrary to event subscriptions, the handler will be
         * invoked with the raw transmission data before the local Container
         * has been updated.
         *
         * @attention Regular users are usually not required to create
         * subscriptions to transmissions and are advised to use event
         * subscriptions instead.
         *
         * @param descriptor The type to subscribe to.
         *
         * @param handler The handler to invoke asynchronously every time a
         * corresponding transmission is received. If the given type is a
         * cached type and the corresponding Container is not empty, the given
         * handler will also be invoked synchronously with each contained
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
        Subscription subscribe(const type::StructDescriptor<>& descriptor, transmission_handler_t handler);

        /*!
         * @brief Subscribe to events of a specific type.
         *
         * This will create a subscription to a given type and cause the given
         * handler to be invoked asynchronously every time a corresponding DOTS
         * event occurs. For cached types, events are created after the local
         * Container has been updated.
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
        Subscription subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<> handler);

        /*!
         * @brief Subscribe to transmissions of a specific type by name.
         *
         * This function is similar to the overload accepting a descriptor,
         * except that the descriptor will be retrieved from the registry by
         * name.
         *
         * @attention Regular users are usually not required to create
         * subscriptions to transmission and are advised to use event
         * subscriptions instead.
         *
         * @param name The name of the type to subscribe to.
         *
         * @param handler The handler to invoke asynchronously every time a
         * corresponding transmission is received. If the given type is a
         * cached type and the corresponding Container is not empty, the given
         * handler will also be invoked synchronously with for each contained
         * instance before this function returns.
         *
         * @return Subscription The Subscription object that manages the state
         * of the subscription. The subscription will stay active until the
         * object is destroyed or Subscription::unsubscribe() is called
         * manually.
         *
         * @exception std::logic_error Thrown if the registry does not contain
         * a struct type of name @p name.
         *
         * @exception std::logic_error Thrown if @p name designates a
         * sub-struct only type.
         */
        Subscription subscribe(std::string_view name, transmission_handler_t handler);

        /*!
         * @brief Subscribe to events of a specific type by name.
         *
         * This function is similar to the overload accepting a descriptor,
         * except that the descriptor will be retrieved from the registry by
         * name.
         *
         * @attention Unless otherwise required, users are advised to use the
         * explicitly typed versions of Transceiver::subscribe() instead.
         *
         * @param name The name of the type to subscribe to.
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
         * @exception std::logic_error Thrown if the registry does not contain
         * a struct type of name @p name.
         *
         * @exception std::logic_error Thrown if @p name designates a
         * sub-struct only type.
         */
        Subscription subscribe(std::string_view name, event_handler_t<> handler);

        /*!
         * @brief Subscribe to new types.
         *
         * This will create a subscription to new types and cause the given
         * handler to be invoked whenever a type is added to the registry.
         *
         * @param handler The handler to invoke asynchronously every time a
         * type is added to the registry. If the registry is not empty, the
         * given handler will also be invoked synchronously with all currently
         * known types before this function returns.
         *
         * @return Subscription The Subscription object that manages the state
         * of the subscription. The subscription will stay active until the
         * object is destroyed or Subscription::unsubscribe() is called
         * manually.
         */
        Subscription subscribe(new_type_handler_t handler);

        /*!
         * @brief Publish an instance of a DOTS struct type.
         *
         * The exact behavior of this is defined by the implementing class.
         *
         * @param instance The instance to publish.
         *
         * @param includedProperties The property set to include in the
         * publish. If no set is given, the valid property set of
         * @p instance will be used.
         *
         * @param remove Specifies whether the publish is a remove.
         */
        virtual void publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false) = 0;

        /*!
         * @brief Remove an instance of a DOTS struct type.
         *
         * This will effectively call Transceiver::publish() with the remove
         * flag set to true and only including the key properties of the type.
         *
         * @param instance The instance to remove.
         */
        void remove(const type::Struct& instance);

        /*!
         * @brief Get a specific container by type.
         *
         * @remark This effectively calls dots::ContainerPool::get() on
         * Transceiver::pool() with the given type.
         *
         * @tparam T The type of the Container to get.
         *
         * @return const Container<>& A reference to the Container.
         */
        template <typename T>
        const Container<T>& container()
        {
            return m_dispatcher.container<T>();
        }

        /*!
         * @brief Subscribe to events of a specific type.
         *
         * This will create a subscription to a given type and cause the given
         * handler to be invoked asynchronously every time a corresponding DOTS
         * event occurs. For cached types, events are created after the local
         * Container has been updated.
         *
         * Note that the @p handler can be any compatible invocable object,
         * including lambdas and class member functions:
         *
         * @code{.cpp}
         * // subscribing to events of a DOTS struct type Foobar with lambda handler
         * transceiver.subscribe<Foobar>([](const dots::Event<Foobar>& event)
         * {
         *     // ...
         * });
         *
         * // subscribing to events of a DOTS struct type Foobar with member function
         * transceiver.subscribe<Foobar>(&SomeClass::handleFoobar, this);
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
        template<typename T, typename EventHandler, typename... Args, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        Subscription subscribe(EventHandler&& handler, Args&&... args)
        {
            static_assert(!T::_SubstructOnly, "it is not allowed to subscribe to a struct that is marked with 'substruct_only'!");

            joinGroup(T::_Descriptor().name());
            Dispatcher::id_t id = m_dispatcher.addEventHandler<T>(std::forward<EventHandler>(handler), std::forward<Args>(args)...);

            return makeSubscription([&, id]{ m_dispatcher.removeEventHandler(T::_Descriptor(), id); });
        }

        /*!
         * @brief Subscribe to new types of specific categories.
         *
         * This will create a subscription to new types of given categories and
         * cause the given handler to be invoked whenever a corresponding type
         * is added to the registry.
         *
         * The descriptor categories can be specified as an arbitrary
         * combination of DOTS descriptor types. Additionally, @p handler can
         * be any compatible invocable object.
         *
         * For example:
         *
         * @code{.cpp}
         * // subscribing to new struct and enum types with lambda handler
         * transceiver.subscribe<dots::type::StructDescriptor<>, dots::type::EnumDescriptor<>>([](const auto& descriptor)
         * {
         *     // ...
         * });
         *
         * // subscribing to new vector types with member function
         * transceiver.subscribe<dots::type::VectorDescriptor>(&SomeClass::handleNewVector, this);
         * @endcode
         *
         * @tparam TDescriptors The descriptor types (e.g.
         * dots::type::StructDescriptor<>).
         *
         * @tparam TypeHandler The type of the handler. Must be invocable with
         * references of all @p TDescriptor types and optionally @p args if given.
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
        template <typename... TDescriptors, typename TypeHandler, typename... Args, std::enable_if_t<sizeof...(TDescriptors) >= 1 && std::conjunction_v<std::is_base_of<type::Descriptor<>, TDescriptors>...>, int> = 0>
        Subscription subscribe(TypeHandler&& handler, Args&&... args)
        {
            constexpr bool IsTypeHandler = std::conjunction_v<std::is_invocable<TypeHandler, Args&..., const TDescriptors&>...>;
            static_assert(IsTypeHandler, "Handler has to be a valid type handler for all TDescriptors types and be invocable by args");

            if constexpr (IsTypeHandler)
            {
                auto handle_type = [](auto& handler, auto& argsTuple, const type::Descriptor<>& descriptor, const auto* wantedDescriptor)
                {
                    using wanted_descriptor_t = std::decay_t<std::remove_pointer_t<decltype(wantedDescriptor)>>;
                    (void)wantedDescriptor;

                    if (wantedDescriptor = descriptor.as<wanted_descriptor_t>(); wantedDescriptor != nullptr)
                    {
                        std::apply([&](auto&... args){ std::invoke(handler, args..., *wantedDescriptor); }, argsTuple);
                    }
                };

                // note that the two branches are intentionally identical except for the mutability of the outer lambda
                if constexpr (std::is_const_v<TypeHandler>)
                {
                    return subscribe([handler{ std::forward<TypeHandler>(handler) }, argsTuple = std::make_tuple(std::forward<Args>(args)...), &handle_type](const type::Descriptor<>& descriptor)
                    {
                        (handle_type(handler, argsTuple, descriptor, static_cast<const TDescriptors*>(nullptr)), ...);
                    });
                }
                else
                {
                    return subscribe([handler{ std::forward<TypeHandler>(handler) }, argsTuple = std::make_tuple(std::forward<Args>(args)...), &handle_type](const type::Descriptor<>& descriptor) mutable
                    {
                        (handle_type(handler, argsTuple, descriptor, static_cast<const TDescriptors*>(nullptr)), ...);
                    });
                }
            }
            else
            {
                return std::declval<Subscription>();
            }
        }

    protected:

        Dispatcher& dispatcher();

    private:

        using id_t = uint64_t;
        using new_type_handlers_t = std::map<id_t, new_type_handler_t, std::greater<>>;

        virtual void joinGroup(std::string_view name) = 0;
        virtual void leaveGroup(std::string_view name) = 0;

        template <typename UnsubscribeHandler>
        Subscription makeSubscription(UnsubscribeHandler&& unsubscribeHandler);

        void handleNewType(const type::Descriptor<>& descriptor) noexcept;

        id_t m_nextId;
        std::optional<id_t> m_currentlyDispatchingId;
        std::vector<id_t> m_removeIds;
        std::shared_ptr<Transceiver*> m_this;
        type::Registry m_registry;
        Dispatcher m_dispatcher;
        std::string m_selfName;
        std::reference_wrapper<boost::asio::io_context> m_ioContext;
        new_type_handlers_t m_newTypeHandlers;
    };

    template <typename UnsubscribeHandler>
    Subscription Transceiver::makeSubscription(UnsubscribeHandler&& unsubscribeHandler)
    {
        return Subscription{ [this, this_ = std::weak_ptr<Transceiver*>{ m_this }, unsubscribeHandler{ std::forward<UnsubscribeHandler>(unsubscribeHandler) }]()
        {
            if (this_.lock())
            {
                unsubscribeHandler();
            }
        } };
    }
}