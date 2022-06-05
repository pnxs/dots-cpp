#include <dots/testing/gtest/gtest.h>
#include <dots/serialization/RapidJsonSerializer.h>
#include <serialization/TestSerializer.h>

struct RapidJsonSerializerTestDataEncoded : SerializerTestDataEncoded<dots::serialization::RapidJsonSerializer<>>
{
    //
    // fundamental
    //

    data_t boolFalse{ "false" };
    data_t boolTrue{ "true" };

    data_t int8Zero{ "0" };
    data_t int8Positive{ "42" };
    data_t int8Negative{ "-42" };

    data_t uint8Zero{ "0" };
    data_t uint8Positive1{ "42" };
    data_t uint8Positive2{ "170" };

    data_t int16Zero{ "0" };
    data_t int16Positive{ "12345" };
    data_t int16Negative{ "-12345" };

    data_t uint16Zero{ "0" };
    data_t uint16Positive1{ "12345" };
    data_t uint16Positive2{ "45113" };

    data_t int32Zero{ "0" };
    data_t int32Positive{ "12345789" };
    data_t int32Negative{ "-12345789" };

    data_t uint32Zero{ "0" };
    data_t uint32Positive1{ "12345789" };
    data_t uint32Positive2{ "2159829437" };

    data_t int64Zero{ "0" };
    data_t int64Positive{ "12345678910111213" };
    data_t int64Negative{ "-12345678910111213" };

    data_t uint64Zero{ "0" };
    data_t uint64Positive1{ "12345678910111213" };
    data_t uint64Positive2{ "18434398394799440403" };

    data_t float32Zero{ "0.0" };
    data_t float32Positive{ "3.1414999961853029" };
    data_t float32Negative{ "-2.7183001041412355" };

    data_t float64Zero{ "0.0" };
    data_t float64Positive{ "3.14159265359" };
    data_t float64Negative{ "-2.71828182846" };

    data_t propertySetNone{ "0" };
    data_t propertySetAll{ "4294967295" };
    data_t propertySetMixed1{ "2868838485" };

    data_t timePoint1{ "1583960877.5" };
    data_t steadyTimePoint1{ "324702.125" };

    data_t duration1{ "123.456" };
    data_t duration2{ "342.073" };

    data_t uuid1{ "\"8c96148e-58bd-11eb-ae93-0242ac130002\"" };

    data_t string1{ "\"foobar\"" };
    data_t string2{ "\"\\\"foo\\\" bar baz\"" };
    data_t string3{ "\"foo \\\"bar\\\" baz\"" };
    data_t string4{ "\"foo bar \\\"baz\\\"\"" };
    data_t string5{ u8"\"foo\\\\ \u0062\u0061\u0072\u00A9\\n b\\\\az\"" };
    data_t stringInvalid{ "\"fo\\obar\"" };

    //
    // enum
    //

    data_t enum1{ "5" };

    //
    // property
    //

    data_t structSimple1_int32Property = Concat("\"int32Property\":", int32Positive);
    data_t structSimple1_stringProperty = Concat("\"stringProperty\":", string1);
    data_t structSimple1_boolProperty = Concat("\"boolProperty\":null");
    data_t structSimple1_float32Property = Concat("\"float32Property\":", float32Positive);

    data_t structComplex1_enumProperty = Concat("\"enumProperty\":", enum1);
    data_t structComplex1_float64Property = Concat("\"float64Property\":", float64Negative);
    data_t structComplex1_timepointProperty = Concat("\"timepointProperty\":", timePoint1);
    data_t structComplex1_structSimpleProperty = Concat("\"structSimpleProperty\":{\"boolProperty\":", boolFalse, "}");

    data_t structComplex2_propertySetProperty = Concat("\"propertySetProperty\":", propertySetMixed1);
    data_t structComplex2_durationVectorProperty = Concat("\"durationVectorProperty\":[", duration1, ",", duration2, "]");
    data_t structComplex2_uuidProperty = Concat("\"uuidProperty\":", uuid1);

    //
    // vector
    //

    data_t vectorBool = Concat("[", boolTrue, ",", boolFalse, ",", boolFalse, "]");
    data_t vectorFloat = Concat("[", float32Positive, ",", float32Negative, "]");
    data_t vectorStructSimple = Concat("[{\"int32Property\":", int32Positive, "},{\"boolProperty\":", boolFalse, "}]");
    data_t vectorEmpty = Concat("[]");

    //
    // struct
    //

