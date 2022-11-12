// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <string>
#include <type_traits>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/Property.h>
#include <dots/type/FundamentalTypes.h>
#include <TestStruct.dots.h>

using namespace dots::type;

struct TestProperty : ::testing::Test
{
protected:

    TestProperty() :
        m_sut(m_propertyArea.stringProperty),
        m_sutLhs(m_propertyAreaLhs.stringProperty),
        m_sutRhs(m_propertyAreaRhs.stringProperty) {}

    TestStruct m_propertyArea;
    TestStruct m_propertyAreaLhs;
    TestStruct m_propertyAreaRhs;

    TestStruct::stringProperty_pt& m_sut;
    TestStruct::stringProperty_pt& m_sutLhs;
    TestStruct::stringProperty_pt& m_sutRhs;
};

TEST_F(TestProperty, isValid_InvalidWithoutValue)
{
    EXPECT_FALSE(m_sut.isValid());
    EXPECT_EQ(m_sut, dots::invalid);
    EXPECT_EQ(dots::invalid, m_sut);
    EXPECT_THROW(m_sut.value(), std::runtime_error);
}

TEST_F(TestProperty, isValid_ValidWithValue)
{
    m_sut.emplace();

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_NE(m_sut, dots::invalid);
    EXPECT_NE(dots::invalid, m_sut);
    EXPECT_NO_THROW(m_sut.value());
}

TEST_F(TestProperty, emplace_EqualValueAfterExplicitConstruction)
{
    m_sut.emplace(std::string{ "foo" });

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(m_sut.value(), std::string{ "foo" });
}

TEST_F(TestProperty, emplace_EqualValueAfterImplicitConstruction)
{
    m_sut.emplace("foo");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProperty, emplace_EqualValueAfterEmplaceConstruction)
{
    m_sut.emplace(std::string{ "barfoo" }, 3u, 3u);

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProperty, emplace_EqualValueAfterConstructionFromOther)
{
    m_sutRhs.emplace("foo");
    m_sutLhs = m_sutRhs;

    EXPECT_TRUE(m_sutLhs.isValid());
    EXPECT_EQ(m_sutLhs.value(), "foo");
}

TEST_F(TestProperty, emplace_InvalidAfterConstructionFromInvalidOther)
{
    m_sutLhs = m_sutRhs;
    EXPECT_FALSE(m_sutLhs.isValid());
}

TEST_F(TestProperty, destroy_InvalidAfterDestroy)
{
    {
        m_sut.emplace("foo");
        m_sut.reset();

        EXPECT_FALSE(m_sut.isValid());
    }

    {
        m_sut.emplace("foo");
        m_sut = dots::invalid;

        EXPECT_FALSE(m_sut.isValid());
    }
}

TEST_F(TestProperty, valueOrDefault_ValueOnValid)
{
    m_sut.emplace("foo");
    std::string value = m_sut.valueOrDefault("bar");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(value, "foo");
}

TEST_F(TestProperty, valueOrDefault_DefaultOnValid)
{
    std::string value = m_sut.valueOrDefault("bar");

    EXPECT_FALSE(m_sut.isValid());
    EXPECT_EQ(value, "bar");
}

TEST_F(TestProperty, valueOrEmplace_EmplaceOnInvalid)
{
    std::string& value = m_sut.valueOrEmplace("foo");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(value, "foo");
}

TEST_F(TestProperty, valueOrEmplace_ValueOnValid)
{
    m_sut.emplace("foo");

    std::string& value = m_sut.valueOrEmplace("bar");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(value, "foo");
}

TEST_F(TestProperty, swap_OppositeValuesAfterSwapValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("bar");

    m_sutLhs.swap(m_sutRhs);

    EXPECT_EQ(m_sutLhs.value(), "bar");
    EXPECT_EQ(m_sutRhs.value(), "foo");
}

TEST_F(TestProperty, swap_OppositeValuesAfterSwapInvalid)
{
    m_sutLhs.emplace("foo");
    m_sutLhs.swap(m_sutRhs);

    EXPECT_FALSE(m_sutLhs.isValid());
    EXPECT_TRUE(m_sutRhs.isValid());
    EXPECT_EQ(m_sutRhs.value(), "foo");
}

TEST_F(TestProperty, swap_OppositeValuesAfterInvalidSwap)
{
    m_sutRhs.emplace("bar");
    m_sutLhs.swap(m_sutRhs);

    EXPECT_FALSE(m_sutRhs.isValid());
    EXPECT_TRUE(m_sutLhs.isValid());
    EXPECT_EQ(m_sutLhs.value(), "bar");
}

TEST_F(TestProperty, equal_CompareNotEqualToValueWhenInvalid)
{
    std::string rhs{ "foo" };

    EXPECT_FALSE(m_sut == rhs);
    EXPECT_TRUE(m_sut != rhs);
}
TEST_F(TestProperty, equal_CompareEqualToValueWhenValid)
{
    std::string rhs{ "foo" };
    m_sut.emplace("foo");

    EXPECT_TRUE(m_sut == rhs);
    EXPECT_FALSE(m_sut != rhs);
}

TEST_F(TestProperty, equal_CompareNotEqualToValueWhenValid)
{
    std::string rhs{ "bar" };
    m_sut.emplace("foo");

    EXPECT_FALSE(m_sut == rhs);
    EXPECT_TRUE(m_sut != rhs);
}

TEST_F(TestProperty, equal_CompareNotEqualToValidPropertyWhenInvalid)
{
    m_sutRhs.emplace("foo");

    EXPECT_FALSE(m_sutLhs == m_sutRhs);
    EXPECT_TRUE(m_sutLhs != m_sutRhs);
}

TEST_F(TestProperty, equal_CompareEqualToValidPropertyWhenValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("foo");

    EXPECT_TRUE(m_sutLhs == m_sutRhs);
    EXPECT_FALSE(m_sutLhs != m_sutRhs);
}

TEST_F(TestProperty, equal_CompareNotEqualToValidPropertyWhenValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("bar");

    EXPECT_FALSE(m_sutLhs == m_sutRhs);
    EXPECT_TRUE(m_sutLhs != m_sutRhs);
}

TEST_F(TestProperty, less_CompareNotLessToValueWhenInvalid)
{
    std::string rhs{ "fou" };
    EXPECT_FALSE(m_sut < rhs);
    EXPECT_FALSE(m_sut < dots::invalid);
}

TEST_F(TestProperty, less_CompareLessToValueWhenValid)
{
    std::string rhs{ "fou" };
    m_sut.emplace("foo");

    EXPECT_TRUE(m_sut < rhs);
    EXPECT_TRUE(dots::invalid < m_sut);
}

TEST_F(TestProperty, less_CompareNotLessToValueWhenValid)
{
    std::string rhs{ "bar" };
    m_sut.emplace("foo");

    EXPECT_FALSE(m_sut < rhs);
}

TEST_F(TestProperty, less_CompareNotLessToInvalidPropertyWhenInvalid)
{
    EXPECT_FALSE(m_sutLhs < m_sutRhs);
}

TEST_F(TestProperty, less_CompareNotLessToValidPropertyWhenInvalid)
{
    m_sutRhs.emplace("fou");
    EXPECT_FALSE(m_sutLhs < m_sutRhs);
}

TEST_F(TestProperty, less_CompareLessToValidPropertyValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("fou");

    EXPECT_TRUE(m_sutLhs < m_sutRhs);
}

TEST_F(TestProperty, less_CompareNotLessToValidPropertyValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("bar");

    EXPECT_FALSE(m_sutLhs < m_sutRhs);
}
