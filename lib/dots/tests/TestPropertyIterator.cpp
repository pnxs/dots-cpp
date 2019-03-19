#include <gtest/gtest.h>
#include "dots/type/StructDescriptor.h"
#include "dots/type/EnumDescriptor.h"
#include "dots/type/Registry.h"
#include "dots/type/PropertyIterator.h"
#include "dots/type/PropertyProxy.h"
#include "DotsTestStruct.dots.h"

TEST(TestPropertyIterator_Forward, operator_dereference_ThrowOnInvalid)
{
	DotsTestStruct dts;
	auto sut = dts._end();

	EXPECT_THROW(*sut, std::logic_error);
}

TEST(TestPropertyIterator_Forward, operator_prefixIncrement_ExpectedNumberOfIterations)
{
	size_t numIterations = 0;
	DotsTestStruct dts;
	auto sut = dts._begin();

	std::for_each(sut, dts._end(), [&](const auto&) { ++numIterations; });

	EXPECT_EQ(numIterations, dts._descriptor().properties().size());
}

TEST(TestPropertyIterator_Forward, operator_prefixIncrement_ExpectedIterationWithAllProperties)
{
	DotsTestStruct dts;
	auto sut = dts._begin();

	EXPECT_EQ(sut->tag(), dts.stringField.tag());
	EXPECT_EQ((++sut)->tag(), dts.indKeyfField.tag());
	EXPECT_EQ((++sut)->tag(), dts.floatField.tag());
	EXPECT_EQ((++sut)->tag(), dts.enumField.tag());
	EXPECT_EQ((++sut)->tag(), dts.tp.tag());
	EXPECT_EQ((++sut)->tag(), dts.subStruct.tag());
	EXPECT_EQ((++sut)->tag(), dts.uuid.tag());
	EXPECT_EQ((++sut), dts._end());
}

TEST(TestPropertyIterator_Forward, operator_prefixIncrement_ExpectedIterationWithPartialProperties)
{
	DotsTestStruct dts;
	auto sut = dts._begin(dts.indKeyfField.set() | dts.enumField.set() | dts.tp.set() | dts.subStruct.set());

	EXPECT_EQ(sut->tag(), dts.indKeyfField.tag());
	EXPECT_EQ((++sut)->tag(), dts.enumField.tag());
	EXPECT_EQ((++sut)->tag(), dts.tp.tag());
	EXPECT_EQ((++sut)->tag(), dts.subStruct.tag());
	EXPECT_EQ((++sut), dts._end());
}

TEST(TestPropertyIterator_Forward, operator_postfixIncrement_ExpectedIterationWithAllProperties)
{
	DotsTestStruct dts;
	auto sut = dts._begin();

	EXPECT_EQ((sut++)->tag(), dts.stringField.tag());
	EXPECT_EQ((sut++)->tag(), dts.indKeyfField.tag());
	EXPECT_EQ((sut++)->tag(), dts.floatField.tag());
	EXPECT_EQ((sut++)->tag(), dts.enumField.tag());
	EXPECT_EQ((sut++)->tag(), dts.tp.tag());
	EXPECT_EQ((sut++)->tag(), dts.subStruct.tag());
	EXPECT_EQ((sut++)->tag(), dts.uuid.tag());
	EXPECT_EQ(sut, dts._end());
}

TEST(TestPropertyIterator_Forward, operator_postfixIncrement_ExpectedIterationWithPartialProperties)
{
	DotsTestStruct dts;
	auto sut = dts._begin(dts.indKeyfField.set() | dts.enumField.set() | dts.tp.set() | dts.subStruct.set());

	EXPECT_EQ((sut++)->tag(), dts.indKeyfField.tag());
	EXPECT_EQ((sut++)->tag(), dts.enumField.tag());
	EXPECT_EQ((sut++)->tag(), dts.tp.tag());
	EXPECT_EQ((sut++)->tag(), dts.subStruct.tag());
	EXPECT_EQ(sut, dts._end());
}

TEST(TestPropertyIterator_Reverse, operator_dereference_ThrowOnInvalid)
{
	DotsTestStruct dts;
	auto sut = dts._rend();

	EXPECT_THROW(*sut, std::logic_error);
}

TEST(TestPropertyIterator_Reverse, operator_prefixIncrement_ExpectedNumberOfIterations)
{
	size_t numIterations = 0;
	DotsTestStruct dts;
	auto sut = dts._rbegin();

	std::for_each(sut, dts._rend(), [&](const auto&) { ++numIterations; });

	EXPECT_EQ(numIterations, dts._descriptor().properties().size());
}

TEST(TestPropertyIterator_Reverse, operator_prefixIncrement_ExpectedIterationWithAllProperties)
{
	DotsTestStruct dts;
	auto sut = dts._rbegin();

	EXPECT_EQ(sut->tag(), dts.uuid.tag());
	EXPECT_EQ((++sut)->tag(), dts.subStruct.tag());
	EXPECT_EQ((++sut)->tag(), dts.tp.tag());
	EXPECT_EQ((++sut)->tag(), dts.enumField.tag());
	EXPECT_EQ((++sut)->tag(), dts.floatField.tag());
	EXPECT_EQ((++sut)->tag(), dts.indKeyfField.tag());
	EXPECT_EQ((++sut)->tag(), dts.stringField.tag());
	EXPECT_EQ((++sut), dts._rend());
}

TEST(TestPropertyIterator_Reverse, operator_prefixIncrement_ExpectedIterationWithPartialProperties)
{
	DotsTestStruct dts;
	auto sut = dts._rbegin(dts.indKeyfField.set() | dts.enumField.set() | dts.tp.set() | dts.subStruct.set());

	EXPECT_EQ(sut->tag(), dts.subStruct.tag());
	EXPECT_EQ((++sut)->tag(), dts.tp.tag());
	EXPECT_EQ((++sut)->tag(), dts.enumField.tag());
	EXPECT_EQ((++sut)->tag(), dts.indKeyfField.tag());
	EXPECT_EQ((++sut), dts._rend());
}

TEST(TestPropertyIterator_Reverse, operator_postfixIncrement_ExpectedIterationWithAllProperties)
{
	DotsTestStruct dts;
	auto sut = dts._rbegin();

	EXPECT_EQ((sut++)->tag(), dts.uuid.tag());
	EXPECT_EQ((sut++)->tag(), dts.subStruct.tag());
	EXPECT_EQ((sut++)->tag(), dts.tp.tag());
	EXPECT_EQ((sut++)->tag(), dts.enumField.tag());
	EXPECT_EQ((sut++)->tag(), dts.floatField.tag());
	EXPECT_EQ((sut++)->tag(), dts.indKeyfField.tag());
	EXPECT_EQ((sut++)->tag(), dts.stringField.tag());
	EXPECT_EQ(sut, dts._rend());
}

TEST(TestPropertyIterator_Reverse, operator_postfixIncrement_ExpectedIterationWithPartialProperties)
{
	DotsTestStruct dts;
	auto sut = dts._rbegin(dts.indKeyfField.set() | dts.enumField.set() | dts.tp.set() | dts.subStruct.set());

	EXPECT_EQ((sut++)->tag(), dts.subStruct.tag());
	EXPECT_EQ((sut++)->tag(), dts.tp.tag());
	EXPECT_EQ((sut++)->tag(), dts.enumField.tag());
	EXPECT_EQ((sut++)->tag(), dts.indKeyfField.tag());
	EXPECT_EQ(sut, dts._rend());
}