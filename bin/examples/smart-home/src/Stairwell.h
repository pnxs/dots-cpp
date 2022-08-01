// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <dots/dots.h>
#include <StatelessSwitch.dots.h>

namespace examples
{
    struct Stairwell
    {
        static constexpr char LowerSwitch[] = "Stairwell_LowerSwitch";
        static constexpr char UpperSwitch[] = "Stairwell_UpperSwitch";
        static constexpr char Light[] = "Stairwell_Light";

        Stairwell();

    private:
        
        void handleStatelessSwitch(const dots::Event<StatelessSwitch>& event);

        dots::Subscription m_statelessSwitchSubscription;
    };
}
