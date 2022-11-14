// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <string>
#include <type_traits>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/ProxyProperty.h>
#include <dots/type/FundamentalTypes.h>
#include <TestStruct.dots.h>

using namespace dots::type;

struct TestProxyProperty : ::testing::Test
{
protected:

    TestProxyProperty() :
        m_sut(m_propertyArea.stringProperty),
        m_sutLhs(m_propertyAreaLhs.stringProperty),
        m_sutRhs(m_propertyAreaRhs.stringProperty) {}

    using proxy_t = ProxyProperty<dots::string_t>;

    TestStruct m_propertyArea;
    TestStruct m_propertyAreaLhs;
    TestStruct m_propertyAreaRhs;

    proxy_t m_sut;
    proxy_t m_sutLhs;
    proxy_t m_sutRhs;
};

TEST_F(TestProxyProperty, isValid_InvalidWithoutValue)
{
    EXPECT_FALSE(m_sut.isValid());
    EXPECT_EQ(m_sut, dots::invalid);
    EXPECT_EQ(dots::invalid, m_sut);
    EXPECT_THROW(m_sut.value(), std::runtime_error);
}

TEST_F(TestProxyProperty, isValid_ValidWithValue)
{
    m_sut.emplace();

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_NE(m_sut, dots::invalid);
    EXPECT_NE(dots::invalid, m_sut);
    EXPECT_NO_THROW(m_sut.value());
}

TEST_F(TestProxyProperty, emplace_EqualValueAfterExplicitConstruction)
{
    m_sut.emplace(std::string{ "foo" });

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(m_sut.value(), std::string{ "foo" });
}

TEST_F(TestProxyProperty, emplace_EqualValueAfterImplicitConstruction)
{
    m_sut.emplace("foo");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProxyProperty, emplace_EqualValueAfterEmplaceConstruction)
{
    m_sut.emplace(std::string{ "barfoo" }, 3u, 3u);

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProxyProperty, destroy_InvalidAfterDestroy)
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

TEST_F(TestProxyProperty, valueOrEmplace_EmplaceOnInvalid)
{
    std::string& value = m_sut.valueOrEmplace("foo");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(value, "foo");
}

TEST_F(TestProxyProperty, valueOrEmplace_ValueOnValid)
{
    m_sut.emplace("foo");

    std::string& value = m_sut.valueOrEmplace("bar");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(value, "foo");
}

TEST_F(TestProxyProperty, swap_OppositeValuesAfterSwapValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("bar");

    m_sutLhs.swap(m_sutRhs);

    EXPECT_EQ(m_sutLhs.value(), "bar");
    EXPECT_EQ(m_sutRhs.value(), "foo");
}

TEST_F(TestProxyProperty, swap_OppositeValuesAfterSwapInvalid)
{
    m_sutLhs.emplace("foo");
    m_sutLhs.swap(m_sutRhs);

    EXPECT_FALSE(m_sutLhs.isValid());
    EXPECT_TRUE(m_sutRhs.isValid());
    EXPECT_EQ(m_sutRhs.value(), "foo");
}

TEST_F(TestProxyProperty, swap_OppositeValuesAfterInvalidSwap)
{
    m_sutRhs.emplace("bar");
    m_sutLhs.swap(m_sutRhs);

    EXPECT_FALSE(m_sutRhs.isValid());
    EXPECT_TRUE(m_sutLhs.isValid());
    EXPECT_EQ(m_sutLhs.value(), "bar");
}

TEST_F(TestProxyProperty, is)
{
    ProxyProperty<> sut{ m_sut };
    EXPECT_TRUE(sut.is<std::string>());
    EXPECT_FALSE(sut.is<int>());
}

TEST_F(TestProxyProperty, as)
{
    m_sut.emplace("foo");
    ProxyProperty<> sut{ m_sut };

    EXPECT_EQ(*sut.as<std::string>(), "foo");
    EXPECT_EQ(sut.as<int>(), nullptr);
}

TEST_F(TestProxyProperty, to)
{
    m_sut.emplace("foo");
    ProxyProperty<> sut{ m_sut };

    EXPECT_EQ(sut.to<std::string>(), "foo");
    EXPECT_THROW((sut.to<int, true>()), std::logic_error);
}
