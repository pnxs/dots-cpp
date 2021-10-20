#include <optional>
#include <dots/testing/gtest/gtest.h>
#include <dots/testing/gtest/PublishTestBase.h>
#include <dots/HostTransceiver.h>

struct TestHostTransceiver : dots::testing::EventTestBase
{
protected:

    TestHostTransceiver()
    {
        /* do nothing */
    }
};

TEST_F(TestHostTransceiver, HandleEchoRequest)
{
    dots::testing::mock_subscription_handler_t mockGuestSubscriber;
    dots::Subscription guestSubscription = dots::subscribe<DotsEcho>(mockGuestSubscriber.AsStdFunction());

    DOTS_EXPECTATION_SEQUENCE(
        [this]
        {
            dots::publish(DotsEcho{
                DotsEcho::request_i(true),
                DotsEcho::identifier_i(42),
                DotsEcho::sequenceNumber_i(1)
            });
        },
        EXPECT_DOTS_PUBLISH_AT_SUBSCRIBER(mockGuestSubscriber, DotsEcho{
            DotsEcho::request_i(false),
            DotsEcho::identifier_i(42),
            DotsEcho::sequenceNumber_i(1)
        }),
        [this]
        {
            dots::publish(DotsEcho{
                DotsEcho::request_i(true),
                DotsEcho::identifier_i(72),
                DotsEcho::sequenceNumber_i(2),
                DotsEcho::data_i{ DotsEcho::_Name }
            });
        },
        EXPECT_DOTS_PUBLISH_AT_SUBSCRIBER(mockGuestSubscriber, DotsEcho{
            DotsEcho::request_i(false),
            DotsEcho::identifier_i(72),
            DotsEcho::sequenceNumber_i(2),
            DotsEcho::data_i{ DotsEcho::_Name }
        })
    );

    processEvents();
}
