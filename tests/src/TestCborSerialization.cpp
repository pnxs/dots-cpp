#include "dots/io/serialization/CborNativeSerialization.h"
#include "dots/io/serialization/JsonSerialization.h"
#include "StructDescriptorData.dots.h"
#include "DotsTestStruct.dots.h"
#include "DotsTransportHeader.dots.h"
#include "DotsTestVectorStruct.dots.h"
#include "EnumDescriptorData.dots.h"
#include "dots/io/Registry.h"
#include <dots/dots.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace dots::type;

using ::testing::ElementsAreArray;


TEST(TestCborSerialization, rttrDepends)
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

    std::vector<uint8_t> expectData = {
            0xa3,                                   // map(3)
            0x01,                                   // unsigned(1)
            0x65,                                   // text(5)
            0x61,0x4e,0x61,0x6d,0x65,               // "aName"
            0x02,                                   // unsigned(2)
            0x82,                                   // array(2)
            0xa4,                                   // map(4)
            0x01,                                   // unsigned(1)
            0x69,                                   // text(9)
            0x61,0x50,0x72,0x6f,0x70,0x65,0x72,0x74,0x79,  // "aProperty"
            0x02,                                   // unsigned(2)
            0x01,                                   // unsigned(1)
            0x03,                                   // unsigned(3)
            0xf4,                                   // primitive(20)
            0x04,                                   // unsigned(4)
            0x64,                                   // text(4)
            0x74,0x79,0x70,0x65,                    // "type"
            0xa4,                                   // map(4)
            0x01,                                   // unsigned(1)
            0x6f,                                   // text(15)
            0x61,0x6e,0x6f,0x74,0x68,0x65,0x72,0x50,0x72,0x6f,0x70,0x65,0x72,0x74,0x79, // "anotherProperty"
            0x02,                                   // unsigned(2)
            0x02,                                   // unsigned(2)
            0x03,                                   // unsigned(3)
            0xf4,                                   // primitive(20)
            0x04,                                   // unsigned(4)
            0x64,                                   // text(4)
            0x74,0x79,0x70,0x65,                    // "type"
            0x03,                                   // unsigned(3)
            0xa2,                                   // map(2)
            0x01,                                   // unsigned(1)
            0x6c,                                   // text(12)
            0x61,0x44,0x65,0x73,0x63,0x72,0x69,0x70,0x74,0x69,0x6f,0x6e, // "aDescription"
            0x02,                                   // unsigned(2)
            0x68,                                   // text(8)
            0x61,0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74 // "aComment"
    };

    EXPECT_THAT(dots::to_cbor(sd), ElementsAreArray(expectData));
}

