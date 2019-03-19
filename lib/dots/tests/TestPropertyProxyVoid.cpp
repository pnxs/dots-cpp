#include <gtest/gtest.h>
#include "dots/type/StructDescriptor.h"
#include "dots/type/EnumDescriptor.h"
#include "dots/type/Registry.h"
#include "dots/type/PropertyProxy.h"
#include "DotsTestStruct.dots.h"

TEST(TestPropertyProxyVoid, isValid_InvalidWithoutValue)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };

	EXPECT_FALSE(sut.isValid());
	EXPECT_THROW(sut.value(), std::runtime_error);
}

TEST(TestPropertyProxyVoid, isValid_ValidWithValue)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };

	sut.construct();

	EXPECT_TRUE(sut.isValid());
	EXPECT_NO_THROW(sut.value());
}

TEST(TestPropertyProxyVoid, construct_EqualValueAfterExplicitConstruction)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string initValue{ "foo" };

	sut.construct(reinterpret_cast<std::byte&>(initValue));

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(reinterpret_cast<std::string&>(sut.value()), std::string{ "foo" });
}

TEST(TestPropertyProxyVoid, construct_EqualValueAfterImplicitConstruction)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string initValue{ "foo" };

	sut.construct(reinterpret_cast<std::byte&>(initValue));

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(reinterpret_cast<std::string&>(sut.value()), "foo");
}

// note: emplace construction is not yet supported by the typeless PropertyProxy
//TEST(TestPropertyProxyVoid, construct_EqualValueAfterEmplaceConstruction)
//{
//	dots::types::DotsTestStruct dts;
//	dots::type::PropertyProxy<void> sut{ dts.stringField };
//
//	sut.construct(std::string{ "barfoo" }, 3, 3);
//
//	EXPECT_TRUE(sut.isValid());
//	EXPECT_EQ(reinterpret_cast<std::string&>(sut.value()), "foo");
//}

TEST(TestPropertyProxyVoid, construct_ThrowOnOverconstruction)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };

	sut.construct();

	EXPECT_THROW(sut.construct(), std::runtime_error);
}

TEST(TestPropertyProxyVoid, destroy_InvalidAfterDestroy)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string initValue{ "foo" };
	sut.construct(reinterpret_cast<std::byte&>(initValue));

	sut.destroy();

	EXPECT_FALSE(sut.isValid());
}

TEST(TestPropertyProxyVoid, constructOrValue_ConstructOnInvalid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string initValue{ "foo" };
	std::string& value = reinterpret_cast<std::string&>(sut.constructOrValue(reinterpret_cast<std::byte&>(initValue)));

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST(TestPropertyProxyVoid, constructOrValue_ValueOnValid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string initValue1{ "foo" };
	sut.construct(reinterpret_cast<std::byte&>(initValue1));
	std::string initValue2{ "bar" };

	std::string& value = reinterpret_cast<std::string&>(sut.constructOrValue(reinterpret_cast<std::byte&>(initValue2)));

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST(TestPropertyProxyVoid, assign_ExceptionOnInvalid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string assignValue;

	EXPECT_THROW(sut.assign(reinterpret_cast<std::byte&>(assignValue)), std::runtime_error);
}

TEST(TestPropertyProxyVoid, assign_EqualValueAfterExplicitAssign)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	sut.construct();
	std::string assignValue{ "foo" };

	sut.assign(reinterpret_cast<std::byte&>(assignValue));

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(reinterpret_cast<std::string&>(sut.value()), std::string{ "foo" });
}

TEST(TestPropertyProxyVoid, assign_EqualValueAfterImplicitAssign)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	sut.construct();
	std::string assignValue{ "foo" };

	sut.assign(reinterpret_cast<std::byte&>(assignValue));

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(reinterpret_cast<std::string&>(sut.value()), "foo");
}

// note: emplace assignment is not yet supported by the typeless PropertyProxy
//TEST(TestPropertyProxyVoid, assign_EqualValueAfterEmplaceAssign)
//{
//	dots::types::DotsTestStruct dts;
//	dots::type::PropertyProxy<void> sut{ dts.stringField };
//	std::string initValue;
//	sut.construct(reinterpret_cast<std::byte&>(initValue));
//
//	sut.assign(std::string{ "barfoo" }, 3, 3);
//
//	EXPECT_TRUE(sut.isValid());
//	EXPECT_EQ(reinterpret_cast<std::string&>(sut.value()), "foo");
//}

