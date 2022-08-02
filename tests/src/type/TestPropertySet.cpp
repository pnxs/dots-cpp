// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/PropertySet.h>

using dots::type::PropertySet;

TEST(TestPropertySet, ctor)
{
    EXPECT_TRUE(PropertySet{}.empty());
    EXPECT_TRUE(PropertySet{ 0x00000000 }.empty());
    EXPECT_TRUE(PropertySet{ PropertySet::None }.empty());

    EXPECT_FALSE(PropertySet{ 0xFFFFFFFF }.empty());
    EXPECT_FALSE(PropertySet{ PropertySet::All }.empty());
}

TEST(TestPropertySet, count)
{
    EXPECT_EQ(PropertySet{ 0x00000000 }.count(), 0);
    EXPECT_EQ(PropertySet{ 0xFFFFFFFF }.count(), 32);
    EXPECT_EQ(PropertySet{ 0x11FFF387 }.count(), 20);
    EXPECT_EQ(PropertySet{ 0x00408231 }.count(), 6);
}

TEST(TestPropertySet, operator_equal)
{
    EXPECT_EQ(PropertySet{ PropertySet::None }, PropertySet{ 0x00000000 });
    EXPECT_EQ(PropertySet{ PropertySet::All }, PropertySet{ 0xFFFFFFFF });

    EXPECT_EQ(PropertySet{ 0x00000000 }, PropertySet{ 0x00000000 });
    EXPECT_EQ(PropertySet{ 0xFFFFFFFF }, PropertySet{ 0xFFFFFFFF });
    EXPECT_EQ(PropertySet{ 0x12345678 }, PropertySet{ 0x12345678 });

    EXPECT_NE(PropertySet{ 0x00000000 }, PropertySet{ 0xFFFFFFFF });
    EXPECT_NE(PropertySet{ 0x12345678 }, PropertySet{ 0x87654321 });
}

TEST(TestPropertySet, operator_union)
{
    EXPECT_EQ(PropertySet{ 0x00000000 } + PropertySet{ 0x00000000 }, PropertySet{ 0x00000000 });
    EXPECT_EQ(PropertySet{ 0x00000000 } + PropertySet{ 0xFFFFFFFF }, PropertySet{ 0xFFFFFFFF });
    EXPECT_EQ(PropertySet{ 0xFFFFFFFF } + PropertySet{ 0x00000000 }, PropertySet{ 0xFFFFFFFF });
    EXPECT_EQ(PropertySet{ 0xFFFFFFFF } + PropertySet{ 0xFFFFFFFF }, PropertySet{ 0xFFFFFFFF });

    EXPECT_EQ(PropertySet{ 0x00FF00FF } + PropertySet{ 0x0000FF00 }, PropertySet{ 0x00FFFFFF });
    EXPECT_EQ(PropertySet{ 0x00FF00FF } + PropertySet{ 0x00FF0100 }, PropertySet{ 0x00FF01FF });
}

TEST(TestPropertySet, operator_difference)
{
    EXPECT_EQ(PropertySet{ 0x00000000 } - PropertySet{ 0x00000000 }, PropertySet{ 0x00000000 });
    EXPECT_EQ(PropertySet{ 0x00000000 } - PropertySet{ 0xFFFFFFFF }, PropertySet{ 0x00000000 });
    EXPECT_EQ(PropertySet{ 0xFFFFFFFF } - PropertySet{ 0xFFFFFFFF }, PropertySet{ 0x00000000 });
    EXPECT_EQ(PropertySet{ 0xFFFFFFFF } - PropertySet{ 0x00000000 }, PropertySet{ 0xFFFFFFFF });

    EXPECT_EQ(PropertySet{ 0x00FF00FF } - PropertySet{ 0x000000FF }, PropertySet{ 0x00FF0000 });
    EXPECT_EQ(PropertySet{ 0x00FF00FF } - PropertySet{ 0xFFFF0000 }, PropertySet{ 0x000000FF });
}