TEST(TestCborSerialization, deserialize)
{
    std::vector<uint8_t> inputData = {
            0xa3,                                   // map(3)
            0x01,                                   // unsigned(1)
            0x65,                                   // text(5)
            0x61,0x4e,0x61,0x6d,0x65,               // "aName"
            0x02,                                   // unsigned(2)
            0x82,                                   // array(2)
            0xa4,                                   // map(4)
            0x01,                                   // unsigned(1)
            0x69,                                   // text(9)
            0x61,0x50,0x72,0x6f,0x70,0x65,0x72,0x74,0x79,  // "aProperty"
            0x02,                                   // unsigned(2)
            0x01,                                   // unsigned(1)
            0x03,                                   // unsigned(3)
            0xf4,                                   // primitive(20)
            0x04,                                   // unsigned(4)
            0x64,                                   // text(4)
            0x74,0x79,0x70,0x65,                    // "type"
            0xa4,                                   // map(4)
            0x01,                                   // unsigned(1)
            0x6f,                                   // text(15)
            0x61,0x6e,0x6f,0x74,0x68,0x65,0x72,0x50,0x72,0x6f,0x70,0x65,0x72,0x74,0x79, // "anotherProperty"
            0x02,                                   // unsigned(2)
            0x02,                                   // unsigned(2)
            0x03,                                   // unsigned(3)
            0xf4,                                   // primitive(20)
            0x04,                                   // unsigned(4)
            0x64,                                   // text(4)
            0x74,0x79,0x70,0x65,                    // "type"
            0x03,                                   // unsigned(3)
            0xa2,                                   // map(2)
            0x01,                                   // unsigned(1)
            0x6c,                                   // text(12)
            0x61,0x44,0x65,0x73,0x63,0x72,0x69,0x70,0x74,0x69,0x6f,0x6e, // "aDescription"
            0x02,                                   // unsigned(2)
            0x68,                                   // text(8)
            0x61,0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74 // "aComment"
    };

    auto sd = dots::decodeInto_cbor<StructDescriptorData>(inputData);

    {
        StructDescriptorData nativeTest;
        dots::from_cbor(inputData.data(), inputData.size(), &nativeTest._Descriptor(), &nativeTest);

        ASSERT_TRUE(nativeTest.name.isValid());
        ASSERT_TRUE(nativeTest.properties.isValid());
        ASSERT_TRUE(nativeTest.documentation.isValid());

        auto properties = StructDescriptorData::name_p + StructDescriptorData::properties_p + StructDescriptorData::documentation_p;
        ASSERT_EQ(nativeTest._validProperties(), properties);

        EXPECT_EQ(nativeTest.name, "aName");
        ASSERT_EQ(nativeTest.properties->size(), 2u);

        EXPECT_EQ(nativeTest.properties->at(0).name, "aProperty");
        EXPECT_EQ(nativeTest.properties->at(0).tag,  1u);
        EXPECT_EQ(nativeTest.properties->at(0).type, "type");
        EXPECT_EQ(nativeTest.properties->at(0).isKey, false);

        EXPECT_EQ(nativeTest.properties->at(1).name, "anotherProperty");
        EXPECT_EQ(nativeTest.properties->at(1).tag,  2u);
        EXPECT_EQ(nativeTest.properties->at(1).type, "type");
        EXPECT_EQ(nativeTest.properties->at(1).isKey, false);

        EXPECT_EQ(nativeTest.documentation->description, "aDescription");
        EXPECT_EQ(nativeTest.documentation->comment, "aComment");
    }

    ASSERT_TRUE(sd.name.isValid());
    ASSERT_TRUE(sd.properties.isValid());
    ASSERT_TRUE(sd.documentation.isValid());

    auto properties = StructDescriptorData::name_p + StructDescriptorData::properties_p + StructDescriptorData::documentation_p;
    ASSERT_EQ(sd._validProperties(), properties);

    EXPECT_EQ(sd.name, "aName");
    ASSERT_EQ(sd.properties->size(), 2u);

    EXPECT_EQ(sd.properties->at(0).name, "aProperty");
    EXPECT_EQ(sd.properties->at(0).tag,  1u);
    EXPECT_EQ(sd.properties->at(0).type, "type");
    EXPECT_EQ(sd.properties->at(0).isKey, false);

    EXPECT_EQ(sd.properties->at(1).name, "anotherProperty");
    EXPECT_EQ(sd.properties->at(1).tag,  2u);
    EXPECT_EQ(sd.properties->at(1).type, "type");
    EXPECT_EQ(sd.properties->at(1).isKey, false);

    EXPECT_EQ(sd.documentation->description, "aDescription");
    EXPECT_EQ(sd.documentation->comment, "aComment");
}

