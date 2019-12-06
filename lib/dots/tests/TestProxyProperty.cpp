#include <string>
#include <type_traits>
#include <cstddef>
#include <gtest/gtest.h>
#include <dots/type/ProxyProperty.h>
#include <dots/type/FundamentalTypes.h>

using namespace dots::type;

struct TestProxyProperty : ::testing::Test
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
	EXPECT_THROW(m_sut.value(), std::runtime_error);
}

TEST_F(TestProxyProperty, isValid_ValidWithValue)
{
	m_sut.construct();

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_NO_THROW(m_sut.value());
}

TEST_F(TestProxyProperty, construct_EqualValueAfterExplicitConstruction)
{
	m_sut.construct(std::string{ "foo" });

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(m_sut.value(), std::string{ "foo" });
}

TEST_F(TestProxyProperty, construct_EqualValueAfterImplicitConstruction)
{
	m_sut.construct("foo");

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProxyProperty, construct_EqualValueAfterEmplaceConstruction)
{
	m_sut.construct(std::string{ "barfoo" }, 3u, 3u);

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProxyProperty, construct_ThrowOnOverconstruction)
{
	m_sut.construct();

	EXPECT_THROW(m_sut.construct(), std::runtime_error);
}

TEST_F(TestProxyProperty, destroy_InvalidAfterDestroy)
{
	m_sut.construct("foo");
	m_sut.destroy();

	EXPECT_FALSE(m_sut.isValid());
}

