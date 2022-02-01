#include <dots/Subscription.h>

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