TEST(TestCborSerialization, deserializeDynamic)
{
    std::vector<uint8_t> inputData = {
            0xa3,                                   // map(3)
            0x01,                                   // unsigned(1)
            0x65,                                   // text(5)
            0x61,0x4e,0x61,0x6d,0x65,               // "aName"
            0x02,                                   // unsigned(2)
            0x82,                                   // array(2)
            0xa4,                                   // map(4)
            0x01,                                   // unsigned(1)
            0x69,                                   // text(9)
            0x61,0x50,0x72,0x6f,0x70,0x65,0x72,0x74,0x79,  // "aProperty"
            0x02,                                   // unsigned(2)
            0x01,                                   // unsigned(1)
            0x03,                                   // unsigned(3)
            0xf4,                                   // primitive(20)
            0x04,                                   // unsigned(4)
            0x64,                                   // text(4)
            0x74,0x79,0x70,0x65,                    // "type"
            0xa4,                                   // map(4)
            0x01,                                   // unsigned(1)
            0x6f,                                   // text(15)
            0x61,0x6e,0x6f,0x74,0x68,0x65,0x72,0x50,0x72,0x6f,0x70,0x65,0x72,0x74,0x79, // "anotherProperty"
            0x02,                                   // unsigned(2)
            0x02,                                   // unsigned(2)
            0x03,                                   // unsigned(3)
            0xf4,                                   // primitive(20)
            0x04,                                   // unsigned(4)
            0x64,                                   // text(4)
            0x74,0x79,0x70,0x65,                    // "type"
            0x03,                                   // unsigned(3)
            0xa2,                                   // map(2)
            0x01,                                   // unsigned(1)
            0x6c,                                   // text(12)
            0x61,0x44,0x65,0x73,0x63,0x72,0x69,0x70,0x74,0x69,0x6f,0x6e, // "aDescription"
            0x02,                                   // unsigned(2)
            0x68,                                   // text(8)
            0x61,0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74 // "aComment"
    };

    StructDescriptorData sd;

    dots::from_cbor(inputData.data(), inputData.size(), sd);

    ASSERT_TRUE(sd.name.isValid());
    ASSERT_TRUE(sd.properties.isValid());
    ASSERT_TRUE(sd.documentation.isValid());

    auto properties = StructDescriptorData::name_p + StructDescriptorData::properties_p + StructDescriptorData::documentation_p;
    ASSERT_EQ(sd._validProperties(), properties);

    EXPECT_EQ(sd.name, "aName");
    ASSERT_EQ(sd.properties->size(), 2u);

    EXPECT_EQ(sd.properties->at(0).name, "aProperty");
    EXPECT_EQ(sd.properties->at(0).tag,  1u);
    EXPECT_EQ(sd.properties->at(0).type, "type");
    EXPECT_EQ(sd.properties->at(0).isKey, false);

    EXPECT_EQ(sd.properties->at(1).name, "anotherProperty");
    EXPECT_EQ(sd.properties->at(1).tag,  2u);
    EXPECT_EQ(sd.properties->at(1).type, "type");
    EXPECT_EQ(sd.properties->at(1).isKey, false);

    EXPECT_EQ(sd.documentation->description, "aDescription");
    EXPECT_EQ(sd.documentation->comment, "aComment");
}

TEST(TestCborSerialization, serializeTransportHeader)
{
    DotsTransportHeader header;
    header.nameSpace("SYS");
    header.destinationGroup("DotsMsgHello");
    header.payloadSize(16);
    auto& dots_header = header.dotsHeader();
    dots_header.typeName("DotsMsgHello");
    dots_header.attributes(1);
    dots_header.removeObj(false);
    dots_header.sentTime(dots::type::TimePoint(Duration{ 1 }));

    std::vector<uint8_t> expectData = {
            0xa4,                                // map(4)
            0x01,                             // unsigned(1)
            0x63,                             // text(3)
            0x53,0x59,0x53,                      // "SYS"
            0x02,                             // unsigned(2)
            0x6c,                             // text(12)
            0x44,0x6f,0x74,0x73,0x4d,0x73,0x67,0x48,0x65,0x6c,0x6c,0x6f,    // "DotsMsgHello"
            0x03,                             // unsigned(3)
            0xa4,                             // map(4)
            0x01,                          // unsigned(1)
            0x6c,                          // text(12)
            0x44,0x6f,0x74,0x73,0x4d,0x73,0x67,0x48,0x65,0x6c,0x6c,0x6f, // "DotsMsgHello"
            0x02,                          // unsigned(2)
            0xfb,0x3f,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,         // primitive(4607182418800017408)
            0x03,                          // unsigned(3)
            0x01,                          // unsigned(1)
            0x04,                          // unsigned(4)
            0xf4,                          // primitive(20)
            0x04,                             // unsigned(4)
            0x10,                             // unsigned(16)
    };

    EXPECT_THAT(dots::to_cbor(header), ElementsAreArray(expectData));

    /*
    //std::cout << "\n" << dots::to_cbor(header) << "\n";
    std::string cbor_data = dots::to_cbor(header);
    printf("CBOR: ");
    for (auto& d : cbor_data) {
        printf("%02X ", d&0xff);
    }
    printf("\n");
     */

}

