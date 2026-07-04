// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/Dispatcher.h>
#include <algorithm>

namespace dots
{
    Dispatcher::Dispatcher(error_handler_t handler) :
        m_nextId(0),
        m_errorHandler{ std::move(handler) }
    {
        /* do nothing */
    }

    Dispatcher::~Dispatcher()
    {
        clear();
    }

    void Dispatcher::clear()
    {
        m_clearingHandlers = true;
        m_transmissionHandlerPool.clear();
        m_eventHandlerPool.clear();
        m_clearingHandlers = false;
    }

    const ContainerPool& Dispatcher::pool() const
    {
        return m_containerPool;
    }

    ContainerPool& Dispatcher::pool()
    {
        return m_containerPool;
    }

    const Container<>& Dispatcher::container(const type::StructDescriptor& descriptor) const
    {
        return m_containerPool.get(descriptor);
    }

    Container<>& Dispatcher::container(const type::StructDescriptor& descriptor)
    {
        return m_containerPool.get(descriptor);
    }

    auto Dispatcher::addTransmissionHandler(const type::StructDescriptor& descriptor, transmission_handler_t handler) -> id_t
    {
        id_t id = m_nextId++;
        if (m_clearingHandlers) return id;
        m_transmissionHandlerPool[&descriptor].emplace(id, std::move(handler));

        return id;
    }

    auto Dispatcher::addEventHandler(const type::StructDescriptor& descriptor, event_handler_t<> handler) -> id_t
    {
        id_t id = addEventHandlerNoReplay(descriptor, std::move(handler));
        replayCacheToHandler(descriptor, id);
        return id;
    }

    auto Dispatcher::addEventHandlerNoReplay(const type::StructDescriptor& descriptor, event_handler_t<> handler) -> id_t
    {
        id_t id = m_nextId++;
        if (m_clearingHandlers) return id;
        m_eventHandlerPool[&descriptor].emplace(id, std::move(handler));
        return id;
    }

    void Dispatcher::replayCacheToHandler(const type::StructDescriptor& descriptor, id_t id)
    {
        if (m_clearingHandlers) return;

        auto itHandlers = m_eventHandlerPool.find(&descriptor);
        if (itHandlers == m_eventHandlerPool.end()) return;

        auto& handlers = itHandlers->second;
        auto itHandler = handlers.find(id);
        if (itHandler == handlers.end()) return;

        const event_handler_t<>& handler_ = itHandler->second;
        const Container<>& container = m_containerPool.get(descriptor);
        if (container.empty()) return;

        DotsHeader header{
            .typeName = descriptor.name(),
            .fromCache = static_cast<uint32_t>(container.size()),
            .removeObj = false,
            .isFromMyself = false
        };

        for (const auto& [instance, cloneInfo] : container)
        {
            header.attributes = instance->_validProperties();
            --*header.fromCache;
            handler_(Event<>{ header, instance, instance, cloneInfo, DotsMt::create });
        }
    }

    void Dispatcher::removeTransmissionHandler(const type::StructDescriptor& descriptor, id_t id)
    {
        if (m_clearingHandlers) return;
        removeHandler(m_transmissionHandlerPool, descriptor, id);
    }

    void Dispatcher::removeEventHandler(const type::StructDescriptor& descriptor, id_t id)
    {
        if (m_clearingHandlers) return;
        removeHandler(m_eventHandlerPool, descriptor, id);
    }

    void Dispatcher::dispatch(const io::Transmission& transmission)
    {
        dispatchTransmission(transmission);
        dispatchEvent(transmission.header(), transmission.instance());
    }

    template <typename HandlerPool>
    void Dispatcher::removeHandler(HandlerPool& handlerPool, const type::StructDescriptor& descriptor, id_t id)
    {
        if (m_clearingHandlers) return;
        if (auto itHandlers = handlerPool.find(&descriptor); itHandlers != handlerPool.end())
        {
            auto& handlers = itHandlers->second;

            if (auto itHandler = handlers.find(id); itHandler != handlers.end())
            {
                // defer removal if the handler is currently being dispatched
                // at any level of the (possibly re-entrant) dispatch stack
                if (std::find(m_currentlyDispatchingIds.begin(), m_currentlyDispatchingIds.end(), id) != m_currentlyDispatchingIds.end())
                {
                    m_removeIds.emplace_back(id);
                }
                else
                {
                    handlers.erase(itHandler);
                }

                return;
            }
        }

        throw std::logic_error{ "cannot remove unknown handler for type: " + descriptor.name() };
    }

    void Dispatcher::dispatchTransmission(const io::Transmission& transmission)
    {
        const type::StructDescriptor& descriptor = transmission.instance()->_descriptor();

        auto itHandlers = m_transmissionHandlerPool.find(&descriptor);

        if (itHandlers == m_transmissionHandlerPool.end())
        {
            return;
        }

        transmission_handlers_t& handlers = itHandlers->second;
        dispatchToHandlers(descriptor, handlers, transmission);
    }

    void Dispatcher::dispatchEvent(const DotsHeader& header, const type::AnyStruct& instance)
    {
        const type::StructDescriptor& descriptor = instance->_descriptor();
        event_handlers_t& handlers = m_eventHandlerPool[&descriptor];

        if (descriptor.cached())
        {
            Container<>& container = m_containerPool.get(descriptor);

            if (header.removeObj == true)
            {
                if (Container<>::node_t removed = container.remove(header, instance); !removed.empty())
                {
                    dispatchToHandlers(descriptor, handlers, Event<>{ header, instance, removed.key(), removed.mapped() });
                }
            }
            else
            {
                const auto& [updated, cloneInfo] = container.insert(header, instance);
                dispatchToHandlers(descriptor, handlers, Event<>{ header, instance, updated, cloneInfo });
            }
        }
        else
        {
            if (header.removeObj == true)
            {
                throw std::logic_error{ "cannot remove uncached instance for type: " + descriptor.name() };
            }

            DotsCloneInformation cloneInfo{
                .lastOperation = DotsMt::create,
                .created = header.sentTime,
                .createdFrom = header.sender,
                .localUpdateTime = timepoint_t::Now()
            };

            dispatchToHandlers(descriptor, handlers, Event<>{ header, instance, instance, cloneInfo });
        }
    }

    template <typename Handlers, typename Dispatchable>
    void Dispatcher::dispatchToHandlers(const type::StructDescriptor& descriptor, Handlers& handlers, const Dispatchable& dispatchable)
    {
        for (const auto& [id, handler] : handlers)
        {
            m_currentlyDispatchingIds.emplace_back(id);

            try
            {
                handler(dispatchable);
            }
            catch (...)
            {
                m_errorHandler(descriptor, std::current_exception());
            }

            m_currentlyDispatchingIds.pop_back();
        }

        // Drain deferred removals belonging to this frame's handler map.
        // Ids that are still being dispatched in an enclosing frame or that
        // live in another frame's map are kept for that frame to drain (ids
        // are globally unique, so erase() only succeeds on the owning map).
        m_removeIds.erase(std::remove_if(m_removeIds.begin(), m_removeIds.end(), [&](id_t id)
        {
            if (std::find(m_currentlyDispatchingIds.begin(), m_currentlyDispatchingIds.end(), id) != m_currentlyDispatchingIds.end())
            {
                return false;
            }

            return handlers.erase(id) > 0;
        }), m_removeIds.end());
    }
}
