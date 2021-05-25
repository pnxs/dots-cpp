#include <dots/testing/gtest/gtest.h>
#include <dots/io/serialization/RapidJsonSerializer.h>
#include <io/serialization/data/rapid_json_serialization_data.h>

struct TestRapidJsonSerializer : ::testing::Test, TestSerializationInput
{
protected:

    using data_t = std::string;
    using buffer_t = rapidjson::StringBuffer;
    using writer_t = rapidjson::Writer<buffer_t>;
    using serializer_t = dots::io::RapidJsonSerializer<writer_t>;

    serializer_t m_sut;
};

TEST_F(TestRapidJsonSerializer, serialize_TypedArgument)
{
    EXPECT_EQ(serializer_t::Serialize(BoolFalse), data_t(RAPID_JSON_BOOL_FALSE));
    EXPECT_EQ(serializer_t::Serialize(BoolTrue), data_t(RAPID_JSON_BOOL_TRUE));

    EXPECT_EQ(serializer_t::Serialize(Int8Zero), data_t(RAPID_JSON_INT8_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Int8Positive),  data_t(RAPID_JSON_INT8_POSITIVE));
    EXPECT_EQ(serializer_t::Serialize(Int8Negative), data_t(RAPID_JSON_INT8_NEGATIVE));

    EXPECT_EQ(serializer_t::Serialize(Uint8Zero), data_t(RAPID_JSON_UINT8_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Uint8Positive1), data_t(RAPID_JSON_UINT8_POSITIVE_1));
    EXPECT_EQ(serializer_t::Serialize(Uint8Positive2), data_t(RAPID_JSON_UINT8_POSITIVE_2));

    EXPECT_EQ(serializer_t::Serialize(Int16Zero), data_t(RAPID_JSON_INT16_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Int16Positive), data_t(RAPID_JSON_INT16_POSITIVE));
    EXPECT_EQ(serializer_t::Serialize(Int16Negative), data_t(RAPID_JSON_INT16_NEGATIVE));

    EXPECT_EQ(serializer_t::Serialize(Uint16Zero), data_t(RAPID_JSON_UINT16_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Uint16Positive1), data_t(RAPID_JSON_UINT16_POSITIVE_1));
    EXPECT_EQ(serializer_t::Serialize(Uint16Positive2), data_t(RAPID_JSON_UINT16_POSITIVE_2));

    EXPECT_EQ(serializer_t::Serialize(Int32Zero), data_t(RAPID_JSON_INT32_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Int32Positive), data_t(RAPID_JSON_INT32_POSITIVE));
    EXPECT_EQ(serializer_t::Serialize(Int32Negative), data_t(RAPID_JSON_INT32_NEGATIVE));

    EXPECT_EQ(serializer_t::Serialize(Uint32Zero), data_t(RAPID_JSON_UINT32_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Uint32Positive1), data_t(RAPID_JSON_UINT32_POSITIVE_1));
    EXPECT_EQ(serializer_t::Serialize(Uint32Positive2), data_t(RAPID_JSON_UINT32_POSITIVE_2));

    EXPECT_EQ(serializer_t::Serialize(Int64Zero), data_t(RAPID_JSON_INT64_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Int64Positive), data_t(RAPID_JSON_INT64_POSITIVE));
    EXPECT_EQ(serializer_t::Serialize(Int64Negative), data_t(RAPID_JSON_INT64_NEGATIVE));

    EXPECT_EQ(serializer_t::Serialize(Uint64Zero), data_t(RAPID_JSON_UINT64_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Uint64Positive1), data_t(RAPID_JSON_UINT64_POSITIVE_1));
    EXPECT_EQ(serializer_t::Serialize(Uint64Positive2), data_t(RAPID_JSON_UINT64_POSITIVE_2));

    EXPECT_EQ(serializer_t::Serialize(Float32Zero), data_t(RAPID_JSON_FLOAT32_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Float32Positive), data_t(RAPID_JSON_FLOAT32_POSITIVE));
    EXPECT_EQ(serializer_t::Serialize(Float32Negative), data_t(RAPID_JSON_FLOAT32_NEGATIVE));

    EXPECT_EQ(serializer_t::Serialize(Float64Zero), data_t(RAPID_JSON_FLOAT64_ZERO));
    EXPECT_EQ(serializer_t::Serialize(Float64Positive), data_t(RAPID_JSON_FLOAT64_POSITIVE));
    EXPECT_EQ(serializer_t::Serialize(Float64Negative), data_t(RAPID_JSON_FLOAT64_NEGATIVE));

    EXPECT_EQ(serializer_t::Serialize(PropertySetNone), data_t(RAPID_JSON_PROPERTY_SET_NONE));
    EXPECT_EQ(serializer_t::Serialize(PropertySetAll), data_t(RAPID_JSON_PROPERTY_SET_ALL));
    EXPECT_EQ(serializer_t::Serialize(PropertySetMixed1), data_t(RAPID_JSON_PROPERTY_SET_MIXED_1));

    EXPECT_EQ(serializer_t::Serialize(TimePoint1), data_t(RAPID_JSON_TIME_POINT_1));
    EXPECT_EQ(serializer_t::Serialize(SteadyTimePoint1), data_t(RAPID_JSON_STEADY_TIME_POINT_1));

    EXPECT_EQ(serializer_t::Serialize(Duration1), data_t(RAPID_JSON_DURATION_1));
    EXPECT_EQ(serializer_t::Serialize(Duration2), data_t(RAPID_JSON_DURATION_2));

    EXPECT_EQ(serializer_t::Serialize(Uuid1), data_t(RAPID_JSON_UUID_1));

    EXPECT_EQ(serializer_t::Serialize(String1), data_t(RAPID_JSON_STRING_1));
    EXPECT_EQ(serializer_t::Serialize(String2), data_t(RAPID_JSON_STRING_2));
    EXPECT_EQ(serializer_t::Serialize(String3), data_t(RAPID_JSON_STRING_3));
    EXPECT_EQ(serializer_t::Serialize(String4), data_t(RAPID_JSON_STRING_4));
    EXPECT_EQ(serializer_t::Serialize(String5), data_t(RAPID_JSON_STRING_5));

    EXPECT_EQ(serializer_t::Serialize(SerializationEnum1), data_t(RAPID_JSON_TEST_ENUM_1));
}

