// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <vector>
#include <dots/dots.h>
#include <Switch.dots.h>
#include <Dimmer.dots.h>

namespace examples
{
    struct LivingRoom
    {
        static constexpr char MasterDimmer[] = "LivingRoom_MasterDimmer";
        static constexpr char CouchLight[] = "LivingRoom_CouchLight";
        static constexpr char CeilingLight[] = "LivingRoom_CeilingLight";

        LivingRoom();

    private:

        void handleDimmer(const dots::Event<Dimmer>& event);

        std::vector<dots::Subscription> m_subscriptions;
    };
}
