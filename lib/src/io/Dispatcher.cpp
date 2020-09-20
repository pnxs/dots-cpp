#include <dots/io/Dispatcher.h>

namespace dots::io
{
    Dispatcher::Dispatcher() :
        m_this(std::make_shared<Dispatcher*>(this))
    {
        /* do nothing*/
    }

    Dispatcher::Dispatcher(Dispatcher&& other) noexcept :
        m_this(std::move(other.m_this)),
        m_containerPool(std::move(other.m_containerPool)),
        m_transmissionHandlerPool(std::move(other.m_transmissionHandlerPool)),
        m_eventHandlerPool(std::move(other.m_eventHandlerPool))
    {
        *m_this = this;
    }

    Dispatcher& Dispatcher::operator = (Dispatcher&& rhs) noexcept
    {
        m_this = std::move(rhs.m_this);
        m_containerPool = std::move(rhs.m_containerPool);
        m_transmissionHandlerPool = std::move(rhs.m_transmissionHandlerPool);
        m_eventHandlerPool = std::move(rhs.m_eventHandlerPool);

        *m_this = this;

        return *this;
    }

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

    Subscription Dispatcher::subscribe(const type::StructDescriptor<>& descriptor, transmission_handler_t&& handler)
    {
        Subscription subscription{ m_this, descriptor };
        m_transmissionHandlerPool[&descriptor].emplace(subscription.id(), std::move(handler));

        return subscription;
    }

    Subscription Dispatcher::subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler)
    {
        Subscription subscription{ m_this, descriptor };
        const event_handler_t<>& handler_ = m_eventHandlerPool[&descriptor].emplace(subscription.id(), std::move(handler)).first->second;

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
                --* header.fromCache;
                handler_(Event<>{ header, instance, instance, cloneInfo, DotsMt::create });
            }
        }

        return subscription;
    }

    void Dispatcher::unsubscribe(const Subscription& subscription)
    {
        auto try_remove_handler = [&subscription](auto& handlerPool)
        {
            if (auto itHandlers = handlerPool.find(&subscription.descriptor()); itHandlers != handlerPool.end())
            {
                auto& handlers = itHandlers->second;
                auto itHandler = handlers.find(subscription.id());

                if (itHandler != handlers.end())
                {
                    handlers.erase(itHandler);
                    return true;
                }
            }

            return false;
        };

        if (!(try_remove_handler(m_eventHandlerPool) || try_remove_handler(m_transmissionHandlerPool)))
        {
            throw std::logic_error{ "cannot unsubscribe unknown subscription for type: " + subscription.descriptor().name() };
        }
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
                Container<>::node_t removed = container.remove(header, instance);
                dispatchEventToHandlers(Event<>{ header, instance, removed.key(), removed.mapped() });
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