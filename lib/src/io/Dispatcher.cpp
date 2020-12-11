#include <dots/io/Dispatcher.h>

namespace dots::io
{
    const ContainerPool& Dispatcher::pool() const
    {
        return m_containerPool;
    }

    ContainerPool& Dispatcher::pool()
    {
        return m_containerPool;
    }

    const Container<>& Dispatcher::container(const type::StructDescriptor<>& descriptor) const
    {
        return m_containerPool.get(descriptor);
    }

    Container<>& Dispatcher::container(const type::StructDescriptor<>& descriptor)
    {
        return m_containerPool.get(descriptor);
    }

    auto Dispatcher::addTransmissionHandler(const type::StructDescriptor<>& descriptor, transmission_handler_t&& handler) -> id_t
    {
        id_t id = m_nextId++;
        m_transmissionHandlerPool[&descriptor].emplace(id, std::move(handler));

        return id;
    }

    auto Dispatcher::addEventHandler(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler) -> id_t
    {
        id_t id = m_nextId++;
        event_handlers_t& handlers = m_eventHandlerPool[&descriptor];
        const event_handler_t<>& handler_ = handlers.emplace(id, std::move(handler)).first->second;

        const Container<>& container = m_containerPool.get(descriptor);

        if (!container.empty())
        {
            DotsHeader header{
                DotsHeader::typeName_i{ descriptor.name() },
                DotsHeader::removeObj_i{ false },
                DotsHeader::fromCache_i{ container.size() },
                DotsHeader::isFromMyself_i{ false }
            };

            for (const auto& [instance, cloneInfo] : container)
            {
                header.attributes = instance->_validProperties();
                --*header.fromCache;
                handler_(Event<>{ header, instance, instance, cloneInfo, DotsMt::create });
            }
        }

        return id;
    }

    void Dispatcher::removeTransmissionHandler(const type::StructDescriptor<>& descriptor, id_t id)
    {
        if (auto itHandlers = m_transmissionHandlerPool.find(&descriptor); itHandlers != m_transmissionHandlerPool.end())
        {
            transmission_handlers_t& handlers = itHandlers->second;

            if (auto itHandler = handlers.find(id); itHandler != handlers.end())
            {
                handlers.erase(itHandler);
                return;
            }
        }

        throw std::logic_error{ "cannot remove unknown transmission handler for type: " + descriptor.name() };
    }

    void Dispatcher::removeEventHandler(const type::StructDescriptor<>& descriptor, id_t id)
    {
        if (auto itHandlers = m_eventHandlerPool.find(&descriptor); itHandlers != m_eventHandlerPool.end())
        {
            event_handlers_t& handlers = itHandlers->second;

            if (auto itHandler = handlers.find(id); itHandler != handlers.end())
            {
                handlers.erase(itHandler);
                return;
            }
        }

        throw std::logic_error{ "cannot remove unknown event handler for type: " + descriptor.name() };
    }

    void Dispatcher::dispatch(const Transmission& transmission)
    {
        dispatchTransmission(transmission);
        dispatchEvent(transmission.header(), transmission.instance());
    }

    void Dispatcher::dispatchTransmission(const Transmission& transmission)
    {
        auto dispatchTransmissionToHandlers = [](const transmission_handlers_t& transmissionHandlers, const Transmission& transmission)
        {
            for (const auto& [id, handler] : transmissionHandlers)
            {
                try
                {
                    (void)id;
                    handler(transmission);
                }
                catch (...)
                {
                    // TODO: logging?
                }
            }
        };
        const type::StructDescriptor<>& descriptor = transmission.instance()->_descriptor();

        auto itHandlers = m_transmissionHandlerPool.find(&descriptor);

        if (itHandlers == m_transmissionHandlerPool.end())
        {
            return;
        }

        const transmission_handlers_t& handlers = itHandlers->second;
        dispatchTransmissionToHandlers(handlers, transmission);
    }

    void Dispatcher::dispatchEvent(const DotsHeader& header, const type::AnyStruct& instance)
    {
        const type::StructDescriptor<>& descriptor = instance->_descriptor();
        const event_handlers_t& handlers = m_eventHandlerPool[&descriptor];

        auto dispatchEventToHandlers = [&](const Event<>& e)
        {
            for (const auto& [id, handler] : handlers)
            {
                try
                {
                    (void)id;
                    handler(e);
                }
                catch (...)
                {
                    // TODO: logging?
                }
            }
        };

        if (descriptor.cached())
        {
            Container<>& container = m_containerPool.get(descriptor);

            if (header.removeObj == true)
            {
                if (Container<>::node_t removed = container.remove(header, instance); !removed.empty())
                {
                    dispatchEventToHandlers(Event<>{ header, instance, removed.key(), removed.mapped() });
                }
            }
            else
            {
                const auto& [updated, cloneInfo] = container.insert(header, instance);
                dispatchEventToHandlers(Event<>{ header, instance, updated, cloneInfo });
            }
        }
        else
        {
            if (header.removeObj == true)
            {
                throw std::logic_error{ "cannot remove uncached instance for type: " + descriptor.name() };
            }

            dispatchEventToHandlers(Event<>{ header, instance, instance,
                DotsCloneInformation{
                    DotsCloneInformation::lastOperation_i{ DotsMt::create },
                    DotsCloneInformation::createdFrom_i{ header.sender },
                    DotsCloneInformation::created_i{ header.sentTime },
                    DotsCloneInformation::localUpdateTime_i{ types::timepoint_t::Now() }
                }
            });
        }
    }
}