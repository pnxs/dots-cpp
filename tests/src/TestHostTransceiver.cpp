// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <optional>
#include <dots/testing/gtest/gtest.h>
#include <dots/testing/gtest/EventTestBase.h>
#include <dots/HostTransceiver.h>

struct TestHostTransceiver : dots::testing::EventTestBase
{
};

TEST_F(TestHostTransceiver, HandleEchoRequest)
{
    dots::testing::mock_subscription_handler_t mockGuestSubscriber;
    dots::Subscription guestSubscription = dots::subscribe<DotsEcho>(mockGuestSubscriber.AsStdFunction());

    DOTS_EXPECTATION_SEQUENCE(
        []
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
        []
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