TEST_F(TestProxyProperty, constructOrValue_ConstructOnInvalid)
{
	std::string& value = m_sut.constructOrValue("foo");

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST_F(TestProxyProperty, constructOrValue_ValueOnValid)
{
	m_sut.construct("foo");

	std::string& value = m_sut.constructOrValue("bar");	

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST_F(TestProxyProperty, assign_ExceptionOnInvalid)
{
	EXPECT_THROW(m_sut.assign(), std::runtime_error);
}

TEST_F(TestProxyProperty, assign_EqualValueAfterExplicitAssign)
{
	m_sut.construct();
	m_sut.assign(std::string{ "foo" });

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(m_sut.value(), std::string{ "foo" });
}

TEST_F(TestProxyProperty, assign_EqualValueAfterImplicitAssign)
{
	m_sut.construct();
	m_sut.assign("foo");

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProxyProperty, assign_EqualValueAfterEmplaceAssign)
{
	m_sut.construct();
	m_sut.assign(std::string{ "barfoo" }, 3u, 3u);

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProxyProperty, constructOrAssign_ConstructOnInvalid)
{
	std::string& value = m_sut.constructOrAssign("foo");

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST_F(TestProxyProperty, constructOrAssign_AssignOnValid)
{
	m_sut.construct("foo");
	std::string& value = m_sut.constructOrAssign("bar");

	EXPECT_TRUE(m_sut.isValid());
	EXPECT_EQ(value, "bar");
}

TEST_F(TestProxyProperty, swap_OppositeValuesAfterSwapValid)
{
	m_sutLhs.construct("foo");
	m_sutRhs.construct("bar");
	
	m_sutLhs.swap(m_sutRhs);

	EXPECT_EQ(m_sutLhs.value(), "bar");
	EXPECT_EQ(m_sutRhs.value(), "foo");
}

TEST_F(TestProxyProperty, swap_OppositeValuesAfterSwapInvalid)
{
	m_sutLhs.construct("foo");
	m_sutLhs.swap(m_sutRhs);

	EXPECT_FALSE(m_sutLhs.isValid());
	EXPECT_TRUE(m_sutRhs.isValid());
	EXPECT_EQ(m_sutRhs.value(), "foo");
}

TEST_F(TestProxyProperty, swap_OppositeValuesAfterInvalidSwap)
{
	m_sutRhs.construct("bar");
	m_sutLhs.swap(m_sutRhs);

	EXPECT_FALSE(m_sutRhs.isValid());
	EXPECT_TRUE(m_sutLhs.isValid());
	EXPECT_EQ(m_sutLhs.value(), "bar");
}

TEST_F(TestProxyProperty, equal_CompareNotEqualToValueWhenInvalid)
{
	std::string rhs{ "foo" };

	EXPECT_FALSE(m_sut.equal(rhs));
	EXPECT_FALSE(m_sut == rhs);
	EXPECT_TRUE(m_sut != rhs);
}
TEST_F(TestProxyProperty, equal_CompareEqualToValueWhenValid)
{
	std::string rhs{ "foo" };

	m_sut.construct("foo");	

	EXPECT_TRUE(m_sut.equal(rhs));
	EXPECT_TRUE(m_sut == rhs);
	EXPECT_FALSE(m_sut != rhs);
}

TEST_F(TestProxyProperty, equal_CompareNotEqualToValueWhenValid)
{
	std::string rhs{ "bar" };

	m_sut.construct("foo");

	EXPECT_FALSE(m_sut.equal(rhs));
	EXPECT_FALSE(m_sut == rhs);
	EXPECT_TRUE(m_sut != rhs);
}

TEST_F(TestProxyProperty, equal_CompareNotEqualToValidPropertyWhenInvalid)
{
	m_sutRhs.construct("foo");

	EXPECT_FALSE(m_sutLhs.equal(m_sutRhs));
	EXPECT_FALSE(m_sutLhs == m_sutRhs);
	EXPECT_TRUE(m_sutLhs != m_sutRhs);
}

TEST_F(TestProxyProperty, equal_CompareEqualToValidPropertyWhenValid)
{
	m_sutLhs.construct("foo");
	m_sutRhs.construct("foo");

	EXPECT_TRUE(m_sutLhs.equal(m_sutRhs));
	EXPECT_TRUE(m_sutLhs == m_sutRhs);
	EXPECT_FALSE(m_sutLhs != m_sutRhs);
}

TEST_F(TestProxyProperty, equal_CompareNotEqualToValidPropertyWhenValid)
{
	m_sutLhs.construct("foo");
	m_sutRhs.construct("bar");

	EXPECT_FALSE(m_sutLhs.equal(m_sutRhs));
	EXPECT_FALSE(m_sutLhs == m_sutRhs);
	EXPECT_TRUE(m_sutLhs != m_sutRhs);
}

TEST_F(TestProxyProperty, less_CompareNotLessToValueWhenInvalid)
{
	std::string rhs{ "fou" };

	EXPECT_FALSE(m_sut.less(rhs));
	EXPECT_FALSE(m_sut < rhs);
}

TEST_F(TestProxyProperty, less_CompareLessToValueWhenValid)
{
	std::string rhs{ "fou" };

	m_sut.construct("foo");

	EXPECT_TRUE(m_sut.less(rhs));
	EXPECT_TRUE(m_sut < rhs);
}

TEST_F(TestProxyProperty, less_CompareNotLessToValueWhenValid)
{
	std::string rhs{ "bar" };

	m_sut.construct("foo");

	EXPECT_FALSE(m_sut.less(rhs));
	EXPECT_FALSE(m_sut < rhs);
}

TEST_F(TestProxyProperty, less_CompareLessToInvalidPropertyWhenInvalid)
{
	EXPECT_FALSE(m_sutLhs.less(m_sutRhs));
	EXPECT_FALSE(m_sutLhs < m_sutRhs);
}

TEST_F(TestProxyProperty, less_CompareNotLessToValidPropertyWhenInvalid)
{
	m_sutRhs.construct("fou");

	EXPECT_FALSE(m_sutLhs.less(m_sutRhs));
	EXPECT_FALSE(m_sutLhs < m_sutRhs);
}

TEST_F(TestProxyProperty, less_CompareLessToValidPropertyValid)
{
	m_sutLhs.construct("foo");
	m_sutRhs.construct("fou");

	EXPECT_TRUE(m_sutLhs.less(m_sutRhs));
	EXPECT_TRUE(m_sutLhs < m_sutRhs);
}

TEST_F(TestProxyProperty, less_CompareNotLessToValidPropertyValid)
{
	m_sutLhs.construct("foo");
	m_sutRhs.construct("bar");

	EXPECT_FALSE(m_sutLhs.less(m_sutRhs));
	EXPECT_FALSE(m_sutLhs < m_sutRhs);
}