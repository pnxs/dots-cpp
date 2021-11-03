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
    struct Transceiver
    {
        using transmission_handler_t = Dispatcher::transmission_handler_t;
        template <typename T = type::Struct>
        using event_handler_t = Dispatcher::event_handler_t<T>;

        using new_type_handler_t = type::Registry::new_type_handler_t;

        Transceiver(std::string selfName, boost::asio::io_context& ioContext = io::global_io_context(), bool staticUserTypes = true);
        Transceiver(const Transceiver& other) = delete;
        Transceiver(Transceiver&& other) noexcept;
        virtual ~Transceiver() = default;

        Transceiver& operator = (const Transceiver& rhs) = delete;
        Transceiver& operator = (Transceiver&& rhs) noexcept;

        const std::string& selfName() const;

        const boost::asio::io_context& ioContext() const;
        boost::asio::io_context& ioContext();

        const type::Registry& registry() const;
        type::Registry& registry();

        const ContainerPool& pool() const;
        const Container<>& container(const type::StructDescriptor<>& descriptor) const;

        Subscription subscribe(const type::StructDescriptor<>& descriptor, transmission_handler_t handler);
        Subscription subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<> handler);

        Subscription subscribe(std::string_view name, transmission_handler_t handler);
        Subscription subscribe(std::string_view name, event_handler_t<> handler);

        Subscription subscribe(new_type_handler_t handler);

        virtual void publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties = std::nullopt, bool remove = false) = 0;
        void remove(const type::Struct& instance);

        template <typename T>
        const Container<T>& container()
        {
            return m_dispatcher.container<T>();
        }

        template<typename T, typename EventHandler, typename... Args, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        Subscription subscribe(EventHandler&& handler, Args&&... args)
        {
            static_assert(!T::_SubstructOnly, "it is not allowed to subscribe to a struct that is marked with 'substruct_only'!");

            joinGroup(T::_Descriptor().name());
            Dispatcher::id_t id = m_dispatcher.addEventHandler<T>(std::forward<EventHandler>(handler), std::forward<Args>(args)...);

            return makeSubscription([&, id]{ m_dispatcher.removeEventHandler(T::_Descriptor(), id); });
        }

        template <typename... TDescriptors, typename TypeHandler, typename... Args, std::enable_if_t<sizeof...(TDescriptors) >= 1 && std::conjunction_v<std::is_base_of<type::Descriptor<>, TDescriptors>...>, int> = 0>
        Subscription subscribe(TypeHandler&& handler, Args&&... args)
        {
            constexpr bool IsTypeHandler = std::conjunction_v<std::is_invocable<TypeHandler, const TDescriptors&>...>;
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