#include "dots/type/ChronoDescriptor.h"
#include "dots/Uuid.h"
#include <gtest/gtest.h>

//using namespace rttr;
#if 0
TEST(TestRttrVariantTypes, testIntegers)
{
    variant var = (int8_t)42;
    EXPECT_EQ(var.get_type(), type::get<int8_t>());
    EXPECT_EQ(var.to_string(), "42");

    var = (int16_t)42;
    EXPECT_EQ(var.get_type(), type::get<int16_t>());
    EXPECT_EQ(var.to_string(), "42");

    var = (int32_t)42;
    EXPECT_EQ(var.get_type(), type::get<int32_t>());
    EXPECT_EQ(var.to_string(), "42");

    var = (int64_t)42;
    EXPECT_EQ(var.get_type(), type::get<int64_t>());
    EXPECT_EQ(var.to_string(), "42");
}

TEST(TestRttrVariantTypes, testUnsingedIntegers)
{
    variant var = (uint8_t)42;
    EXPECT_EQ(var.get_type(), type::get<uint8_t>());
    EXPECT_EQ(var.to_string(), "42");

    var = (uint16_t)42;
    EXPECT_EQ(var.get_type(), type::get<uint16_t>());
    EXPECT_EQ(var.to_string(), "42");

    var = (uint32_t)42;
    EXPECT_EQ(var.get_type(), type::get<uint32_t>());
    EXPECT_EQ(var.to_string(), "42");

    var = (uint64_t)42;
    EXPECT_EQ(var.get_type(), type::get<uint64_t>());
    EXPECT_EQ(var.to_string(), "42");
}

TEST(TestRttrVariantTypes, testFlotingpoint)
{
    variant var = (float)3.141;
    EXPECT_EQ(var.get_type(), type::get<float>());
    EXPECT_EQ(var.to_string(), "3.141");

    var = (double)3.123456;
    EXPECT_EQ(var.get_type(), type::get<double >());
    EXPECT_EQ(var.to_string(), "3.123456");
}

TEST(TestRttrVariantTypes, testBool)
{
    variant var = true;
    EXPECT_EQ(var.get_type(), type::get<bool>());
    EXPECT_EQ(var.to_string(), "true");
}

TEST(TestRttrVariantTypes, testString)
{
    variant var = std::string("Hallo Welt!");
    EXPECT_EQ(var.get_type(), type::get<std::string>());
    EXPECT_EQ(var.to_string(), "Hallo Welt!");
}

TEST(TestRttrVariantTypes, testChrono)
{
    bool ok = false;
    variant var = pnxs::Duration(10);
    EXPECT_EQ(var.get_type(), type::get<pnxs::Duration>());
    EXPECT_EQ(var.to_string(&ok), "10");
    EXPECT_EQ(ok, true);

    var = pnxs::TimePoint(1492084921.7466583);
    ok = false;
    EXPECT_EQ(var.get_type(), type::get<pnxs::TimePoint>());
    EXPECT_EQ(var.to_string(&ok), "2017-04-13T14:02:01,746658CEST");
    EXPECT_EQ(ok, true);

    var = pnxs::SteadyTimePoint(500.1);
    ok = false;
    EXPECT_EQ(var.get_type(), type::get<pnxs::SteadyTimePoint>());
    EXPECT_EQ(var.to_string(&ok), "1970-01-01T01:08:20,100000CET");
    EXPECT_EQ(ok, true);
}

TEST(TestRttrVariantTypes, testPropertySet)
{
    variant var = dots::property_set(0x11);
    EXPECT_EQ(var.get_type(), type::get<dots::property_set>());
    EXPECT_EQ(var.to_string(), "00000000000000000000000000010001");
}

TEST(TestRttrVariantTypes, testUuid)
{
    variant var = dots::uuid();

    EXPECT_EQ(var.get_type(), type::get<dots::uuid>());

}
#endif

// Blob makes a problem with RTTR
// is can't be differd between std::string and blob.
#if 0
TEST(TestRttrVariantTypes, testBlob)
{
    typedef std::string blob;

    variant var = blob("Binary Data");

    EXPECT_EQ(var.get_type(), type::get<blob>());
    EXPECT_NE(var.get_type(), type::get<std::string>());
}
#endif