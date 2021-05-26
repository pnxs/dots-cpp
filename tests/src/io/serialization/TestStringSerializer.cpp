#include <dots/testing/gtest/gtest.h>
#include <dots/io/serialization/StringSerializer.h>
#include <io/serialization/TestSerializerBase.h>
#include <io/serialization/TestStringSerializerBase.h>

struct StringSerializerTestDataEncoded : SerializerBaseTestDataEncoded<dots::io::StringSerializer<>>
{
    //
    // fundamental
    //

    data_t boolFalse{ "false" };
    data_t boolTrue{ "true" };

    data_t int8Zero{ "0" };
    data_t int8Positive{ "42" };
    data_t int8Negative{ "-42" };

    data_t uint8Zero{ "0u" };
    data_t uint8Positive1{ "42u" };
    data_t uint8Positive2{ "170u" };

    data_t int16Zero{ "0" };
    data_t int16Positive{ "12345" };
    data_t int16Negative{ "-12345" };

    data_t uint16Zero{ "0u" };
    data_t uint16Positive1{ "12345u" };
    data_t uint16Positive2{ "45113u" };

    data_t int32Zero{ "0" };
    data_t int32Positive{ "12345789" };
    data_t int32Negative{ "-12345789" };

    data_t uint32Zero{ "0u" };
    data_t uint32Positive1{ "12345789u" };
    data_t uint32Positive2{ "2159829437u" };

    data_t int64Zero{ "0" };
    data_t int64Positive{ "12345678910111213" };
    data_t int64Negative{ "-12345678910111213" };

    data_t uint64Zero{ "0u" };
    data_t uint64Positive1{ "12345678910111213u" };
    data_t uint64Positive2{ "18434398394799440403u" };

    data_t float32Zero{ "0.0f" };
    data_t float32Positive{ "3.1415f" };
    data_t float32Negative{ "-2.7183f" };

    data_t float64Zero{ "0.0" };
    data_t float64Positive{ "3.14159265359" };
    data_t float64Negative{ "-2.71828182846" };

    data_t propertySetNone{ "0b00000000000000000000000000000000" };
    data_t propertySetAll{ "0b11111111111111111111111111111111" };
    data_t propertySetMixed1{ "0b10101010111111110000000001010101" };

    data_t timePoint1{ "\"" + dots::timepoint_t::FromString("2020-03-11T21:07:57.500+00:00").toString() + "\"" };
    data_t steadyTimePoint1{ "324702.125" };

    data_t duration1{ "123.456" };
    data_t duration2{ "342.073" };

    data_t uuid1{ "\"8c96148e-58bd-11eb-ae93-0242ac130002\"" };

    data_t string1{ "\"foobar\"" };
    data_t string2{ "\"\\\"foo\\\" bar baz\"" };
    data_t string3{ "\"foo \\\"bar\\\" baz\"" };
    data_t string4{ "\"foo bar \\\"baz\\\"\"" };
    data_t string5{ u8"\"foo\\\\ \u0062\u0061\u0072\u00A9\\n b\\\\az\"" };

    //
    // enum
    //

    data_t enum1{ "baz" };

    //
    // property
    //

    data_t structSimple1_int32Property = Concat(".int32Property = ", int32Positive);
    data_t structSimple1_stringProperty = Concat(".stringProperty = ", string1);
    data_t structSimple1_boolProperty = Concat(".boolProperty = <invalid>");
    data_t structSimple1_float32Property = Concat(".float32Property = ", float32Positive);

    data_t structComplex1_enumProperty = Concat(".enumProperty = ", enum1);
    data_t structComplex1_float64Property = Concat(".float64Property = ", float64Negative);
    data_t structComplex1_timepointProperty = Concat(".timepointProperty = ", timePoint1);
    data_t structComplex1_structSimpleProperty = Concat(".structSimpleProperty = { .boolProperty = ", boolFalse, " }");

    data_t structComplex2_propertySetProperty = Concat(".propertySetProperty = ", propertySetMixed1);
    data_t structComplex2_durationVectorProperty = Concat(".durationVectorProperty = { ", duration1, ", ", duration2, " }");
    data_t structComplex2_uuidProperty = Concat(".uuidProperty = ", uuid1);

    //
    // vector
    //

    data_t vectorBool = Concat("{ ", boolTrue, ", ", boolFalse, ", ", boolFalse, " }");
    data_t vectorFloat = Concat("{ ", float32Positive, ", ", float32Negative, " }");
    data_t vectorStructSimple = Concat("{ { .int32Property = ", int32Positive, " }, { .boolProperty = ",  boolFalse, " } }");

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
        " }"
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
        string1,
        enum1,
        vectorBool,
        structSimple1_Valid
    );

    //
    // tuple
    //

    data_t serializationTuple1 = Concat(
        "{ ",
        string1, ", ",
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
    // unescaped string
    //

    data_t string5Unescaped = u8"foo\\ \u0062\u0061\u0072\u00A9\n b\\az";
    data_t structSimple_String5Unescaped = Concat("SerializationStructSimple{ .stringProperty = ", string5Unescaped, " }");

    //
    // output style
    //

    data_t structComplex_MinimalStyle = Concat("{.enumProperty=", enum1, ",.float64Property=", float64Negative, ",.timepointProperty=", timePoint1, ",.structSimpleProperty={.boolProperty=", boolFalse, "}}");
    data_t structComplex_CompactStyle = Concat("SerializationStructComplex{ .enumProperty = ", enum1, ", .float64Property = ", float64Negative, ", .timepointProperty = " + timePoint1, ", .structSimpleProperty = { .boolProperty = ", boolFalse, " } }");
    data_t structComplex_SingleLineStyle = Concat("SerializationStructComplex{ .enumProperty = SerializationEnum::", enum1, ", .float64Property = ", float64Negative, ", .timepointProperty = ", timePoint1, ", .structSimpleProperty = SerializationStructSimple{ .boolProperty = ", boolFalse, " } }");

    data_t structComplex_MultiLineStyle = Concat(
        "SerializationStructComplex{\n",
        "    .enumProperty = SerializationEnum::", enum1, ",\n",
        "    .float64Property = ", float64Negative, ",\n",
        "    .timepointProperty = ", timePoint1, ",\n",
        "    .structSimpleProperty = SerializationStructSimple{\n",
        "        .boolProperty = ", boolFalse, "\n",
        "    }\n",
        "}"
    );

    //
    // input policy
    //

    data_t structComplex_RelaxedPolicy1 = Concat("{ .enumProperty = SerializationEnum::", enum1," }");
    data_t structComplex_RelaxedPolicy2 = Concat("{ .enumProperty = ", enum1, " }");
    data_t structComplex_StrictPolicy1 = Concat("{ .enumProperty = SerializationEnum::", enum1, " }");
    data_t structComplex_StrictPolicy2 = Concat("SerializationStructComplex{ .enumProperty = ", enum1, " }");
};

INSTANTIATE_TYPED_TEST_SUITE_P(TestStringSerializer, TestSerializerBase, StringSerializerTestDataEncoded);
INSTANTIATE_TYPED_TEST_SUITE_P(TestStringSerializer, TestStringSerializerBase, StringSerializerTestDataEncoded);
