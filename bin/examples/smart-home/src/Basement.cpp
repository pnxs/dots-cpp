#include <Basement.h>
#include <LightControl.dots.h>

namespace examples
{
    Basement::Basement(dots::duration_t lightTimeout) :
        m_lightTimeout(lightTimeout),
        m_switchSubscription{ dots::subscribe<Switch>(&Basement::handleSwitch, this) }
    {
        /* do nothing */
    }

    Basement::~Basement()
    {
        tryRemoveTimer();
    }

    void Basement::handleSwitch(const dots::Event<Switch>& event)
    {
        if (const Switch& switch_ = event(); switch_.id == MotionSwitch)
        {
            auto* existingBasementLight = dots::container<LightControl>().find(LightControl::id_i{ Light });

            if (switch_.enabled == true)
            {
                if (existingBasementLight == nullptr || existingBasementLight->brightness == 0u)
                {
                    dots::publish(LightControl{
                        LightControl::id_i{ Light },
                        LightControl::brightness_i{ 100u }
                    });
                }
            }
            else if (existingBasementLight != nullptr && existingBasementLight->brightness != 0u)
            {
                tryRemoveTimer();

                m_timerId = dots::add_timer(m_lightTimeout, [this]
                {
                    dots::publish(LightControl{
                        LightControl::id_i{ Light },
                        LightControl::brightness_i{ 0u }
                    });
                });
            }
        }
    }

    void Basement::tryRemoveTimer()
    {
        if (m_timerId != std::nullopt)
        {
            dots::remove_timer(*m_timerId);
        }
    }
}
