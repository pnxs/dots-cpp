#include <gtest/gtest.h>
#include "dots/type/StructDescriptor.h"
#include "dots/type/EnumDescriptor.h"
#include "dots/type/Registry.h"
#include "DotsTestStruct.dots.h"

TEST(TestTProperty, isValid_InvalidWithoutValue)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;

	EXPECT_FALSE(sut.isValid());
	EXPECT_THROW(sut.value(), std::runtime_error);
}

TEST(TestTProperty, isValid_ValidWithValue)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;

	sut.construct();

	EXPECT_TRUE(sut.isValid());
	EXPECT_NO_THROW(sut.value());
}

TEST(TestTProperty, construct_EqualValueAfterExplicitConstruction)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;

	sut.construct(std::string{ "foo" });

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(sut.value(), std::string{ "foo" });
}

TEST(TestTProperty, construct_EqualValueAfterImplicitConstruction)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;

	sut.construct("foo");

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(sut.value(), "foo");
}

TEST(TestTProperty, construct_EqualValueAfterEmplaceConstruction)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;

	sut.construct(std::string{ "barfoo" }, 3, 3);

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(sut.value(), "foo");
}

TEST(TestTProperty, construct_ThrowOnOverconstruction)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;

	sut.construct();

	EXPECT_THROW(sut.construct(), std::runtime_error);
}

TEST(TestTProperty, destroy_InvalidAfterDestroy)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	sut.construct("foo");

	sut.destroy();

	EXPECT_FALSE(sut.isValid());
}

TEST(TestTProperty, constructOrValue_ConstructOnInvalid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;

	std::string& value = sut.constructOrValue("foo");

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST(TestTProperty, constructOrValue_ValueOnValid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	sut.construct("foo");

	std::string& value = sut.constructOrValue("bar");

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST(TestTProperty, assign_ExceptionOnInvalid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	
	EXPECT_THROW(sut.assign(), std::runtime_error);
}

TEST(TestTProperty, assign_EqualValueAfterExplicitAssign)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	sut.construct();

	sut.assign(std::string{ "foo" });

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(sut.value(), std::string{ "foo" });
}

TEST(TestTProperty, assign_EqualValueAfterImplicitAssign)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	sut.construct();

	sut.assign("foo");

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(sut.value(), "foo");
}

TEST(TestTProperty, assign_EqualValueAfterEmplaceAssign)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	sut.construct();

	sut.assign(std::string{ "barfoo" }, 3, 3);

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(sut.value(), "foo");
}

TEST(TestTProperty, constructOrAssign_ConstructOnInvalid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;

	std::string& value = sut.constructOrAssign("foo");

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST(TestTProperty, constructOrAssign_AssignOnValid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	sut.construct("foo");

	std::string& value = sut.constructOrAssign("bar");

	EXPECT_TRUE(sut.isValid());
	EXPECT_EQ(value, "bar");
}

TEST(TestTProperty, extract_InvalidAfterExtract)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	sut.construct("foo");

	std::string&& value = sut.extract();

	EXPECT_FALSE(sut.isValid());
	EXPECT_EQ(value, "foo");
}

TEST(TestTProperty, swap_OppositeValuesAfterSwapValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;
	sutLhs.construct("foo");
	sutRhs.construct("bar");

	sutLhs.swap(sutRhs);

	EXPECT_EQ(sutLhs.value(), "bar");
	EXPECT_EQ(sutRhs.value(), "foo");
}

TEST(TestTProperty, swap_OppositeValuesAfterSwapInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;
	sutLhs.construct("foo");

	sutLhs.swap(sutRhs);

	EXPECT_FALSE(sutLhs.isValid());
	EXPECT_TRUE(sutRhs.isValid());
	EXPECT_EQ(sutRhs.value(), "foo");
}

TEST(TestTProperty, swap_OppositeValuesAfterInvalidSwap)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;
	sutRhs.construct("bar");

	sutLhs.swap(sutRhs);

	EXPECT_FALSE(sutRhs.isValid());
	EXPECT_TRUE(sutLhs.isValid());
	EXPECT_EQ(sutLhs.value(), "bar");
}

