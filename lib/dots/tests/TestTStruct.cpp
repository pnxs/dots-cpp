#include "dots/type/StructDescriptor.h"
#include "dots/type/EnumDescriptor.h"
#include "dots/type/Registry.h"
#include "DotsTestStruct.dots.h"
#include "StructDescriptorData.dots.h"
#include <gtest/gtest.h>

TEST(TestTStruct, size_descriptor)
{
	DotsTestStruct dts;
	EXPECT_EQ(dts._descriptor().sizeOf(), sizeof(dts));
}

TEST(TestTStruct, alignment_descriptor)
{
	DotsTestStruct dts;
	EXPECT_EQ(dts._descriptor().alignOf(), alignof(dts));
}

TEST(TestTStruct, offsets_static)
{
	DotsTestStruct dts;

	auto determine_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&dts); };
	EXPECT_EQ(DotsTestStruct::stringField_t::Offset(), determine_offset(dts.stringField));
	EXPECT_EQ(DotsTestStruct::indKeyfField_t::Offset(), determine_offset(dts.indKeyfField));
	EXPECT_EQ(DotsTestStruct::floatField_t::Offset(), determine_offset(dts.floatField));
	EXPECT_EQ(DotsTestStruct::enumField_t::Offset(), determine_offset(dts.enumField));
	EXPECT_EQ(DotsTestStruct::tp_t::Offset(), determine_offset(dts.tp));
	EXPECT_EQ(DotsTestStruct::subStruct_t::Offset(), determine_offset(dts.subStruct));
	EXPECT_EQ(DotsTestStruct::uuid_t::Offset(), determine_offset(dts.uuid));
}

TEST(TestTStruct, offsets_descriptor)
{
	DotsTestStruct dts;

	auto determine_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&dts); };
	EXPECT_EQ(dts._descriptor().properties()[0].offset(), determine_offset(dts.stringField));
	EXPECT_EQ(dts._descriptor().properties()[1].offset(), determine_offset(dts.indKeyfField));
	EXPECT_EQ(dts._descriptor().properties()[2].offset(), determine_offset(dts.floatField));
	EXPECT_EQ(dts._descriptor().properties()[3].offset(), determine_offset(dts.enumField));
	EXPECT_EQ(dts._descriptor().properties()[4].offset(), determine_offset(dts.tp));
	EXPECT_EQ(dts._descriptor().properties()[5].offset(), determine_offset(dts.subStruct));
	EXPECT_EQ(dts._descriptor().properties()[6].offset(), determine_offset(dts.uuid));
}

TEST(TestTStruct, flags)
{
	EXPECT_TRUE(DotsTestStruct::_IsCached());
	EXPECT_FALSE(DotsTestStruct::_IsInternal());
	EXPECT_FALSE(DotsTestStruct::_IsPersistent());
	EXPECT_FALSE(DotsTestStruct::_IsCleanup());
	EXPECT_FALSE(DotsTestStruct::_IsLocal());
	EXPECT_FALSE(DotsTestStruct::_IsSubstructOnly());
}

TEST(TestTStruct, keyPropertySet)
{
	EXPECT_EQ(DotsTestStruct::_KeyPropertySet(), DotsTestStruct::indKeyfField_t::Set());
}

TEST(TestTStruct, initializerConstruct)
{
	DotsTestStruct dts1{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::stringField_t_i{ "foo" },
		DotsTestStruct::floatField_t_i{ 3.1415f }
	};

	EXPECT_EQ(dts1.indKeyfField, 1);
	EXPECT_EQ(dts1.stringField, "foo");
	EXPECT_EQ(dts1.floatField, 3.1415f);
}

TEST(TestTStruct, copyConstruct)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2{ dts1 };

	EXPECT_EQ(dts1.indKeyfField, dts2.indKeyfField);
	EXPECT_EQ(dts1.stringField, dts2.stringField);
	EXPECT_EQ(dts1.floatField, dts2.floatField);
}

TEST(TestTStruct, copyAssign)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2 = dts1;

	EXPECT_EQ(dts1.indKeyfField, dts2.indKeyfField);
	EXPECT_EQ(dts1.stringField, dts2.stringField);
	EXPECT_EQ(dts1.floatField, dts2.floatField);
}

TEST(TestTStruct, moveConstruct)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2{ std::move(dts1) };

	EXPECT_FALSE(dts1.indKeyfField.isValid());
	EXPECT_FALSE(dts1.stringField.isValid());
	EXPECT_FALSE(dts1.floatField.isValid());

	EXPECT_EQ(dts2.indKeyfField, 1);
	EXPECT_EQ(dts2.stringField, "foo");
	EXPECT_EQ(dts2.floatField, 3.1415f);
}

TEST(TestTStruct, moveAssign)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2 = std::move(dts1);

	EXPECT_FALSE(dts1.indKeyfField.isValid());
	EXPECT_FALSE(dts1.stringField.isValid());
	EXPECT_FALSE(dts1.floatField.isValid());

	EXPECT_EQ(dts2.indKeyfField, 1);
	EXPECT_EQ(dts2.stringField, "foo");
	EXPECT_EQ(dts2.floatField, 3.1415f);
}

TEST(TestTStruct, less)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2;
	dts2.indKeyfField(2);
	dts2.stringField("bar");
	dts2.floatField(2.7183f);

	dts1.swap(dts2, DotsTestStruct::floatField_t::Set());

	EXPECT_LT(dts1, dts2);
}

TEST(TestTStruct, equal)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2;
	dts2.indKeyfField(1);
	dts2.stringField("foo");
	dts2.floatField(3.1415f);

	EXPECT_TRUE(DotsTestStruct::_Descriptor().equal(&dts1, &dts2));
}

TEST(TestTStruct, swap)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2;
	dts2.indKeyfField(2);
	dts2.stringField("bar");

	dts1.swap(dts2);

	EXPECT_EQ(dts1.indKeyfField, 2);
	EXPECT_EQ(dts1.stringField, "bar");
	EXPECT_FALSE(dts1.floatField.isValid());

	EXPECT_EQ(dts2.indKeyfField, 1);
	EXPECT_EQ(dts2.stringField, "foo");
	EXPECT_TRUE(dts2.floatField.isValid());
	EXPECT_EQ(dts2.floatField, 3.1415f);
}

TEST(TestTStruct, swapSpecific)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2;
	dts2.indKeyfField(2);
	dts2.stringField("bar");
	dts2.floatField(2.7183f);

	dts1.swap(dts2, DotsTestStruct::floatField_t::Set());

	EXPECT_EQ(dts1.indKeyfField, 1);
	EXPECT_EQ(dts1.stringField, "foo");
	EXPECT_EQ(dts1.floatField, 2.7183f);

	EXPECT_EQ(dts2.indKeyfField, 2);
	EXPECT_EQ(dts2.stringField, "bar");
	EXPECT_EQ(dts2.floatField, 3.1415f);
}