    data_t structSimple1_Valid = Concat(
        "{",
        structSimple1_int32Property, ",",
        structSimple1_stringProperty, ",",
        structSimple1_float32Property,
        "}"
    );

    data_t structSimple1_All = Concat(
        "{",
        structSimple1_int32Property, ",",
        structSimple1_stringProperty, ",",
        structSimple1_boolProperty, ",",
        structSimple1_float32Property,
        "}"
    );

    data_t structSimple1_Specific = Concat(
        "{",
        structSimple1_boolProperty, ",",
        structSimple1_float32Property,
        "}"
    );

    data_t structSimple1_None = Concat(
        "{}"
    );
    
    data_t structComplex1_Valid = Concat(
        "{",
        structComplex1_enumProperty, ",",
        structComplex1_float64Property, ",",
        structComplex1_timepointProperty, ",",
        structComplex1_structSimpleProperty,
        "}"
    );

    data_t structComplex1_Specific = Concat(
        "{",
        structComplex1_timepointProperty, ",",
        "\"propertySetProperty\":null",
        "}"
    );

    data_t structComplex2_Valid = Concat(
        "{",
        structComplex2_propertySetProperty, ",",
        structComplex2_durationVectorProperty, ",",
        structComplex2_uuidProperty,
        "}"
    );

    data_t structComplex2_Specific = Concat(
        "{",
        "\"enumProperty\":null,",
        structComplex2_durationVectorProperty,
        "}"
    );

    //
    // unescaped string
    //

    data_t string5Unescaped = u8"foo\\ \u0062\u0061\u0072\u00A9\n b\\az";
    data_t structSimple_String5Unescaped = Concat("{\"stringProperty\":", string5Unescaped, "}");

    //
    // tuple
    //

    data_t serializationTuple1 = Concat(
        "[",
        string1, ",",
        enum1, ",",
        vectorBool, ",",
        "{",
        structSimple1_int32Property, ",",
        structSimple1_stringProperty, ",",
        structSimple1_float32Property,
        "}",
        "]"
    );

    //
    // unknown properties
    //

    data_t structSimple1_unknownProperty = Concat("\"unknownProperty\": ", string5);
    data_t structComplex1_unknownProperty = Concat("\"unknownProperty\": ", structComplex2_Valid);

    data_t structSimple1_Unknown = Concat(
        "{ ",
        structSimple1_int32Property, ", ",
        structSimple1_stringProperty, ", ",
        structSimple1_float32Property, ", ",
        structSimple1_unknownProperty,
        " }"
    );

    data_t structComplex1_Unknown = Concat(
        "{ ",
        structComplex1_enumProperty, ", ",
        structComplex1_float64Property, ", ",
        structComplex1_unknownProperty, ", ",
        structComplex1_timepointProperty, ", ",
        structComplex1_structSimpleProperty,
        " }"
    );
};

struct TestRapidJsonSerializer : TestSerializer<RapidJsonSerializerTestDataEncoded>
{
protected:

    using base_t = TestSerializer<RapidJsonSerializerTestDataEncoded>;
    using data_t = std::string;
    using buffer_t = rapidjson::StringBuffer;
    using writer_t = rapidjson::Writer<buffer_t>;
    using format_t = dots::serialization::DefaultRapidJsonSerializerFormat;
    using serializer_t = dots::serialization::RapidJsonSerializer<format_t>;
};