TEST_F(TestRapidJsonSerializer, deserialize_TypedArgument)
{
    EXPECT_EQ(serializer_t::Deserialize<dots::bool_t>(data_t(RAPID_JSON_BOOL_FALSE)), BoolFalse);
    EXPECT_EQ(serializer_t::Deserialize<dots::bool_t>(data_t(RAPID_JSON_BOOL_TRUE)), BoolTrue);

    EXPECT_EQ(serializer_t::Deserialize<dots::int8_t>(data_t(RAPID_JSON_INT8_ZERO)), Int8Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::int8_t>(data_t(RAPID_JSON_INT8_POSITIVE)), Int8Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::int8_t>(data_t(RAPID_JSON_INT8_NEGATIVE)), Int8Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::uint8_t>(data_t(RAPID_JSON_UINT8_ZERO)), Uint8Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint8_t>(data_t(RAPID_JSON_UINT8_POSITIVE_1)), Uint8Positive1);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint8_t>(data_t(RAPID_JSON_UINT8_POSITIVE_2)), Uint8Positive2);

    EXPECT_EQ(serializer_t::Deserialize<dots::int16_t>(data_t(RAPID_JSON_INT16_ZERO)), Int16Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::int16_t>(data_t(RAPID_JSON_INT16_POSITIVE)), Int16Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::int16_t>(data_t(RAPID_JSON_INT16_NEGATIVE)), Int16Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::uint16_t>(data_t(RAPID_JSON_UINT16_ZERO)), Uint16Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint16_t>(data_t(RAPID_JSON_UINT16_POSITIVE_1)), Uint16Positive1);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint16_t>(data_t(RAPID_JSON_UINT16_POSITIVE_2)), Uint16Positive2);

    EXPECT_EQ(serializer_t::Deserialize<dots::int32_t>(data_t(RAPID_JSON_INT32_ZERO)), Int32Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::int32_t>(data_t(RAPID_JSON_INT32_POSITIVE)), Int32Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::int32_t>(data_t(RAPID_JSON_INT32_NEGATIVE)), Int32Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::uint32_t>(data_t(RAPID_JSON_UINT32_ZERO)), Uint32Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint32_t>(data_t(RAPID_JSON_UINT32_POSITIVE_1)), Uint32Positive1);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint32_t>(data_t(RAPID_JSON_UINT32_POSITIVE_2)), Uint32Positive2);

    EXPECT_EQ(serializer_t::Deserialize<dots::int64_t>(data_t(RAPID_JSON_INT64_ZERO)), Int64Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::int64_t>(data_t(RAPID_JSON_INT64_POSITIVE)), Int64Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::int64_t>(data_t(RAPID_JSON_INT64_NEGATIVE)), Int64Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::uint64_t>(data_t(RAPID_JSON_UINT64_ZERO)), Uint64Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint64_t>(data_t(RAPID_JSON_UINT64_POSITIVE_1)), Uint64Positive1);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint64_t>(data_t(RAPID_JSON_UINT64_POSITIVE_2)), Uint64Positive2);

    EXPECT_EQ(serializer_t::Deserialize<dots::float32_t>(data_t(RAPID_JSON_FLOAT32_ZERO)), Float32Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::float32_t>(data_t(RAPID_JSON_FLOAT32_POSITIVE)), Float32Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::float32_t>(data_t(RAPID_JSON_FLOAT32_NEGATIVE)), Float32Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::float64_t>(data_t(RAPID_JSON_FLOAT64_ZERO)), Float64Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::float64_t>(data_t(RAPID_JSON_FLOAT64_POSITIVE)), Float64Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::float64_t>(data_t(RAPID_JSON_FLOAT64_NEGATIVE)), Float64Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::property_set_t>(data_t(RAPID_JSON_PROPERTY_SET_NONE)), PropertySetNone);
    EXPECT_EQ(serializer_t::Deserialize<dots::property_set_t>(data_t(RAPID_JSON_PROPERTY_SET_ALL)), PropertySetAll);
    EXPECT_EQ(serializer_t::Deserialize<dots::property_set_t>(data_t(RAPID_JSON_PROPERTY_SET_MIXED_1)), PropertySetMixed1);

    EXPECT_EQ(serializer_t::Deserialize<dots::timepoint_t>(data_t(RAPID_JSON_TIME_POINT_1)), TimePoint1);
    EXPECT_EQ(serializer_t::Deserialize<dots::steady_timepoint_t>(data_t(RAPID_JSON_STEADY_TIME_POINT_1)), SteadyTimePoint1);

    EXPECT_EQ(serializer_t::Deserialize<dots::duration_t>(data_t(RAPID_JSON_DURATION_1)), Duration1);
    EXPECT_EQ(serializer_t::Deserialize<dots::duration_t>(data_t(RAPID_JSON_DURATION_2)), Duration2);

    EXPECT_EQ(serializer_t::Deserialize<dots::uuid_t>(data_t(RAPID_JSON_UUID_1)), Uuid1);

    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(data_t(RAPID_JSON_STRING_1)), String1);
    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(data_t(RAPID_JSON_STRING_2)), String2);
    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(data_t(RAPID_JSON_STRING_3)), String3);
    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(data_t(RAPID_JSON_STRING_4)), String4);
    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(data_t(RAPID_JSON_STRING_5)), String5);

    EXPECT_EQ(serializer_t::Deserialize<SerializationEnum>(data_t(RAPID_JSON_TEST_ENUM_1)), SerializationEnum1);
}

