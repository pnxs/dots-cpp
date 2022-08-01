// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
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
        ~Basement() = default;

        Basement& operator = (const Basement& rhs) = delete;
        Basement& operator = (Basement&& rhs) = default;

    private:

        void handleSwitch(const dots::Event<Switch>& event);

        std::optional<dots::Timer> m_timer;
        dots::duration_t m_lightTimeout;
        dots::Subscription m_switchSubscription;
    };
}
