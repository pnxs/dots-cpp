#include <optional>
#include <dots/testing/gtest/gtest.h>
#include <dots/testing/gtest/PublishTestBase.h>
#include <dots/HostTransceiver.h>
#include <dots/tools/fun.h>

struct TestHostTransceiver : dots::testing::PublishTestBase
{
protected:

    TestHostTransceiver()
    {
        /* do nothing */
    }

};

TEST_F(TestHostTransceiver, testDotsEcho)
{
    std::optional<DotsEcho> reply;

    dots::Subscription subscription = dots::subscribe<DotsEcho>([&](const DotsEcho::Cbd& event) {
        reply = event();
    });

    dots::publish(DotsEcho{
        DotsEcho::request_i(true),
        DotsEcho::identifier_i(0),
        DotsEcho::sequenceNumber_i(1)
    });

    processEvents();

    ASSERT_TRUE(reply.has_value());
    EXPECT_EQ(false, reply->request);
    EXPECT_EQ(0u, reply->identifier);
    EXPECT_EQ(1u, reply->sequenceNumber);

}
