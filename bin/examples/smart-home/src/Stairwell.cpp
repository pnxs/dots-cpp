// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <Stairwell.h>
#include <LightControl.dots.h>

namespace examples
{
    Stairwell::Stairwell() :
        m_statelessSwitchSubscription(dots::subscribe<StatelessSwitch>({ &Stairwell::handleStatelessSwitch, this }))
    {
        /* do nothing */
    }

    void Stairwell::handleStatelessSwitch(const dots::Event<StatelessSwitch>& event)
    {
        const StatelessSwitch& statelessSwitch = event();

        if (statelessSwitch.id == LowerSwitch || statelessSwitch.id == UpperSwitch)
        {
            LightControl stairwellLight{
                .id = Light
            };

            auto* existingStairwellLight = dots::container<LightControl>().find(stairwellLight);

            if (existingStairwellLight == nullptr)
            {
                stairwellLight.brightness = 100u;
            }
            else
            {
                stairwellLight.brightness = existingStairwellLight->brightness == 0u ? 100u : 0u;
            }

            dots::publish(stairwellLight);
        }
    }
}
