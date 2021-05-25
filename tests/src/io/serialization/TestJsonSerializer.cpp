#include <dots/testing/gtest/gtest.h>
#include <dots/io/serialization/JsonSerializer.h>
#include <io/serialization/TestSerializerBase.h>
#include <io/serialization/TestStringSerializerBase.h>

struct JsonSerializerTestDataEncoded : SerializerTestBaseDataEncoded<dots::io::JsonSerializer<>>
{
    //
    // fundamental types
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

    data_t uuid1{ "\"8c96148e-58bd-11eb-ae93-0242ac130002\"" };

    data_t string1{ "\"foobar\"" };
    data_t string2{ "\"\\\"foo\\\" bar baz\"" };
    data_t string3{ "\"foo \\\"bar\\\" baz\"" };
    data_t string4{ "\"foo bar \\\"baz\\\"\"" };
    data_t string5{ u8"\"foo\\\\ \u0062\u0061\u0072\u00A9\\n b\\\\az\"" };

    //
    // enum types
    //

    data_t serializationEnum1{ "5" };

    //
    // property types
    //

    data_t serializationStructSimple1_int32Property = Concat("\"int32Property\": ", int32Positive);
    data_t serializationStructSimple1_stringProperty = Concat("\"stringProperty\": ", string1);
    data_t serializationStructSimple1_boolProperty = Concat("\"boolProperty\": null");
    data_t serializationStructSimple1_float32Property = Concat("\"float32Property\": ", float32Positive);

    data_t serializationStructComplex1_enumProperty = Concat("\"enumProperty\": ", serializationEnum1);
    data_t serializationStructComplex1_float64Property = Concat("\"float64Property\": ", float64Negative);
    data_t serializationStructComplex1_timepointProperty = Concat("\"timepointProperty\": ", timePoint1);
    data_t serializationStructComplex1_structSimpleProperty = Concat("\"structSimpleProperty\": { \"boolProperty\": ", boolFalse, " }");

    data_t serializationStructComplex2_propertySetProperty = Concat("\"propertySetProperty\": ", propertySetMixed1);
    data_t serializationStructComplex2_durationVectorProperty = Concat("\"durationVectorProperty\": [ ", duration1, ", ", duration2, " ]");
    data_t serializationStructComplex2_uuidProperty = Concat("\"uuidProperty\": ", uuid1);

    //
    // vector types
    //

    data_t vectorBool = Concat("[ ", boolTrue, ", ", boolFalse, ", ", boolFalse, " ]");
    data_t vectorFloat = Concat("[ ", float32Positive, ", ", float32Negative, " ]");
    data_t vectorStructSimple = Concat("[ { \"int32Property\": ", int32Positive, " }, { \"boolProperty\": ",  boolFalse, " } ]");

    //
    // struct types
    //

    data_t serializationStructSimple1_Valid = Concat(
        "{ ",
        serializationStructSimple1_int32Property, ", ",
        serializationStructSimple1_stringProperty, ", ",
        serializationStructSimple1_float32Property,
        " }"
    );

    data_t serializationStructSimple1_All = Concat(
        "{ ",
        serializationStructSimple1_int32Property, ", ",
        serializationStructSimple1_stringProperty, ", ",
        serializationStructSimple1_boolProperty, ", ",
        serializationStructSimple1_float32Property,
        " }"
    );

    data_t serializationStructSimple1_Specific = Concat(
        "{ ",
        serializationStructSimple1_boolProperty, ", ",
        serializationStructSimple1_float32Property,
        " }"
    );
    
    data_t serializationStructComplex1_Valid = Concat(
        "{ ",
        serializationStructComplex1_enumProperty, ", ",
        serializationStructComplex1_float64Property, ", ",
        serializationStructComplex1_timepointProperty, ", ",
        serializationStructComplex1_structSimpleProperty,
        " }"
    );

    data_t serializationStructComplex1_Specific = Concat(
        "{ ",
        serializationStructComplex1_timepointProperty, ", ",
        "\"propertySetProperty\": null",
        " }"
    );

    data_t serializationStructComplex2_Valid = Concat(
        "{ ",
        serializationStructComplex2_propertySetProperty, ", ",
        serializationStructComplex2_durationVectorProperty, ", ",
        serializationStructComplex2_uuidProperty,
        " }"
    );

    data_t serializationStructComplex2_Specific = Concat(
        "{ ",
        "\"enumProperty\": null, ",
        serializationStructComplex2_durationVectorProperty,
        " }"
    );

    //
    // tuple types
    //

    data_t serializationTuple1 = Concat(
        string1,
        serializationEnum1,
        vectorBool,
        serializationStructSimple1_Valid
    );

    //
    // string serializer types
    //

    data_t string5Unescaped = u8"foo\\ \u0062\u0061\u0072\u00A9\n b\\az";
    data_t serializationStructSimple_String5Unescaped = Concat("{ \"stringProperty\": ", string5Unescaped, " }");

    data_t serializationTuple1WithBeginEnd = Concat(
        "[ ",
        string1, ", ",
        serializationEnum1, ", ",
        vectorBool, ", ",
        "{ ",
        serializationStructSimple1_int32Property, ", ",
        serializationStructSimple1_stringProperty, ", ",
        serializationStructSimple1_float32Property,
        " }",
        " ]"
    );

    data_t serializationStructComplex_MinimalStyle = Concat("{\"enumProperty\":", serializationEnum1, ",\"float64Property\":", float64Negative, ",\"timepointProperty\":", timePoint1, ",\"structSimpleProperty\":{\"boolProperty\":", boolFalse, "}}");
    data_t serializationStructComplex_CompactStyle = Concat("{ \"enumProperty\": ", serializationEnum1, ", \"float64Property\": ", float64Negative, ", \"timepointProperty\": " + timePoint1, ", \"structSimpleProperty\": { \"boolProperty\": ", boolFalse, " } }");
    data_t serializationStructComplex_SingleLineStyle = Concat("{ \"enumProperty\": ", serializationEnum1, ", \"float64Property\": ", float64Negative, ", \"timepointProperty\": ", timePoint1, ", \"structSimpleProperty\": { \"boolProperty\": ", boolFalse, " } }");

    data_t serializationStructComplex_MultiLineStyle = Concat(
        "{\n",
        "    \"enumProperty\": ", serializationEnum1, ",\n",
        "    \"float64Property\": ", float64Negative, ",\n",
        "    \"timepointProperty\": ", timePoint1, ",\n",
        "    \"structSimpleProperty\": {\n",
        "        \"boolProperty\": ", boolFalse, "\n",
        "    }\n",
        "}"
    );

    data_t serializationStructComplex_RelaxedPolicy1 = Concat("{ \"enumProperty\": ", serializationEnum1," }");
    data_t serializationStructComplex_RelaxedPolicy2 = Concat("{ \"enumProperty\": ", serializationEnum1, " }");
    data_t serializationStructComplex_StrictPolicy1 = Concat("{ \"enumProperty\": ", serializationEnum1, " }");
    data_t serializationStructComplex_StrictPolicy2 = Concat("{ \"enumProperty\": ", serializationEnum1, " }");
};

INSTANTIATE_TYPED_TEST_SUITE_P(TestJsonSerializer, SerializerTestBase, JsonSerializerTestDataEncoded);
INSTANTIATE_TYPED_TEST_SUITE_P(TestJsonSerializer, StringSerializerTestBase, JsonSerializerTestDataEncoded);