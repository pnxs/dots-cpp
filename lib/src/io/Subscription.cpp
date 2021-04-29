#include <dots/io/Subscription.h>

namespace dots::io
{
    Subscription::Subscription(unsubscribe_handler_t&& handler) :
        m_handler{ std::move(handler) }
    {
        /* do nothing */
    }

    Subscription::Subscription(Subscription&& other) noexcept :
        m_handler(std::move(other.m_handler))
    {
        other.m_handler = nullptr;
    }

    Subscription::~Subscription()
    {
        unsubscribe();
    }

    Subscription& Subscription::operator = (Subscription&& rhs) noexcept
    {
        unsubscribe();

        m_handler = std::move(rhs.m_handler);
        rhs.m_handler = nullptr;

        return *this;
    }

    void Subscription::unsubscribe()
    {
        if (m_handler != nullptr)
        {
            m_handler();
            m_handler = nullptr;
        }
    }

    void Subscription::discard()
    {
        m_handler = nullptr;
    }
}