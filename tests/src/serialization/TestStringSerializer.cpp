#include <dots/testing/gtest/gtest.h>
#include <dots/serialization/StringSerializer.h>
#include <serialization/TestSerializer.h>

using dots::serialization::TextOptions;

struct StringSerializerTestDataEncoded : SerializerTestDataEncoded<dots::serialization::StringSerializer>
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
    data_t float32Positive{ "3.1415" };
    data_t float32Negative{ "-2.7183" };

    data_t float64Zero{ "0.0" };
    data_t float64Positive{ "3.14159265359" };
    data_t float64Negative{ "-2.71828182846" };

    data_t propertySetNone{ "0b0" };
    data_t propertySetAll{ "0b11111111111111111111111111111111" };
    data_t propertySetMixed1{ "0b10101010111111110000000001010101" };

    data_t timePoint1{ dots::timepoint_t::FromString("2020-03-11T21:07:57.500+00:00").toString() };
    data_t steadyTimePoint1{ "324702.125" };

    data_t duration1{ "123.456" };
    data_t duration2{ "342.073" };

    data_t uuid1{ "8c96148e-58bd-11eb-ae93-0242ac130002" };

    data_t string1{ "foobar" };
    data_t string2{ "\"foo\" bar baz" };
    data_t string3{ "foo \"bar\" baz" };
    data_t string4{ "foo bar \"baz\"" };
    data_t string5{ u8"foo\\ \u0062\u0061\u0072\u00A9\n b\\az" };

    //
    // enum
    //

    data_t enum1{ "baz" };

    //
    // property
    //

    data_t structSimple1_int32Property = Concat(".int32Property = ", int32Positive);
    data_t structSimple1_stringProperty = Concat(".stringProperty = \"", string1, "\"");
    data_t structSimple1_boolProperty = Concat(".boolProperty = <invalid>");
    data_t structSimple1_float32Property = Concat(".float32Property = ", float32Positive, "f");

    data_t structComplex1_enumProperty = Concat(".enumProperty = ", enum1);
    data_t structComplex1_float64Property = Concat(".float64Property = ", float64Negative);
    data_t structComplex1_timepointProperty = Concat(".timepointProperty = \"", timePoint1, "\"");
    data_t structComplex1_structSimpleProperty = Concat(".structSimpleProperty = { .boolProperty = ", boolFalse, " }");

    data_t structComplex2_propertySetProperty = Concat(".propertySetProperty = ", propertySetMixed1, "u");
    data_t structComplex2_durationVectorProperty = Concat(".durationVectorProperty = { ", duration1, ", ", duration2, " }");
    data_t structComplex2_uuidProperty = Concat(".uuidProperty = \"", uuid1, "\"");

    //
    // vector
    //

    data_t vectorBool = Concat("{ ", boolTrue, ", ", boolFalse, ", ", boolFalse, " }");
    data_t vectorFloat = Concat("{ ", float32Positive, "f, ", float32Negative, "f }");
    data_t vectorStructSimple = Concat("{ { .int32Property = ", int32Positive, " }, { .boolProperty = ",  boolFalse, " } }");
    data_t vectorEmpty = Concat("{ }");

    //
    // struct
    //

    data_t structSimple1_Valid = Concat(
        "SerializationStructSimple{ ",
        structSimple1_int32Property, ", ",
        structSimple1_stringProperty, ", ",
        structSimple1_float32Property,
        " }"
    );

    data_t structSimple1_All = Concat(
        "SerializationStructSimple{ ",
        structSimple1_int32Property, ", ",
        structSimple1_stringProperty, ", ",
        structSimple1_boolProperty, ", ",
        structSimple1_float32Property,
        " }"
    );

    data_t structSimple1_Specific = Concat(
        "SerializationStructSimple{ ",
        structSimple1_boolProperty, ", ",
        structSimple1_float32Property,
        " }"
    );

    data_t structSimple1_None = Concat(
        "SerializationStructSimple{ ",
        "}"
    );
    
    data_t structComplex1_Valid = Concat(
        "SerializationStructComplex{ ",
        structComplex1_enumProperty, ", ",
        structComplex1_float64Property, ", ",
        structComplex1_timepointProperty, ", ",
        structComplex1_structSimpleProperty,
        " }"
    );

    data_t structComplex1_Specific = Concat(
        "SerializationStructComplex{ ",
        structComplex1_timepointProperty, ", ",
        ".propertySetProperty = <invalid>",
        " }"
    );

    data_t structComplex2_Valid = Concat(
        "SerializationStructComplex{ ",
        structComplex2_propertySetProperty, ", ",
        structComplex2_durationVectorProperty, ", ",
        structComplex2_uuidProperty,
        " }"
    );

    data_t structComplex2_Specific = Concat(
        "SerializationStructComplex{ ",
        ".enumProperty = <invalid>, ",
        structComplex2_durationVectorProperty,
        " }"
    );

    //
    // consecutive
    //

    data_t consecutiveTypes1 = Concat(
        structSimple1_Valid,
        structComplex1_Valid,
        vectorBool,
        structComplex2_Valid
    );

    //
    // tuple
    //

    data_t serializationTuple1 = Concat(
        "{ ",
        "\"", string1, "\", ",
        enum1, ", ",
        vectorBool, ", ",
        "{ ",
        structSimple1_int32Property, ", ",
        structSimple1_stringProperty, ", ",
        structSimple1_float32Property,
        " }",
        " }"
    );

    //
    // unknown properties
    //

    data_t structSimple1_unknownProperty = Concat(".unknownProperty = ", string5);
    data_t structComplex1_unknownProperty = Concat(".unknownProperty = ", structComplex2_Valid);

    data_t structSimple1_Unknown = Concat(
        "SerializationStructSimple{ ",
        structSimple1_int32Property, ", ",
        structSimple1_stringProperty, ", ",
        structSimple1_float32Property, ", ",
        structSimple1_unknownProperty,
        " }"
    );

    data_t structComplex1_Unknown = Concat(
        "SerializationStructComplex{ ",
        structComplex1_enumProperty, ", ",
        structComplex1_float64Property, ", ",
        structComplex1_unknownProperty, ", ",
        structComplex1_timepointProperty, ", ",
        structComplex1_structSimpleProperty,
        " }"
    );
};

