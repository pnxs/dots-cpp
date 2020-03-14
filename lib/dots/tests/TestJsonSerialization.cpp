#include <dots/io/serialization/JsonSerialization.h>
#include "StructDescriptorData.dots.h"
#include "DotsTestStruct.dots.h"
#include "DotsTestVectorStruct.dots.h"
#include "dots/io/Registry.h"
#include <gtest/gtest.h>

using namespace dots::type;
using namespace dots::types;

TEST(TestJsonSerialization, plainSerialization)
{
    StructDescriptorData sd;
    sd.name("aName");

    auto& properties = sd.properties();
    auto& documentation = sd.documentation();

    StructPropertyData pd;
    pd.name("aProperty");
    pd.tag(1);
    pd.type("type");
    pd.isKey(false);

    properties.push_back(pd);
    pd.name = "anotherProperty";
    pd.tag = 2;
    properties.push_back(pd);

    documentation.description("aDescription");
    documentation.comment("aComment");

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

    EXPECT_EQ(dots::to_json(&sd._Descriptor(), &sd), expectedValue);
}

TEST(TestJsonSerialization, plainSerializationSingleLine)
{
    StructDescriptorData sd;
    sd.name("aName");

    auto& properties = sd.properties();
    auto& documentation = sd.documentation();

    StructPropertyData pd;
    pd.name("aProperty");
    pd.tag(1);
    pd.type("type");
    pd.isKey(false);

    properties.push_back(pd);
    pd.name = "anotherProperty";
    pd.tag = 2;
    properties.push_back(pd);

    documentation.description("aDescription");
    documentation.comment("aComment");

    std::string expectedValue =
        R"({"name":"aName","properties":[{"name":"aProperty","tag":1,"isKey":false,"type":"type"},{"name":"anotherProperty","tag":2,"isKey":false,"type":"type"}],"documentation":{"description":"aDescription","comment":"aComment"}})";

    dots::ToJsonOptions opts;
    opts.prettyPrint = false;

    EXPECT_EQ(dots::to_json(sd, PropertySet::All, opts), expectedValue);
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

    EXPECT_EQ(dots::from_json(inputData, &sd._Descriptor(), &sd), inputData.size());

    //EXPECT_EQ(sd.validProperties(), StructDescriptorData::Att::name + StructDescriptorData::Att::properties + StructDescriptorData::Att::documentation);
    ASSERT_TRUE(sd.name.isValid());
    ASSERT_TRUE(sd.properties.isValid());
    ASSERT_TRUE(sd.documentation.isValid());
    EXPECT_FALSE(sd.flags.isValid());
    EXPECT_FALSE(sd.scope.isValid());


    EXPECT_EQ(sd.name, "aName");
    ASSERT_EQ(sd.properties->size(), 2);

    EXPECT_EQ((*sd.properties)[0].name, "aProperty");
    EXPECT_EQ((*sd.properties)[0].tag, 1u);
    EXPECT_EQ((*sd.properties)[0].type, "type");
    EXPECT_EQ((*sd.properties)[0].isKey, false);

    EXPECT_EQ((*sd.properties)[1].name, "anotherProperty");
    EXPECT_EQ((*sd.properties)[1].tag, 2u);
    EXPECT_EQ((*sd.properties)[1].type, "type");
    EXPECT_EQ((*sd.properties)[1].isKey, false);

    EXPECT_EQ(sd.documentation->description, "aDescription");
    EXPECT_EQ(sd.documentation->comment, "aComment");
}

TEST(TestJsonSerialization, serializeDotsTestStruct)
{
    DotsTestStruct dt;
    dots::types::uuid_t testUuid = dots::types::uuid_t::FromData("1234567890123456");

    dt.enumField(DotsTestEnum::value3);
    dt.floatField(3.141f);
    dt.indKeyfField(42);
    dt.stringField("Hallo Welt");
    dt.tp(timepoint_t{ duration_t{ 1503571800.123456 } });
    dt.uuid(testUuid);

    auto& dss = dt.subStruct();
    dss.flag1(true);

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


    EXPECT_EQ(dots::to_json(&dt._Descriptor(), &dt), expectedValue);
}

