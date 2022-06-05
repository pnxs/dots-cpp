#pragma once
#include <map>
#include <unordered_map>
#include <functional>
#include <dots/type/AnyStruct.h>
#include <dots/Event.h>
#include <dots/ContainerPool.h>
#include <dots/io/Transmission.h>
#include <dots/tools/Handler.h>

namespace dots
{
    /*!
     * @class Dispatcher Dispatcher.h <dots/Dispatcher.h>
     *
     * @brief Dispatches transmissions to handlers.
     *
     * This class provides event handling for transmissions. It is usually
     * used by a Transceiver to dispatch (i.e. process) transmissions
     * received via a host or guest connection and will result in the
     * invocation of corresponding registered transmission and event
     * handlers.
     *
     * If the type of the instance contained in the transmission is cached,
     * dispatching includes updating the local cache (i.e. Container)
     * before any event handlers are invoked.
     *
     * @attention Outside of advanced use cases, a regular user is never
     * required to create or manage Dispatcher objects themselves or access
     * them directly.
     */
    struct Dispatcher
    {
        using id_t = uint64_t;
        using error_handler_t = tools::Handler<void(const type::StructDescriptor&, std::exception_ptr)>;
        using transmission_handler_t = tools::Handler<void(const io::Transmission&)>;
        template <typename T = type::Struct>
        using event_handler_t = tools::Handler<void(const Event<T>&)>;

        /*!
         * @brief Construct a new Dispatcher object.
         *
         * @param handler When transmission or event handlers throw an
         * exception, this handler will be invoked with the corresponding
         * error.
         */
        Dispatcher(error_handler_t handler);
        Dispatcher(const Dispatcher& other) = delete;
        Dispatcher(Dispatcher&& other) noexcept = default;
        ~Dispatcher() = default;

        Dispatcher& operator = (const Dispatcher& rhs) = delete;
        Dispatcher& operator = (Dispatcher&& rhs) noexcept = default;

        /*!
         * @brief Get the container pool.
         *
         * @return const ContainerPool& A reference to the container pool.
         */
        const ContainerPool& pool() const;

        /*!
         * @brief Get the container pool.
         *
         * @return const ContainerPool& A reference to the container pool.
         */
        ContainerPool& pool();

        /*!
         * @brief Get a specific container by type.
         *
         * @remark This effectively calls dots::ContainerPool::get() on
         * Dispatcher::pool() with the given descriptor.
         *
         * @param descriptor The type descriptor of the Container to get.
         *
         * @return const Container<>& A reference to the Container.
         */
        const Container<>& container(const type::StructDescriptor& descriptor) const;

        /*!
         * @brief Get a specific container by type.
         *
         * @remark This effectively calls dots::ContainerPool::get() on
         * Dispatcher::pool() with the given descriptor.
         *
         * @param descriptor The type descriptor of the Container to get.
         *
         * @return const Container<>& A reference to the Container.
         */
        Container<>& container(const type::StructDescriptor& descriptor);

        /*!
         * @brief Add a transmission handler for a specific type.
         *
         * This will add a handler for transmissions of a given type and cause
         * it to be invoked every time a corresponding transmission is
         * dispatched.
         *
         * Note that contrary to event handlers, the handler will be invoked
         * with the raw transmission data before the local Container has been
         * updated.
         *
         * @param descriptor The type to add a transmission handler for.
         *
         * @param handler The handler to invoke every time a corresponding
         * transmission is dispatched. If the given type is a cached type and
         * the corresponding Container is not empty, the given handler will
         * also be invoked with each contained instance before this function
         * returns.
         *
         * @return id_t The unique id of the handler. The id be used to remove
         * the transmission handler by calling
         * Dispatcher::removeTransmissionHandler().
         */
        id_t addTransmissionHandler(const type::StructDescriptor& descriptor, transmission_handler_t handler);

