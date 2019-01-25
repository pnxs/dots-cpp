#include <dots/io/serialization/JsonSerialization.h>
#include "StructDescriptorData.dots.h"
#include "DotsTestStruct.dots.h"
#include "DotsTestVectorStruct.dots.h"
#include "dots/type/Registry.h"
#include <gtest/gtest.h>

using namespace dots::type;

TEST(TestJsonSerialization, plainSerialization)
{
    StructDescriptorData sd;
    sd.setName("aName");

    auto& properties = sd.refProperties();
    auto& documentation = sd.refDocumentation();

    StructPropertyData pd;
    pd.setName("aProperty");
    pd.setTag(1);
    pd.setType("type");
    pd.setIsKey(false);

    properties.push_back(pd);
    pd.setName("anotherProperty");
    pd.setTag(2);
    properties.push_back(pd);

    documentation.setDescription("aDescription");
    documentation.setComment("aComment");

    std::string expectedValue = "{\n"
        "    \"name\": \"aName\",\n"
        "    \"properties\": [\n"
        "        {\n"
        "            \"name\": \"aProperty\",\n"
        "            \"tag\": 1,\n"
        "            \"isKey\": false,\n"
        "            \"type\": \"type\"\n"
        "        },\n"
        "        {\n"
        "            \"name\": \"anotherProperty\",\n"
        "            \"tag\": 2,\n"
        "            \"isKey\": false,\n"
        "            \"type\": \"type\"\n"
        "        }\n"
        "    ],\n"
        "    \"documentation\": {\n"
        "        \"description\": \"aDescription\",\n"
        "        \"comment\": \"aComment\"\n"
        "    }\n"
        "}";

    EXPECT_EQ(dots::to_json(sd._td(), &sd), expectedValue);
}

TEST(TestJsonSerialization, plainSerializationSingleLine)
{
    StructDescriptorData sd;
    sd.setName("aName");

    auto& properties = sd.refProperties();
    auto& documentation = sd.refDocumentation();

    StructPropertyData pd;
    pd.setName("aProperty");
    pd.setTag(1);
    pd.setType("type");
    pd.setIsKey(false);

    properties.push_back(pd);
    pd.setName("anotherProperty");
    pd.setTag(2);
    properties.push_back(pd);

    documentation.setDescription("aDescription");
    documentation.setComment("aComment");

    std::string expectedValue =
        R"({"name":"aName","properties":[{"name":"aProperty","tag":1,"isKey":false,"type":"type"},{"name":"anotherProperty","tag":2,"isKey":false,"type":"type"}],"documentation":{"description":"aDescription","comment":"aComment"}})";

    dots::ToJsonOptions opts;
    opts.prettyPrint = false;

    EXPECT_EQ(dots::to_json(sd._td(), &sd, dots::AllProperties(), opts), expectedValue);
}

TEST(TestJsonSerialization, plainDeserialization)
{
    std::string inputData = "{\n"
            "    \"name\": \"aName\",\n"
            "    \"properties\": [\n"
            "        {\n"
            "            \"name\": \"aProperty\",\n"
            "            \"tag\": 1,\n"
            "            \"isKey\": false,\n"
            "            \"type\": \"type\"\n"
            "        },\n"
            "        {\n"
            "            \"name\": \"anotherProperty\",\n"
            "            \"tag\": 2,\n"
            "            \"isKey\": false,\n"
            "            \"type\": \"type\"\n"
            "        }\n"
            "    ],\n"
            "    \"documentation\": {\n"
            "        \"description\": \"aDescription\",\n"
            "        \"comment\": \"aComment\"\n"
            "    }\n"
            "}";


    StructDescriptorData sd;

    EXPECT_EQ(dots::from_json(inputData, sd._td(), &sd), inputData.size());

    //EXPECT_EQ(sd.validProperties(), StructDescriptorData::Att::name + StructDescriptorData::Att::properties + StructDescriptorData::Att::documentation);
    ASSERT_TRUE(sd.hasName());
    ASSERT_TRUE(sd.hasProperties());
    ASSERT_TRUE(sd.hasDocumentation());
    EXPECT_FALSE(sd.hasFlags());
    EXPECT_FALSE(sd.hasScope());


    EXPECT_EQ(sd.name(), "aName");
    ASSERT_EQ(sd.properties().size(), 2);

    EXPECT_EQ(sd.properties()[0].name(), "aProperty");
    EXPECT_EQ(sd.properties()[0].tag(), 1);
    EXPECT_EQ(sd.properties()[0].type(), "type");
    EXPECT_EQ(sd.properties()[0].isKey(), false);

    EXPECT_EQ(sd.properties()[1].name(), "anotherProperty");
    EXPECT_EQ(sd.properties()[1].tag(), 2);
    EXPECT_EQ(sd.properties()[1].type(), "type");
    EXPECT_EQ(sd.properties()[1].isKey(), false);

    EXPECT_EQ(sd.documentation().description(), "aDescription");
    EXPECT_EQ(sd.documentation().comment(), "aComment");
}

