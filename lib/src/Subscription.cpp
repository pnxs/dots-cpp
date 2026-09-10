// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/Subscription.h>

// GCC 15 emits a spurious -Wmaybe-uninitialized on the inlined
// std::function destructor reached through std::optional<Handler>'s
// payload union in ~Subscription / operator=(&&). The optional's
// _M_engaged guard makes the access well-defined; the analyzer just
// can't see it across the inlined chain. Scope the suppression to
// the two members that trigger it.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

namespace dots
{
    Subscription::Subscription(unsubscribe_handler_t handler) :
        m_handler{ std::move(handler) }
    {
        /* do nothing */
    }

    Subscription::Subscription(Subscription&& other) noexcept :
        m_handler(std::move(other.m_handler))
    {
        other.m_handler = std::nullopt;
    }

    Subscription::~Subscription()
    {
        unsubscribe();
    }

    Subscription& Subscription::operator = (Subscription&& rhs) noexcept
    {
        unsubscribe();

        m_handler = std::move(rhs.m_handler);
        rhs.m_handler = std::nullopt;

        return *this;
    }

    void Subscription::unsubscribe()
    {
        if (m_handler != std::nullopt)
        {
            (*m_handler)();
            m_handler = std::nullopt;
        }
    }

    void Subscription::discard()
    {
        m_handler = std::nullopt;
    }
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
