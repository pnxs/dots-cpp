#include <dots/testing/gtest/gtest.h>
#include <dots/io/serialization/StringSerializer.h>
#include <io/serialization/data/string_serialization_data.h>

struct TestStringSerializer : ::testing::Test, TestSerializationInput
{
protected:

    using data_t = dots::io::StringSerializer<>::data_t;
};

TEST_F(TestStringSerializer, serialize_TypedArgument)
{
    EXPECT_EQ(dots::io::to_string(BoolFalse), data_t(STRING_BOOL_FALSE));
    EXPECT_EQ(dots::io::to_string(BoolTrue), data_t(STRING_BOOL_TRUE));

    EXPECT_EQ(dots::io::to_string(Int8Zero), data_t(STRING_INT8_ZERO));
    EXPECT_EQ(dots::io::to_string(Int8Positive),  data_t(STRING_INT8_POSITIVE));
    EXPECT_EQ(dots::io::to_string(Int8Negative), data_t(STRING_INT8_NEGATIVE));

    EXPECT_EQ(dots::io::to_string(Uint8Zero), data_t(STRING_UINT8_ZERO));
    EXPECT_EQ(dots::io::to_string(Uint8Positive1), data_t(STRING_UINT8_POSITIVE_1));
    EXPECT_EQ(dots::io::to_string(Uint8Positive2), data_t(STRING_UINT8_POSITIVE_2));

    EXPECT_EQ(dots::io::to_string(Int16Zero), data_t(STRING_INT16_ZERO));
    EXPECT_EQ(dots::io::to_string(Int16Positive), data_t(STRING_INT16_POSITIVE));
    EXPECT_EQ(dots::io::to_string(Int16Negative), data_t(STRING_INT16_NEGATIVE));

    EXPECT_EQ(dots::io::to_string(Uint16Zero), data_t(STRING_UINT16_ZERO));
    EXPECT_EQ(dots::io::to_string(Uint16Positive1), data_t(STRING_UINT16_POSITIVE_1));
    EXPECT_EQ(dots::io::to_string(Uint16Positive2), data_t(STRING_UINT16_POSITIVE_2));

    EXPECT_EQ(dots::io::to_string(Int32Zero), data_t(STRING_INT32_ZERO));
    EXPECT_EQ(dots::io::to_string(Int32Positive), data_t(STRING_INT32_POSITIVE));
    EXPECT_EQ(dots::io::to_string(Int32Negative), data_t(STRING_INT32_NEGATIVE));

    EXPECT_EQ(dots::io::to_string(Uint32Zero), data_t(STRING_UINT32_ZERO));
    EXPECT_EQ(dots::io::to_string(Uint32Positive1), data_t(STRING_UINT32_POSITIVE_1));
    EXPECT_EQ(dots::io::to_string(Uint32Positive2), data_t(STRING_UINT32_POSITIVE_2));

    EXPECT_EQ(dots::io::to_string(Int64Zero), data_t(STRING_INT64_ZERO));
    EXPECT_EQ(dots::io::to_string(Int64Positive), data_t(STRING_INT64_POSITIVE));
    EXPECT_EQ(dots::io::to_string(Int64Negative), data_t(STRING_INT64_NEGATIVE));

    EXPECT_EQ(dots::io::to_string(Uint64Zero), data_t(STRING_UINT64_ZERO));
    EXPECT_EQ(dots::io::to_string(Uint64Positive1), data_t(STRING_UINT64_POSITIVE_1));
    EXPECT_EQ(dots::io::to_string(Uint64Positive2), data_t(STRING_UINT64_POSITIVE_2));

    EXPECT_EQ(dots::io::to_string(Float32Zero), data_t(STRING_FLOAT32_ZERO));
    EXPECT_EQ(dots::io::to_string(Float32Positive), data_t(STRING_FLOAT32_POSITIVE));
    EXPECT_EQ(dots::io::to_string(Float32Negative), data_t(STRING_FLOAT32_NEGATIVE));

    EXPECT_EQ(dots::io::to_string(Float64Zero), data_t(STRING_FLOAT64_ZERO));
    EXPECT_EQ(dots::io::to_string(Float64Positive), data_t(STRING_FLOAT64_POSITIVE));
    EXPECT_EQ(dots::io::to_string(Float64Negative), data_t(STRING_FLOAT64_NEGATIVE));

    EXPECT_EQ(dots::io::to_string(PropertySetNone), data_t(STRING_PROPERTY_SET_NONE));
    EXPECT_EQ(dots::io::to_string(PropertySetAll), data_t(STRING_PROPERTY_SET_ALL));
    EXPECT_EQ(dots::io::to_string(PropertySetMixed1), data_t(STRING_PROPERTY_SET_MIXED_1));

    EXPECT_EQ(dots::io::to_string(TimePoint1), data_t(STRING_TIME_POINT_1));
    EXPECT_EQ(dots::io::to_string(SteadyTimePoint1), data_t(STRING_STEADY_TIME_POINT_1));

    EXPECT_EQ(dots::io::to_string(Duration1), data_t(STRING_DURATION_1));
    EXPECT_EQ(dots::io::to_string(Duration2), data_t(STRING_DURATION_2));

    EXPECT_EQ(dots::io::to_string(Uuid1), data_t(STRING_UUID_1));

    EXPECT_EQ(dots::io::to_string(String1), data_t(STRING_STRING_1));
    EXPECT_EQ(dots::io::to_string(String2), data_t(STRING_STRING_2));
    EXPECT_EQ(dots::io::to_string(String3), data_t(STRING_STRING_3));
    EXPECT_EQ(dots::io::to_string(String4), data_t(STRING_STRING_4));
    EXPECT_EQ(dots::io::to_string(String5), data_t(STRING_STRING_5));

    EXPECT_EQ(dots::io::to_string(SerializationEnum1), data_t(STRING_TEST_ENUM_1_COMPACT));
}

