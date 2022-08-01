// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <string>
#include <type_traits>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/ProxyProperty.h>
#include <dots/type/FundamentalTypes.h>

using namespace dots::type;

struct TestProxyProperty : ::testing::Test
{
protected:

    template <typename T>
    struct TestProperty : Property<T, TestProperty<T>>
    {
        TestProperty(const PropertyArea& area, std::string name, uint32_t tag) :
            m_descriptor{ PropertyDescriptor{ Descriptor<T>::Instance(), std::move(name), tag, false, PropertyOffset{ std::in_place, static_cast<uint32_t>(reinterpret_cast<char*>(this) - reinterpret_cast<const char*>(&area)) } } } {}
        TestProperty(const TestProperty& other) = delete;
        TestProperty(TestProperty&& other) = delete;
        ~TestProperty() { Property<T, TestProperty<T>>::reset(); }

        TestProperty& operator = (const TestProperty& rhs) = delete;
        TestProperty& operator = (TestProperty&& rhs) = delete;

        using Property<T, TestProperty<T>>::operator=;

    private:

        friend struct Property<T, TestProperty<T>>;

        PropertySet validProperties() const { return PropertyArea::GetArea(*this, m_descriptor.offset()).validProperties(); }
        PropertySet& validProperties() { return PropertyArea::GetArea(*this, m_descriptor.offset()).validProperties(); }

        const T& derivedStorage() const { return m_value; }
        const PropertyDescriptor& derivedDescriptor() const { return m_descriptor; }

        bool derivedIsValid() const { return m_descriptor.set() <= validProperties(); }
        void derivedSetValid(){ validProperties() += derivedDescriptor().set(); }
        void derivedSetInvalid(){ validProperties() -= derivedDescriptor().set(); }

        union { T m_value; };
        const PropertyDescriptor m_descriptor;
    };

    struct TestPropertyArea : PropertyArea
    {
        TestPropertyArea() : intProperty{ *this, "intProperty", 1 }, stringProperty{ *this, "stringProperty", 2 } {}
        TestProperty<int> intProperty;
        TestProperty<std::string> stringProperty;
    };

    TestProxyProperty() :
        m_sut(m_propertyArea.stringProperty),
        m_sutLhs(m_propertyAreaLhs.stringProperty),
        m_sutRhs(m_propertyAreaRhs.stringProperty) {}

    TestPropertyArea m_propertyArea;
    TestPropertyArea m_propertyAreaLhs;
    TestPropertyArea m_propertyAreaRhs;

    ProxyProperty<std::string> m_sut;
    ProxyProperty<std::string> m_sutLhs;
    ProxyProperty<std::string> m_sutRhs;
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

TEST_F(TestProxyProperty, equal_CompareNotEqualToValueWhenInvalid)
{
    std::string rhs{ "foo" };

    EXPECT_FALSE(m_sut == rhs);
    EXPECT_TRUE(m_sut != rhs);
}
TEST_F(TestProxyProperty, equal_CompareEqualToValueWhenValid)
{
    std::string rhs{ "foo" };
    m_sut.emplace("foo");

    EXPECT_TRUE(m_sut == rhs);
    EXPECT_FALSE(m_sut != rhs);
}

TEST_F(TestProxyProperty, equal_CompareNotEqualToValueWhenValid)
{
    std::string rhs{ "bar" };
    m_sut.emplace("foo");

    EXPECT_FALSE(m_sut == rhs);
    EXPECT_TRUE(m_sut != rhs);
}

TEST_F(TestProxyProperty, equal_CompareNotEqualToValidPropertyWhenInvalid)
{
    m_sutRhs.emplace("foo");

    EXPECT_FALSE(m_sutLhs == m_sutRhs);
    EXPECT_TRUE(m_sutLhs != m_sutRhs);
}

TEST_F(TestProxyProperty, equal_CompareEqualToValidPropertyWhenValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("foo");

    EXPECT_TRUE(m_sutLhs == m_sutRhs);
    EXPECT_FALSE(m_sutLhs != m_sutRhs);
}

TEST_F(TestProxyProperty, equal_CompareNotEqualToValidPropertyWhenValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("bar");

    EXPECT_FALSE(m_sutLhs == m_sutRhs);
    EXPECT_TRUE(m_sutLhs != m_sutRhs);
}

TEST_F(TestProxyProperty, less_CompareNotLessToValueWhenInvalid)
{
    std::string rhs{ "fou" };
    EXPECT_FALSE(m_sut < rhs);
    EXPECT_FALSE(m_sut < dots::invalid);
}

TEST_F(TestProxyProperty, less_CompareLessToValueWhenValid)
{
    std::string rhs{ "fou" };
    m_sut.emplace("foo");

    EXPECT_TRUE(m_sut < rhs);
}

TEST_F(TestProxyProperty, less_CompareNotLessToValueWhenValid)
{
    std::string rhs{ "bar" };
    m_sut.emplace("foo");

    EXPECT_FALSE(m_sut < rhs);
    EXPECT_TRUE(dots::invalid < m_sut);
}

TEST_F(TestProxyProperty, less_CompareLessToInvalidPropertyWhenInvalid)
{
    EXPECT_FALSE(m_sutLhs < m_sutRhs);
}

TEST_F(TestProxyProperty, less_CompareNotLessToValidPropertyWhenInvalid)
{
    m_sutRhs.emplace("fou");
    EXPECT_FALSE(m_sutLhs < m_sutRhs);
}

TEST_F(TestProxyProperty, less_CompareLessToValidPropertyValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("fou");

    EXPECT_TRUE(m_sutLhs < m_sutRhs);
}

TEST_F(TestProxyProperty, less_CompareNotLessToValidPropertyValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("bar");

    EXPECT_FALSE(m_sutLhs < m_sutRhs);
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