TEST(TestTProperty, equal_CompareNotEqualToValueWhenInvalid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	std::string rhs{ "foo" };

	EXPECT_FALSE(sut.equal(rhs));
	EXPECT_FALSE(sut == rhs);
	EXPECT_TRUE(sut != rhs);
}
TEST(TestTProperty, equal_CompareEqualToValueWhenValid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	std::string rhs{ "foo" };

	sut.construct("foo");

	EXPECT_TRUE(sut.equal(rhs));
	EXPECT_TRUE(sut == rhs);
	EXPECT_FALSE(sut != rhs);
}

TEST(TestTProperty, equal_CompareNotEqualToValueWhenValid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	std::string rhs{ "bar" };

	sut.construct("foo");

	EXPECT_FALSE(sut.equal(rhs));
	EXPECT_FALSE(sut == rhs);
	EXPECT_TRUE(sut != rhs);
}

TEST(TestTProperty, equal_CompareNotEqualToInvalidPropertyWhenInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;

	EXPECT_FALSE(sutLhs.equal(sutRhs));
	EXPECT_FALSE(sutLhs == sutRhs);
	EXPECT_TRUE(sutLhs != sutRhs);
}

TEST(TestTProperty, equal_CompareNotEqualToValidPropertyWhenInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;

	sutRhs.construct("foo");

	EXPECT_FALSE(sutLhs.equal(sutRhs));
	EXPECT_FALSE(sutLhs == sutRhs);
	EXPECT_TRUE(sutLhs != sutRhs);
}

TEST(TestTProperty, equal_CompareEqualToValidPropertyWhenValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;

	sutLhs.construct("foo");
	sutRhs.construct("foo");

	EXPECT_TRUE(sutLhs.equal(sutRhs));
	EXPECT_TRUE(sutLhs == sutRhs);
	EXPECT_FALSE(sutLhs != sutRhs);
}

TEST(TestTProperty, equal_CompareNotEqualToValidPropertyWhenValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;

	sutLhs.construct("foo");
	sutRhs.construct("bar");

	EXPECT_FALSE(sutLhs.equal(sutRhs));
	EXPECT_FALSE(sutLhs == sutRhs);
	EXPECT_TRUE(sutLhs != sutRhs);
}

TEST(TestTProperty, less_CompareNotLessToValueWhenInvalid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	std::string rhs{ "fou" };

	EXPECT_FALSE(sut.less(rhs));
	EXPECT_FALSE(sut < rhs);
}

TEST(TestTProperty, less_CompareLessToValueWhenValid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	std::string rhs{ "fou" };

	sut.construct("foo");

	EXPECT_TRUE(sut.less(rhs));
	EXPECT_TRUE(sut < rhs);
}

TEST(TestTProperty, less_CompareNotLessToValueWhenValid)
{
	dots::types::DotsTestStruct dts;
	auto& sut = dts.stringField;
	std::string rhs{ "bar" };

	sut.construct("foo");

	EXPECT_FALSE(sut.less(rhs));
	EXPECT_FALSE(sut < rhs);
}

TEST(TestTProperty, less_CompareLessToInvalidPropertyWhenInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;

	EXPECT_TRUE(sutLhs.less(sutRhs));
	EXPECT_TRUE(sutLhs < sutRhs);
}

TEST(TestTProperty, less_CompareNotLessToValidPropertyWhenInvalid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;

	sutRhs.construct("fou");

	EXPECT_FALSE(sutLhs.less(sutRhs));
	EXPECT_FALSE(sutLhs < sutRhs);
}

TEST(TestTProperty, less_CompareLessToValidPropertyValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;

	sutLhs.construct("foo");
	sutRhs.construct("fou");

	EXPECT_TRUE(sutLhs.less(sutRhs));
	EXPECT_TRUE(sutLhs < sutRhs);
}

TEST(TestTProperty, less_CompareNotLessToValidPropertyValid)
{
	dots::types::DotsTestStruct dtsLhs;
	dots::types::DotsTestStruct dtsRhs;
	auto& sutLhs = dtsLhs.stringField;
	auto& sutRhs = dtsRhs.stringField;

	sutLhs.construct("foo");
	sutRhs.construct("bar");

	EXPECT_FALSE(sutLhs.less(sutRhs));
	EXPECT_FALSE(sutLhs < sutRhs);
}