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
                .id = LivingRoom::MasterDimmer,
                .brightness = 42u
            });
        },
        DOTS_EXPECTATION_SET(
            EXPECT_DOTS_PUBLISH(LightControl{
                .id = LivingRoom::CeilingLight,
                .brightness = 42u
            }),
            EXPECT_DOTS_PUBLISH(LightControl{
                .id = LivingRoom::CouchLight,
                .brightness = 42u
            })
        ),
        [this]
        {
            SPOOF_DOTS_PUBLISH(Dimmer{
                .id = LivingRoom::MasterDimmer,
                .brightness = 72u
            });
        },
        DOTS_EXPECTATION_SET(
            EXPECT_DOTS_PUBLISH(LightControl{
                .id = LivingRoom::CeilingLight,
                .brightness = 72u
            }),
            EXPECT_DOTS_PUBLISH(LightControl{
                .id = LivingRoom::CouchLight,
                .brightness = 72u
            })
        )
    );

    processEvents();
}
