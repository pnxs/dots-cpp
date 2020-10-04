#include <dots/io/Transceiver.h>
#include <dots/tools/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>
#include <DotsMember.dots.h>

namespace dots::io
{
    Transceiver::Transceiver(std::string selfName) :
        m_nextId(0),
        m_this(std::make_shared<Transceiver*>(this)),
        m_registry{ [&](const type::Descriptor<>& descriptor){ handleNewType(descriptor); } },
        m_selfName{ std::move(selfName) }
    {
        /* do nothing */
    }

    Transceiver::Transceiver(Transceiver&& other) noexcept :
        m_nextId(other.m_nextId),
        m_this{ std::move(other.m_this) },
        m_registry{ std::move(other.m_registry) },
        m_dispatcher{ std::move(other.m_dispatcher) },
        m_selfName{ std::move(other.m_selfName) },
        m_newTypeHandlers{ std::move(other.m_newTypeHandlers) }
    {
        /* do nothing */
    }

    Transceiver& Transceiver::operator = (Transceiver&& rhs) noexcept
    {
        m_nextId = rhs.m_nextId;
        m_this = std::move(rhs.m_this);
        m_registry = std::move(rhs.m_registry);
        m_dispatcher = std::move(rhs.m_dispatcher);
        m_selfName = std::move(rhs.m_selfName);
        m_newTypeHandlers = std::move(rhs.m_newTypeHandlers);

        *m_this = this;

        return *this;
    }

    const std::string& Transceiver::selfName() const
    {
        return m_selfName;
    }

    const io::Registry& Transceiver::registry() const
    {
        return m_registry;
    }

    io::Registry& Transceiver::registry()
    {
        return m_registry;
    }

    Dispatcher& Transceiver::dispatcher()
    {
        return m_dispatcher;
    }

    const ContainerPool& Transceiver::pool() const
    {
        return m_dispatcher.pool();
    }

    const Container<>& Transceiver::container(const type::StructDescriptor<>& descriptor)
    {
        return m_dispatcher.container(descriptor);
    }

    Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, transmission_handler_t&& handler)
    {
        if (descriptor.substructOnly())
        {
            throw std::logic_error{ "attempt to subscribe to substruct-only type: " + descriptor.name() };
        }

        joinGroup(descriptor.name());
        Dispatcher::id_t id = m_dispatcher.addTransmissionHandler(descriptor, std::move(handler));

        return makeSubscription([&, id]{ m_dispatcher.removeTransmissionHandler(descriptor, id); });
    }

    Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler)
    {
        if (descriptor.substructOnly())
        {
            throw std::logic_error{ "attempt to subscribe to substruct-only type: " + descriptor.name() };
        }

        joinGroup(descriptor.name());
        Dispatcher::id_t id = m_dispatcher.addEventHandler(descriptor, std::move(handler));

        return makeSubscription([&, id]{ m_dispatcher.removeEventHandler(descriptor, id); });
    }

    Subscription Transceiver::subscribe(const std::string_view& name, transmission_handler_t&& handler)
    {
        return subscribe(m_registry.getStructType(name), std::move(handler));
    }

    Subscription Transceiver::subscribe(const std::string_view& name, event_handler_t<>&& handler)
    {
        return subscribe(m_registry.getStructType(name), std::move(handler));
    }

    Subscription Transceiver::subscribe(new_type_handler_t&& handler)
    {
        const auto& [id, handler_] = *m_newTypeHandlers.try_emplace(m_nextId++, std::move(handler)).first;

        for (const auto& [name, descriptor] : type::StaticDescriptorMap::Descriptors())
        {
            (void)name;
            handler_(*descriptor);
        }

        for (const auto& [name, descriptor] : m_registry)
        {
            (void)name;
            handler_(*descriptor);
        }

        return makeSubscription([&, id(id)]{ m_newTypeHandlers.extract(id); });
    }

    void Transceiver::remove(const type::Struct& instance)
    {
        publish(instance, instance._keyProperties(), true);
    }

    void Transceiver::publish(const type::StructDescriptor<>*/* td*/, const type::Struct& instance, types::property_set_t what, bool remove)
    {
        publish(instance, what, remove);
    }

    void Transceiver::handleNewType(const type::Descriptor<>& descriptor) noexcept
    {
        for (const auto& [id, handler] : m_newTypeHandlers)
        {
            try
            {
                handler(descriptor);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR_S("error in new type handler -> " << e.what());
            }
        }
    }
}