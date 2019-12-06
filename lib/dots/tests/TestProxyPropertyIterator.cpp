#include <gtest/gtest.h>
#include <dots/type/ProxyPropertyIterator.h>
#include <dots/type/FundamentalTypes.h>

using namespace dots::type;

struct TestProxyPropertyIterator : ::testing::Test
{
protected:

	template <typename T>
	struct TestProperty : Property<T, TestProperty<T>>
	{
		TestProperty(const dots::type::PropertyArea& area, std::string name, uint32_t tag) :
			m_Descriptor{ dots::type::PropertyDescriptor<T>{ std::move(name), static_cast<uint32_t>(reinterpret_cast<char*>(this) - reinterpret_cast<const char*>(&area)), tag, false } } {}
		TestProperty(const TestProperty& other) = delete;
		TestProperty(TestProperty&& other) = delete;
		~TestProperty() { Property<T, TestProperty<T>>::destroy(); }

		TestProperty& operator = (const TestProperty& rhs) = delete;
		TestProperty& operator = (TestProperty&& rhs) = delete;

	private:

		friend struct Property<T, TestProperty<T>>;

		T& derivedStorage()	{ return m_value; }
		const T& derivedValue() const {	return const_cast<TestProperty&>(*this).derivedValue();	}
		const PropertyDescriptor<T>& derivedDescriptor() const {	return m_Descriptor; }

		union {	T m_value; };
		const PropertyDescriptor<T> m_Descriptor;
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
	
	TestProxyPropertyIterator() :
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
	property_descriptor_container_t m_propertyDescriptors;

	proxy_property_iterator m_forwardBegin;
	proxy_property_iterator m_forwardEnd;

	reverse_proxy_property_iterator m_reverseBegin;
	reverse_proxy_property_iterator m_reverseEnd;
};

TEST_F(TestProxyPropertyIterator, forward_operator_dereference_ThrowOnInvalid)
{
	proxy_property_iterator sut{ m_forwardEnd };
	EXPECT_THROW(*sut, std::logic_error);
}

TEST_F(TestProxyPropertyIterator, forward_operator_prefixIncrement_ExpectedNumberOfIterations)
{
	proxy_property_iterator sut{ m_forwardBegin };
	
	size_t numIterations = 0;
	std::for_each(sut, m_forwardEnd, [&](const auto&) { ++numIterations; });

	EXPECT_EQ(numIterations, 4);
}

TEST_F(TestProxyPropertyIterator, forward_operator_prefixIncrement_ExpectedIterationWithAllProperties)
{
	proxy_property_iterator sut{ m_forwardBegin };
	
	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((++sut), m_forwardEnd);
}

TEST_F(TestProxyPropertyIterator, forward_operator_prefixIncrement_ExpectedIterationWithPartialProperties)
{
	proxy_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.begin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((++sut), m_forwardEnd);
}

TEST_F(TestProxyPropertyIterator, forward_operator_postfixIncrement_ExpectedIterationWithAllProperties)
{
	proxy_property_iterator sut{ m_forwardBegin };
	
	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut), m_forwardEnd);
}

TEST_F(TestProxyPropertyIterator, forward_operator_postfixIncrement_ExpectedIterationWithPartialProperties)
{
	proxy_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.begin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ(sut, m_forwardEnd);
}

TEST_F(TestProxyPropertyIterator, reverse_operator_dereference_ThrowOnInvalid)
{
	reverse_proxy_property_iterator sut{ m_reverseEnd };
	EXPECT_THROW(*sut, std::logic_error);
}

TEST_F(TestProxyPropertyIterator, reverse_operator_prefixIncrement_ExpectedNumberOfIterations)
{
	reverse_proxy_property_iterator sut{ m_reverseBegin };
	
	size_t numIterations = 0;
	std::for_each(sut, m_reverseEnd, [&](const auto&) { ++numIterations; });

	EXPECT_EQ(numIterations, 4);
}

TEST_F(TestProxyPropertyIterator, reverse_operator_prefixIncrement_ExpectedIterationWithAllProperties)
{
	reverse_proxy_property_iterator sut{ m_reverseBegin };
	
	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((++sut), m_reverseEnd);
}

TEST_F(TestProxyPropertyIterator, reverse_operator_prefixIncrement_ExpectedIterationWithPartialProperties)
{
	reverse_proxy_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rbegin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((++sut)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((++sut), m_reverseEnd);
}

TEST_F(TestProxyPropertyIterator, reverse_operator_postfixIncrement_ExpectedIterationWithAllProperties)
{
	reverse_proxy_property_iterator sut{ m_reverseBegin };
	
	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.floatProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.intProperty.descriptor().tag());
	EXPECT_EQ((sut), m_reverseEnd);
}

TEST_F(TestProxyPropertyIterator, reverse_operator_postfixIncrement_ExpectedIterationWithPartialProperties)
{
	reverse_proxy_property_iterator sut{ m_propertyArea, m_propertyDescriptors, m_propertyDescriptors.rbegin(), m_propertyArea.stringProperty.descriptor().set() + m_propertyArea.shortProperty.descriptor().set() };

	EXPECT_EQ(sut->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.shortProperty.descriptor().tag());
	EXPECT_EQ((sut++)->descriptor().tag(), m_propertyArea.stringProperty.descriptor().tag());
	EXPECT_EQ((sut++), m_reverseEnd);
}