TEST_F(TestRapidJsonSerializer, serialize_VectorArgument)
{
    EXPECT_EQ(serializer_t::Serialize(VectorBool), data_t("[" RAPID_JSON_BOOL_TRUE "," RAPID_JSON_BOOL_FALSE "," RAPID_JSON_BOOL_FALSE "]"));
    EXPECT_EQ(serializer_t::Serialize(VectorFloat), data_t("[" RAPID_JSON_FLOAT32_POSITIVE"," RAPID_JSON_FLOAT32_NEGATIVE "]"));
    EXPECT_EQ(serializer_t::Serialize(VectorStructSimple), data_t("[{\"int32Property\":" RAPID_JSON_INT32_POSITIVE "},{\"boolProperty\":" RAPID_JSON_BOOL_FALSE "}]"));
}

TEST_F(TestRapidJsonSerializer, deserialize_VectorArgument)
{
    dots::vector_t<dots::bool_t> vectorBool;
    serializer_t::Deserialize(data_t("[" RAPID_JSON_BOOL_TRUE "," RAPID_JSON_BOOL_FALSE "," RAPID_JSON_BOOL_FALSE "]"), vectorBool);
    EXPECT_EQ(vectorBool, VectorBool);

    dots::vector_t<dots::float32_t> vectorFloat32;
    serializer_t::Deserialize(data_t("[" RAPID_JSON_FLOAT32_POSITIVE"," RAPID_JSON_FLOAT32_NEGATIVE "]"), vectorFloat32);
    EXPECT_EQ(vectorFloat32, VectorFloat);

    dots::vector_t<SerializationStructSimple> vectorStructSimple;
    serializer_t::Deserialize(data_t("[{\"int32Property\":" RAPID_JSON_INT32_POSITIVE "},{\"boolProperty\":" RAPID_JSON_BOOL_FALSE "}]"), vectorStructSimple);
    EXPECT_EQ(vectorStructSimple, VectorStructSimple);
}