TEST_F(TestStringSerializer, deserialize_TypedArgument)
{
    EXPECT_EQ(dots::io::from_string<dots::bool_t>(data_t(STRING_BOOL_FALSE)), BoolFalse);
    EXPECT_EQ(dots::io::from_string<dots::bool_t>(data_t(STRING_BOOL_TRUE)), BoolTrue);

    EXPECT_EQ(dots::io::from_string<dots::int8_t>(data_t(STRING_INT8_ZERO)), Int8Zero);
    EXPECT_EQ(dots::io::from_string<dots::int8_t>(data_t(STRING_INT8_POSITIVE)), Int8Positive);
    EXPECT_EQ(dots::io::from_string<dots::int8_t>(data_t(STRING_INT8_NEGATIVE)), Int8Negative);

    EXPECT_EQ(dots::io::from_string<dots::uint8_t>(data_t(STRING_UINT8_ZERO)), Uint8Zero);
    EXPECT_EQ(dots::io::from_string<dots::uint8_t>(data_t(STRING_UINT8_POSITIVE_1)), Uint8Positive1);
    EXPECT_EQ(dots::io::from_string<dots::uint8_t>(data_t(STRING_UINT8_POSITIVE_2)), Uint8Positive2);

    EXPECT_EQ(dots::io::from_string<dots::int16_t>(data_t(STRING_INT16_ZERO)), Int16Zero);
    EXPECT_EQ(dots::io::from_string<dots::int16_t>(data_t(STRING_INT16_POSITIVE)), Int16Positive);
    EXPECT_EQ(dots::io::from_string<dots::int16_t>(data_t(STRING_INT16_NEGATIVE)), Int16Negative);

    EXPECT_EQ(dots::io::from_string<dots::uint16_t>(data_t(STRING_UINT16_ZERO)), Uint16Zero);
    EXPECT_EQ(dots::io::from_string<dots::uint16_t>(data_t(STRING_UINT16_POSITIVE_1)), Uint16Positive1);
    EXPECT_EQ(dots::io::from_string<dots::uint16_t>(data_t(STRING_UINT16_POSITIVE_2)), Uint16Positive2);

    EXPECT_EQ(dots::io::from_string<dots::int32_t>(data_t(STRING_INT32_ZERO)), Int32Zero);
    EXPECT_EQ(dots::io::from_string<dots::int32_t>(data_t(STRING_INT32_POSITIVE)), Int32Positive);
    EXPECT_EQ(dots::io::from_string<dots::int32_t>(data_t(STRING_INT32_NEGATIVE)), Int32Negative);

    EXPECT_EQ(dots::io::from_string<dots::uint32_t>(data_t(STRING_UINT32_ZERO)), Uint32Zero);
    EXPECT_EQ(dots::io::from_string<dots::uint32_t>(data_t(STRING_UINT32_POSITIVE_1)), Uint32Positive1);
    EXPECT_EQ(dots::io::from_string<dots::uint32_t>(data_t(STRING_UINT32_POSITIVE_2)), Uint32Positive2);

    EXPECT_EQ(dots::io::from_string<dots::int64_t>(data_t(STRING_INT64_ZERO)), Int64Zero);
    EXPECT_EQ(dots::io::from_string<dots::int64_t>(data_t(STRING_INT64_POSITIVE)), Int64Positive);
    EXPECT_EQ(dots::io::from_string<dots::int64_t>(data_t(STRING_INT64_NEGATIVE)), Int64Negative);

    EXPECT_EQ(dots::io::from_string<dots::uint64_t>(data_t(STRING_UINT64_ZERO)), Uint64Zero);
    EXPECT_EQ(dots::io::from_string<dots::uint64_t>(data_t(STRING_UINT64_POSITIVE_1)), Uint64Positive1);
    EXPECT_EQ(dots::io::from_string<dots::uint64_t>(data_t(STRING_UINT64_POSITIVE_2)), Uint64Positive2);

    EXPECT_EQ(dots::io::from_string<dots::float32_t>(data_t(STRING_FLOAT32_ZERO)), Float32Zero);
    EXPECT_EQ(dots::io::from_string<dots::float32_t>(data_t(STRING_FLOAT32_POSITIVE)), Float32Positive);
    EXPECT_EQ(dots::io::from_string<dots::float32_t>(data_t(STRING_FLOAT32_NEGATIVE)), Float32Negative);

    EXPECT_EQ(dots::io::from_string<dots::float64_t>(data_t(STRING_FLOAT64_ZERO)), Float64Zero);
    EXPECT_EQ(dots::io::from_string<dots::float64_t>(data_t(STRING_FLOAT64_POSITIVE)), Float64Positive);
    EXPECT_EQ(dots::io::from_string<dots::float64_t>(data_t(STRING_FLOAT64_NEGATIVE)), Float64Negative);

    EXPECT_EQ(dots::io::from_string<dots::property_set_t>(data_t(STRING_PROPERTY_SET_NONE)), PropertySetNone);
    EXPECT_EQ(dots::io::from_string<dots::property_set_t>(data_t(STRING_PROPERTY_SET_ALL)), PropertySetAll);
    EXPECT_EQ(dots::io::from_string<dots::property_set_t>(data_t(STRING_PROPERTY_SET_MIXED_1)), PropertySetMixed1);

    EXPECT_EQ(dots::io::from_string<dots::timepoint_t>(data_t(STRING_TIME_POINT_1)), TimePoint1);
    EXPECT_EQ(dots::io::from_string<dots::steady_timepoint_t>(data_t(STRING_STEADY_TIME_POINT_1)), SteadyTimePoint1);

    EXPECT_EQ(dots::io::from_string<dots::duration_t>(data_t(STRING_DURATION_1)), Duration1);
    EXPECT_EQ(dots::io::from_string<dots::duration_t>(data_t(STRING_DURATION_2)), Duration2);

    EXPECT_EQ(dots::io::from_string<dots::uuid_t>(data_t(STRING_UUID_1)), Uuid1);

    EXPECT_EQ(dots::io::from_string<dots::string_t>(data_t(STRING_STRING_1)), String1);
    EXPECT_EQ(dots::io::from_string<dots::string_t>(data_t(STRING_STRING_2)), String2);
    EXPECT_EQ(dots::io::from_string<dots::string_t>(data_t(STRING_STRING_3)), String3);
    EXPECT_EQ(dots::io::from_string<dots::string_t>(data_t(STRING_STRING_4)), String4);
    EXPECT_EQ(dots::io::from_string<dots::string_t>(data_t(STRING_STRING_5)), String5);

    EXPECT_EQ(dots::io::from_string<SerializationEnum>(data_t(STRING_TEST_ENUM_1)), SerializationEnum1);
    EXPECT_EQ(dots::io::from_string<SerializationEnum>(data_t(STRING_TEST_ENUM_1_COMPACT)), SerializationEnum1);
}