TEST(TestPropertySet, operator_intersection)
{
    EXPECT_EQ(PropertySet{ 0x00000000 } ^ PropertySet{ 0x00000000 }, PropertySet{ 0x00000000 });
    EXPECT_EQ(PropertySet{ 0x00000000 } ^ PropertySet{ 0xFFFFFFFF }, PropertySet{ 0x00000000 });
    EXPECT_EQ(PropertySet{ 0xFFFFFFFF } ^ PropertySet{ 0xFFFFFFFF }, PropertySet{ 0xFFFFFFFF });
    EXPECT_EQ(PropertySet{ 0xFFFFFFFF } ^ PropertySet{ 0x00000000 }, PropertySet{ 0x00000000 });

    EXPECT_EQ(PropertySet{ 0x00FF00FF } ^ PropertySet{ 0x000000FF }, PropertySet{ 0x000000FF });
    EXPECT_EQ(PropertySet{ 0x00FF00FF } ^ PropertySet{ 0xFFFF0000 }, PropertySet{ 0x00FF0000 });
}

TEST(TestPropertySet, operator_subset)
{
    EXPECT_TRUE(PropertySet{ 0x00000000 } <= PropertySet{ 0x00000000 });
    EXPECT_TRUE(PropertySet{ 0x00000000 } <= PropertySet{ 0xFFFFFFFF });
    EXPECT_TRUE(PropertySet{ 0xFFFFFFFF } <= PropertySet{ 0xFFFFFFFF });
    EXPECT_FALSE(PropertySet{ 0xFFFFFFFF } <= PropertySet{ 0x00000000 });

    EXPECT_TRUE(PropertySet{ 0x000000FF } <= PropertySet{ 0x00FF00FF });
    EXPECT_FALSE(PropertySet{ 0x00FF00FF } <= PropertySet{ 0x00FF00F0 });
}

TEST(TestPropertySet, operator_strict_subset)
{
    EXPECT_FALSE(PropertySet{ 0x00000000 } < PropertySet{ 0x00000000 });
    EXPECT_TRUE(PropertySet{ 0x00000000 } < PropertySet{ 0xFFFFFFFF });
    EXPECT_FALSE(PropertySet{ 0xFFFFFFFF } < PropertySet{ 0xFFFFFFFF });
    EXPECT_FALSE(PropertySet{ 0xFFFFFFFF } < PropertySet{ 0x00000000 });

    EXPECT_TRUE(PropertySet{ 0x000000FF } < PropertySet{ 0x00FF00FF });
    EXPECT_FALSE(PropertySet{ 0x00FF00FF } < PropertySet{ 0x00FF00F0 });
}

TEST(TestPropertySet, operator_superset)
{
    EXPECT_TRUE(PropertySet{ 0x00000000 } >= PropertySet{ 0x00000000 });
    EXPECT_FALSE(PropertySet{ 0x00000000 } >= PropertySet{ 0xFFFFFFFF });
    EXPECT_TRUE(PropertySet{ 0xFFFFFFFF } >= PropertySet{ 0xFFFFFFFF });
    EXPECT_TRUE(PropertySet{ 0xFFFFFFFF } >= PropertySet{ 0x00000000 });

    EXPECT_FALSE(PropertySet{ 0x000000FF } >= PropertySet{ 0x00FF00FF });
    EXPECT_TRUE(PropertySet{ 0x00FF00FF } >= PropertySet{ 0x00FF00F0 });
}

TEST(TestPropertySet, operator_strict_superset)
{
    EXPECT_FALSE(PropertySet{ 0x00000000 } > PropertySet{ 0x00000000 });
    EXPECT_FALSE(PropertySet{ 0x00000000 } > PropertySet{ 0xFFFFFFFF });
    EXPECT_FALSE(PropertySet{ 0xFFFFFFFF } > PropertySet{ 0xFFFFFFFF });
    EXPECT_TRUE(PropertySet{ 0xFFFFFFFF } > PropertySet{ 0x00000000 });

    EXPECT_FALSE(PropertySet{ 0x000000FF } > PropertySet{ 0x00FF00FF });
    EXPECT_TRUE(PropertySet{ 0x00FF00FF } > PropertySet{ 0x00FF00F0 });
}

TEST(TestPropertySet, toString)
{
    EXPECT_EQ(PropertySet{ 0b00000000000000000000000000000000 }.toString(), "00000000000000000000000000000000");
    EXPECT_EQ(PropertySet{ 0b11111111111111111111111111111111 }.toString(), "11111111111111111111111111111111");
    EXPECT_EQ(PropertySet{ 0b10101010101010101010101010101010 }.toString(), "10101010101010101010101010101010");
    EXPECT_EQ(PropertySet{ 0b10101010101010101010101010101010 }.toString('B', 'A'), "ABABABABABABABABABABABABABABABAB");
}
