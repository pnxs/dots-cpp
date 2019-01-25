
#include "dots/type/property_set.h"
#include <gtest/gtest.h>

TEST(TestPropertyTest, testCount)
{
    dots::property_set all = PROPERTY_SET_ALL;
    EXPECT_EQ(all.count(), 32u);

    EXPECT_EQ(dots::property_set(0x01).count(), 1u);
    EXPECT_EQ(dots::property_set(0x02).count(), 1u);
    EXPECT_EQ(dots::property_set(0x04).count(), 1u);
    EXPECT_EQ(dots::property_set(0x10).count(), 1u);
    EXPECT_EQ(dots::property_set(0x100000).count(), 1u);
    EXPECT_EQ(dots::property_set(0x800000).count(), 1u);

    EXPECT_EQ(dots::property_set(0x11).count(), 2u);
    EXPECT_EQ(dots::property_set(0x12).count(), 2u);
    EXPECT_EQ(dots::property_set(0x14).count(), 2u);
    EXPECT_EQ(dots::property_set(0x1040).count(), 2u);
    EXPECT_EQ(dots::property_set(0x100080).count(), 2u);
    EXPECT_EQ(dots::property_set(0x800020).count(), 2u);

    EXPECT_EQ(dots::property_set(0xFFFFFFFE).count(), 31u);
}

TEST(TestPropertyTest, testInvert)
{
    dots::property_set all = PROPERTY_SET_ALL;

    dots::property_set p = ~all;
    EXPECT_EQ(p.count(), 0u);

    p = dots::property_set(0x0000FFFF);
    EXPECT_EQ(p, dots::property_set(0x0000FFFF));
    EXPECT_EQ(p.count(), 16u);

    p = ~p;
    EXPECT_EQ(p, dots::property_set(0xFFFF0000));
    EXPECT_EQ(p.count(), 16u);
}

TEST(TestPropertyTest, testAnd)
{
    dots::property_set lhs(0x10);
    dots::property_set rhs(0x01);

    auto r = lhs & rhs;
    EXPECT_EQ(r, dots::property_set(0));

    lhs &= rhs;
    EXPECT_EQ(lhs, dots::property_set(0));

    lhs = dots::property_set(0x11);
    rhs = dots::property_set(0x01);

    r = lhs & rhs;
    EXPECT_EQ(r, dots::property_set(0x01));

    lhs &= rhs;
    EXPECT_EQ(lhs, dots::property_set(0x01));
}

TEST(TestPropertyTest, testLogic)
{
    //dots::property_set all = PROPERTY_SET_ALL;

    dots::property_set lhs(0x16);
    dots::property_set rhs(0x06);

    dots::property_set props = PROPERTY_SET_ALL;

    lhs &= ~props; // Clear all flags in lhs that are set in props
    EXPECT_EQ(lhs.count(), 0u);

    props &= rhs; // Combine (AND) props and rhs
    EXPECT_EQ(props, dots::property_set(0x06));

    lhs |= props;
    EXPECT_EQ(lhs, dots::property_set(0x06));
}

TEST(TestPropertyTest, testCompare)
{
    dots::property_set lhs(0x02);
    dots::property_set rhs(0x06);

    EXPECT_FALSE(lhs == rhs);
    EXPECT_TRUE(lhs != rhs);
}