TEST_F(TestStringSerializer, serialize_PropertyArgument)
{
    EXPECT_EQ(dots::io::to_string(SerializationStructSimple1.int32Property), data_t(".int32Property = " STRING_INT32_POSITIVE));
    EXPECT_EQ(dots::io::to_string(SerializationStructSimple1.stringProperty), data_t(".stringProperty = " STRING_STRING_1));
    EXPECT_EQ(dots::io::to_string(SerializationStructSimple1.float32Property), data_t(".float32Property = " STRING_FLOAT32_POSITIVE));
}

TEST_F(TestStringSerializer, deserialize_PropertyArgument)
{
    SerializationStructSimple serializationStructSimple;
    dots::io::from_string(data_t(STRING_INT32_POSITIVE), serializationStructSimple.int32Property);
    dots::io::from_string(data_t(STRING_STRING_1), serializationStructSimple.stringProperty);
    dots::io::from_string(data_t(STRING_FLOAT32_POSITIVE), serializationStructSimple.float32Property);

    EXPECT_EQ(serializationStructSimple.int32Property, SerializationStructSimple1.int32Property);
    EXPECT_EQ(serializationStructSimple.stringProperty, SerializationStructSimple1.stringProperty);
    EXPECT_EQ(serializationStructSimple.float32Property, SerializationStructSimple1.float32Property);
}