        /*!
         * @brief Add an event handler for a specific type.
         *
         * This will add a handler for events of a given type and cause it to
         * be invoked every time a corresponding transmission is dispatched.
         * For cached types, events are created after the local Container has
         * been updated.
         *
         * @param descriptor The type to add an event handler for.
         *
         * @param handler The handler to invoke every time a corresponding
         * transmission is dispatched. If the given type is a cached type and
         * the corresponding Container is not empty, the given handler will
         * also be invoked with create events for each contained instance
         * before this function returns.
         *
         * @return id_t The unique id of the handler. The id can be used to
         * remove the event handler by calling
         * Dispatcher::removeEventHandler().
         */
        id_t addEventHandler(const type::StructDescriptor& descriptor, event_handler_t<> handler);

        /*!
         * @brief Add an event handler for a specific type.
         *
         * This will add a handler for events of a given type and cause it to
         * be invoked every time a corresponding transmission is dispatched.
         * For cached types, events are created after the local Container has
         * been updated.
         *
         * Note that the @p handler can be any compatible invocable object,
         * including lambdas and class member functions:
         *
         * @code{.cpp}
         * // adding event handler for events of a DOTS struct type Foobar with lambda handler
         * dispatcher.addEventHandler<Foobar>([](const dots::Event<Foobar>& event)
         * {
         *     // ...
         * });
         *
         * // adding event handler for events a DOTS struct type Foobar member function
         * dispatcher.addEventHandler<Foobar>({ &SomeClass::handleFoobar, this });
         * @endcode
         *
         * @tparam T The type to subscribe to.
         *
         * @param handler The handler to invoke every time a corresponding
         * transmission is dispatched. If the given type is a cached type and
         * the corresponding Container is not empty, the given handler will
         * also be invoked with create events for each contained instance
         * before this function returns.
         *
         * @return id_t The unique id of the handler. The id can be used to
         * remove the event handler by calling
         * Dispatcher::removeEventHandler().
         */
        template<typename T>
        id_t addEventHandler(event_handler_t<T> handler)
        {
            constexpr bool IsTopLevelStruct = std::conjunction_v<std::is_base_of<type::Struct, T>, std::negation<std::bool_constant<T::_SubstructOnly>>>;
            static_assert(IsTopLevelStruct, "T has to be a top-level DOTS struct type");

            if constexpr (IsTopLevelStruct)
            {
                return addEventHandler(T::_Descriptor(), event_handler_t<>{ tools::static_argument_cast, std::move(handler) });
            }
            else
            {
                return 0;
            }
        }

        /*!
         * @brief Remove a specific transmission handler.
         *
         * This removes a specific transmission handler that was previously
         * added via Dispatcher::addTransmissionHandler().
         *
         * @param descriptor The type for which the transmission handler was
         * registered.
         *
         * @param id The unique id of the handler as returned by
         * Dispatcher::addTransmissionHandler().
         */
        void removeTransmissionHandler(const type::StructDescriptor& descriptor, id_t id);

        /*!
         * @brief Remove a specific event handler.
         *
         * This removes a specific event handler that was previously added via
         * Dispatcher::addEventHandler().
         *
         * @param descriptor The type for which the event handler was
         * registered.
         *
         * @param id The unique id of the handler as returned by
         * Dispatcher::addEventHandler().
         */
        void removeEventHandler(const type::StructDescriptor& descriptor, id_t id);

        /*!
         * @brief Dispatch a transmission.
         *
         * This dispatches a given transmission which will result in all
         * corresponding transmission and event handlers to be invoked
         * accordingly.
         *
         * If the type of the instance contained in the transmission is a
         * cached type, the corresponding Container will be updated before the
         * event is created.
         *
         * Note that transmission handlers will always be invoked first and
         * before any updates on the Container take place.
         *
         * @param transmission The transmission to dispatch.
         */
        void dispatch(const io::Transmission& transmission);

