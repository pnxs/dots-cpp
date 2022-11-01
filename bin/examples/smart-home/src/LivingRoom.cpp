// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
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
                .id = CouchLight,
                .brightness = dimmer.brightness
            });

            dots::publish(LightControl{
                .id = CeilingLight,
                .brightness = dimmer.brightness
            });
        }
    }
}
