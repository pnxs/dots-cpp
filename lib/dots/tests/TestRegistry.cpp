#include "dots/type/Registry.h"
#include "DotsTestStruct.dots.h"
#include <gtest/gtest.h>

//using namespace rttr;
using namespace dots::type;

#if 0
TEST(TestRegistry, toWireType)
{
    EXPECT_EQ(Registry::toWireName(type::get<int8_t>()),  "int8");
    EXPECT_EQ(Registry::toWireName(type::get<int16_t>()), "int16");
    EXPECT_EQ(Registry::toWireName(type::get<int32_t>()), "int32");
    EXPECT_EQ(Registry::toWireName(type::get<int64_t>()), "int64");

    EXPECT_EQ(Registry::toWireName(type::get<uint8_t>()),  "uint8");
    EXPECT_EQ(Registry::toWireName(type::get<uint16_t>()), "uint16");
    EXPECT_EQ(Registry::toWireName(type::get<uint32_t>()), "uint32");
    EXPECT_EQ(Registry::toWireName(type::get<uint64_t>()), "uint64");

    EXPECT_EQ(Registry::toWireName(type::get<float>()), "float32");
    EXPECT_EQ(Registry::toWireName(type::get<double>()), "float64");

    EXPECT_EQ(Registry::toWireName(type::get<bool>()), "bool");

    EXPECT_EQ(Registry::toWireName(type::get<std::string>()), "string");

    EXPECT_EQ(Registry::toWireName(type::get<StructPropertyData>()), "StructPropertyData");
    EXPECT_EQ(Registry::toWireName(type::get<DotsStructScope>()), "DotsStructScope");
    EXPECT_EQ(Registry::toWireName(type::get<pnxs::Duration>()), "duration");
    EXPECT_EQ(Registry::toWireName(type::get<pnxs::TimePoint>()), "timepoint");

    dots::Vector<DotsStructScope> enumVector;
    dots::Vector<StructPropertyData> structVector;
    dots::Vector<int16_t> int16Vector;
    dots::Vector<std::string> stringVector;
    dots::Vector<pnxs::Duration> durationVector;

    EXPECT_EQ(Registry::toWireName(type::get(enumVector)),     "vector<DotsStructScope>");
    EXPECT_EQ(Registry::toWireName(type::get(structVector)),   "vector<StructPropertyData>");
    EXPECT_EQ(Registry::toWireName(type::get(int16Vector)),    "vector<int16>");
    EXPECT_EQ(Registry::toWireName(type::get(stringVector)),   "vector<string>");
    EXPECT_EQ(Registry::toWireName(type::get(durationVector)), "vector<duration>");
}

TEST(TestRegistry, fromWireType)
{
    EXPECT_EQ(Registry::fromWireName("int8"), type::get<int8_t>());
    EXPECT_EQ(Registry::fromWireName("int16"), type::get<int16_t>());
    EXPECT_EQ(Registry::fromWireName("int32"), type::get<int32_t>());
    EXPECT_EQ(Registry::fromWireName("int64"), type::get<int64_t>());

    EXPECT_EQ(Registry::fromWireName("uint8"), type::get<uint8_t>());
    EXPECT_EQ(Registry::fromWireName("uint16"), type::get<uint16_t>());
    EXPECT_EQ(Registry::fromWireName("uint32"), type::get<uint32_t>());
    EXPECT_EQ(Registry::fromWireName("uint64"), type::get<uint64_t>());

    EXPECT_EQ(Registry::fromWireName("float32"), type::get<float>());
    EXPECT_EQ(Registry::fromWireName("float64"), type::get<double>());

    EXPECT_EQ(Registry::fromWireName("bool"), type::get<bool>());
    EXPECT_EQ(Registry::fromWireName("string"), type::get<std::string>());

    EXPECT_EQ(Registry::fromWireName("StructPropertyData"), type::get<StructPropertyData>());
    EXPECT_EQ(Registry::fromWireName("DotsStructScope"), type::get<DotsStructScope>());
    EXPECT_EQ(Registry::fromWireName("duration"), type::get<pnxs::Duration>());
    EXPECT_EQ(Registry::fromWireName("timepoint"), type::get<pnxs::TimePoint>());

    dots::Vector<DotsStructScope> enumVector;
    dots::Vector<StructPropertyData> structVector;
    dots::Vector<int16_t> int16Vector;
    dots::Vector<std::string> stringVector;
    dots::Vector<pnxs::Duration> durationVector;

    EXPECT_EQ(Registry::fromWireName("vector<DotsStructScope>"), type::get(enumVector));
    EXPECT_EQ(Registry::fromWireName("vector<StructPropertyData>"), type::get(structVector));
    EXPECT_EQ(Registry::fromWireName("vector<int16>"), type::get(int16Vector));
    EXPECT_EQ(Registry::fromWireName("vector<string>"), type::get(stringVector));
    EXPECT_EQ(Registry::fromWireName("vector<duration>"), type::get(durationVector));
}
#endif