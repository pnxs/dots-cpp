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

TEST(TestStruct, assign_ExpectedPropertiesAfterCompleteAssign)
{
	DotsTestStruct sut;
	sut.indKeyfField(1);
	sut.stringField("foo");
	sut.floatField(3.1415f);

	DotsTestStruct other;
	other.indKeyfField(2);
	other.stringField("bar");
	other.floatField(2.7183f);

	static_cast<dots::type::Struct&>(sut)._assign(other);

	EXPECT_EQ(sut.indKeyfField, 2);
	EXPECT_EQ(sut.stringField, "bar");
	EXPECT_EQ(sut.floatField, 2.7183f);
}

TEST(TestStruct, assign_ExpectedPropertiesAfterPartialAssign)
{
	DotsTestStruct sut;
	sut.indKeyfField(1);
	sut.stringField("foo");
	sut.floatField(3.1415f);

	DotsTestStruct other;
	other.indKeyfField(2);
	other.stringField("bar");

	static_cast<dots::type::Struct&>(sut)._assign(other, ~DotsTestStruct::floatField_t::Set());

	EXPECT_EQ(sut.indKeyfField, 2);
	EXPECT_EQ(sut.stringField, "bar");
	EXPECT_FALSE(sut.floatField.isValid());
}

TEST(TestStruct, copy_ExpectedPropertiesAfterCompleteCopy)
{
	DotsTestStruct sut;
	sut.indKeyfField(1);
	sut.stringField("foo");
	sut.floatField(3.1415f);

	DotsTestStruct other;
	other.indKeyfField(2);
	other.stringField("bar");
	other.floatField(2.7183f);

	static_cast<dots::type::Struct&>(sut)._copy(other);

	EXPECT_EQ(sut.indKeyfField, 2);
	EXPECT_EQ(sut.stringField, "bar");
	EXPECT_EQ(sut.floatField, 2.7183f);
}

TEST(TestStruct, copy_ExpectedPropertiesAfterPartialCopy)
{
	DotsTestStruct sut;
	sut.indKeyfField(1);
	sut.stringField("foo");
	sut.floatField(3.1415f);

	DotsTestStruct other;
	other.indKeyfField(2);
	other.stringField("bar");

	static_cast<dots::type::Struct&>(sut)._copy(other, ~DotsTestStruct::floatField_t::Set());

	EXPECT_EQ(sut.indKeyfField, 2);
	EXPECT_EQ(sut.stringField, "bar");
	EXPECT_EQ(sut.floatField, 3.1415f);
}

TEST(TestStruct, merge_ExpectedPropertiesAfterCompleteMerge)
{
	DotsTestStruct sut;
	sut.indKeyfField(1);
	sut.stringField("foo");
	sut.floatField(3.1415f);

	DotsTestStruct other;
	other.indKeyfField(2);
	other.stringField("bar");

	static_cast<dots::type::Struct&>(sut)._merge(other);

	EXPECT_EQ(sut.indKeyfField, 2);
	EXPECT_EQ(sut.stringField, "bar");
	EXPECT_EQ(sut.floatField, 3.1415f);
}

TEST(TestStruct, merge_ExpectedPropertiesAfterPartialMerge)
{
	DotsTestStruct sut;
	sut.indKeyfField(1);
	sut.stringField("foo");
	sut.floatField(3.1415f);

	DotsTestStruct other;
	other.indKeyfField(2);
	other.stringField("bar");
	other.floatField(2.7183f);

	static_cast<dots::type::Struct&>(sut)._merge(other, ~DotsTestStruct::stringField_t::Set());

	EXPECT_EQ(sut.indKeyfField, 2);
	EXPECT_EQ(sut.stringField, "foo");
	EXPECT_EQ(sut.floatField, 2.7183f);
}

TEST(TestStruct, swap_ExpectedPropertiesAfterCompleteSwap)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2;
	dts2.indKeyfField(2);
	dts2.stringField("bar");

	static_cast<dots::type::Struct&>(dts1)._swap(dts2);

	EXPECT_EQ(dts1.indKeyfField, 2);
	EXPECT_EQ(dts1.stringField, "bar");
	EXPECT_FALSE(dts1.floatField.isValid());

	EXPECT_EQ(dts2.indKeyfField, 1);
	EXPECT_EQ(dts2.stringField, "foo");
	EXPECT_TRUE(dts2.floatField.isValid());
	EXPECT_EQ(dts2.floatField, 3.1415f);
}

TEST(TestStruct, swap_ExpectedPropertiesAfterPartialSwap)
{
	DotsTestStruct dts1;
	dts1.indKeyfField(1);
	dts1.stringField("foo");
	dts1.floatField(3.1415f);

	DotsTestStruct dts2;
	dts2.indKeyfField(2);
	dts2.stringField("bar");
	dts2.floatField(2.7183f);

	static_cast<dots::type::Struct&>(dts1)._swap(dts2, DotsTestStruct::floatField_t::Set());

	EXPECT_EQ(dts1.indKeyfField, 1);
	EXPECT_EQ(dts1.stringField, "foo");
	EXPECT_EQ(dts1.floatField, 2.7183f);

	EXPECT_EQ(dts2.indKeyfField, 2);
	EXPECT_EQ(dts2.stringField, "bar");
	EXPECT_EQ(dts2.floatField, 3.1415f);
}

TEST(TestStruct, clear_AllPropertiesInvalidAfterCompleteClear)
{
	DotsTestStruct sut;
	sut.indKeyfField(1);
	sut.stringField("foo");
	sut.floatField(3.1415f);

	static_cast<dots::type::Struct&>(sut)._clear();

	EXPECT_FALSE(sut.indKeyfField.isValid());
	EXPECT_FALSE(sut.stringField.isValid());
	EXPECT_FALSE(sut.floatField.isValid());
}

TEST(TestStruct, clear_OnlyClearedPropertiesInvalidAfterPartialClear)
{
	DotsTestStruct sut;
	sut.indKeyfField(1);
	sut.stringField("foo");
	sut.floatField(3.1415f);

	static_cast<dots::type::Struct&>(sut)._clear(~DotsTestStruct::stringField_t::Set());

	EXPECT_FALSE(sut.indKeyfField.isValid());
	EXPECT_TRUE(sut.stringField.isValid());
	EXPECT_EQ(sut.stringField, "foo");
	EXPECT_FALSE(sut.floatField.isValid());
}