TEST_F(TestStringSerializer, serialize_VectorArgument)
{
    EXPECT_EQ(dots::io::to_string(VectorBool), data_t("{ " STRING_BOOL_TRUE ", " STRING_BOOL_FALSE ", " STRING_BOOL_FALSE " }"));
    EXPECT_EQ(dots::io::to_string(VectorFloat), data_t("{ " STRING_FLOAT32_POSITIVE", " STRING_FLOAT32_NEGATIVE " }"));
    EXPECT_EQ(dots::io::to_string(VectorStructSimple), data_t("{ { .int32Property = " STRING_INT32_POSITIVE " }, { .boolProperty = " STRING_BOOL_FALSE " } }"));
}

TEST_F(TestStringSerializer, deserialize_VectorArgument)
{
    dots::vector_t<dots::bool_t> vectorBool;
    dots::io::from_string(data_t("{ " STRING_BOOL_TRUE ", " STRING_BOOL_FALSE ", " STRING_BOOL_FALSE " }"), vectorBool);
    EXPECT_EQ(vectorBool, VectorBool);

    dots::vector_t<dots::float32_t> vectorFloat32;
    dots::io::from_string(data_t("{ " STRING_FLOAT32_POSITIVE", " STRING_FLOAT32_NEGATIVE " }"), vectorFloat32);
    EXPECT_EQ(vectorFloat32, VectorFloat);

    dots::vector_t<SerializationStructSimple> vectorStructSimple;
    dots::io::from_string(data_t("{ SerializationStructSimple{ .int32Property = " STRING_INT32_POSITIVE " }, SerializationStructSimple{ .boolProperty = " STRING_BOOL_FALSE " } }"), vectorStructSimple);
    EXPECT_EQ(vectorStructSimple, VectorStructSimple);
}

TEST_F(TestStringSerializer, serialize_SimpleStructArgument)
{
    data_t expectedValid = "SerializationStructSimple{ .int32Property = " STRING_INT32_POSITIVE ", .stringProperty = " STRING_STRING_1 ", .float32Property = " STRING_FLOAT32_POSITIVE " }";
    EXPECT_EQ(dots::io::to_string(SerializationStructSimple1), expectedValid);

    data_t expectedAll = "SerializationStructSimple{ .int32Property = " STRING_INT32_POSITIVE ", .stringProperty = " STRING_STRING_1 ", .boolProperty = <invalid>, .float32Property = " STRING_FLOAT32_POSITIVE " }";
    EXPECT_EQ(dots::io::to_string(SerializationStructSimple1, dots::property_set_t::All), expectedAll);

    data_t expectedSpecific = "SerializationStructSimple{ .boolProperty = <invalid>, .float32Property = " STRING_FLOAT32_POSITIVE " }";
    EXPECT_EQ(dots::io::to_string(SerializationStructSimple1, SerializationStructSimple::boolProperty_p + SerializationStructSimple::float32Property_p), expectedSpecific);
}