TEST(TestCborSerialization, deserializeTransportHeader)
{
    std::vector<uint8_t> data = {
            0xa4,                                // map(4)
            0x01,                             // unsigned(1)
            0x63,                             // text(3)
            0x53, 0x59, 0x53,                      // "SYS"
            0x02,                             // unsigned(2)
            0x6c,                             // text(12)
            0x44, 0x6f, 0x74, 0x73, 0x4d, 0x73, 0x67, 0x48, 0x65, 0x6c, 0x6c, 0x6f,    // "DotsMsgHello"
            0x03,                             // unsigned(3)
            0xa4,                             // map(4)
            0x01,                          // unsigned(1)
            0x6c,                          // text(12)
            0x44, 0x6f, 0x74, 0x73, 0x4d, 0x73, 0x67, 0x48, 0x65, 0x6c, 0x6c, 0x6f, // "DotsMsgHello"
            0x02,                          // unsigned(2)
            0xfb, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,         // primitive(4607182418800017408)
            0x03,                          // unsigned(3)
            0x01,                          // unsigned(1)
            0x04,                          // unsigned(4)
            0xf4,                          // primitive(20)
            0x04,                             // unsigned(4)
            0x10,                             // unsigned(16)
    };

    auto transportHeader = dots::decodeInto_cbor<DotsTransportHeader>(data);

    ASSERT_TRUE(transportHeader.nameSpace.isValid());
    ASSERT_TRUE(transportHeader.destinationGroup.isValid());
    ASSERT_TRUE(transportHeader.payloadSize.isValid());
    ASSERT_TRUE(transportHeader.dotsHeader.isValid());

    EXPECT_EQ(transportHeader.nameSpace, "SYS");
    EXPECT_EQ(transportHeader.destinationGroup, "DotsMsgHello");
    EXPECT_EQ(transportHeader.payloadSize, 16u);

    auto& dots_header = *transportHeader.dotsHeader;

    ASSERT_TRUE(dots_header.typeName.isValid());
    ASSERT_TRUE(dots_header.attributes.isValid());
    ASSERT_TRUE(dots_header.removeObj.isValid());
    ASSERT_TRUE(dots_header.sentTime.isValid());
    ASSERT_FALSE(dots_header.sender.isValid());

    EXPECT_EQ(dots_header.typeName, "DotsMsgHello");
    EXPECT_EQ(dots_header.attributes, dots::type::PropertySet(1));
    EXPECT_EQ(dots_header.removeObj, false);
    EXPECT_EQ(dots_header.sentTime, dots::type::TimePoint(Duration{ 1 }));
}

TEST(TestCborSerialization, serializeTestStruct)
{
    DotsTestStruct testStruct;
    testStruct.indKeyfField(123);
    testStruct.enumField(DotsTestEnum::value3);

    std::vector<uint8_t> expectData = {
            0xa2,      // map(2)
            0x02,      // unsigned(2)
            0x18,0x7b, // unsigned(123)
            0x04,      // unsigned(4)
            0x03,      // unsigned(3)
    };

    EXPECT_THAT(dots::to_cbor(testStruct), ElementsAreArray(expectData));
}

TEST(TestCborSerialization, deserializeTestStruct)
{
    std::vector<uint8_t> data = {
            0xa2,      // map(2)
            0x02,      // unsigned(2)
            0x18,0x7b, // unsigned(123)
            0x04,      // unsigned(4)
            0x03,      // unsigned(3)
    };

    auto testStruct = dots::decodeInto_cbor<DotsTestStruct>(data);

    ASSERT_TRUE(testStruct.enumField.isValid());
    EXPECT_EQ(testStruct.enumField, DotsTestEnum::value3);

    ASSERT_TRUE(testStruct.indKeyfField.isValid());
    EXPECT_EQ(testStruct.indKeyfField, 123);

}

