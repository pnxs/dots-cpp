#include <dots/testing/gtest/gtest.h>
#include <dots/serialization/JsonSerializer.h>
#include <serialization/TestSerializer.h>
#include <serialization/TestSerializer.h>

using dots::serialization::TextOptions;

struct JsonSerializerTestDataEncoded : SerializerTestDataEncoded<dots::serialization::JsonSerializer>
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

    data_t propertySetNone{ "0" };
    data_t propertySetAll{ "4294967295" };
    data_t propertySetMixed1{ "2868838485" };

    data_t timePoint1{ "1583960877.5" };
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

    data_t enum1{ "5" };

    //
    // property
    //

    data_t structSimple1_int32Property = Concat("\"int32Property\": ", int32Positive);
    data_t structSimple1_stringProperty = Concat("\"stringProperty\": \"", string1, "\"");
    data_t structSimple1_boolProperty = Concat("\"boolProperty\": null");
    data_t structSimple1_float32Property = Concat("\"float32Property\": ", float32Positive);

    data_t structComplex1_enumProperty = Concat("\"enumProperty\": ", enum1);
    data_t structComplex1_float64Property = Concat("\"float64Property\": ", float64Negative);
    data_t structComplex1_timepointProperty = Concat("\"timepointProperty\": ", timePoint1);
    data_t structComplex1_structSimpleProperty = Concat("\"structSimpleProperty\": { \"boolProperty\": ", boolFalse, " }");

    data_t structComplex2_propertySetProperty = Concat("\"propertySetProperty\": ", propertySetMixed1);
    data_t structComplex2_durationVectorProperty = Concat("\"durationVectorProperty\": [ ", duration1, ", ", duration2, " ]");
    data_t structComplex2_uuidProperty = Concat("\"uuidProperty\": \"", uuid1, "\"");

    //
    // vector
    //

    data_t vectorBool = Concat("[ ", boolTrue, ", ", boolFalse, ", ", boolFalse, " ]");
    data_t vectorFloat = Concat("[ ", float32Positive, ", ", float32Negative, " ]");
    data_t vectorStructSimple = Concat("[ { \"int32Property\": ", int32Positive, " }, { \"boolProperty\": ",  boolFalse, " } ]");
    data_t vectorEmpty = Concat("[ ]");

    //
    // struct
    //

    data_t structSimple1_Valid = Concat(
        "{ ",
        structSimple1_int32Property, ", ",
        structSimple1_stringProperty, ", ",
        structSimple1_float32Property,
        " }"
    );

    data_t structSimple1_All = Concat(
        "{ ",
        structSimple1_int32Property, ", ",
        structSimple1_stringProperty, ", ",
        structSimple1_boolProperty, ", ",
        structSimple1_float32Property,
        " }"
    );

    data_t structSimple1_Specific = Concat(
        "{ ",
        structSimple1_boolProperty, ", ",
        structSimple1_float32Property,
        " }"
    );

    data_t structSimple1_None = Concat(
        "{ ",
        "}"
    );
    
    data_t structComplex1_Valid = Concat(
        "{ ",
        structComplex1_enumProperty, ", ",
        structComplex1_float64Property, ", ",
        structComplex1_timepointProperty, ", ",
        structComplex1_structSimpleProperty,
        " }"
    );

    data_t structComplex1_Specific = Concat(
        "{ ",
        structComplex1_timepointProperty, ", ",
        "\"propertySetProperty\": null",
        " }"
    );

    data_t structComplex2_Valid = Concat(
        "{ ",
        structComplex2_propertySetProperty, ", ",
        structComplex2_durationVectorProperty, ", ",
        structComplex2_uuidProperty,
        " }"
    );

    data_t structComplex2_Specific = Concat(
        "{ ",
        "\"enumProperty\": null, ",
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
        "[ ",
        "\"", string1, "\", ",
        enum1, ", ",
        vectorBool, ", ",
        "{ ",
        structSimple1_int32Property, ", ",
        structSimple1_stringProperty, ", ",
        structSimple1_float32Property,
        " }",
        " ]"
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

INSTANTIATE_TYPED_TEST_SUITE_P(TestJsonSerializer, TestSerializer, JsonSerializerTestDataEncoded);

struct TestJsonSerializer : ::testing::Test
{
    using sut_t = dots::serialization::JsonSerializer;
};

TEST_F(TestJsonSerializer, serialize_EscapedString)
{
    {
        SerializationStructSimple instance{
            SerializationStructSimple::stringProperty_i{ "\"foo\" bar baz" }
        };
        EXPECT_EQ(sut_t::Serialize(instance), "{ \"stringProperty\": \"\\\"foo\\\" bar baz\" }");
    }

    {
        SerializationStructSimple instance{
            SerializationStructSimple::stringProperty_i{ "foo \"bar\" baz" }
        };
        EXPECT_EQ(sut_t::Serialize(instance), "{ \"stringProperty\": \"foo \\\"bar\\\" baz\" }");
    }

    {
        SerializationStructSimple instance{
            SerializationStructSimple::stringProperty_i{ "foo bar \"baz\"" }
        };
        EXPECT_EQ(sut_t::Serialize(instance), "{ \"stringProperty\": \"foo bar \\\"baz\\\"\" }");
    }

    {
        SerializationStructSimple instance{
            SerializationStructSimple::stringProperty_i{ "foo\\ \u0062\u0061\u0072\u00A9\n b\\az" }
        };
        EXPECT_EQ(sut_t::Serialize(instance), "{ \"stringProperty\": \"foo\\\\ \u0062\u0061\u0072\u00A9\\n b\\\\az\" }");
    }
}

TEST_F(TestJsonSerializer, deserialize_EscapedString)
{
    {
        std::string input = "{ \"stringProperty\": \"\\\"foo\\\" bar baz\" }";
        SerializationStructSimple expected{
            SerializationStructSimple::stringProperty_i{ "\"foo\" bar baz" }
        };
        EXPECT_EQ(sut_t::Deserialize<SerializationStructSimple>(input), expected);
    }

    {
        std::string input = "{ \"stringProperty\": \"foo \\\"bar\\\" baz\" }";
        SerializationStructSimple expected{
            SerializationStructSimple::stringProperty_i{ "foo \"bar\" baz" }
        };
        EXPECT_EQ(sut_t::Deserialize<SerializationStructSimple>(input), expected);
    }

    {
        std::string input = "{ \"stringProperty\": \"foo bar \\\"baz\\\"\" }";
        SerializationStructSimple expected{
            SerializationStructSimple::stringProperty_i{ "foo bar \"baz\"" }
        };
        EXPECT_EQ(sut_t::Deserialize<SerializationStructSimple>(input), expected);
    }

    {
        std::string input = "{ \"stringProperty\": \"foo\\\\ \u0062\u0061\u0072\u00A9\\n b\\\\az\" }";
        SerializationStructSimple expected{
            SerializationStructSimple::stringProperty_i{ "foo\\ \u0062\u0061\u0072\u00A9\n b\\az" }
        };
        EXPECT_EQ(sut_t::Deserialize<SerializationStructSimple>(input), expected);
    }
}

TEST_F(TestJsonSerializer, deserialize_PermitTopLevelUnescapedStringArgument)
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

TEST_F(TestJsonSerializer, deserialize_RejectNonTopLevelUnescapedStringArgument)
{
    std::string input = "{ \"stringProperty\": foo\\ \u0062\u0061\u0072\u00A9\n b\\az }";

    SerializationStructSimple actual;
    EXPECT_THROW(sut_t::Deserialize(input, actual), std::runtime_error);
}

TEST_F(TestJsonSerializer, serialize_WithOutputStyle)
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
        std::string expected = R"({"enumProperty":5,"uint32Property":12345789,"structSimpleProperty":{"boolProperty":false,"float32Property":-2.7183}})";
        EXPECT_EQ(sut_t::Serialize(instance, TextOptions{ TextOptions::Minified }), expected);
    }

    {
        std::string expected = R"({ "enumProperty": 5, "uint32Property": 12345789, "structSimpleProperty": { "boolProperty": false, "float32Property": -2.7183 } })";
        EXPECT_EQ(sut_t::Serialize(instance, TextOptions{ TextOptions::SingleLine }), expected);
    }

    {
        std::string expected = 
            "{\n"
            "    \"enumProperty\": 5,\n"
            "    \"uint32Property\": 12345789,\n"
            "    \"structSimpleProperty\": {\n"
            "        \"boolProperty\": false,\n"
            "        \"float32Property\": -2.7183\n"
            "    }\n"
            "}"
       ;
        EXPECT_EQ(sut_t::Serialize(instance, TextOptions{ TextOptions::MultiLine }), expected);
    }
}