TEST_F(TestStringSerializer, deserialize_SimpleStructArgument)
{
    SerializationStructSimple serializationStructSimple1;
    dots::io::from_string(data_t{ "SerializationStructSimple{ .int32Property = " STRING_INT32_POSITIVE ", .stringProperty = " STRING_STRING_1 ", .float32Property = " STRING_FLOAT32_POSITIVE " }" }, serializationStructSimple1);
    EXPECT_EQ(serializationStructSimple1, SerializationStructSimple1);

    SerializationStructSimple serializationStructSimple2;
    dots::io::from_string(data_t{ "SerializationStructSimple{ .int32Property = " STRING_INT32_POSITIVE ", .stringProperty = " STRING_STRING_1 ", .boolProperty = <invalid>, .float32Property = " STRING_FLOAT32_POSITIVE " }" }, serializationStructSimple2);
    EXPECT_EQ(serializationStructSimple2, SerializationStructSimple1);

    SerializationStructSimple serializationStructSimple3;
    dots::io::from_string(data_t{ "SerializationStructSimple{ .boolProperty = <invalid>, .float32Property = " STRING_FLOAT32_POSITIVE " }" }, serializationStructSimple3);
    EXPECT_TRUE(serializationStructSimple3._equal(SerializationStructSimple1, SerializationStructSimple::boolProperty_p + SerializationStructSimple::float32Property_p));
}

TEST_F(TestStringSerializer, serialize_ComplexStructArgument)
{
    data_t expectedValid1 = "SerializationStructComplex{ .enumProperty = " STRING_TEST_ENUM_1_COMPACT ", .float64Property = " STRING_FLOAT64_NEGATIVE ", .timepointProperty = " STRING_TIME_POINT_1 ", .structSimpleProperty = { .boolProperty = " STRING_BOOL_FALSE " } }";
    EXPECT_EQ(dots::io::to_string(SerializationStructComplex1), expectedValid1);

    data_t expectedValid2 = "SerializationStructComplex{ .propertySetProperty = " STRING_PROPERTY_SET_MIXED_1 ", .durationVectorProperty = { " STRING_DURATION_1 ", " STRING_DURATION_2 " }, .uuidProperty = " STRING_UUID_1 " }";
    EXPECT_EQ(dots::io::to_string(SerializationStructComplex2), expectedValid2);

    data_t expectedSpecific1 = "SerializationStructComplex{ .timepointProperty = " STRING_TIME_POINT_1 ", .propertySetProperty = <invalid> }";
    EXPECT_EQ(dots::io::to_string(SerializationStructComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p), expectedSpecific1);

    data_t expectedSpecific2 = "SerializationStructComplex{ .enumProperty = <invalid>, .durationVectorProperty = { " STRING_DURATION_1 ", " STRING_DURATION_2 " } }";
    EXPECT_EQ(dots::io::to_string(SerializationStructComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p), expectedSpecific2);
}

TEST_F(TestStringSerializer, deserialize_ComplexStructArgument)
{
    SerializationStructComplex serializationStructComplex1;
    dots::io::from_string(data_t{ "SerializationStructComplex{ .enumProperty = " STRING_TEST_ENUM_1_COMPACT ", .float64Property = " STRING_FLOAT64_NEGATIVE ", .timepointProperty = " STRING_TIME_POINT_1 ", .structSimpleProperty = SerializationStructSimple{ .boolProperty = " STRING_BOOL_FALSE " } }" }, serializationStructComplex1);
    EXPECT_EQ(serializationStructComplex1, SerializationStructComplex1);

    SerializationStructComplex serializationStructComplex2;
    dots::io::from_string(data_t{ "SerializationStructComplex{ .propertySetProperty = " STRING_PROPERTY_SET_MIXED_1 ", .durationVectorProperty = { " STRING_DURATION_1 ", " STRING_DURATION_2 " }, .uuidProperty = " STRING_UUID_1 " }" }, serializationStructComplex2);
    EXPECT_EQ(serializationStructComplex2, SerializationStructComplex2);

    SerializationStructComplex serializationStructComplex3;
    dots::io::from_string(data_t{ "SerializationStructComplex{ .timepointProperty = " STRING_TIME_POINT_1 ", .propertySetProperty = <invalid> }" }, serializationStructComplex3);
    EXPECT_TRUE(serializationStructComplex3._equal(SerializationStructComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p));

    SerializationStructComplex serializationStructComplex4;
    dots::io::from_string(data_t{ "SerializationStructComplex{ .enumProperty = <invalid>, .durationVectorProperty = { " STRING_DURATION_1 ", " STRING_DURATION_2 " } }" }, serializationStructComplex4);
    EXPECT_TRUE(serializationStructComplex4._equal(SerializationStructComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p));
}