TEST_F(TestRapidJsonSerializer, serialize_TypedArgument)
{
    EXPECT_EQ(serializer_t::Serialize(Decoded().boolFalse), Encoded().boolFalse);
    EXPECT_EQ(serializer_t::Serialize(Decoded().boolTrue), Encoded().boolTrue);

    EXPECT_EQ(serializer_t::Serialize(Decoded().int8Zero), Encoded().int8Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().int8Positive),  Encoded().int8Positive);
    EXPECT_EQ(serializer_t::Serialize(Decoded().int8Negative), Encoded().int8Negative);

    EXPECT_EQ(serializer_t::Serialize(Decoded().uint8Zero), Encoded().uint8Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().uint8Positive1), Encoded().uint8Positive1);
    EXPECT_EQ(serializer_t::Serialize(Decoded().uint8Positive2), Encoded().uint8Positive2);

    EXPECT_EQ(serializer_t::Serialize(Decoded().int16Zero), Encoded().int16Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().int16Positive), Encoded().int16Positive);
    EXPECT_EQ(serializer_t::Serialize(Decoded().int16Negative), Encoded().int16Negative);

    EXPECT_EQ(serializer_t::Serialize(Decoded().uint16Zero), Encoded().uint16Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().uint16Positive1), Encoded().uint16Positive1);
    EXPECT_EQ(serializer_t::Serialize(Decoded().uint16Positive2), Encoded().uint16Positive2);

    EXPECT_EQ(serializer_t::Serialize(Decoded().int32Zero), Encoded().int32Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().int32Positive), Encoded().int32Positive);
    EXPECT_EQ(serializer_t::Serialize(Decoded().int32Negative), Encoded().int32Negative);

    EXPECT_EQ(serializer_t::Serialize(Decoded().uint32Zero), Encoded().uint32Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().uint32Positive1), Encoded().uint32Positive1);
    EXPECT_EQ(serializer_t::Serialize(Decoded().uint32Positive2), Encoded().uint32Positive2);

    EXPECT_EQ(serializer_t::Serialize(Decoded().int64Zero), Encoded().int64Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().int64Positive), Encoded().int64Positive);
    EXPECT_EQ(serializer_t::Serialize(Decoded().int64Negative), Encoded().int64Negative);

    EXPECT_EQ(serializer_t::Serialize(Decoded().uint64Zero), Encoded().uint64Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().uint64Positive1), Encoded().uint64Positive1);
    EXPECT_EQ(serializer_t::Serialize(Decoded().uint64Positive2), Encoded().uint64Positive2);

    EXPECT_EQ(serializer_t::Serialize(Decoded().float32Zero), Encoded().float32Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().float32Positive), Encoded().float32Positive);
    EXPECT_EQ(serializer_t::Serialize(Decoded().float32Negative), Encoded().float32Negative);

    EXPECT_EQ(serializer_t::Serialize(Decoded().float64Zero), Encoded().float64Zero);
    EXPECT_EQ(serializer_t::Serialize(Decoded().float64Positive), Encoded().float64Positive);
    EXPECT_EQ(serializer_t::Serialize(Decoded().float64Negative), Encoded().float64Negative);

    EXPECT_EQ(serializer_t::Serialize(Decoded().propertySetNone), Encoded().propertySetNone);
    EXPECT_EQ(serializer_t::Serialize(Decoded().propertySetAll), Encoded().propertySetAll);
    EXPECT_EQ(serializer_t::Serialize(Decoded().propertySetMixed1), Encoded().propertySetMixed1);

    EXPECT_EQ(serializer_t::Serialize(Decoded().timePoint1), Encoded().timePoint1);
    EXPECT_EQ(serializer_t::Serialize(Decoded().steadyTimePoint1), Encoded().steadyTimePoint1);

    EXPECT_EQ(serializer_t::Serialize(Decoded().duration1), Encoded().duration1);
    EXPECT_EQ(serializer_t::Serialize(Decoded().duration2), Encoded().duration2);

    EXPECT_EQ(serializer_t::Serialize(Decoded().uuid1), Encoded().uuid1);

    EXPECT_EQ(serializer_t::Serialize(Decoded().string1), Encoded().string1);
    EXPECT_EQ(serializer_t::Serialize(Decoded().string2), Encoded().string2);
    EXPECT_EQ(serializer_t::Serialize(Decoded().string3), Encoded().string3);
    EXPECT_EQ(serializer_t::Serialize(Decoded().string4), Encoded().string4);
    EXPECT_EQ(serializer_t::Serialize(Decoded().string5), Encoded().string5);

    EXPECT_EQ(serializer_t::Serialize(Decoded().enum1), Encoded().enum1);
}

