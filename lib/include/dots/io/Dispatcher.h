#pragma once
#include <map>
#include <unordered_map>
#include <functional>
#include <dots/type/AnyStruct.h>
#include <dots/io/Event.h>
#include <dots/io/ContainerPool.h>
#include <dots/io/Transmission.h>

namespace dots::io
{
    struct Dispatcher
    {
        using id_t = uint64_t;
        using transmission_handler_t = std::function<void(const Transmission&)>;
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

        void dispatch(const Transmission& transmission);

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

        template<typename T>
        id_t addEventHandler(event_handler_t<T>&& handler)
        {
            static_assert(std::is_base_of_v<type::Struct, T>, "T has to be a sub-class of Struct");

            return addEventHandler(T::_Descriptor(), [_handler(std::move(handler))](const Event<>& e)
            {
                _handler(e.as<T>());
            });
        }

    private:

        using transmission_handlers_t = std::map<id_t, transmission_handler_t>;
        using transmission_handler_pool_t = std::unordered_map<const type::StructDescriptor<>*, transmission_handlers_t>;

        using event_handlers_t = std::map<id_t, event_handler_t<>>;
        using event_handler_pool_t = std::unordered_map<const type::StructDescriptor<>*, event_handlers_t>;

        void dispatchTransmission(const Transmission& transmission);
        void dispatchEvent(const DotsHeader& header, const type::AnyStruct& instance);

        id_t m_nextId = 0;
        ContainerPool m_containerPool;
        transmission_handler_pool_t m_transmissionHandlerPool;
        event_handler_pool_t m_eventHandlerPool;
    };
}