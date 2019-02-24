#include "dots/type/StructDescriptor.h"
#include "dots/type/EnumDescriptor.h"
#include "dots/type/Registry.h"
#include "DotsTestStruct.dots.h"
#include "StructDescriptorData.dots.h"
#include <gtest/gtest.h>

//using namespace rttr;
using namespace dots::type;

#if 0
TEST(TestStruct, constructEnum)
{
	auto testEnumType = type::get_by_name("DotsTestEnum");
	ASSERT_EQ(testEnumType.is_valid(), true);

	auto ed = Registry::toEnumDescriptorData(testEnumType);

	EXPECT_EQ(ed.name(), "DotsTestEnum");

	auto newStruct = EnumDescriptor::createFromEnumDescriptorData(ed);
	EXPECT_TRUE(newStruct != nullptr);

}
#endif

#if 0
TEST(TestStruct, registerSubStruct)
{
	auto t = type::get_by_name("dots::types::DotsTestSubStruct");
	Descriptor::registry().registerType(t);
}
#endif


TEST(TestStruct, construct)
{
	//auto testStructType = type::get_by_name("dots::types::DotsTestStruct");
	//ASSERT_EQ(testStructType.is_valid(), true);

	auto sd = DotsTestStruct::_Descriptor().descriptorData();

	EXPECT_EQ(sd.name, "DotsTestStruct");
	EXPECT_EQ(sd.properties->size(), 7u);

	EXPECT_EQ(sd.properties->at(0).name, "stringField");
	EXPECT_EQ(sd.properties->at(0).type, "string");
	EXPECT_EQ(sd.properties->at(0).tag, 1u);
	EXPECT_EQ(sd.properties->at(0).isKey, false);

	EXPECT_EQ(sd.properties->at(1).name, "indKeyfField");
	EXPECT_EQ(sd.properties->at(1).type, "int32");
	EXPECT_EQ(sd.properties->at(1).tag, 2u);
	EXPECT_EQ(sd.properties->at(1).isKey, true);

	EXPECT_EQ(sd.properties->at(2).name, "floatField");
	EXPECT_EQ(sd.properties->at(2).type, "float32");
	EXPECT_EQ(sd.properties->at(2).tag, 3u);

	EXPECT_EQ(sd.properties->at(3).name, "enumField");
	EXPECT_EQ(sd.properties->at(3).type, "DotsTestEnum");
	EXPECT_EQ(sd.properties->at(3).tag, 4u);

	auto newStruct = StructDescriptor::createFromStructDescriptorData(sd);
	EXPECT_TRUE(newStruct != nullptr);

	DotsTestStruct ts;
	ts.stringField("Hello World");
	ts.indKeyfField(42);
	ts.floatField(3.141);
	ts.enumField(DotsTestEnum::value4);
	ts.tp(pnxs::SystemNow());

	auto& subStruct = ts.subStruct();
	subStruct.flag1(true);

	auto structInstance = newStruct->New();

	newStruct->swap(structInstance, &ts);

	int stringFieldCnt = 0;
	int indKeyfFieldCnt = 0;
	int floatFieldCnt = 0;
	int enumFieldCnt = 0;
	int tpCnt = 0;
	int subStructCnt = 0;
	int uuidCnt = 0;

	for (auto& p : newStruct->properties())
	{
		auto strValue = p.td()->to_string(p.address(structInstance));
		if (p.name() == "stringField") {
			stringFieldCnt++;
			EXPECT_EQ(strValue, "Hello World");
		}
		else if (p.name() == "indKeyfField") {
			indKeyfFieldCnt++;
			EXPECT_EQ(strValue, "42");
		}
		else if (p.name() == "floatField") {
			floatFieldCnt++;
			EXPECT_EQ(strValue, "3.141");
		}
		else if (p.name() == "enumField") {
			enumFieldCnt++;
			EXPECT_EQ(strValue, "value4");
		}
		else if (p.name() == "tp") {
			tpCnt++;
			//TODO: add test-expectation
		}
		else if (p.name() == "subStruct") {
			subStructCnt++;
			//TODO: add test-expectation
		}
		else if (p.name() == "uuid") {
			uuidCnt++;
		}
		else {
			ASSERT_TRUE(false);
		}
	}

	newStruct->Delete(structInstance);

	EXPECT_EQ(stringFieldCnt, 1);
	EXPECT_EQ(indKeyfFieldCnt, 1);
	EXPECT_EQ(floatFieldCnt, 1);
	EXPECT_EQ(enumFieldCnt, 1);
	EXPECT_EQ(tpCnt, 1);
	EXPECT_EQ(subStructCnt, 1);
	EXPECT_EQ(uuidCnt, 1);
}

TEST(TestStruct, size_descriptor)
{
	DotsTestStruct dts;
	EXPECT_EQ(dts._descriptor().sizeOf(), sizeof(dts));
}

TEST(TestStruct, alignment_descriptor)
{
	DotsTestStruct dts;
	EXPECT_EQ(dts._descriptor().alignOf(), alignof(dts));
}

TEST(TestStruct, offsets_static)
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

TEST(TestStruct, offsets_descriptor)
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

TEST(TestStruct, flags)
{
	EXPECT_TRUE(DotsTestStruct::_IsCached());
	EXPECT_FALSE(DotsTestStruct::_IsInternal());
	EXPECT_FALSE(DotsTestStruct::_IsPersistent());
	EXPECT_FALSE(DotsTestStruct::_IsCleanup());
	EXPECT_FALSE(DotsTestStruct::_IsLocal());
	EXPECT_FALSE(DotsTestStruct::_IsSubstructOnly());
}

TEST(TestStruct, keyPropertySet)
{
	EXPECT_EQ(DotsTestStruct::_KeyPropertySet(), DotsTestStruct::indKeyfField_t::Set());
}

TEST(TestStruct, initializerConstruct)
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

TEST(TestStruct, copyConstruct)
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

TEST(TestStruct, copyAssign)
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

TEST(TestStruct, moveConstruct)
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

TEST(TestStruct, moveAssign)
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

TEST(TestStruct, less)
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

TEST(TestStruct, equal)
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

TEST(TestStruct, swap)
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

TEST(TestStruct, swapSpecific)
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