TEST_F(TestRapidJsonSerializer, serialize_SimpleStructArgument)
{
    data_t expectedValid = "{\"int32Property\":" RAPID_JSON_INT32_POSITIVE ",\"stringProperty\":" RAPID_JSON_STRING_1 ",\"float32Property\":" RAPID_JSON_FLOAT32_POSITIVE "}";
    EXPECT_EQ(serializer_t::Serialize(SerializationStructSimple1), expectedValid);

    data_t expectedAll = "{\"int32Property\":" RAPID_JSON_INT32_POSITIVE ",\"stringProperty\":" RAPID_JSON_STRING_1 ",\"boolProperty\":null,\"float32Property\":" RAPID_JSON_FLOAT32_POSITIVE "}";
    EXPECT_EQ(serializer_t::Serialize(SerializationStructSimple1, dots::property_set_t::All), expectedAll);

    data_t expectedSpecific = "{\"boolProperty\":null,\"float32Property\":" RAPID_JSON_FLOAT32_POSITIVE "}";
    EXPECT_EQ(serializer_t::Serialize(SerializationStructSimple1, SerializationStructSimple::boolProperty_p + SerializationStructSimple::float32Property_p), expectedSpecific);
}

TEST_F(TestRapidJsonSerializer, deserialize_SimpleStructArgument)
{
    SerializationStructSimple serializationStructSimple1;
    serializer_t::Deserialize(data_t{ "{\"int32Property\":" RAPID_JSON_INT32_POSITIVE ",\"stringProperty\":" RAPID_JSON_STRING_1 ",\"float32Property\":" RAPID_JSON_FLOAT32_POSITIVE "}" }, serializationStructSimple1);
    EXPECT_EQ(serializationStructSimple1, SerializationStructSimple1);

    SerializationStructSimple serializationStructSimple2;
    serializer_t::Deserialize(data_t{ "{\"int32Property\":" RAPID_JSON_INT32_POSITIVE ",\"stringProperty\":" RAPID_JSON_STRING_1 ",\"boolProperty\":null,\"float32Property\":" RAPID_JSON_FLOAT32_POSITIVE "}" }, serializationStructSimple2);
    EXPECT_EQ(serializationStructSimple2, SerializationStructSimple1);

    SerializationStructSimple serializationStructSimple3;
    serializer_t::Deserialize(data_t{ "{\"boolProperty\":null,\"float32Property\":" RAPID_JSON_FLOAT32_POSITIVE "}" }, serializationStructSimple3);
    EXPECT_TRUE(serializationStructSimple3._equal(SerializationStructSimple1, SerializationStructSimple::boolProperty_p + SerializationStructSimple::float32Property_p));
}

TEST_F(TestRapidJsonSerializer, serialize_ComplexStructArgument)
{
    data_t expectedValid1 = "{\"enumProperty\":" RAPID_JSON_TEST_ENUM_1 ",\"float64Property\":" RAPID_JSON_FLOAT64_NEGATIVE ",\"timepointProperty\":" RAPID_JSON_TIME_POINT_1 ",\"structSimpleProperty\":{\"boolProperty\":" RAPID_JSON_BOOL_FALSE "}}";
    EXPECT_EQ(serializer_t::Serialize(SerializationStructComplex1), expectedValid1);

    data_t expectedValid2 = "{\"propertySetProperty\":" RAPID_JSON_PROPERTY_SET_MIXED_1 ",\"durationVectorProperty\":[" RAPID_JSON_DURATION_1 "," RAPID_JSON_DURATION_2 "],\"uuidProperty\":" RAPID_JSON_UUID_1 "}";
    EXPECT_EQ(serializer_t::Serialize(SerializationStructComplex2), expectedValid2);

    data_t expectedSpecific1 = "{\"timepointProperty\":" RAPID_JSON_TIME_POINT_1 ",\"propertySetProperty\":null}";
    EXPECT_EQ(serializer_t::Serialize(SerializationStructComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p), expectedSpecific1);

    data_t expectedSpecific2 = "{\"enumProperty\":null,\"durationVectorProperty\":[" RAPID_JSON_DURATION_1 "," RAPID_JSON_DURATION_2 "]}";
    EXPECT_EQ(serializer_t::Serialize(SerializationStructComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p), expectedSpecific2);
}

