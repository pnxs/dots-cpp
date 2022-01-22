#include <LivingRoom.h>
#include <LightControl.dots.h>

namespace examples
{
    LivingRoom::LivingRoom()
    {
        m_subscriptions.emplace_back(dots::subscribe<Dimmer>({ &LivingRoom::handleDimmer, this }));
    }

    void LivingRoom::handleDimmer(const dots::Event<Dimmer>& event)
    {
        if (const Dimmer& dimmer = event(); dimmer.id == MasterDimmer)
        {
            dots::publish(LightControl{
                LightControl::id_i{ CouchLight },
                LightControl::brightness_i{ dimmer.brightness }
            });

            dots::publish(LightControl{
                LightControl::id_i{ CeilingLight },
                LightControl::brightness_i{ dimmer.brightness }
            });
        }
    }
}