TEST_F(TestRapidJsonSerializer, deserialize_TypedArgument)
{
    EXPECT_EQ(serializer_t::Deserialize<dots::bool_t>(Encoded().boolFalse), Decoded().boolFalse);
    EXPECT_EQ(serializer_t::Deserialize<dots::bool_t>(Encoded().boolTrue), Decoded().boolTrue);

    EXPECT_EQ(serializer_t::Deserialize<dots::int8_t>(Encoded().int8Zero), Decoded().int8Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::int8_t>(Encoded().int8Positive), Decoded().int8Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::int8_t>(Encoded().int8Negative), Decoded().int8Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::uint8_t>(Encoded().uint8Zero), Decoded().uint8Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint8_t>(Encoded().uint8Positive1), Decoded().uint8Positive1);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint8_t>(Encoded().uint8Positive2), Decoded().uint8Positive2);

    EXPECT_EQ(serializer_t::Deserialize<dots::int16_t>(Encoded().int16Zero), Decoded().int16Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::int16_t>(Encoded().int16Positive), Decoded().int16Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::int16_t>(Encoded().int16Negative), Decoded().int16Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::uint16_t>(Encoded().uint16Zero), Decoded().uint16Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint16_t>(Encoded().uint16Positive1), Decoded().uint16Positive1);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint16_t>(Encoded().uint16Positive2), Decoded().uint16Positive2);

    EXPECT_EQ(serializer_t::Deserialize<dots::int32_t>(Encoded().int32Zero), Decoded().int32Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::int32_t>(Encoded().int32Positive), Decoded().int32Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::int32_t>(Encoded().int32Negative), Decoded().int32Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::uint32_t>(Encoded().uint32Zero), Decoded().uint32Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint32_t>(Encoded().uint32Positive1), Decoded().uint32Positive1);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint32_t>(Encoded().uint32Positive2), Decoded().uint32Positive2);

    EXPECT_EQ(serializer_t::Deserialize<dots::int64_t>(Encoded().int64Zero), Decoded().int64Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::int64_t>(Encoded().int64Positive), Decoded().int64Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::int64_t>(Encoded().int64Negative), Decoded().int64Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::uint64_t>(Encoded().uint64Zero), Decoded().uint64Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint64_t>(Encoded().uint64Positive1), Decoded().uint64Positive1);
    EXPECT_EQ(serializer_t::Deserialize<dots::uint64_t>(Encoded().uint64Positive2), Decoded().uint64Positive2);

    EXPECT_EQ(serializer_t::Deserialize<dots::float32_t>(Encoded().float32Zero), Decoded().float32Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::float32_t>(Encoded().float32Positive), Decoded().float32Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::float32_t>(Encoded().float32Negative), Decoded().float32Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::float64_t>(Encoded().float64Zero), Decoded().float64Zero);
    EXPECT_EQ(serializer_t::Deserialize<dots::float64_t>(Encoded().float64Positive), Decoded().float64Positive);
    EXPECT_EQ(serializer_t::Deserialize<dots::float64_t>(Encoded().float64Negative), Decoded().float64Negative);

    EXPECT_EQ(serializer_t::Deserialize<dots::property_set_t>(Encoded().propertySetNone), Decoded().propertySetNone);
    EXPECT_EQ(serializer_t::Deserialize<dots::property_set_t>(Encoded().propertySetAll), Decoded().propertySetAll);
    EXPECT_EQ(serializer_t::Deserialize<dots::property_set_t>(Encoded().propertySetMixed1), Decoded().propertySetMixed1);

    EXPECT_EQ(serializer_t::Deserialize<dots::timepoint_t>(Encoded().timePoint1), Decoded().timePoint1);
    EXPECT_EQ(serializer_t::Deserialize<dots::steady_timepoint_t>(Encoded().steadyTimePoint1), Decoded().steadyTimePoint1);

    EXPECT_EQ(serializer_t::Deserialize<dots::duration_t>(Encoded().duration1), Decoded().duration1);
    EXPECT_EQ(serializer_t::Deserialize<dots::duration_t>(Encoded().duration2), Decoded().duration2);

    EXPECT_EQ(serializer_t::Deserialize<dots::uuid_t>(Encoded().uuid1), Decoded().uuid1);

    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(Encoded().string1), Decoded().string1);
    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(Encoded().string2), Decoded().string2);
    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(Encoded().string3), Decoded().string3);
    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(Encoded().string4), Decoded().string4);
    EXPECT_EQ(serializer_t::Deserialize<dots::string_t>(Encoded().string5), Decoded().string5);
    EXPECT_ANY_THROW(serializer_t::Deserialize<dots::string_t>(Encoded().stringInvalid));

    EXPECT_EQ(serializer_t::Deserialize<SerializationEnum>(Encoded().enum1), Decoded().enum1);
}

TEST_F(TestRapidJsonSerializer, serialize_PropertyArgument)
{
    EXPECT_EQ(serializer_t::Serialize(Decoded().structSimple1_int32Property), Encoded().int32Positive);
    EXPECT_EQ(serializer_t::Serialize(Decoded().structSimple1_stringProperty), Encoded().string1);
    EXPECT_EQ(serializer_t::Serialize(Decoded().structSimple1_float32Property), Encoded().float32Positive);
}