TEST(TestCborSerialization, deserializeCustomType)
{
    std::vector<uint8_t> customTypeDescriptorData = {
        0xa2, 0x02, 0x83, 0xa4, 0x02, 0x01, 0x01, 0x62, 0x49, 0x44, 0x03, 0xf5, 0x04, 0x65, 0x69, 0x6e, 0x74, 0x36,
        0x34, 0xa4, 0x02, 0x02, 0x01, 0x67, 0x43, 0x72, 0x65, 0x61, 0x74, 0x65, 0x64, 0x03, 0xf4, 0x04, 0x69, 0x74,
        0x69, 0x6d, 0x65, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0xa4, 0x02, 0x03, 0x01, 0x64, 0x54, 0x65, 0x78, 0x74, 0x03,
        0xf4, 0x04, 0x66, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x01, 0x6a, 0x43, 0x75, 0x73, 0x74, 0x6f, 0x6d, 0x54,
        0x79, 0x70, 0x65
    };

    std::vector<uint8_t> customTypeSerialized = {
            0xa3,                     // map(3)
            0x02,                  // unsigned(2)
            0xfb, 0x41,0xd6,0x3b,0xda,0x2e,0x6f,0xc9,0x40, // primitive(4744045065749383488)
            0x01,                  // unsigned(1)
            0x0a,                  // unsigned(10)
            0x03,                  // unsigned(3)
            0x64,                  // text(4)
            0x54,0x65,0x78,0x74         // "Text"
    };

    auto sd = dots::decodeInto_cbor<StructDescriptorData>(customTypeDescriptorData);
    auto descriptor = dots::type::StructDescriptor<>::createFromStructDescriptorData(sd);

    auto customObj = descriptor->New();

    dots::from_cbor(customTypeSerialized.data(), customTypeSerialized.size(), descriptor, customObj);

    std::set<std::string> expectProperties = { "ID", "Created", "Text" };

    for (auto& property : descriptor->propertyDescriptors())
    {
        auto iter = expectProperties.find(property.name().data());
        if (iter != expectProperties.end()) expectProperties.erase(iter);
        else FAIL() << "unexpected property " << property.name();

        if (property.name() == "Created") {
            EXPECT_EQ(property.valueDescriptor().name(), "timepoint");
            auto tp = reinterpret_cast<dots::type::TimePoint*>(property.address(customObj));
            EXPECT_EQ(tp->duration().toFractionalSeconds(), 1492084921.7466583);
        }
    }

    EXPECT_EQ(expectProperties.size(), 0u); // All expected properties are found


    auto json = dots::to_json(descriptor, customObj, PropertySet::All);

    std::cout << "Json: " << json << "\n";

    descriptor->Delete(customObj);


}

TEST(TestCborSerialization, dynamicEnum)
{

    EnumDescriptorData ed;
    {
        ed.name("MyEnum");
        auto &elements = ed.elements();

        EnumElementDescriptor element;
        element.name("myvalue1");
        element.enum_value(10);
        element.tag(1);
        elements.push_back(element);

        element.name = "myvalue2";
        element.enum_value = 11;
        element.tag = 2;
        elements.push_back(element);
    }

    StructDescriptorData sd;
    {
        sd.name("MyEnumStruct");
        auto& properties = sd.properties();

        StructPropertyData pd;
        pd.name("enum");
        pd.tag(1);
        pd.isKey(false);
        pd.type("MyEnum");

        properties.push_back(pd);
    }

    std::vector<uint8_t> myStructData = {
        0xa1, 0x01, 0x02
    };

    auto enumDescriptor = dots::type::EnumDescriptor<>::createFromEnumDescriptorData(ed);
    auto structDescriptor = dots::type::StructDescriptor<>::createFromStructDescriptorData(sd);

    //auto et = rttr::type::get_by_name("MyEnum");
    //auto enumt = et.get_enumeration();
    //auto _et = enumt.get_type();
    //auto ut = enumt.get_underlying_type();

    auto myStructObj = structDescriptor->New();

    dots::from_cbor(myStructData.data(), myStructData.size(), structDescriptor, myStructObj);

    ASSERT_TRUE(structDescriptor->propertyDescriptors().size() > 0);

    EXPECT_EQ(enumDescriptor->enumeratorFromValue(*Typeless::From(structDescriptor->propertyDescriptors()[0].address(myStructObj))).name(), "myvalue2");
    EXPECT_EQ(enumDescriptor->enumeratorFromValue(*Typeless::From(structDescriptor->propertyDescriptors()[0].address(myStructObj))).valueTypeless().to<int32_t>(), 11);

    EXPECT_THAT(dots::to_cbor({structDescriptor, myStructObj}, PropertySet::All), ElementsAreArray(myStructData));

    structDescriptor->Delete(myStructObj);
}

TEST(TestCborSerialization, serializeStringVector)
{
    DotsTestVectorStruct ts;
    auto& sl = ts.stringList();
    sl.push_back("String1");
    sl.push_back("String2");


    std::vector<uint8_t> expectData = {
            0xA1,                // map(1)
            0x03,                // unsigned(1)
            0x82,                // array(2)
            0x67,                // text(7)
            0x53,0x74,0x72,0x69,0x6E,0x67,0x31, // "String1"
            0x67,                // text(7)
            0x53,0x74,0x72,0x69,0x6E,0x67,0x32 // "String2"
    };

    EXPECT_THAT(dots::to_cbor(ts), ElementsAreArray(expectData));
}

