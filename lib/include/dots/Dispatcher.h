#pragma once
#include <map>
#include <unordered_map>
#include <functional>
#include <dots/type/AnyStruct.h>
#include <dots/Event.h>
#include <dots/ContainerPool.h>
#include <dots/io/Transmission.h>

namespace dots
{
    struct Dispatcher
    {
        using id_t = uint64_t;
        using transmission_handler_t = std::function<void(const io::Transmission&)>;
        template <typename T = type::Struct>
        using event_handler_t = std::function<void(const Event<T>&)>;

        Dispatcher() = default;
        Dispatcher(const Dispatcher& other) = delete;
        Dispatcher(Dispatcher&& other) noexcept = default;
        ~Dispatcher() = default;

        Dispatcher& operator = (const Dispatcher& rhs) = delete;
        Dispatcher& operator = (Dispatcher&& rhs) noexcept = default;

        const ContainerPool& pool() const;
        ContainerPool& pool();

        const Container<>& container(const type::StructDescriptor<>& descriptor) const;
        Container<>& container(const type::StructDescriptor<>& descriptor);

        id_t addTransmissionHandler(const type::StructDescriptor<>& descriptor, transmission_handler_t&& handler);
        id_t addEventHandler(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler);

        void removeTransmissionHandler(const type::StructDescriptor<>& descriptor, id_t id);
        void removeEventHandler(const type::StructDescriptor<>& descriptor, id_t id);

        void dispatch(const io::Transmission& transmission);

        template <typename T>
        const Container<T>& container() const
        {
            return m_containerPool.get<T>();
        }

        template <typename T>
        Container<T>& container()
        {
            return m_containerPool.get<T>();
        }

        template<typename T, typename EventHandler, typename... Args>
        id_t addEventHandler(EventHandler&& handler, Args&&... args)
        {
            constexpr bool IsTopLevelStruct = std::conjunction_v<std::is_base_of<type::Struct, T>, std::negation<std::bool_constant<T::_SubstructOnly>>>;
            constexpr bool IsEventHandler = std::is_invocable_v<EventHandler, Args&..., const Event<T>&>;

            static_assert(IsTopLevelStruct, "T has to be a top-level DOTS struct type");
            static_assert(IsEventHandler, "EventHandler has to be a valid DOTS event handler type and be invocable with Args");

            if constexpr (IsTopLevelStruct && IsEventHandler)
            {
                auto handle_event = [](auto& handler, auto& argsTuple, const Event<>& e)
                {
                    std::apply([&](auto&... args)
                    {
                        std::invoke(handler, args..., e.as<T>());
                    }, argsTuple);
                };

                // note that the two branches are intentionally identical except for the mutability of the outer lambda
                if constexpr (std::is_const_v<EventHandler>)
                {
                    return addEventHandler(T::_Descriptor(), [handler{ std::forward<EventHandler>(handler) }, argsTuple = std::make_tuple(std::forward<Args>(args)...), &handle_event](const Event<>& e)
                    {
                        handle_event(handler, argsTuple, e);
                    });
                }
                else
                {
                    return addEventHandler(T::_Descriptor(), [handler{ std::forward<EventHandler>(handler) }, argsTuple = std::make_tuple(std::forward<Args>(args)...), &handle_event](const Event<>& e) mutable
                    {
                        handle_event(handler, argsTuple, e);
                    });
                }
            }
            else
            {
                return 0;
            }
        }

    private:

        using transmission_handlers_t = std::map<id_t, transmission_handler_t, std::greater<>>;
        using transmission_handler_pool_t = std::unordered_map<const type::StructDescriptor<>*, transmission_handlers_t>;

        using event_handlers_t = std::map<id_t, event_handler_t<>, std::greater<>>;
        using event_handler_pool_t = std::unordered_map<const type::StructDescriptor<>*, event_handlers_t>;

        void dispatchTransmission(const io::Transmission& transmission);
        void dispatchEvent(const DotsHeader& header, const type::AnyStruct& instance);

        template <typename Handlers, typename Dispatchable>
        void dispatchToHandlers(Handlers& handlers, const Dispatchable& dispatchable);

        id_t m_nextId = 0;
        std::optional<id_t> m_currentlyDispatchingId;
        std::vector<id_t> m_removeIds;
        ContainerPool m_containerPool;
        transmission_handler_pool_t m_transmissionHandlerPool;
        event_handler_pool_t m_eventHandlerPool;
    };
}