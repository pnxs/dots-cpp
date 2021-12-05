#pragma once
#include <optional>
#include <dots/dots.h>
#include <Switch.dots.h>

namespace examples
{
    struct Basement
    {
        static constexpr char MotionSwitch[] = "Basement_MotionSwitch";
        static constexpr char Light[] = "Basement_Light";

        Basement(dots::duration_t lightTimeout);
        Basement(const Basement& other) = delete;
        Basement(Basement&& other) = default;
        ~Basement();

        Basement& operator = (const Basement& rhs) = delete;
        Basement& operator = (Basement&& rhs) = default;

    private:

        void handleSwitch(const dots::Event<Switch>& event);
        void tryRemoveTimer();

        std::optional<dots::Timer::id_t> m_timerId;
        dots::duration_t m_lightTimeout;
        dots::Subscription m_switchSubscription;
    };
}