TEST(TestJsonSerialization, deserializeDotsTestStruct)
{
    dots::types::uuid_t testUuid = dots::types::uuid_t::FromData("1234567890123456");

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

    EXPECT_EQ(dots::from_json(inputData, &dt._Descriptor(), &dt), inputData.size());

    ASSERT_TRUE(dt.enumField.isValid());
    ASSERT_TRUE(dt.floatField.isValid());
    ASSERT_TRUE(dt.indKeyfField.isValid());
    ASSERT_TRUE(dt.stringField.isValid());
    ASSERT_TRUE(dt.tp.isValid());
    ASSERT_TRUE(dt.uuid.isValid());
    ASSERT_TRUE(dt.subStruct.isValid());
    ASSERT_TRUE(dt.subStruct->flag1.isValid());

    EXPECT_EQ(dt.enumField, DotsTestEnum::value3);
    EXPECT_FLOAT_EQ(dt.floatField, 3.141f);
    EXPECT_EQ(dt.indKeyfField, 42);
    EXPECT_EQ(dt.stringField, "Hallo Welt");
    EXPECT_EQ(dt.tp, timepoint_t{ duration_t{ 1503571800.123456 } });
    EXPECT_EQ(dt.uuid, testUuid);
    EXPECT_EQ(dt.subStruct->flag1, true);

}

TEST(TestJsonSerialization, serializeDynamicallyRegistered)
{
    // Create dynamically registered type "DotsTestStructX" from DotsTestStruct
    /*auto sd = NewDynamicTypeConverter{}(DotsTestStruct::_Descriptor());
    sd.name = "DotsTestStructX";
    auto sdx = NewDynamicTypeConverter{}(sd);*/
	auto sdx = DotsTestStruct::_Descriptor();

    // Assign testdata
    DotsTestStruct testData;
    testData.enumField(DotsTestEnum::value3);
    testData.indKeyfField(41);
    testData.stringField("Hallo");
    auto& subStruct = testData.subStruct();

    subStruct.flag1(true);

    DotsTestStruct* dts = reinterpret_cast<DotsTestStruct*>(sdx.New());

    // Copy testdata into instance of DotsTestStructX
    sdx.swap(*dts, testData, PropertySet::All);

    std::string expectedValue = "{\n"
            "    \"stringField\": \"Hallo\",\n"
            "    \"indKeyfField\": 41,\n"
            "    \"enumField\": 3,\n"
            "    \"subStruct\": {\n"
            "        \"flag1\": true\n"
            "    }\n"
            "}";

    EXPECT_EQ(dots::to_json(*dts), expectedValue);

    delete dts;
}

//TEST(TestJsonSerialization, serializeDynamicallyRegisteredVector)
//{
//    {
//        // Dynamically create DotsTestSubStructX
//        auto testSd = DotsTestSubStruct::_Descriptor().descriptorData();
//        testSd.name = "DotsTestSubStructX";
//        StructDescriptor<>::createFromStructDescriptorData(testSd);
//    }
//
//    // Create dynamically registered type "DotsTestStructX" from DotsTestStruct
//    auto sd = DotsTestVectorStruct::_Descriptor().descriptorData();
//
//    sd.name = "DotsTestVectorStructX";
//    sd.properties->at(1).type = "vector<DotsTestSubStructX>";
//
//
//    auto sdx = StructDescriptor<>::createFromStructDescriptorData(sd);
//
//    DotsTestVectorStruct *dts = reinterpret_cast<DotsTestVectorStruct *>(sdx->New());
//
//    // Assign testdata
//    {
//        DotsTestVectorStruct testData;
//        auto &subStructVector = testData.subStructList();
//
//        DotsTestSubStruct subStruct;
//        subStruct.flag1(true);
//        subStructVector.push_back(subStruct);
//
//        // Copy testdata into instance of DotsTestSubStructX
//        sdx->swap(*dts, testData, PropertySet::All);
//    }
//
//    std::string expectedValue = "{\n"
//            "    \"subStructList\": [\n"
//            "        {\n"
//            "            \"flag1\": true\n"
//            "        }\n"
//            "    ]\n"
//            "}";
//
//    //std::cout << "JSON:\n" << dots::to_json(inst) << "\n";
//    EXPECT_EQ(dots::to_json(sdx, dts), expectedValue);
//
//    dts->_Descriptor().Delete(dts);
//}