TEST(TestPropertyProxyVoid, constructOrAssign_ConstructOnInvalid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string assignValue{ "foo" };

	std::string& value = reinterpret_cast<std::string&>(sut.constructOrAssign(reinterpret_cast<std::byte&>(assignValue)));

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST(TestPropertyProxyVoid, constructOrAssign_AssignOnValid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string initValue{ "foo" };
	sut.construct(reinterpret_cast<std::byte&>(initValue));
	std::string assignValue{ "bar" };

	std::string& value = reinterpret_cast<std::string&>(sut.constructOrAssign(reinterpret_cast<std::byte&>(assignValue)));

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(value, "bar");
}

//TEST(TestPropertyProxyVoid, extract_InvalidAfterExtract)
//{
//	dots::types::DotsTestStruct dts;
//	dots::type::PropertyProxy<void> sut{ dts.stringField };
//	std::string initValue{ "foo" };
//	sut.construct(reinterpret_cast<std::byte&>(initValue));
//
//	std::string&& value = reinterpret_cast<std::string&&>(sut.extract());
//
//	EXPECT_FALSE(sut.isValid());
//	EXPECT_EQ(value, "foo");
//}

TEST(TestPropertyProxyVoid, swap_OppositeValuesAfterSwapValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };
	std::string initValueLhs{ "foo" };
	std::string initValueRhs{ "bar" };
	sutLhs.construct(reinterpret_cast<std::byte&>(initValueLhs));
	sutRhs.construct(reinterpret_cast<std::byte&>(initValueRhs));

	sutLhs.swap(sutRhs);

	EXPECT_EQ(reinterpret_cast<std::string&>(sutLhs.value()), "bar");
	EXPECT_EQ(reinterpret_cast<std::string&>(sutRhs.value()), "foo");
}

TEST(TestPropertyProxyVoid, swap_OppositeValuesAfterSwapInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };
	std::string initValueLhs{ "foo" };
	sutLhs.construct(reinterpret_cast<std::byte&>(initValueLhs));

	sutLhs.swap(sutRhs);

	EXPECT_FALSE(sutLhs.isValid());
	EXPECT_TRUE(sutRhs.isValid());
	EXPECT_EQ(reinterpret_cast<std::string&>(sutRhs.value()), "foo");
}

TEST(TestPropertyProxyVoid, swap_OppositeValuesAfterInvalidSwap)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };
	std::string initValueRhs{ "bar" };
	sutRhs.construct(reinterpret_cast<std::byte&>(initValueRhs));

	sutLhs.swap(sutRhs);

	EXPECT_FALSE(sutRhs.isValid());
	EXPECT_TRUE(sutLhs.isValid());
	EXPECT_EQ(reinterpret_cast<std::string&>(sutLhs.value()), "bar");
}

TEST(TestPropertyProxyVoid, equal_CompareNotEqualToValueWhenInvalid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string rhs{ "foo" };

	EXPECT_FALSE(sut.equal(reinterpret_cast<std::byte&>(rhs)));
	EXPECT_FALSE(sut == reinterpret_cast<std::byte&>(rhs));
	EXPECT_TRUE(sut != reinterpret_cast<std::byte&>(rhs));
}
TEST(TestPropertyProxyVoid, equal_CompareEqualToValueWhenValid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string rhs{ "foo" };
	std::string initValue{ "foo" };

	sut.construct(reinterpret_cast<std::byte&>(initValue));

	EXPECT_TRUE(sut.equal(reinterpret_cast<std::byte&>(rhs)));
	EXPECT_TRUE(sut == reinterpret_cast<std::byte&>(rhs));
	EXPECT_FALSE(sut != reinterpret_cast<std::byte&>(rhs));
}

TEST(TestPropertyProxyVoid, equal_CompareNotEqualToValueWhenValid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string rhs{ "bar" };
	std::string initValue{ "foo" };

	sut.construct(reinterpret_cast<std::byte&>(initValue));

	EXPECT_FALSE(sut.equal(reinterpret_cast<std::byte&>(rhs)));
	EXPECT_FALSE(sut == reinterpret_cast<std::byte&>(rhs));
	EXPECT_TRUE(sut != reinterpret_cast<std::byte&>(rhs));
}

TEST(TestPropertyProxyVoid, equal_CompareNotEqualToInvalidPropertyWhenInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };

	EXPECT_FALSE(sutLhs.equal(sutRhs));
	EXPECT_FALSE(sutLhs == sutRhs);
	EXPECT_TRUE(sutLhs != sutRhs);
}