INSTANTIATE_TYPED_TEST_SUITE_P(TestStringSerializer, TestSerializer, StringSerializerTestDataEncoded);

struct TestStringSerializer : ::testing::Test
{
    using sut_t = dots::serialization::StringSerializer;
};

TEST_F(TestStringSerializer, serialize_EscapedString)
{
    {
        SerializationStructSimple instance{
            SerializationStructSimple::stringProperty_i{ "\"foo\" bar baz" }
        };
        EXPECT_EQ(sut_t::Serialize(instance), "SerializationStructSimple{ .stringProperty = \"\\\"foo\\\" bar baz\" }");
    }

    {
        SerializationStructSimple instance{
            SerializationStructSimple::stringProperty_i{ "foo \"bar\" baz" }
        };
        EXPECT_EQ(sut_t::Serialize(instance), "SerializationStructSimple{ .stringProperty = \"foo \\\"bar\\\" baz\" }");
    }

    {
        SerializationStructSimple instance{
            SerializationStructSimple::stringProperty_i{ "foo bar \"baz\"" }
        };
        EXPECT_EQ(sut_t::Serialize(instance), "SerializationStructSimple{ .stringProperty = \"foo bar \\\"baz\\\"\" }");
    }

    {
        SerializationStructSimple instance{
            SerializationStructSimple::stringProperty_i{ "foo\\ \u0062\u0061\u0072\u00A9\n b\\az" }
        };
        EXPECT_EQ(sut_t::Serialize(instance), "SerializationStructSimple{ .stringProperty = \"foo\\\\ \u0062\u0061\u0072\u00A9\\n b\\\\az\" }");
    }
}

TEST_F(TestStringSerializer, deserialize_EscapedString)
{
    {
        std::string input = "SerializationStructSimple{ .stringProperty = \"\\\"foo\\\" bar baz\" }";
        SerializationStructSimple expected{
            SerializationStructSimple::stringProperty_i{ "\"foo\" bar baz" }
        };
        EXPECT_EQ(sut_t::Deserialize<SerializationStructSimple>(input), expected);
    }

    {
        std::string input = "SerializationStructSimple{ .stringProperty = \"foo \\\"bar\\\" baz\" }";
        SerializationStructSimple expected{
            SerializationStructSimple::stringProperty_i{ "foo \"bar\" baz" }
        };
        EXPECT_EQ(sut_t::Deserialize<SerializationStructSimple>(input), expected);
    }

    {
        std::string input = "SerializationStructSimple{ .stringProperty = \"foo bar \\\"baz\\\"\" }";
        SerializationStructSimple expected{
            SerializationStructSimple::stringProperty_i{ "foo bar \"baz\"" }
        };
        EXPECT_EQ(sut_t::Deserialize<SerializationStructSimple>(input), expected);
    }

    {
        std::string input = "SerializationStructSimple{ .stringProperty = \"foo\\\\ \u0062\u0061\u0072\u00A9\\n b\\\\az\" }";
        SerializationStructSimple expected{
            SerializationStructSimple::stringProperty_i{ "foo\\ \u0062\u0061\u0072\u00A9\n b\\az" }
        };
        EXPECT_EQ(sut_t::Deserialize<SerializationStructSimple>(input), expected);
    }
}

TEST_F(TestStringSerializer, deserialize_PermitTopLevelUnescapedStringArgument)
{
    std::string input = u8"foo\\ \u0062\u0061\u0072\u00A9\n b\\az";
    std::string expected = input;

    {
        EXPECT_EQ(sut_t::Deserialize<dots::string_t>(input), expected);
    }

    {
        SerializationStructSimple actual;
        sut_t::Deserialize(input, actual.stringProperty);
        EXPECT_EQ(*actual.stringProperty, expected);
    }
}

