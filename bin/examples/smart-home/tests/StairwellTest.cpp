// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/testing/gtest/EventTestBase.h>
#include <Stairwell.h>
#include <LightControl.dots.h>

using examples::Stairwell;

struct StairwellTest : dots::testing::EventTestBase
{
    Stairwell m_sut;
};

TEST_F(StairwellTest, ToggleLightWhenUsingSameSwitch)
{
    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            SPOOF_DOTS_PUBLISH(StatelessSwitch{
                StatelessSwitch::id_i{ Stairwell::LowerSwitch }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Stairwell::Light },
            LightControl::brightness_i{ 100u }
        }),
        [this]
        {
            SPOOF_DOTS_PUBLISH(StatelessSwitch{
                StatelessSwitch::id_i{ Stairwell::LowerSwitch }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Stairwell::Light },
            LightControl::brightness_i{ 0u }
        }),
        [this]
        {
            SPOOF_DOTS_PUBLISH(StatelessSwitch{
                StatelessSwitch::id_i{ Stairwell::UpperSwitch }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Stairwell::Light },
            LightControl::brightness_i{ 100u }
        }),
        [this]
        {
            SPOOF_DOTS_PUBLISH(StatelessSwitch{
                StatelessSwitch::id_i{ Stairwell::UpperSwitch }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Stairwell::Light },
            LightControl::brightness_i{ 0u }
        })
    );

    processEvents();
}
TEST_F(StairwellTest, ToggleLightWhenUsingDifferentSwitches)
{
    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            SPOOF_DOTS_PUBLISH(StatelessSwitch{
                StatelessSwitch::id_i{ Stairwell::LowerSwitch }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Stairwell::Light },
            LightControl::brightness_i{ 100u }
        }),
        [this]
        {
            SPOOF_DOTS_PUBLISH(StatelessSwitch{
                StatelessSwitch::id_i{ Stairwell::UpperSwitch }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Stairwell::Light },
            LightControl::brightness_i{ 0u }
        }),
        [this]
        {
            SPOOF_DOTS_PUBLISH(StatelessSwitch{
                StatelessSwitch::id_i{ Stairwell::UpperSwitch }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Stairwell::Light },
            LightControl::brightness_i{ 100u }
        }),
        [this]
        {
            SPOOF_DOTS_PUBLISH(StatelessSwitch{
                StatelessSwitch::id_i{ Stairwell::LowerSwitch }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Stairwell::Light },
            LightControl::brightness_i{ 0u }
        })
    );

    processEvents();
}