TEST(TestCborSerialization, serializeDynamicRegisteredIntVector)
{
    // Register a new type StructDescriptorDataTest and deserialize into it
    //auto descriptorData = Registry::toStructDescriptorData(type::get<DotsTestVectorStruct>());

    DotsTestVectorStruct ts;

    auto& sl = ts.intList();
    sl.push_back(1);
    sl.push_back(2);

    std::vector<uint8_t> expectData = {
            0xA1,                // map(1)
            0x01,                // unsigned(1)
            0x82,                // array(2)
            0x01,                // int 1
            0x02                 // int 2
    };

    EXPECT_THAT(dots::to_cbor(ts, PropertySet::All), ElementsAreArray(expectData));
}

TEST(TestCborSerialization, deserializeDynamicRegisteredIntVector)
{
    DotsTestVectorStruct ts;

    std::vector<uint8_t> inputData = {
            0xA1,                // map(1)
            0x01,                // unsigned(1)
            0x82,                // array(2)
            0x01,                // int 1
            0x02                 // int 2
    };

    dots::from_cbor(inputData.data(), inputData.size(), &ts._descriptor(), &ts);

    ASSERT_TRUE(ts.intList.isValid());
    ASSERT_EQ(ts.intList->size(), 2u);
    EXPECT_EQ((*ts.intList)[0],1);
    EXPECT_EQ((*ts.intList)[1], 2);
}

TEST(TestCborSerialization, serializeDynamicRegisteredStringVector)
{
    DotsTestVectorStruct ts;

    auto& sl = ts.stringList();
    sl.push_back("String1");
    sl.push_back("String2");

    std::vector<uint8_t> expectData = {
            0xA1,                // map(1)
            0x03,                // unsigned(1)
            0x82,                // array(2)
            0x67,                // text(7)
            0x53,0x74,0x72,0x69,0x6E,0x67,0x31, // "String1"
            0x67,                // text(7)
            0x53,0x74,0x72,0x69,0x6E,0x67,0x32 // "String2"
    };

    EXPECT_THAT(dots::to_cbor(ts, PropertySet::All), ElementsAreArray(expectData));
}

TEST(TestCborSerialization, deserializeDynamicRegisteredStringVector)
{
    DotsTestVectorStruct ts;

    std::vector<uint8_t> inputData = {
            0xA1,                // map(1)
            0x03,                // unsigned(1)
            0x82,                // array(2)
            0x67,                // text(7)
            0x53,0x74,0x72,0x69,0x6E,0x67,0x31, // "String1"
            0x67,                // text(7)
            0x53,0x74,0x72,0x69,0x6E,0x67,0x32 // "String2"
    };

    dots::from_cbor(inputData.data(), inputData.size(), ts);

    ASSERT_TRUE(ts.stringList.isValid());
    ASSERT_EQ(ts.stringList->size(), 2u);
    EXPECT_EQ((*ts.stringList)[0], "String1");
    EXPECT_EQ((*ts.stringList)[1], "String2");

}

TEST(TestCborSerialization, serializeUuid)
{
    uint8_t myUuid[] = {0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0a ,0x0b, 0x0c,0x0d,0x0e,0x0f };

    DotsTestStruct ts;
    ts.indKeyfField(41);
    ts.uuid(myUuid);

    std::vector<uint8_t> expectData = {
            0xa2,                                   // map(2)
            0x02,                                   // unsigned(2)
            0x18,
            0x29,
            0x07,
            0x50,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };

    EXPECT_THAT(dots::to_cbor(ts), ElementsAreArray(expectData));
}

TEST(TestCborSerialization, deserializeUuid)
{
    std::array<uint8_t, 16> expectedUuid = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    DotsTestStruct ts;

    std::vector<uint8_t> inputData = {
            0xa2,                                   // map(2)
            0x02,                                   // unsigned(2)
            0x18,
            0x29,
            0x07,
            0x50,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };

    dots::from_cbor(inputData.data(), inputData.size(), ts);

    ASSERT_TRUE(ts.indKeyfField.isValid());
    ASSERT_TRUE(ts.uuid.isValid());
    ASSERT_EQ(ts.uuid->data(), expectedUuid);
}