TEST_F(TestStringSerializer, serialize_WriteTupleToContinuousInternalBuffer)
{
    dots::io::StringSerializer<> sut;

    EXPECT_EQ(sut.serializeTupleBegin(), sizeof("{ ") - 1);
    {
        EXPECT_EQ(sut.serialize(String1), sizeof(STRING_STRING_1) - 1);
        EXPECT_EQ(sut.serialize(SerializationEnum1), sizeof(", ") + sizeof(STRING_TEST_ENUM_1_COMPACT) - 2);
        EXPECT_EQ(sut.serialize(VectorBool), sizeof(", ") + sizeof("{ ") + sizeof(STRING_BOOL_TRUE) + sizeof(", ") + sizeof(STRING_BOOL_FALSE) + sizeof(", ") + sizeof(STRING_BOOL_FALSE) + sizeof(" }") - 8);
        EXPECT_EQ(sut.serialize(SerializationStructSimple1), sizeof(", ") + sizeof("{ .int32Property = ") + sizeof(STRING_INT32_POSITIVE) + sizeof(", .stringProperty = ") + sizeof(STRING_STRING_1) + sizeof(", .float32Property = " ) + sizeof(STRING_FLOAT32_POSITIVE) + sizeof(" }") - 8);
    }
    EXPECT_EQ(sut.serializeTupleEnd(), sizeof(" }") - 1);

    data_t output{
        "{ "
            STRING_STRING_1 ", "
            STRING_TEST_ENUM_1_COMPACT ", "
            "{ " STRING_BOOL_TRUE ", " STRING_BOOL_FALSE ", " STRING_BOOL_FALSE " }" ", "
            "{ .int32Property = " STRING_INT32_POSITIVE ", .stringProperty = " STRING_STRING_1 ", .float32Property = " STRING_FLOAT32_POSITIVE " }"
        " }"
    };
    EXPECT_EQ(sut.output(), output);
}

TEST_F(TestStringSerializer, deserialize_ReadTupleFromContinuousExternalBuffer)
{
    dots::io::StringSerializer<> sut;
    data_t input{
        "{ "
            STRING_STRING_1 ", "
            STRING_TEST_ENUM_1_COMPACT ", "
            "{ " STRING_BOOL_TRUE ", " STRING_BOOL_FALSE ", " STRING_BOOL_FALSE " }" ", "
            "SerializationStructSimple{ .int32Property = " STRING_INT32_POSITIVE ", .stringProperty = " STRING_STRING_1 ", .float32Property = " STRING_FLOAT32_POSITIVE " }"
        " }"
    };
    sut.setInput(input);

    EXPECT_TRUE(sut.inputAvailable());

    EXPECT_EQ(sut.deserializeTupleBegin(), sizeof("{") - 1);
    {
        EXPECT_EQ(sut.deserialize<std::string>(), String1);
        EXPECT_EQ(sut.lastDeserializeSize(), sizeof(" ") + sizeof(STRING_STRING_1) - 2);

        EXPECT_EQ(sut.deserialize<SerializationEnum>(), SerializationEnum1);
        EXPECT_EQ(sut.lastDeserializeSize(), sizeof(", ") + sizeof(STRING_TEST_ENUM_1_COMPACT) - 2);

        EXPECT_EQ(sut.deserialize<dots::vector_t<dots::bool_t>>(), VectorBool);
        EXPECT_EQ(sut.lastDeserializeSize(), sizeof(", ") + sizeof("{ ") + sizeof(STRING_BOOL_TRUE) + sizeof(", ") + sizeof(STRING_BOOL_FALSE) + sizeof(", ") + sizeof(STRING_BOOL_FALSE) + sizeof(" }") - 8);

        EXPECT_EQ(sut.deserialize<SerializationStructSimple>(), SerializationStructSimple1);
        EXPECT_EQ(sut.lastDeserializeSize(), sizeof(", ") + sizeof("SerializationStructSimple{ .int32Property = ") + sizeof(STRING_INT32_POSITIVE) + sizeof(", .stringProperty = ") + sizeof(STRING_STRING_1) + sizeof(", .float32Property = " ) + sizeof(STRING_FLOAT32_POSITIVE) + sizeof(" }") - 8);
    }
    EXPECT_EQ(sut.deserializeTupleEnd(), sizeof(" }") - 1);

    EXPECT_FALSE(sut.inputAvailable());
}