TEST(TestJsonSerialization, serializeDotsTestStruct)
{
    DotsTestStruct dt;
    dots::uuid testUuid("1234567890123456");

    dt.setEnumField(DotsTestEnum::value3);
    dt.setFloatField(3.141);
    dt.setIndKeyfField(42);
    dt.setStringField("Hallo Welt");
    dt.setTp(dots::TimePoint(1503571800.123456));
    dt.setUuid(testUuid);

    auto& dss = dt.refSubStruct();
    dss.setFlag1(true);

    std::string expectedValue = "{\n"
        "    \"stringField\": \"Hallo Welt\",\n"
        "    \"indKeyfField\": 42,\n"
        "    \"floatField\": 3.1410000324249269,\n"
        "    \"enumField\": 3,\n"
        "    \"tp\": 1503571800.123456,\n"
        "    \"subStruct\": {\n"
        "        \"flag1\": true\n"
        "    },\n"
        "    \"uuid\": \"31323334-3536-3738-3930-313233343536\"\n"
        "}";


    EXPECT_EQ(dots::to_json(dt._td(), &dt), expectedValue);
}

TEST(TestJsonSerialization, deserializeDotsTestStruct)
{
    dots::uuid testUuid("1234567890123456");

    std::string inputData = "{\n"
            "    \"stringField\": \"Hallo Welt\",\n"
            "    \"indKeyfField\": 42,\n"
            "    \"floatField\": 3.1410000324249269,\n"
            "    \"enumField\": 3,\n"
            "    \"tp\": 1503571800.123456,\n"
            "    \"subStruct\": {\n"
            "        \"flag1\": true\n"
            "    },\n"
            "    \"uuid\": \"31323334-3536-3738-3930-313233343536\"\n"
            "}";

    DotsTestStruct dt;

    EXPECT_EQ(dots::from_json(inputData, dt._td(), &dt), inputData.size());

    ASSERT_TRUE(dt.hasEnumField());
    ASSERT_TRUE(dt.hasFloatField());
    ASSERT_TRUE(dt.hasIndKeyfField());
    ASSERT_TRUE(dt.hasStringField());
    ASSERT_TRUE(dt.hasTp());
    ASSERT_TRUE(dt.hasUuid());
    ASSERT_TRUE(dt.hasSubStruct());
    ASSERT_TRUE(dt.subStruct().hasFlag1());

    EXPECT_EQ(dt.enumField(), DotsTestEnum::value3);
    EXPECT_FLOAT_EQ(dt.floatField(), 3.141);
    EXPECT_EQ(dt.indKeyfField(), 42);
    EXPECT_EQ(dt.stringField(), "Hallo Welt");
    EXPECT_EQ(dt.tp(), dots::TimePoint(1503571800.123456));
    EXPECT_EQ(dt.uuid(), testUuid);
    EXPECT_EQ(dt.subStruct().flag1(), true);

}

TEST(TestJsonSerialization, serializeDynamicallyRegistered)
{
    // Create dynamically registered type "DotsTestStructX" from DotsTestStruct
    auto sd = DotsTestStruct::_dd();
    sd.setName("DotsTestStructX");
    auto sdx = StructDescriptor::createFromStructDescriptorData(sd);

    // Assign testdata
    DotsTestStruct testData;
    testData.setEnumField(DotsTestEnum::value3);
    testData.setIndKeyfField(41);
    testData.setStringField("Hallo");
    auto& subStruct = testData.refSubStruct();

    subStruct.setFlag1(true);

    DotsTestStruct* dts = reinterpret_cast<DotsTestStruct*>(sdx->New());

    // Copy testdata into instance of DotsTestStructX
    sdx->swap(dts, &testData);

    std::string expectedValue = "{\n"
            "    \"stringField\": \"Hallo\",\n"
            "    \"indKeyfField\": 41,\n"
            "    \"enumField\": 3,\n"
            "    \"subStruct\": {\n"
            "        \"flag1\": true\n"
            "    }\n"
            "}";

    EXPECT_EQ(dots::to_json(sdx, dts), expectedValue);

    delete dts;
}

TEST(TestJsonSerialization, serializeDynamicallyRegisteredVector)
{
    {
        // Dynamically create DotsTestSubStructX
        auto testSd = DotsTestSubStruct::_dd();
        testSd.setName("DotsTestSubStructX");
        StructDescriptor::createFromStructDescriptorData(testSd);
    }

    // Create dynamically registered type "DotsTestStructX" from DotsTestStruct
    auto sd = DotsTestVectorStruct::_dd();

    sd.setName("DotsTestVectorStructX");
    sd.refProperties().at(1).setType("vector<DotsTestSubStructX>");


    auto sdx = StructDescriptor::createFromStructDescriptorData(sd);

    DotsTestVectorStruct *dts = reinterpret_cast<DotsTestVectorStruct *>(sdx->New());

    // Assign testdata
    {
        DotsTestVectorStruct testData;
        auto &subStructVector = testData.refSubStructList();

        DotsTestSubStruct subStruct;
        subStruct.setFlag1(true);
        subStructVector.push_back(subStruct);

        // Copy testdata into instance of DotsTestSubStructX
        sdx->swap(dts, &testData);
    }

    std::string expectedValue = "{\n"
            "    \"subStructList\": [\n"
            "        {\n"
            "            \"flag1\": true\n"
            "        }\n"
            "    ]\n"
            "}";

    //std::cout << "JSON:\n" << dots::to_json(inst) << "\n";
    EXPECT_EQ(dots::to_json(sdx, dts), expectedValue);

    dts->_td()->Delete(dts);
}