TEST(TestPropertyProxyVoid, equal_CompareNotEqualToValidPropertyWhenInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };
	std::string initValueRhs{ "foo" };

	sutRhs.construct(reinterpret_cast<std::byte&>(initValueRhs));

	EXPECT_FALSE(sutLhs.equal(sutRhs));
	EXPECT_FALSE(sutLhs == sutRhs);
	EXPECT_TRUE(sutLhs != sutRhs);
}

TEST(TestPropertyProxyVoid, equal_CompareEqualToValidPropertyWhenValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };
	std::string initValueLhs{ "foo" };
	std::string initValueRhs{ "foo" };

	sutLhs.construct(reinterpret_cast<std::byte&>(initValueLhs));
	sutRhs.construct(reinterpret_cast<std::byte&>(initValueRhs));

	EXPECT_TRUE(sutLhs.equal(sutRhs));
	EXPECT_TRUE(sutLhs == sutRhs);
	EXPECT_FALSE(sutLhs != sutRhs);
}

TEST(TestPropertyProxyVoid, equal_CompareNotEqualToValidPropertyWhenValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };
	std::string initValueLhs{ "foo" };
	std::string initValueRhs{ "bar" };

	sutLhs.construct(reinterpret_cast<std::byte&>(initValueLhs));
	sutRhs.construct(reinterpret_cast<std::byte&>(initValueRhs));

	EXPECT_FALSE(sutLhs.equal(sutRhs));
	EXPECT_FALSE(sutLhs == sutRhs);
	EXPECT_TRUE(sutLhs != sutRhs);
}

TEST(TestPropertyProxyVoid, less_CompareNotLessToValueWhenInvalid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string rhs{ "fou" };

	EXPECT_FALSE(sut.less(reinterpret_cast<std::byte&>(rhs)));
	EXPECT_FALSE(sut < reinterpret_cast<std::byte&>(rhs));
}

TEST(TestPropertyProxyVoid, less_CompareLessToValueWhenValid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string rhs{ "fou" };
	std::string initValue{ "foo" };

	sut.construct(reinterpret_cast<std::byte&>(initValue));

	EXPECT_TRUE(sut.less(reinterpret_cast<std::byte&>(rhs)));
	EXPECT_TRUE(sut < reinterpret_cast<std::byte&>(rhs));
}

TEST(TestPropertyProxyVoid, less_CompareNotLessToValueWhenValid)
{
	dots::types::DotsTestStruct dts;
	dots::type::PropertyProxy<void> sut{ dts.stringField };
	std::string rhs{ "bar" };
	std::string initValue{ "foo" };

	sut.construct(reinterpret_cast<std::byte&>(initValue));

	EXPECT_FALSE(sut.less(reinterpret_cast<std::byte&>(rhs)));
	EXPECT_FALSE(sut < reinterpret_cast<std::byte&>(rhs));
}

TEST(TestPropertyProxyVoid, less_CompareLessToInvalidPropertyWhenInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };

	EXPECT_TRUE(sutLhs.less(sutRhs));
	EXPECT_TRUE(sutLhs < sutRhs);
}

TEST(TestPropertyProxyVoid, less_CompareNotLessToValidPropertyWhenInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };
	std::string initValueRhs{ "fou" };

	sutRhs.construct(reinterpret_cast<std::byte&>(initValueRhs));

	EXPECT_FALSE(sutLhs.less(sutRhs));
	EXPECT_FALSE(sutLhs < sutRhs);
}

TEST(TestPropertyProxyVoid, less_CompareLessToValidPropertyValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };
	std::string initValueLhs{ "foo" };
	std::string initValueRhs{ "fou" };

	sutLhs.construct(reinterpret_cast<std::byte&>(initValueLhs));
	sutRhs.construct(reinterpret_cast<std::byte&>(initValueRhs));

	EXPECT_TRUE(sutLhs.less(sutRhs));
	EXPECT_TRUE(sutLhs < sutRhs);
}

TEST(TestPropertyProxyVoid, less_CompareNotLessToValidPropertyValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	dots::type::PropertyProxy<void> sutLhs{ dtsLhs.stringField };
	dots::type::PropertyProxy<void> sutRhs{ dtsRhs.stringField };
	std::string initValueLhs{ "foo" };
	std::string initValueRhs{ "bar" };

	sutLhs.construct(reinterpret_cast<std::byte&>(initValueLhs));
	sutRhs.construct(reinterpret_cast<std::byte&>(initValueRhs));

	EXPECT_FALSE(sutLhs.less(sutRhs));
	EXPECT_FALSE(sutLhs < sutRhs);
}