TEST(TestCborSerialization, serializeIntegers)
{
    DotsTestStruct testStruct;
    testStruct.indKeyfField(123);

    // Encoded as 1 Byte
    testStruct.uint64Field = 42;
    testStruct.int64Field = -42;

    std::vector<uint8_t> expectData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x18,0x2a, // unsigned(42)
        0x09,      // unsigned(9)
        0x38,0x29  // negative(41)
    };

    EXPECT_THAT(dots::to_cbor(testStruct), ElementsAreArray(expectData));

    // Encoded as 2 Byte
    testStruct.uint64Field = 311;
    testStruct.int64Field = -311;

    expectData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x19,0x01,0x37, // unsigned(311)
        0x09,      // unsigned(9)
        0x39,0x01,0x36 // negative(310)
    };

    EXPECT_THAT(dots::to_cbor(testStruct), ElementsAreArray(expectData));

    // Encoded as 4 Byte
    testStruct.uint64Field = 90001;
    testStruct.int64Field = -90001;

    expectData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x1a,0x00,0x01,0x5f,0x91, // unsigned(90001)
        0x09,      // unsigned(9)
        0x3a,0x00,0x01,0x5f,0x90, // negative(90000)
    };

    EXPECT_THAT(dots::to_cbor(testStruct), ElementsAreArray(expectData));

    // Encoded as 4 Byte
    testStruct.uint64Field = 4294967295;
    testStruct.int64Field = -2147483648L;

    expectData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x1a,0xff,0xff,0xff,0xff, // unsigned(4294967295)
        0x09,     // unsigned(9)
        0x3a,0x7f,0xff,0xff,0xff, // negative(2147483647)
    };

    EXPECT_THAT(dots::to_cbor(testStruct), ElementsAreArray(expectData));

    // Encoded as 8 Byte
    testStruct.uint64Field = 18446744073709551615ULL;
    testStruct.int64Field = -9223372036854775807LL;

    expectData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x1b,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // unsigned(18446744073709551615)
        0x09,      // unsigned(9)
        0x3b,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xfe, // negative(9223372036854775806)
    };

    EXPECT_THAT(dots::to_cbor(testStruct), ElementsAreArray(expectData));
}

TEST(TestCborSerialization, deserializeIntegers)
{
    DotsTestStruct testStruct;

    // Encoded as 1 Byte
    std::vector<uint8_t> inputData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x18,0x2a, // unsigned(42)
        0x09,      // unsigned(9)
        0x38,0x29  // negative(41)
    };

    dots::from_cbor(inputData.data(), inputData.size(), testStruct);
    ASSERT_TRUE(testStruct.uint64Field.isValid() && testStruct.indKeyfField.isValid());
    EXPECT_EQ(*testStruct.uint64Field, 42);
    EXPECT_EQ(*testStruct.int64Field, -42);


    // Encoded as 2 Byte
    inputData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x19,0x01,0x37, // unsigned(311)
        0x09,      // unsigned(9)
        0x39,0x01,0x36 // negative(310)
    };

    dots::from_cbor(inputData.data(), inputData.size(), testStruct);
    EXPECT_EQ(*testStruct.uint64Field, 311);
    EXPECT_EQ(*testStruct.int64Field, -311);


    // Encoded as 4 Byte
    inputData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x1a,0x00,0x01,0x5f,0x91, // unsigned(90001)
        0x09,      // unsigned(9)
        0x3a,0x00,0x01,0x5f,0x90, // negative(90000)
    };

    dots::from_cbor(inputData.data(), inputData.size(), testStruct);
    EXPECT_EQ(*testStruct.uint64Field, 90001);
    EXPECT_EQ(*testStruct.int64Field, -90001);

    // Encoded as 4 Byte
    inputData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x1a,0xff,0xff,0xff,0xff, // unsigned(4294967295)
        0x09,     // unsigned(9)
        0x3a,0x7f,0xff,0xff,0xff, // negative(2147483647)
    };

    dots::from_cbor(inputData.data(), inputData.size(), testStruct);
    EXPECT_EQ(*testStruct.uint64Field, 4294967295);
    EXPECT_EQ(*testStruct.int64Field, -2147483648L);

    // Encoded as 8 Byte
    inputData = {
        0xa3,      // map(2)
        0x02,      // unsigned(2)
        0x18,0x7b, // unsigned(123)
        0x08,      // unsigned(8)
        0x1b,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // unsigned(18446744073709551615)
        0x09,      // unsigned(9)
        0x3b,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xfe, // negative(9223372036854775806)
    };

    dots::from_cbor(inputData.data(), inputData.size(), testStruct);
    EXPECT_EQ(*testStruct.uint64Field, 18446744073709551615ULL);
    EXPECT_EQ(*testStruct.int64Field, -9223372036854775807LL);
}