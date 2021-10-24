#include <sstream>
#include <optional>
#include <dots/testing/gtest/EventTestBase.h>
#include <DotsTestStruct.dots.h>

struct TestGuestTransceiver : dots::testing::EventTestBase
{
    TestGuestTransceiver()
    {
        /* do nothing */
    }

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