        /*!
         * @brief Get a specific container by type.
         *
         * @remark This effectively calls dots::ContainerPool::get() on
         * Dispatcher::pool() with the given type.
         *
         * @tparam T The type of the Container to get.
         *
         * @return const Container<>& A reference to the Container.
         */
        template <typename T>
        const Container<T>& container() const
        {
            return m_containerPool.get<T>();
        }

        /*!
         * @brief Get a specific container by type.
         *
         * @remark This effectively calls dots::ContainerPool::get() on
         * Dispatcher::pool() with the given type.
         *
         * @tparam T The type of the Container to get.
         *
         * @return const Container<>& A reference to the Container.
         */
        template <typename T>
        Container<T>& container()
        {
            return m_containerPool.get<T>();
        }

        /*!
         * @brief Add an event handler for a specific type.
         *
         * This will add a handler for events of a given type and cause it to
         * be invoked every time a corresponding transmission is dispatched.
         * For cached types, events are created after the local Container has
         * been updated.
         *
         * Note that the @p handler can be any compatible invocable object,
         * including lambdas and class member functions:
         *
         * @code{.cpp}
         * // adding event handler for events of a DOTS struct type Foobar with lambda handler
         * dispatcher.addEventHandler<Foobar>([](const dots::Event<Foobar>& event)
         * {
         *     // ...
         * });
         *
         * // adding event handler for events a DOTS struct type Foobar member function
         * dispatcher.addEventHandler<Foobar>(&SomeClass::handleFoobar, this);
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
         * @param handler The handler to invoke every time a corresponding
         * transmission is dispatched. If the given type is a cached type and
         * the corresponding Container is not empty, the given handler will
         * also be invoked with create events for each contained instance
         * before this function returns.
         *
         * @param args Optional arguments that will be bound and passed to the
         * handler upon invocation (e.g. 'this' when specifying a class member
         * function as @p handler ).
         *
         * @return id_t The unique id of the handler. The id can be used to
         * remove the event handler by calling
         * Dispatcher::removeEventHandler().
         */
        template<typename T, typename EventHandler, typename... Args, std::enable_if_t<sizeof...(Args) >= 1, int> = 0>
        [[deprecated("superseded by event_handler_t<T> overload")]]
        id_t addEventHandler(EventHandler&& handler, Args&&... args)
        {
            constexpr bool IsEventHandler = std::is_constructible_v<event_handler_t<T>, EventHandler, Args...>;
            static_assert(IsEventHandler, "EventHandler has to be a valid DOTS event handler type and be invocable with Args");

            if constexpr (IsEventHandler)
            {
                return addEventHandler(event_handler_t<T>{ std::forward<EventHandler>(handler), std::forward<Args>(args)... });
            }
            else
            {
                return 0;
            }
        }

    private:

        using transmission_handlers_t = std::map<id_t, transmission_handler_t, std::greater<>>;
        using transmission_handler_pool_t = std::unordered_map<const type::StructDescriptor*, transmission_handlers_t>;

        using event_handlers_t = std::map<id_t, event_handler_t<>, std::greater<>>;
        using event_handler_pool_t = std::unordered_map<const type::StructDescriptor*, event_handlers_t>;

        template <typename HandlerPool>
        void removeHandler(HandlerPool& handlerPool, const type::StructDescriptor& descriptor, id_t id);

        void dispatchTransmission(const io::Transmission& transmission);
        void dispatchEvent(const DotsHeader& header, const type::AnyStruct& instance);

        template <typename Handlers, typename Dispatchable>
        void dispatchToHandlers(const type::StructDescriptor& descriptor, Handlers& handlers, const Dispatchable& dispatchable);

        std::optional<id_t> m_currentlyDispatchingId;
        std::vector<id_t> m_removeIds;
        ContainerPool m_containerPool;
        transmission_handler_pool_t m_transmissionHandlerPool;
        event_handler_pool_t m_eventHandlerPool;
        id_t m_nextId;
        error_handler_t m_errorHandler;
    };
}
