#include <string>
#include <type_traits>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/Property.h>
#include <dots/type/FundamentalTypes.h>

using namespace dots::type;

struct TestProperty : ::testing::Test
{
protected:

    template <typename T>
    struct test_property_t : Property<T, test_property_t<T>>
    {
        test_property_t(const PropertyArea& area, std::string name, uint32_t tag) :
            m_descriptor{ PropertyDescriptor{ Descriptor<T>::Instance(), std::move(name), tag, false, PropertyOffset{ std::in_place, static_cast<uint32_t>(reinterpret_cast<char*>(this) - reinterpret_cast<const char*>(&area)) } } } {}
        test_property_t(const test_property_t& other) = delete;
        test_property_t(test_property_t&& other) = delete;
        ~test_property_t() { Property<T, test_property_t<T>>::destroy(); }

        test_property_t& operator = (const test_property_t& rhs) = delete;
        test_property_t& operator = (test_property_t&& rhs) = delete;

    private:

        friend struct Property<T, test_property_t<T>>;

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

    struct test_property_area_t : PropertyArea
    {
        test_property_area_t() : intProperty{ *this, "intProperty", 1 }, stringProperty{ *this, "stringProperty", 2 } {}
        test_property_t<int> intProperty;
        test_property_t<std::string> stringProperty;
    };

    TestProperty() :
        m_sut(m_propertyArea.stringProperty),
        m_sutLhs(m_propertyAreaLhs.stringProperty),
        m_sutRhs(m_propertyAreaRhs.stringProperty) {}

    test_property_area_t m_propertyArea;
    test_property_area_t m_propertyAreaLhs;
    test_property_area_t m_propertyAreaRhs;

    test_property_t<std::string>& m_sut;
    test_property_t<std::string>& m_sutLhs;
    test_property_t<std::string>& m_sutRhs;
};

TEST_F(TestProperty, isValid_InvalidWithoutValue)
{
    EXPECT_FALSE(m_sut.isValid());
    EXPECT_THROW(m_sut.value(), std::runtime_error);
}

TEST_F(TestProperty, isValid_ValidWithValue)
{
    m_sut.emplace();

    EXPECT_TRUE(m_sut.isValid());
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
    m_sutLhs.emplace(m_sutRhs);

    EXPECT_TRUE(m_sutLhs.isValid());
    EXPECT_EQ(m_sutLhs.value(), "foo");
}

TEST_F(TestProperty, emplace_InvalidAfterConstructionFromInvalidOther)
{
    m_sutLhs.emplace(m_sutRhs);
    EXPECT_FALSE(m_sutLhs.isValid());
}

TEST_F(TestProperty, destroy_InvalidAfterDestroy)
{
    m_sut.emplace("foo");
    m_sut.destroy();

    EXPECT_FALSE(m_sut.isValid());
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
}

TEST_F(TestProperty, less_CompareLessToValueWhenValid)
{
    std::string rhs{ "fou" };
    m_sut.emplace("foo");

    EXPECT_TRUE(m_sut < rhs);
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
