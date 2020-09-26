#pragma once
#include <string_view>
#include <optional>
#include <deque>
#include <dots/io/Connection.h>
#include <dots/io/Dispatcher.h>
#include <dots/io/Subscription.h>
#include <dots/io/Publisher.h>
#include <dots/io/Registry.h>

namespace dots::io
{
    struct Transceiver : Publisher
    {
        using descriptor_map_t = std::map<std::string_view, type::StructDescriptor<>*>;

        using transmission_handler_t = Dispatcher::transmission_handler_t;
        template <typename T = type::Struct>
        using event_handler_t = Dispatcher::event_handler_t<T>;

        using new_type_handler_t = Registry::new_type_handler_t;

        Transceiver(std::string selfName);
        Transceiver(const Transceiver& other) = delete;
        Transceiver(Transceiver&& other) noexcept;
        virtual ~Transceiver() = default;

        Transceiver& operator = (const Transceiver& rhs) = delete;
        Transceiver& operator = (Transceiver&& rhs) noexcept;

        const std::string& selfName() const;

        const io::Registry& registry() const;
        io::Registry& registry();

        const ContainerPool& pool() const;
        const Container<>& container(const type::StructDescriptor<>& descriptor);

        Subscription subscribe(const type::StructDescriptor<>& descriptor, transmission_handler_t&& handler);
        Subscription subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler);

        Subscription subscribe(const std::string_view& name, transmission_handler_t&& handler);
        Subscription subscribe(const std::string_view& name, event_handler_t<>&& handler);

        Subscription subscribe(new_type_handler_t&& handler);

        virtual void publish(const type::Struct& instance, types::property_set_t what = types::property_set_t::All, bool remove = false) = 0;
        void remove(const type::Struct& instance);

        template <typename T>
        const Container<T>& container()
        {
            return m_dispatcher.container<T>();
        }

        template<typename T>
        Subscription subscribe(event_handler_t<T>&& handler)
        {
            static_assert(!T::_SubstructOnly, "it is not allowed to subscribe to a struct that is marked with 'substruct_only'!");

            joinGroup(T::_Descriptor().name());
            Dispatcher::id_t id = m_dispatcher.addEventHandler(std::move(handler));

            return makeSubscription([&, id]{ m_dispatcher.removeEventHandler(T::_Descriptor(), id); });
        }

        template <type::Type TType, typename NewTypeHandler>
        Subscription subscribe(NewTypeHandler&& handler)
        {
            return subscribe([handler_ = std::move(handler)](const type::Descriptor<>& descriptor)
            {
                // TODO: shorten once descriptor traits were added
                if constexpr (TType == type::Type::Vector)
                {
                    if (descriptor.type() == type::Type::Vector)
                    {
                        handler_(static_cast<const type::VectorDescriptor&>(descriptor));
                    }
                }
                else if constexpr (TType == type::Type::Enum)
                {
                    if (descriptor.type() == type::Type::Enum)
                    {
                        handler_(static_cast<const type::EnumDescriptor<>&>(descriptor));
                    }
                }
                else if constexpr (TType == type::Type::Struct)
                {
                    if (descriptor.type() == type::Type::Struct)
                    {
                        handler_(static_cast<const type::StructDescriptor<>&>(descriptor));
                    }
                }
                else
                {
                    static_assert(!std::is_same_v<NewTypeHandler, NewTypeHandler>, "TType has to to be one of the supported complex descriptor types");
                }
            });
        }

        [[deprecated("only available for backwards compatibility")]]
        void publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t includedProperties, bool remove) override;

    protected:

        Dispatcher& dispatcher();

    private:

        using id_t = uint64_t;
        using new_type_handlers_t = std::map<id_t, new_type_handler_t>;

        virtual void joinGroup(const std::string_view& name) = 0;
        virtual void leaveGroup(const std::string_view& name) = 0;

        template <typename UnsubscribeHandler>
        Subscription makeSubscription(UnsubscribeHandler&& unsubscribeHandler);

        void handleNewType(const type::Descriptor<>& descriptor) noexcept;

        id_t m_nextId;
        std::shared_ptr<Transceiver*> m_this;
        io::Registry m_registry;
        Dispatcher m_dispatcher;
        std::string m_selfName;
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