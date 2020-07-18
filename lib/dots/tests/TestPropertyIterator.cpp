#include <gtest/gtest.h>
#include <dots/type/PropertyIterator.h>
#include <dots/type/FundamentalTypes.h>

using namespace dots::type;

struct TestPropertyIterator : ::testing::Test
{
protected:

    template <typename T>
    struct TestProperty : Property<T, TestProperty<T>>
    {
        TestProperty(const dots::type::PropertyArea& area, std::string name, uint32_t tag) :
            m_descriptor{ dots::type::PropertyDescriptor{ Descriptor<T>::InstancePtr(), std::move(name), tag, false, PropertyOffset<>{ std::in_place, static_cast<uint32_t>(reinterpret_cast<char*>(this) - reinterpret_cast<const char*>(&area)) } } } {}
        TestProperty(const TestProperty& other) = delete;
        TestProperty(TestProperty&& other) = delete;
        ~TestProperty() { Property<T, TestProperty<T>>::destroy(); }

        TestProperty& operator = (const TestProperty& rhs) = delete;
        TestProperty& operator = (TestProperty&& rhs) = delete;

    private:

        friend struct Property<T, TestProperty<T>>;

        T& derivedStorage() { return m_value; }
        const T& derivedValue() const { return const_cast<TestProperty&>(*this).derivedValue(); }
        const PropertyDescriptor& derivedDescriptor() const { return m_descriptor; }

        union { T m_value; };
        const PropertyDescriptor m_descriptor;
    };

    struct TestPropertyArea : dots::type::PropertyArea
    {
        TestPropertyArea() :
            intProperty{ *this, "intProperty", 1 },
            stringProperty{ *this, "stringProperty", 2 },
            floatProperty{ *this, "floatProperty", 3 },
            shortProperty{ *this, "shortProperty", 5 }
        {}
        TestProperty<int32_t> intProperty;
        TestProperty<std::string> stringProperty;
        TestProperty<float> floatProperty;
        TestProperty<uint16_t> shortProperty;
    };

    TestPropertyIterator() :
        m_propertyDescriptors{
            m_propertyArea.intProperty.descriptor(),
            m_propertyArea.stringProperty.descriptor(),
            m_propertyArea.floatProperty.descriptor(),
            m_propertyArea.shortProperty.descriptor()
        },
        m_forwardBegin{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.begin() },
        m_forwardEnd{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.end() },
        m_reverseBegin{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rbegin() },
        m_reverseEnd{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rend() }
    {}

    TestPropertyArea m_propertyArea;
    property_descriptor_container_t m_propertyDescriptors;

    property_iterator m_forwardBegin;
    property_iterator m_forwardEnd;

    reverse_property_iterator m_reverseBegin;
    reverse_property_iterator m_reverseEnd;
};

TEST_F(TestPropertyIterator, forward_operator_dereference_ThrowOnInvalid)
{
    property_iterator sut{ m_forwardEnd };
    EXPECT_THROW(*sut, std::logic_error);
}

TEST_F(TestPropertyIterator, forward_operator_prefixIncrement_ExpectedNumberOfIterations)
{
    property_iterator sut{ m_forwardBegin };

    size_t numIterations = 0;
    std::for_each(sut, m_forwardEnd, [&](const auto&) { ++numIterations; });

    EXPECT_EQ(numIterations, 4);
}

TEST_F(TestPropertyIterator, forward_operator_prefixIncrement_ExpectedIterationWithAllProperties)
{
    property_iterator sut{ m_forwardBegin };

    EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
    EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
    EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
    EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ((++sut), m_forwardEnd);
}

TEST_F(TestPropertyIterator, forward_operator_prefixIncrement_ExpectedIterationWithPartialProperties)
{
    property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.begin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

    EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
    EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ((++sut), m_forwardEnd);
}

TEST_F(TestPropertyIterator, forward_operator_postfixIncrement_ExpectedIterationWithAllProperties)
{
    property_iterator sut{ m_forwardBegin };

    EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ((sut), m_forwardEnd);
}

TEST_F(TestPropertyIterator, forward_operator_postfixIncrement_ExpectedIterationWithPartialProperties)
{
    property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.begin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

    EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ(sut, m_forwardEnd);
}

TEST_F(TestPropertyIterator, reverse_operator_dereference_ThrowOnInvalid)
{
    reverse_property_iterator sut{ m_reverseEnd };
    EXPECT_THROW(*sut, std::logic_error);
}

TEST_F(TestPropertyIterator, reverse_operator_prefixIncrement_ExpectedNumberOfIterations)
{
    reverse_property_iterator sut{ m_reverseBegin };

    size_t numIterations = 0;
    std::for_each(sut, m_reverseEnd, [&](const auto&) { ++numIterations; });

    EXPECT_EQ(numIterations, 4);
}

TEST_F(TestPropertyIterator, reverse_operator_prefixIncrement_ExpectedIterationWithAllProperties)
{
    reverse_property_iterator sut{ m_reverseBegin };

    EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
    EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
    EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
    EXPECT_EQ((++sut), m_reverseEnd);
}

TEST_F(TestPropertyIterator, reverse_operator_prefixIncrement_ExpectedIterationWithPartialProperties)
{
    reverse_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rbegin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

    EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
    EXPECT_EQ((++sut), m_reverseEnd);
}

TEST_F(TestPropertyIterator, reverse_operator_postfixIncrement_ExpectedIterationWithAllProperties)
{
    reverse_property_iterator sut{ m_reverseBegin };

    EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
    EXPECT_EQ((sut), m_reverseEnd);
}

TEST_F(TestPropertyIterator, reverse_operator_postfixIncrement_ExpectedIterationWithPartialProperties)
{
    reverse_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rbegin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

    EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
    EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
    EXPECT_EQ((sut++), m_reverseEnd);
}