TEST_F(TestRapidJsonSerializer, deserialize_ComplexStructArgument)
{
    SerializationStructComplex serializationStructComplex1;
    serializer_t::Deserialize(data_t{ "{\"enumProperty\":" RAPID_JSON_TEST_ENUM_1 ",\"float64Property\":" RAPID_JSON_FLOAT64_NEGATIVE ",\"timepointProperty\":" RAPID_JSON_TIME_POINT_1 ",\"structSimpleProperty\":{\"boolProperty\":" RAPID_JSON_BOOL_FALSE "}}" }, serializationStructComplex1);
    EXPECT_EQ(serializationStructComplex1, SerializationStructComplex1);

    SerializationStructComplex serializationStructComplex2;
    serializer_t::Deserialize(data_t{ "{\"propertySetProperty\":" RAPID_JSON_PROPERTY_SET_MIXED_1 ",\"durationVectorProperty\":[" RAPID_JSON_DURATION_1 "," RAPID_JSON_DURATION_2 "],\"uuidProperty\":" RAPID_JSON_UUID_1 "}" }, serializationStructComplex2);
    EXPECT_EQ(serializationStructComplex2, SerializationStructComplex2);

    SerializationStructComplex serializationStructComplex3;
    serializer_t::Deserialize(data_t{ "{\"timepointProperty\":" RAPID_JSON_TIME_POINT_1 ",\"propertySetProperty\":null}" }, serializationStructComplex3);
    EXPECT_TRUE(serializationStructComplex3._equal(SerializationStructComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p));

    SerializationStructComplex serializationStructComplex4;
    serializer_t::Deserialize(data_t{ "{\"enumProperty\":null,\"durationVectorProperty\":[" RAPID_JSON_DURATION_1 "," RAPID_JSON_DURATION_2 "]}" }, serializationStructComplex4);
    EXPECT_TRUE(serializationStructComplex4._equal(SerializationStructComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p));
}

TEST_F(TestRapidJsonSerializer, serialize_WriteTupleToContinuousInternalBuffer)
{
    buffer_t buffer;
    writer_t writer{ buffer };
    m_sut.setWriter(writer);

    m_sut.serializeTupleBegin();
    {
        m_sut.serialize(String1);
        m_sut.serialize(SerializationEnum1);
        m_sut.serialize(VectorBool);
        m_sut.serialize(SerializationStructSimple1);
    }
    m_sut.serializeTupleEnd();

    data_t output{
        "["
            RAPID_JSON_STRING_1 ","
            RAPID_JSON_TEST_ENUM_1 ","
            "[" RAPID_JSON_BOOL_TRUE "," RAPID_JSON_BOOL_FALSE "," RAPID_JSON_BOOL_FALSE "]" ","
            "{\"int32Property\":" RAPID_JSON_INT32_POSITIVE ",\"stringProperty\":" RAPID_JSON_STRING_1 ",\"float32Property\":" RAPID_JSON_FLOAT32_POSITIVE "}"
        "]"
    };
    EXPECT_EQ(buffer.GetString(), output);
}

TEST_F(TestRapidJsonSerializer, deserialize_ReadTupleFromContinuousExternalBuffer)
{
    data_t input{
        "["
            RAPID_JSON_STRING_1 ","
            RAPID_JSON_TEST_ENUM_1 ","
            "[" RAPID_JSON_BOOL_TRUE "," RAPID_JSON_BOOL_FALSE "," RAPID_JSON_BOOL_FALSE "]" ","
            "{\"int32Property\":" RAPID_JSON_INT32_POSITIVE ",\"stringProperty\":" RAPID_JSON_STRING_1 ",\"float32Property\":" RAPID_JSON_FLOAT32_POSITIVE "}"
        "]"
    };
    m_sut.setInputValue(input);

    EXPECT_TRUE(m_sut.hasInputValue());

    m_sut.deserializeTupleBegin();
    {
        EXPECT_EQ(m_sut.deserialize<std::string>(), String1);
        EXPECT_EQ(m_sut.deserialize<SerializationEnum>(), SerializationEnum1);
        EXPECT_EQ(m_sut.deserialize<dots::vector_t<dots::bool_t>>(), VectorBool);
        EXPECT_EQ(m_sut.deserialize<SerializationStructSimple>(), SerializationStructSimple1);
    }
    m_sut.deserializeTupleEnd();

    EXPECT_FALSE(m_sut.hasInputValue());
}