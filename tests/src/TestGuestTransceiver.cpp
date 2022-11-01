// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <sstream>
#include <optional>
#include <dots/testing/gtest/EventTestBase.h>
#include <DotsTestStruct.dots.h>

struct TestGuestTransceiver : dots::testing::EventTestBase
{

protected:

    std::optional<dots::Subscription> m_testStructSubscription;
};

TEST_F(TestGuestTransceiver, IncrementPropertyOfCachedTypeOnSelfUpdate)
{
    DOTS_EXPECTATION_SEQUENCE(
        []
        {
            dots::publish(DotsTestStruct{
                .indKeyfField = 42,
                .int64Field = 0
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            .indKeyfField = 42,
            .int64Field = 0
        }),
        EXPECT_DOTS_SELF_PUBLISH(DotsTestStruct{
            .indKeyfField = 42,
            .int64Field = 0
        }),
        []
        {
            dots::publish(DotsTestStruct{
                .indKeyfField = 42,
                .int64Field = 1
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            .indKeyfField = 42,
            .int64Field = 1
        }),
        EXPECT_DOTS_SELF_PUBLISH(DotsTestStruct{
            .indKeyfField = 42,
            .int64Field = 1
        }),
        []
        {
            dots::publish(DotsTestStruct{
                .indKeyfField = 42,
                .int64Field = 2
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            .indKeyfField = 42,
            .int64Field = 2
        }),
        EXPECT_DOTS_SELF_PUBLISH(DotsTestStruct{
            .indKeyfField = 42,
            .int64Field = 2
        }),
        EXPECT_DOTS_PUBLISH(DotsTestStruct{}).Times(0)
    );

    processEvents();
}

TEST_F(TestGuestTransceiver, IncrementPropertyOfCachedTypeOnOtherUpdate)
{
    m_testStructSubscription = dots::subscribe<DotsTestSubStruct>([](const dots::Event<DotsTestSubStruct>& event)
    {
        ASSERT_FALSE(event.isFromMyself());
        dots::int64_t i = *dots::container<DotsTestStruct>().get({ .indKeyfField = 42 }).int64Field;

        if (i < 2)
        {
            dots::publish(DotsTestStruct{
                .indKeyfField = 42,
                .int64Field = i + 1
            });
        }
    });

    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            SPOOF_DOTS_PUBLISH(DotsTestStruct{
                .indKeyfField = 42,
                .int64Field = 0
            });
            SPOOF_DOTS_PUBLISH(DotsTestSubStruct{
                .flag1 = true
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            .indKeyfField = 42,
            .int64Field = 1
        }),
        [this]
        {
            SPOOF_DOTS_PUBLISH(DotsTestSubStruct{
                .flag1 = true
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            .indKeyfField = 42,
            .int64Field = 2
        }),
        EXPECT_DOTS_PUBLISH(DotsTestStruct{}).Times(0)
    );

    processEvents();
}

TEST_F(TestGuestTransceiver, PublishPartialInstanceWhenPropertiesAreGiven)
{
    DotsTestStruct instance{
        .stringField = "foo",
        .indKeyfField = 42,
        .floatField = 3.1415f,
        .enumField = DotsTestEnum::value3,
        .int64Field = 1
    };

    DOTS_EXPECTATION_SEQUENCE(
        [&]
        {
            dots::publish(instance, DotsTestStruct::floatField_p);
        },
        EXPECT_DOTS_PUBLISH(instance, DotsTestStruct::indKeyfField_p + DotsTestStruct::floatField_p),
        [&]
        {
            dots::publish(instance, DotsTestStruct::int64Field_p);
        },
        EXPECT_DOTS_PUBLISH(instance, DotsTestStruct::indKeyfField_p + DotsTestStruct::int64Field_p),
        [&]
        {
            dots::publish(instance, DotsTestStruct::stringField_p + DotsTestStruct::enumField_p);
        },
        EXPECT_DOTS_PUBLISH(instance, DotsTestStruct::indKeyfField_p + DotsTestStruct::stringField_p + DotsTestStruct::enumField_p)
    );

    processEvents();
}
