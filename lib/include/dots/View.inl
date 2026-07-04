// SPDX-License-Identifier: LGPL-3.0-only
#pragma once
#include <algorithm>
#include <stdexcept>
#include <dots/Connection.h>
#include <dots/GuestTransceiver.h>
#include <dots/io/Transmission.h>
#include <DotsMember.dots.h>

namespace dots
{
    GuestTransceiver& global_transceiver();
}

namespace dots
{
    template <typename T>
    View<T>::View(GuestTransceiver& transceiver, DotsFilter filter) :
        m_tx(&transceiver)
    {
        m_filter = std::move(filter);

        if (!m_tx->connection().peerCapabilities().filteredSubscriptions.isValid()
            || *m_tx->connection().peerCapabilities().filteredSubscriptions != true)
        {
            throw std::runtime_error{
                "broker does not advertise filteredSubscriptions capability — "
                "cannot create dots::View"
            };
        }

        m_subId = m_tx->_allocateSubscriptionId();
        m_tx->_registerView(m_subId, this);

        // Ensure the broker has the type descriptor on file before the join —
        // statically-declared subscribes get this for free during the early
        // handshake, but runtime-created Views need to push the descriptor
        // explicitly so the join doesn't land on an unknown type.
        m_tx->_ensureHostKnowsType(T::_Descriptor());

        DotsMember member{
            .groupName = T::_Descriptor().name(),
            .event     = DotsMemberEvent::join,
            .subscriptionId = m_subId,
            .filter    = m_filter
        };
        m_tx->publish(member);
    }

    template <typename T>
    View<T>::~View()
    {
        if (m_tx == nullptr) return;
        try
        {
            if (m_tx->connected())
            {
                DotsMember member{
                    .groupName      = T::_Descriptor().name(),
                    .event          = DotsMemberEvent::leave,
                    .subscriptionId = m_subId
                };
                m_tx->publish(member);
            }
        }
        catch (...)
        {
            // best-effort during destruction
        }
        m_tx->_unregisterView(m_subId);
    }

    template <typename T>
    Subscription View<T>::subscribe(handler_t handler)
    {
        const handler_id_t id = m_nextHandlerId++;
        m_handlers.emplace_back(id, std::move(handler));

        // Synchronous replay: fire the new handler for each instance currently
        // in this view's container. The handler may re-entrantly subscribe
        // (reallocating m_handlers) or unsubscribe itself, so it is re-located
        // by id on every iteration instead of holding a reference.
        ++m_invokeDepth;
        try
        {
            for (const auto& [instance, cloneInfo] : m_container)
            {
                auto it = std::find_if(m_handlers.begin(), m_handlers.end(),
                    [id](const auto& kv) { return kv.first == id; });
                if (it == m_handlers.end() || isMarkedForRemoval(id)) break;

                DotsHeader header{
                    .typeName       = m_container.descriptor().name(),
                    .sentTime       = *cloneInfo.modified,
                    .serverSentTime = *cloneInfo.modified,
                    .attributes     = instance->_validProperties(),
                    .sender         = *cloneInfo.lastUpdateFrom,
                    .removeObj      = false,
                    .subscriptionId = m_subId
                };
                event_base_t e{ header, *instance, *instance, cloneInfo, DotsMt::create };
                it->second(static_cast<const event_t&>(e));
            }
        }
        catch (...)
        {
            --m_invokeDepth;
            drainRemovedHandlers();
            throw;
        }
        --m_invokeDepth;
        drainRemovedHandlers();

        return Subscription{ [this, id]
        {
            eraseHandler(id);
        } };
    }

    template <typename T>
    void View<T>::invokeHandlers(const event_base_t& event)
    {
        // Handlers may subscribe or unsubscribe re-entrantly: iterate by index
        // over the initial count (re-entrant subscribes push_back and possibly
        // reallocate, and their replay already covers the current event) and
        // defer removals until the dispatch unwinds.
        ++m_invokeDepth;
        try
        {
            const std::size_t count = m_handlers.size();
            for (std::size_t i = 0; i < count; ++i)
            {
                if (isMarkedForRemoval(m_handlers[i].first)) continue;
                m_handlers[i].second(static_cast<const event_t&>(event));
            }
        }
        catch (...)
        {
            --m_invokeDepth;
            drainRemovedHandlers();
            throw;
        }
        --m_invokeDepth;
        drainRemovedHandlers();
    }

    template <typename T>
    void View<T>::eraseHandler(handler_id_t id)
    {
        if (m_invokeDepth > 0)
        {
            // Deferred: erasing now would invalidate the live dispatch (or
            // destroy the handler while it is executing).
            m_removeIds.push_back(id);
            return;
        }

        auto it = std::find_if(m_handlers.begin(), m_handlers.end(),
            [id](const auto& kv) { return kv.first == id; });
        if (it != m_handlers.end()) m_handlers.erase(it);
    }

    template <typename T>
    void View<T>::drainRemovedHandlers()
    {
        if (m_invokeDepth > 0 || m_removeIds.empty()) return;

        std::vector<handler_id_t> removeIds = std::move(m_removeIds);
        m_removeIds.clear();

        for (handler_id_t id : removeIds)
        {
            eraseHandler(id);
        }
    }

    template <typename T>
    bool View<T>::isMarkedForRemoval(handler_id_t id) const
    {
        return std::find(m_removeIds.begin(), m_removeIds.end(), id) != m_removeIds.end();
    }

    template <typename T>
    void View<T>::_dispatch(const io::Transmission& transmission)
    {
        const DotsHeader& header = transmission.header();
        const type::Struct& instance = *transmission.instance();

        if (header.removeObj == true)
        {
            Container<>::node_t removed = m_container.remove(header, instance);
            if (!removed.empty())
            {
                event_base_t e{ header, instance, removed.key().get(), removed.mapped(), DotsMt::remove };
                invokeHandlers(e);
            }
        }
        else
        {
            const auto& [updated, cloneInfo] = m_container.insert(header, instance);
            event_base_t e{ header, instance, updated.get(), cloneInfo };
            invokeHandlers(e);
        }
    }

    template <typename T>
    View<T> view(filter::Filter f)
    {
        return View<T>(global_transceiver(), std::move(f).build());
    }

    template <typename T>
    View<T> view(DotsFilter f)
    {
        return View<T>(global_transceiver(), std::move(f));
    }
}
