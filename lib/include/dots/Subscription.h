#pragma once
#include <functional>

namespace dots
{
    struct [[nodiscard]] Subscription
    {
        using unsubscribe_handler_t = std::function<void()>;

        Subscription(unsubscribe_handler_t&& handler);
        Subscription(const Subscription& other) = delete;
        Subscription(Subscription&& other) noexcept;
        ~Subscription();

        Subscription& operator = (const Subscription& rhs) = delete;
        Subscription& operator = (Subscription&& rhs) noexcept;

        void unsubscribe();
        void discard();

    private:

        unsubscribe_handler_t m_handler;
    };
}