TEST_F(TestStringSerializer, deserialize_RejectNonTopLevelUnescapedStringArgument)
{
    std::string input = "SerializationStructSimple{ .stringProperty = \"foo\\ \u0062\u0061\u0072\u00A9\n b\\az\" }";

    SerializationStructSimple actual;
    EXPECT_THROW(sut_t::Deserialize(input, actual), std::runtime_error);
}

TEST_F(TestStringSerializer, serialize_WithOutputStyle)
{
    SerializationStructComplex instance{
        SerializationStructComplex::enumProperty_i{ SerializationEnum::baz },
        SerializationStructComplex::uint32Property_i{ 12345789u },
        SerializationStructComplex::structSimpleProperty_i{
            SerializationStructSimple::boolProperty_i{ false },
            SerializationStructSimple::float32Property_i{ -2.7183f }
        }
    };

    {
        std::string expected = "{.enumProperty=baz,.uint32Property=12345789,.structSimpleProperty={.boolProperty=false,.float32Property=-2.7183}}";
        EXPECT_EQ(sut_t::Serialize(instance, TextOptions{ TextOptions::Minified }), expected);
    }

    {
        std::string expected = "SerializationStructComplex{ .enumProperty = baz, .uint32Property = 12345789u, .structSimpleProperty = { .boolProperty = false, .float32Property = -2.7183f } }";
        EXPECT_EQ(sut_t::Serialize(instance, TextOptions{ TextOptions::SingleLine }), expected);
    }

    {
        std::string expected = 
           "SerializationStructComplex{\n"
           "    .enumProperty = SerializationEnum::baz,\n"
           "    .uint32Property = 12345789u,\n"
           "    .structSimpleProperty = SerializationStructSimple{\n"
           "        .boolProperty = false,\n"
           "        .float32Property = -2.7183f\n"
           "    }\n"
           "}"
       ;
        EXPECT_EQ(sut_t::Serialize(instance, TextOptions{ TextOptions::MultiLine }), expected);
    }
}

TEST_F(TestStringSerializer, deserialize_EnumWithInputPolicy)
{
    TextOptions relaxedOptions{ TextOptions::SingleLine, TextOptions::Relaxed };
    TextOptions strictOptions{ TextOptions::SingleLine, TextOptions::Strict };

    SerializationStructComplex expected{
        SerializationStructComplex::enumProperty_i{ SerializationEnum::baz }
    };
    
    {
        std::string input = "SerializationStructComplex{ .enumProperty = baz }";
        SerializationStructComplex actual;
        sut_t::Deserialize(input, actual, relaxedOptions);
        EXPECT_THROW(sut_t::Deserialize(input, actual, strictOptions), std::runtime_error);
        EXPECT_TRUE(actual._equal(expected));
    }

    {
        std::string input = "{ .enumProperty = SerializationEnum::baz }";
        SerializationStructComplex actual;
        sut_t::Deserialize(input, actual, relaxedOptions);
        EXPECT_THROW(sut_t::Deserialize(input, actual, strictOptions), std::runtime_error);
        EXPECT_TRUE(actual._equal(expected));
    }

    {
        std::string input = "{ .enumProperty = baz }";
        SerializationStructComplex actual;
        sut_t::Deserialize(input, actual, relaxedOptions);
        EXPECT_THROW(sut_t::Deserialize(input, actual, strictOptions), std::runtime_error);
        EXPECT_TRUE(actual._equal(expected));
    }
}

TEST_F(TestStringSerializer, deserialize_UnsignedIntegerWithInputPolicy)
{
    TextOptions relaxedOptions{ TextOptions::SingleLine, TextOptions::Relaxed };
    TextOptions strictOptions{ TextOptions::SingleLine, TextOptions::Strict };

    SerializationStructComplex expected{
        SerializationStructComplex::uint32Property_i{ 12345789u }
    };

    {
        std::string input = "SerializationStructComplex{ .uint32Property = 12345789 }";
        SerializationStructComplex actual;
        sut_t::Deserialize(input, actual, relaxedOptions);
        EXPECT_THROW(sut_t::Deserialize(input, actual, strictOptions), std::runtime_error);
        EXPECT_TRUE(actual._equal(expected));
    }

    {
        std::string input = "{ .uint32Property = 12345789u }";
        SerializationStructComplex actual;
        sut_t::Deserialize(input, actual, relaxedOptions);
        EXPECT_THROW(sut_t::Deserialize(input, actual, strictOptions), std::runtime_error);
        EXPECT_TRUE(actual._equal(expected));
    }

    {
        std::string input = "{ .uint32Property = 12345789 }";
        SerializationStructComplex actual;
        sut_t::Deserialize(input, actual, relaxedOptions);
        EXPECT_THROW(sut_t::Deserialize(input, actual, strictOptions), std::runtime_error);
        EXPECT_TRUE(actual._equal(expected));
    }
}
