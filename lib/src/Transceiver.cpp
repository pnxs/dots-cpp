#include <dots/Transceiver.h>
#include <dots/tools/logging.h>
#include <dots/serialization/AsciiSerialization.h>
#include <DotsMember.dots.h>

namespace dots
{
    Transceiver::Transceiver(std::string selfName, boost::asio::io_context& ioContext/* = global_io_context()*/, type::Registry::StaticTypePolicy staticTypePolicy/* = StaticTypePolicy::All*/) :
        m_nextId(0),
        m_this(std::make_shared<Transceiver*>(this)),
        m_registry{ [&](const type::Descriptor<>& descriptor){ handleNewType(descriptor); }, staticTypePolicy },
        m_selfName{ std::move(selfName) },
        m_ioContext(std::ref(ioContext))
    {
        /* do nothing */
    }

    Transceiver::Transceiver(Transceiver&& other) noexcept :
        m_nextId(other.m_nextId),
        m_currentlyDispatchingId{ other.m_currentlyDispatchingId },
        m_removeIds{ std::move(other.m_removeIds) },
        m_this{ std::move(other.m_this) },
        m_registry{ std::move(other.m_registry) },
        m_dispatcher{ std::move(other.m_dispatcher) },
        m_selfName{ std::move(other.m_selfName) },
        m_ioContext{ other.m_ioContext },
        m_newTypeHandlers{ std::move(other.m_newTypeHandlers) }
    {
        /* do nothing */
    }

    Transceiver& Transceiver::operator = (Transceiver&& rhs) noexcept
    {
        m_nextId = rhs.m_nextId;
        m_currentlyDispatchingId = rhs.m_currentlyDispatchingId;
        m_removeIds = std::move(rhs.m_removeIds);
        m_this = std::move(rhs.m_this);
        m_registry = std::move(rhs.m_registry);
        m_dispatcher = std::move(rhs.m_dispatcher);
        m_selfName = std::move(rhs.m_selfName);
        m_ioContext = rhs.m_ioContext;
        m_newTypeHandlers = std::move(rhs.m_newTypeHandlers);

        *m_this = this;

        return *this;
    }

    const std::string& Transceiver::selfName() const
    {
        return m_selfName;
    }

    const boost::asio::io_context& Transceiver::ioContext() const
    {
        return m_ioContext;
    }

    boost::asio::io_context& Transceiver::ioContext()
    {
        return m_ioContext;
    }

    const type::Registry& Transceiver::registry() const
    {
        return m_registry;
    }

    type::Registry& Transceiver::registry()
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

    const Container<>& Transceiver::container(const type::StructDescriptor<>& descriptor) const
    {
        return m_dispatcher.container(descriptor);
    }

    Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, transmission_handler_t handler)
    {
        if (descriptor.substructOnly())
        {
            throw std::logic_error{ "attempt to subscribe to substruct-only type '" + descriptor.name() + "'" };
        }

        joinGroup(descriptor.name());
        Dispatcher::id_t id = m_dispatcher.addTransmissionHandler(descriptor, std::move(handler));

        return makeSubscription([&, id]{ m_dispatcher.removeTransmissionHandler(descriptor, id); });
    }

    Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<> handler)
    {
        if (descriptor.substructOnly())
        {
            throw std::logic_error{ "attempt to subscribe to substruct-only type '" + descriptor.name() + "'" };
        }

        joinGroup(descriptor.name());
        Dispatcher::id_t id = m_dispatcher.addEventHandler(descriptor, std::move(handler));

        return makeSubscription([&, id]{ m_dispatcher.removeEventHandler(descriptor, id); });
    }

    Subscription Transceiver::subscribe(std::string_view name, transmission_handler_t handler)
    {
        return subscribe(m_registry.getStructType(name), std::move(handler));
    }

    Subscription Transceiver::subscribe(std::string_view name, event_handler_t<> handler)
    {
        return subscribe(m_registry.getStructType(name), std::move(handler));
    }

    Subscription Transceiver::subscribe(new_type_handler_t<> handler)
    {
        const auto& [id, handler_] = *m_newTypeHandlers.try_emplace(m_nextId++, std::move(handler)).first;
        m_registry.forEach(handler_);

        return makeSubscription([&, id(id)]
        {
            if (id == m_currentlyDispatchingId)
            {
                m_removeIds.emplace_back(id);
            }
            else
            {
                m_newTypeHandlers.extract(id);
            }
        });
    }

    void Transceiver::remove(const type::Struct& instance)
    {
        publish(instance, instance._keyProperties(), true);
    }

    void Transceiver::handleNewType(const type::Descriptor<>& descriptor) noexcept
    {
        for (const auto& [id, handler] : m_newTypeHandlers)
        {
            try
            {
                m_currentlyDispatchingId = id;
                handler(descriptor);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR_S("error in new type handler for type '" << descriptor.name() << "' -> " << e.what());
            }

            m_currentlyDispatchingId = std::nullopt;
        }

        for (id_t id : m_removeIds)
        {
            m_newTypeHandlers.extract(id);
        }

        m_removeIds.clear();
    }
}