TEST_F(TestStringSerializer, serialize_Style)
{
    data_t expectedMinimal = "{.enumProperty=" STRING_TEST_ENUM_1_COMPACT ",.float64Property=" STRING_FLOAT64_NEGATIVE ",.timepointProperty=" STRING_TIME_POINT_1 ",.structSimpleProperty={.boolProperty=" STRING_BOOL_FALSE "}}";
    EXPECT_EQ(dots::io::to_string(SerializationStructComplex1, { dots::io::StringSerializerOptions::Minimal }), expectedMinimal);

    data_t expectedCompact = "SerializationStructComplex{ .enumProperty = " STRING_TEST_ENUM_1_COMPACT ", .float64Property = " STRING_FLOAT64_NEGATIVE ", .timepointProperty = " STRING_TIME_POINT_1 ", .structSimpleProperty = { .boolProperty = " STRING_BOOL_FALSE " } }";
    EXPECT_EQ(dots::io::to_string(SerializationStructComplex1, { dots::io::StringSerializerOptions::Compact }), expectedCompact);

    data_t expectedSingleLine = "SerializationStructComplex{ .enumProperty = " STRING_TEST_ENUM_1 ", .float64Property = " STRING_FLOAT64_NEGATIVE ", .timepointProperty = " STRING_TIME_POINT_1 ", .structSimpleProperty = SerializationStructSimple{ .boolProperty = " STRING_BOOL_FALSE " } }";
    EXPECT_EQ(dots::io::to_string(SerializationStructComplex1, { dots::io::StringSerializerOptions::SingleLine }), expectedSingleLine);

    data_t expectedMultiLine{
        "SerializationStructComplex{\n"
        "    .enumProperty = " STRING_TEST_ENUM_1 ",\n"
        "    .float64Property = " STRING_FLOAT64_NEGATIVE ",\n"
        "    .timepointProperty = " STRING_TIME_POINT_1 ",\n"
        "    .structSimpleProperty = SerializationStructSimple{\n"
        "        .boolProperty = " STRING_BOOL_FALSE "\n"
        "    }\n"
        "}"
    };
    EXPECT_EQ(dots::io::to_string(SerializationStructComplex1, { dots::io::StringSerializerOptions::MultiLine }), expectedMultiLine);
}

TEST_F(TestStringSerializer, deserialize_Policy)
{
    SerializationStructComplex serializationStruct1;
    dots::io::from_string(data_t{ "{ .enumProperty = " STRING_TEST_ENUM_1 " }" }, serializationStruct1);
    EXPECT_TRUE(serializationStruct1._equal(SerializationStructComplex1, SerializationStructComplex::enumProperty_p));

    SerializationStructComplex serializationStruct2;
    dots::io::from_string(data_t{ "{ .enumProperty = " STRING_TEST_ENUM_1_COMPACT " }" }, serializationStruct2);
    EXPECT_TRUE(serializationStruct2._equal(SerializationStructComplex1, SerializationStructComplex::enumProperty_p));

    SerializationStructComplex serializationStruct3;
    EXPECT_THROW(dots::io::from_string(data_t{ "{ .enumProperty = " STRING_TEST_ENUM_1 " }" }, serializationStruct3, { dots::io::StringSerializerOptions::SingleLine, dots::io::StringSerializerOptions::Strict }), std::runtime_error);
    EXPECT_THROW(dots::io::from_string(data_t{ "SerializationStructComplex{ .enumProperty = " STRING_TEST_ENUM_1_COMPACT " }" }, serializationStruct3, { dots::io::StringSerializerOptions::SingleLine, dots::io::StringSerializerOptions::Strict }), std::runtime_error);
}