TEST_F(TestRapidJsonSerializer, deserialize_PropertyArgument)
{
    SerializationStructSimple serializationProperties;
    serializer_t::Deserialize(Encoded().int32Positive, serializationProperties.int32Property);
    serializer_t::Deserialize(Encoded().string1, serializationProperties.stringProperty);
    serializer_t::Deserialize(Encoded().float32Positive, serializationProperties.float32Property);

    EXPECT_EQ(serializationProperties.int32Property, Decoded().structSimple1_int32Property);
    EXPECT_EQ(serializationProperties.stringProperty, Decoded().structSimple1_stringProperty);
    EXPECT_EQ(serializationProperties.float32Property, Decoded().structSimple1_float32Property);
}

TEST_F(TestRapidJsonSerializer, serialize_VectorArgument)
{
    EXPECT_EQ(serializer_t::Serialize(Decoded().vectorBool), Encoded().vectorBool);
    EXPECT_EQ(serializer_t::Serialize(Decoded().vectorFloat), Encoded().vectorFloat);
    EXPECT_EQ(serializer_t::Serialize(Decoded().vectorStructSimple), Encoded().vectorStructSimple);
    EXPECT_EQ(serializer_t::Serialize(Decoded().vectorEmpty), Encoded().vectorEmpty);
}

TEST_F(TestRapidJsonSerializer, deserialize_VectorArgument)
{
    {
        dots::vector_t<dots::bool_t> vectorBool;
        serializer_t::Deserialize(Encoded().vectorBool, vectorBool);
        EXPECT_EQ(vectorBool, Decoded().vectorBool);
    }

    {
        dots::vector_t<dots::float32_t> vectorFloat32;
        serializer_t::Deserialize(Encoded().vectorFloat, vectorFloat32);
        EXPECT_EQ(vectorFloat32, Decoded().vectorFloat);
    }

    {
        dots::vector_t<SerializationStructSimple> vectorStructSimple;
        serializer_t::Deserialize(Encoded().vectorStructSimple, vectorStructSimple);
        EXPECT_EQ(vectorStructSimple, Decoded().vectorStructSimple);
    }

    {
        dots::vector_t<dots::bool_t> vectorBool;
        serializer_t::Deserialize(Encoded().vectorEmpty, vectorBool);
        EXPECT_TRUE(vectorBool.empty());
    }
}

TEST_F(TestRapidJsonSerializer, serialize_SimpleStructArgument)
{
    EXPECT_EQ(serializer_t::Serialize(Decoded().structSimple1), Encoded().structSimple1_Valid);
    EXPECT_EQ(serializer_t::Serialize(Decoded().structSimple1, dots::property_set_t::All), Encoded().structSimple1_All);
    EXPECT_EQ(serializer_t::Serialize(Decoded().structSimple1, SerializationStructSimple::boolProperty_p + SerializationStructSimple::float32Property_p), Encoded().structSimple1_Specific);
    EXPECT_EQ(serializer_t::Serialize(Decoded().structSimple1, dots::property_set_t::None), base_t::Encoded().structSimple1_None);
}

TEST_F(TestRapidJsonSerializer, deserialize_SimpleStructArgument)
{
    {
        SerializationStructSimple structSimple;
        serializer_t::Deserialize(Encoded().structSimple1_Valid, structSimple);
        EXPECT_EQ(structSimple, Decoded().structSimple1);
    }

    {
        SerializationStructSimple structSimple;
        serializer_t::Deserialize(Encoded().structSimple1_Specific, structSimple);
        EXPECT_TRUE(structSimple._equal(Decoded().structSimple1, SerializationStructSimple::float32Property_p));
    }

    {
        SerializationStructSimple structSimple;
        base_t::serializer_t::Deserialize(Encoded().structSimple1_None, structSimple);
        EXPECT_TRUE(structSimple._validProperties().empty());
    }
}

TEST_F(TestRapidJsonSerializer, serialize_ComplexStructArgument)
{
    EXPECT_EQ(serializer_t::Serialize(Decoded().structComplex1), Encoded().structComplex1_Valid);
    EXPECT_EQ(serializer_t::Serialize(Decoded().structComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p), Encoded().structComplex1_Specific);

    EXPECT_EQ(serializer_t::Serialize(Decoded().structComplex2), Encoded().structComplex2_Valid);
    EXPECT_EQ(serializer_t::Serialize(Decoded().structComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p), Encoded().structComplex2_Specific);
}

