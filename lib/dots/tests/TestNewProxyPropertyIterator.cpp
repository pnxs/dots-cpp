#include <gtest/gtest.h>
#include <dots/type/NewProxyPropertyIterator.h>
#include <dots/type/NewFundamentalTypes.h>

using namespace dots::type;

struct TestNewProxyPropertyIterator : ::testing::Test
{
protected:

	template <typename T>
	struct TestProperty : NewProperty<T, TestProperty<T>>
	{
		TestProperty(const dots::type::NewPropertyArea& area, std::string name, uint32_t tag) :
			m_Descriptor{ dots::type::NewPropertyDescriptor<T>{ std::move(name), static_cast<uint32_t>(reinterpret_cast<char*>(this) - reinterpret_cast<const char*>(&area)), tag, false } } {}
		TestProperty(const TestProperty& other) = delete;
		TestProperty(TestProperty&& other) = delete;
		~TestProperty() { NewProperty<T, TestProperty<T>>::destroy(); }

		TestProperty& operator = (const TestProperty& rhs) = delete;
		TestProperty& operator = (TestProperty&& rhs) = delete;

	private:

		friend struct NewProperty<T, TestProperty<T>>;

		T& derivedStorage()	{ return m_value; }
		const T& derivedValue() const {	return const_cast<TestProperty&>(*this).derivedValue();	}
		const NewPropertyDescriptor<T>& derivedDescriptor() const {	return m_Descriptor; }

		union {	T m_value; };
		const NewPropertyDescriptor<T> m_Descriptor;
	};

	struct TestPropertyArea : dots::type::NewPropertyArea
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
	
	TestNewProxyPropertyIterator() :
		m_propertyDescriptors{
			nullptr,
			&m_propertyArea.intProperty.descriptor(),
			&m_propertyArea.stringProperty.descriptor(),
			&m_propertyArea.floatProperty.descriptor(),
			nullptr,
			&m_propertyArea.shortProperty.descriptor()
		},
		m_forwardBegin{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.begin() },
		m_forwardEnd{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.end() },
		m_reverseBegin{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rbegin() },
		m_reverseEnd{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rend() }
	{}

	TestPropertyArea m_propertyArea;
	new_property_descriptor_container_t m_propertyDescriptors;

	new_proxy_property_iterator m_forwardBegin;
	new_proxy_property_iterator m_forwardEnd;

	new_reverse_proxy_property_iterator m_reverseBegin;
	new_reverse_proxy_property_iterator m_reverseEnd;
};

TEST_F(TestNewProxyPropertyIterator, forward_operator_dereference_ThrowOnInvalid)
{
	new_proxy_property_iterator sut{ m_forwardEnd };
	EXPECT_THROW(*sut, std::logic_error);
}

TEST_F(TestNewProxyPropertyIterator, forward_operator_prefixIncrement_ExpectedNumberOfIterations)
{
	new_proxy_property_iterator sut{ m_forwardBegin };
	
	size_t numIterations = 0;
	std::for_each(sut, m_forwardEnd, [&](const auto&) { ++numIterations; });

	EXPECT_EQ(numIterations, 4);
}

TEST_F(TestNewProxyPropertyIterator, forward_operator_prefixIncrement_ExpectedIterationWithAllProperties)
{
	new_proxy_property_iterator sut{ m_forwardBegin };
	
	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((++sut), m_forwardEnd);
}

TEST_F(TestNewProxyPropertyIterator, forward_operator_prefixIncrement_ExpectedIterationWithPartialProperties)
{
	new_proxy_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.begin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((++sut), m_forwardEnd);
}

TEST_F(TestNewProxyPropertyIterator, forward_operator_postfixIncrement_ExpectedIterationWithAllProperties)
{
	new_proxy_property_iterator sut{ m_forwardBegin };
	
	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut), m_forwardEnd);
}

TEST_F(TestNewProxyPropertyIterator, forward_operator_postfixIncrement_ExpectedIterationWithPartialProperties)
{
	new_proxy_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.begin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ(sut, m_forwardEnd);
}

TEST_F(TestNewProxyPropertyIterator, reverse_operator_dereference_ThrowOnInvalid)
{
	new_reverse_proxy_property_iterator sut{ m_reverseEnd };
	EXPECT_THROW(*sut, std::logic_error);
}

TEST_F(TestNewProxyPropertyIterator, reverse_operator_prefixIncrement_ExpectedNumberOfIterations)
{
	new_reverse_proxy_property_iterator sut{ m_reverseBegin };
	
	size_t numIterations = 0;
	std::for_each(sut, m_reverseEnd, [&](const auto&) { ++numIterations; });

	EXPECT_EQ(numIterations, 4);
}

TEST_F(TestNewProxyPropertyIterator, reverse_operator_prefixIncrement_ExpectedIterationWithAllProperties)
{
	new_reverse_proxy_property_iterator sut{ m_reverseBegin };
	
	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((++sut), m_reverseEnd);
}

TEST_F(TestNewProxyPropertyIterator, reverse_operator_prefixIncrement_ExpectedIterationWithPartialProperties)
{
	new_reverse_proxy_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rbegin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((++sut), m_reverseEnd);
}

TEST_F(TestNewProxyPropertyIterator, reverse_operator_postfixIncrement_ExpectedIterationWithAllProperties)
{
	new_reverse_proxy_property_iterator sut{ m_reverseBegin };
	
	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((sut), m_reverseEnd);
}

TEST_F(TestNewProxyPropertyIterator, reverse_operator_postfixIncrement_ExpectedIterationWithPartialProperties)
{
	new_reverse_proxy_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rbegin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++), m_reverseEnd);
}