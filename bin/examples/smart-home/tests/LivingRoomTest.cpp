// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/testing/gtest/EventTestBase.h>
#include <LivingRoom.h>
#include <LightControl.dots.h>

using examples::LivingRoom;

struct LivingRoomTest : dots::testing::EventTestBase
{
    LivingRoom m_sut;
};

TEST_F(LivingRoomTest, UpdateLightsWhenDimmerChanges)
{
    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            SPOOF_DOTS_PUBLISH(Dimmer{
                Dimmer::id_i{ LivingRoom::MasterDimmer },
                Dimmer::brightness_i{ 42u }
            });
        },
        DOTS_EXPECTATION_SET(
            EXPECT_DOTS_PUBLISH(LightControl{
                LightControl::id_i{ LivingRoom::CeilingLight },
                LightControl::brightness_i{ 42u }
            }),
            EXPECT_DOTS_PUBLISH(LightControl{
                LightControl::id_i{ LivingRoom::CouchLight },
                LightControl::brightness_i{ 42u }
            })
        ),
        [this]
        {
            SPOOF_DOTS_PUBLISH(Dimmer{
                Dimmer::id_i{ LivingRoom::MasterDimmer },
                Dimmer::brightness_i{ 72u }
            });
        },
        DOTS_EXPECTATION_SET(
            EXPECT_DOTS_PUBLISH(LightControl{
                LightControl::id_i{ LivingRoom::CeilingLight },
                LightControl::brightness_i{ 72u }
            }),
            EXPECT_DOTS_PUBLISH(LightControl{
                LightControl::id_i{ LivingRoom::CouchLight },
                LightControl::brightness_i{ 72u }
            })
        )
    );

    processEvents();
}