TEST_F(TestRapidJsonSerializer, deserialize_ComplexStructArgument)
{
    {
        SerializationStructComplex structComplex;
        serializer_t::Deserialize(Encoded().structComplex1_Valid, structComplex);
        EXPECT_EQ(structComplex, Decoded().structComplex1);
    }

    {
        SerializationStructComplex structComplex;
        serializer_t::Deserialize(Encoded().structComplex1_Specific, structComplex);
        EXPECT_TRUE(structComplex._equal(Decoded().structComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p));
    }

    {
        SerializationStructComplex structComplex;
        serializer_t::Deserialize(Encoded().structComplex2_Valid, structComplex);
        EXPECT_EQ(structComplex, Decoded().structComplex2);
    }

    {
        SerializationStructComplex structComplex;
        serializer_t::Deserialize(Encoded().structComplex2_Specific, structComplex);
        EXPECT_TRUE(structComplex._equal(Decoded().structComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p));
    }
}

TEST_F(TestRapidJsonSerializer, serialize_TupleToContinuousInternalBuffer)
{
    buffer_t buffer;
    writer_t writer{ buffer };
    serializer_t sut{ writer };

    sut.writer().writeArrayBegin();
    {
        sut.serialize(Decoded().string1);
        sut.serialize(Decoded().enum1);
        sut.serialize(Decoded().vectorBool);
        sut.serialize(Decoded().structSimple1);
    }
    sut.writer().writeArrayEnd();

    EXPECT_EQ(buffer.GetString(), Encoded().serializationTuple1);
}

TEST_F(TestRapidJsonSerializer, deserialize_TupleFromContinuousExternalBuffer)
{
    serializer_t sut{ Encoded().serializationTuple1 };

    EXPECT_TRUE(sut.hasInput());

    sut.reader().readArrayBegin();
    {
        EXPECT_EQ(sut.deserialize<std::string>(), Decoded().string1);
        EXPECT_EQ(sut.deserialize<SerializationEnum>(), Decoded().enum1);
        EXPECT_EQ(sut.deserialize<dots::vector_t<dots::bool_t>>(), Decoded().vectorBool);
        EXPECT_EQ(sut.deserialize<SerializationStructSimple>(), Decoded().structSimple1);
    }
    sut.reader().readArrayEnd();

    EXPECT_FALSE(sut.hasInput());
}

TEST_F(TestRapidJsonSerializer, deserialize_UnknownProperties)
{
    {
        SerializationStructSimple structSimple;
        serializer_t::Deserialize(Encoded().structSimple1_Unknown, structSimple);
        EXPECT_EQ(structSimple, Decoded().structSimple1);
    }

    {
        SerializationStructComplex structComplex;
        serializer_t::Deserialize(Encoded().structComplex1_Unknown, structComplex);
        EXPECT_EQ(structComplex, Decoded().structComplex1);
    }
}

TEST_F(TestRapidJsonSerializer, deserialize_SpecificProperties)
{
    {
        serializer_t sut;
        sut.setInput(Encoded().structSimple1_Valid);

        SerializationStructSimple structSimple;
        dots::property_set_t deserializerProperties = SerializationStructSimple::boolProperty_p + SerializationStructSimple::float32Property_p;
        sut.deserialize(structSimple, deserializerProperties);

        EXPECT_EQ(structSimple._validProperties(), deserializerProperties - SerializationStructSimple::boolProperty_p);
        EXPECT_TRUE(structSimple._equal(Decoded().structSimple1, deserializerProperties));
    }

    {
        serializer_t sut;
        sut.setInput(Encoded().structComplex1_Valid);

        SerializationStructComplex structComplex;
        dots::property_set_t deserializerProperties = SerializationStructComplex::enumProperty_p + SerializationStructComplex::structSimpleProperty_p;
        sut.deserialize(structComplex, deserializerProperties);

        EXPECT_EQ(structComplex._validProperties(), deserializerProperties);
        EXPECT_TRUE(structComplex._equal(Decoded().structComplex1, deserializerProperties));
    }
}

TEST_F(TestRapidJsonSerializer, deserialize_InvalidatePropertyWhenInputIsInvalid)
{
    SerializationStructSimple instance{
        SerializationStructSimple::int32Property_i{ 42 }
    };

    std::string input = "{ \"int32Property\": null }";
    EXPECT_TRUE(instance.int32Property.isValid());
    serializer_t::Deserialize(input, instance);
    EXPECT_FALSE(instance.int32Property.isValid());
}
