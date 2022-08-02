// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/testing/gtest/EventTestBase.h>
#include <Basement.h>
#include <LightControl.dots.h>

using namespace dots::type::literals;
using examples::Basement;

struct BasementTest : dots::testing::EventTestBase
{
    static constexpr dots::duration_t LightTimeout = 1s;
    static constexpr dots::duration_t TimerPrecision = 100ms;

    Basement m_sut{ LightTimeout };
};

#define EXPECT_DURATION_NEAR(lhs_, rhs_, error_)                                                                                                                         \
{                                                                                                                                                                        \
    EXPECT_NEAR(dots::duration_t{ (lhs_) }.toFractionalSeconds(), dots::duration_t{ (rhs_) }.toFractionalSeconds(), dots::duration_t{ (error_) }.toFractionalSeconds()); \
}

TEST_F(BasementTest, TurnLightOnWhenMotionIsDetected)
{
    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            SPOOF_DOTS_PUBLISH(Switch{
                Switch::id_i{ Basement::MotionSwitch },
                Switch::enabled_i{ true }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Basement::Light },
            LightControl::brightness_i{ 100u }
        })
    );

    processEvents();
}

TEST_F(BasementTest, TurnLightOffAfterTimeoutOccurs)
{
    dots::timepoint_t motionSwitchDisabled = dots::timepoint_t::max();
    dots::timepoint_t lightDisabled = dots::timepoint_t::max();

    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            SPOOF_DOTS_PUBLISH(Switch{
                Switch::id_i{ Basement::MotionSwitch },
                Switch::enabled_i{ true }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Basement::Light },
            LightControl::brightness_i{ 100u }
        }),
        [&]
        {
            SPOOF_DOTS_PUBLISH(Switch{
                Switch::id_i{ Basement::MotionSwitch },
                Switch::enabled_i{ false }
            });
            motionSwitchDisabled = dots::timepoint_t::Now();
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Basement::Light },
            LightControl::brightness_i{ 0u }
        }),
        [&](const dots::Event<>& event)
        {
            lightDisabled = *event.header().sentTime;
        }
    );

    processEvents(LightTimeout + TimerPrecision);
    EXPECT_DURATION_NEAR(lightDisabled - motionSwitchDisabled, LightTimeout, TimerPrecision);
}


TEST_F(BasementTest, DoNotTurnLightOnWhenItIsAlreadyOn)
{
    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            SPOOF_DOTS_PUBLISH(LightControl{
                LightControl::id_i{ Basement::Light },
                LightControl::brightness_i{ 100u }
            });
            SPOOF_DOTS_PUBLISH(Switch{
                Switch::id_i{ Basement::MotionSwitch },
                Switch::enabled_i{ true }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Basement::Light },
            LightControl::brightness_i{ 100u }
        }).Times(0)
    );

    processEvents();
}

TEST_F(BasementTest, DoNotTurnLightOffWhenItIsAlreadyOff)
{
    DOTS_EXPECTATION_SEQUENCE(
        [&]
        {
            SPOOF_DOTS_PUBLISH(LightControl{
                LightControl::id_i{ Basement::Light },
                LightControl::brightness_i{ 0u }
            });
            SPOOF_DOTS_PUBLISH(Switch{
                Switch::id_i{ Basement::MotionSwitch },
                Switch::enabled_i{ false }
            });
        },
        EXPECT_DOTS_PUBLISH(LightControl{
            LightControl::id_i{ Basement::Light }
        }).Times(0)
    );

    processEvents(LightTimeout + TimerPrecision);
}
