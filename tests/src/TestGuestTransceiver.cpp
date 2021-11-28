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
        [this]
        {
            dots::publish(DotsTestStruct{
                DotsTestStruct::indKeyfField_i{ 42 },
                DotsTestStruct::int64Field_i{ 0 }
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            DotsTestStruct::indKeyfField_i{ 42 },
            DotsTestStruct::int64Field_i{ 0 }
        }),
        EXPECT_DOTS_SELF_PUBLISH(DotsTestStruct{
            DotsTestStruct::indKeyfField_i{ 42 },
            DotsTestStruct::int64Field_i{ 0 }
        }),
        [this]
        {
            dots::publish(DotsTestStruct{
                DotsTestStruct::indKeyfField_i{ 42 },
                DotsTestStruct::int64Field_i{ 1 }
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            DotsTestStruct::indKeyfField_i{ 42 },
            DotsTestStruct::int64Field_i{ 1 }
        }),
        EXPECT_DOTS_SELF_PUBLISH(DotsTestStruct{
            DotsTestStruct::indKeyfField_i{ 42 },
            DotsTestStruct::int64Field_i{ 1 }
        }),
        [this]
        {
            dots::publish(DotsTestStruct{
                DotsTestStruct::indKeyfField_i{ 42 },
                DotsTestStruct::int64Field_i{ 2 }
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            DotsTestStruct::indKeyfField_i{ 42 },
            DotsTestStruct::int64Field_i{ 2 }
        }),
        EXPECT_DOTS_SELF_PUBLISH(DotsTestStruct{
            DotsTestStruct::indKeyfField_i{ 42 },
            DotsTestStruct::int64Field_i{ 2 }
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
        dots::int64_t i = *dots::container<DotsTestStruct>().get(DotsTestStruct::indKeyfField_i{ 42 }).int64Field;

        if (i < 2)
        {
            dots::publish(DotsTestStruct{
                DotsTestStruct::indKeyfField_i{ 42 },
                DotsTestStruct::int64Field_i{ i + 1 }
            });
        }
    });

    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            SPOOF_DOTS_PUBLISH(DotsTestStruct{
                DotsTestStruct::indKeyfField_i{ 42 },
                DotsTestStruct::int64Field_i{ 0 }
            });
            SPOOF_DOTS_PUBLISH(DotsTestSubStruct{
                DotsTestSubStruct::flag1_i{ true }
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            DotsTestStruct::indKeyfField_i{ 42 },
            DotsTestStruct::int64Field_i{ 1 }
        }),
        [this]
        {
            SPOOF_DOTS_PUBLISH(DotsTestSubStruct{
                DotsTestSubStruct::flag1_i{ true }
            });
        },
        EXPECT_DOTS_PUBLISH(DotsTestStruct{
            DotsTestStruct::indKeyfField_i{ 42 },
            DotsTestStruct::int64Field_i{ 2 }
        }),
        EXPECT_DOTS_PUBLISH(DotsTestStruct{}).Times(0)
    );

    processEvents();
}

TEST_F(TestGuestTransceiver, PublishPartialInstanceWhenPropertiesAreGiven)
{
    DotsTestStruct instance{
        DotsTestStruct::stringField_i{ "foo" },
        DotsTestStruct::indKeyfField_i{ 42 },
        DotsTestStruct::floatField_i{ 3.1415f },
        DotsTestStruct::enumField_i{ DotsTestEnum::value3 },
        DotsTestStruct::int64Field_i{ 1 }
    };

    DOTS_EXPECTATION_SEQUENCE(
        [&]
        {
            dots::publish(instance, DotsTestStruct::indKeyfField_p + DotsTestStruct::floatField_p);
        },
        EXPECT_DOTS_PUBLISH(instance, DotsTestStruct::indKeyfField_p + DotsTestStruct::floatField_p),
        [&]
        {
            dots::publish(instance, DotsTestStruct::indKeyfField_p + DotsTestStruct::int64Field_p);
        },
        EXPECT_DOTS_PUBLISH(instance, DotsTestStruct::indKeyfField_p + DotsTestStruct::int64Field_p),
        [&]
        {
            dots::publish(instance, DotsTestStruct::indKeyfField_p + DotsTestStruct::stringField_p + DotsTestStruct::enumField_p);
        },
        EXPECT_DOTS_PUBLISH(instance, DotsTestStruct::indKeyfField_p + DotsTestStruct::stringField_p + DotsTestStruct::enumField_p)
    );

    processEvents();
}
