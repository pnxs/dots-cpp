#include <gtest/gtest.h>
#include <dots/io/serialization/CborSerializer.h>
#include <io/serialization/data/cbor_serialization_data.h>

struct TestCborSerializer : ::testing::Test, TestSerializationInput
{
protected:

    using data_t = dots::io::CborSerializer::data_t;
    dots::io::CborSerializer m_sut;
};

TEST_F(TestCborSerializer, serialize_TypedArgument)
{
    EXPECT_EQ(m_sut.serialize(BoolFalse), data_t({ CBOR_BOOL_FALSE }));
    EXPECT_EQ(m_sut.serialize(BoolTrue), data_t({ CBOR_BOOL_TRUE }));

    EXPECT_EQ(m_sut.serialize(Int8Zero), data_t({ CBOR_INT8_ZERO }));
    EXPECT_EQ(m_sut.serialize(Int8Positive),  data_t({ CBOR_INT8_POSITIVE }));
    EXPECT_EQ(m_sut.serialize(Int8Negative), data_t({ CBOR_INT8_NEGATIVE }));

    EXPECT_EQ(m_sut.serialize(Uint8Zero), data_t({ CBOR_UINT8_ZERO }));
    EXPECT_EQ(m_sut.serialize(Uint8Positive1), data_t({ CBOR_UINT8_POSITIVE_1 }));
    EXPECT_EQ(m_sut.serialize(Uint8Positive2), data_t({ CBOR_UINT8_POSITIVE_2 }));

    EXPECT_EQ(m_sut.serialize(Int16Zero), data_t({ CBOR_INT16_ZERO }));
    EXPECT_EQ(m_sut.serialize(Int16Positive), data_t({ CBOR_INT16_POSITIVE }));
    EXPECT_EQ(m_sut.serialize(Int16Negative), data_t({ CBOR_INT16_NEGATIVE }));

    EXPECT_EQ(m_sut.serialize(Uint16Zero), data_t({ CBOR_UINT16_ZERO }));
    EXPECT_EQ(m_sut.serialize(Uint16Positive1), data_t({ CBOR_UINT16_POSITIVE_1 }));
    EXPECT_EQ(m_sut.serialize(Uint16Positive2), data_t({ CBOR_UINT16_POSITIVE_2 }));

    EXPECT_EQ(m_sut.serialize(Int32Zero), data_t({ CBOR_INT32_ZERO }));
    EXPECT_EQ(m_sut.serialize(Int32Positive), data_t({ CBOR_INT32_POSITIVE }));
    EXPECT_EQ(m_sut.serialize(Int32Negative), data_t({ CBOR_INT32_NEGATIVE }));

    EXPECT_EQ(m_sut.serialize(Uint32Zero), data_t({ CBOR_UINT32_ZERO }));
    EXPECT_EQ(m_sut.serialize(Uint32Positive1), data_t({ CBOR_UINT32_POSITIVE_1 }));
    EXPECT_EQ(m_sut.serialize(Uint32Positive2), data_t({ CBOR_UINT32_POSITIVE_2 }));

    EXPECT_EQ(m_sut.serialize(Int64Zero), data_t({ CBOR_INT64_ZERO }));
    EXPECT_EQ(m_sut.serialize(Int64Positive), data_t({ CBOR_INT64_POSITIVE }));
    EXPECT_EQ(m_sut.serialize(Int64Negative), data_t({ CBOR_INT64_NEGATIVE }));

    EXPECT_EQ(m_sut.serialize(Uint64Zero), data_t({ CBOR_UINT64_ZERO }));
    EXPECT_EQ(m_sut.serialize(Uint64Positive1), data_t({ CBOR_UINT64_POSITIVE_1 }));
    EXPECT_EQ(m_sut.serialize(Uint64Positive2), data_t({ CBOR_UINT64_POSITIVE_2 }));

    EXPECT_EQ(m_sut.serialize(Float32Zero), data_t({ CBOR_FLOAT32_ZERO }));
    EXPECT_EQ(m_sut.serialize(Float32Positive), data_t({ CBOR_FLOAT32_POSITIVE }));
    EXPECT_EQ(m_sut.serialize(Float32Negative), data_t({ CBOR_FLOAT32_NEGATIVE }));

    EXPECT_EQ(m_sut.serialize(Float64Zero), data_t({ CBOR_FLOAT64_ZERO }));
    EXPECT_EQ(m_sut.serialize(Float64Positive), data_t({ CBOR_FLOAT64_POSITIVE }));
    EXPECT_EQ(m_sut.serialize(Float64Negative), data_t({ CBOR_FLOAT64_NEGATIVE }));

    EXPECT_EQ(m_sut.serialize(PropertySetNone), data_t({ CBOR_PROPERTY_SET_NONE }));
    EXPECT_EQ(m_sut.serialize(PropertySetAll), data_t({ CBOR_PROPERTY_SET_ALL }));
    EXPECT_EQ(m_sut.serialize(PropertySetMixed1), data_t({ CBOR_PROPERTY_SET_MIXED_1 }));

    EXPECT_EQ(m_sut.serialize(TimePoint1), data_t({ CBOR_TIME_POINT_1 }));
    EXPECT_EQ(m_sut.serialize(SteadyTimePoint1), data_t({ CBOR_STEADY_TIME_POINT_1 }));

    EXPECT_EQ(m_sut.serialize(Duration1), data_t({ CBOR_DURATION_1 }));
    EXPECT_EQ(m_sut.serialize(Duration2), data_t({ CBOR_DURATION_2 }));

    EXPECT_EQ(m_sut.serialize(Uuid1), data_t({ CBOR_UUID_1 }));
    EXPECT_EQ(m_sut.serialize(String1), data_t({ CBOR_STRING_1 }));

    EXPECT_EQ(m_sut.serialize(SerializationEnum1), data_t({ CBOR_TEST_ENUM_1 }));
}

TEST_F(TestCborSerializer, deserialize_TypedArgument)
{
    EXPECT_EQ(m_sut.deserialize<dots::bool_t>(data_t({ CBOR_BOOL_FALSE })), BoolFalse);
    EXPECT_EQ(m_sut.deserialize<dots::bool_t>(data_t({ CBOR_BOOL_TRUE })), BoolTrue);

    EXPECT_EQ(m_sut.deserialize<dots::int8_t>(data_t({ CBOR_INT8_ZERO })), Int8Zero);
    EXPECT_EQ(m_sut.deserialize<dots::int8_t>(data_t({ CBOR_INT8_POSITIVE })), Int8Positive);
    EXPECT_EQ(m_sut.deserialize<dots::int8_t>(data_t({ CBOR_INT8_NEGATIVE })), Int8Negative);

    EXPECT_EQ(m_sut.deserialize<dots::uint8_t>(data_t({ CBOR_UINT8_ZERO })), Uint8Zero);
    EXPECT_EQ(m_sut.deserialize<dots::uint8_t>(data_t({ CBOR_UINT8_POSITIVE_1 })), Uint8Positive1);
    EXPECT_EQ(m_sut.deserialize<dots::uint8_t>(data_t({ CBOR_UINT8_POSITIVE_2 })), Uint8Positive2);

    EXPECT_EQ(m_sut.deserialize<dots::int16_t>(data_t({ CBOR_INT16_ZERO })), Int16Zero);
    EXPECT_EQ(m_sut.deserialize<dots::int16_t>(data_t({ CBOR_INT16_POSITIVE })), Int16Positive);
    EXPECT_EQ(m_sut.deserialize<dots::int16_t>(data_t({ CBOR_INT16_NEGATIVE })), Int16Negative);

    EXPECT_EQ(m_sut.deserialize<dots::uint16_t>(data_t({ CBOR_UINT16_ZERO })), Uint16Zero);
    EXPECT_EQ(m_sut.deserialize<dots::uint16_t>(data_t({ CBOR_UINT16_POSITIVE_1 })), Uint16Positive1);
    EXPECT_EQ(m_sut.deserialize<dots::uint16_t>(data_t({ CBOR_UINT16_POSITIVE_2 })), Uint16Positive2);

    EXPECT_EQ(m_sut.deserialize<dots::int32_t>(data_t({ CBOR_INT32_ZERO })), Int32Zero);
    EXPECT_EQ(m_sut.deserialize<dots::int32_t>(data_t({ CBOR_INT32_POSITIVE })), Int32Positive);
    EXPECT_EQ(m_sut.deserialize<dots::int32_t>(data_t({ CBOR_INT32_NEGATIVE })), Int32Negative);

    EXPECT_EQ(m_sut.deserialize<dots::uint32_t>(data_t({ CBOR_UINT32_ZERO })), Uint32Zero);
    EXPECT_EQ(m_sut.deserialize<dots::uint32_t>(data_t({ CBOR_UINT32_POSITIVE_1 })), Uint32Positive1);
    EXPECT_EQ(m_sut.deserialize<dots::uint32_t>(data_t({ CBOR_UINT32_POSITIVE_2 })), Uint32Positive2);

    EXPECT_EQ(m_sut.deserialize<dots::int64_t>(data_t({ CBOR_INT64_ZERO })), Int64Zero);
    EXPECT_EQ(m_sut.deserialize<dots::int64_t>(data_t({ CBOR_INT64_POSITIVE })), Int64Positive);
    EXPECT_EQ(m_sut.deserialize<dots::int64_t>(data_t({ CBOR_INT64_NEGATIVE })), Int64Negative);

    EXPECT_EQ(m_sut.deserialize<dots::uint64_t>(data_t({ CBOR_UINT64_ZERO })), Uint64Zero);
    EXPECT_EQ(m_sut.deserialize<dots::uint64_t>(data_t({ CBOR_UINT64_POSITIVE_1 })), Uint64Positive1);
    EXPECT_EQ(m_sut.deserialize<dots::uint64_t>(data_t({ CBOR_UINT64_POSITIVE_2 })), Uint64Positive2);

    EXPECT_EQ(m_sut.deserialize<dots::float32_t>(data_t({ CBOR_FLOAT32_ZERO })), Float32Zero);
    EXPECT_EQ(m_sut.deserialize<dots::float32_t>(data_t({ CBOR_FLOAT32_POSITIVE })), Float32Positive);
    EXPECT_EQ(m_sut.deserialize<dots::float32_t>(data_t({ CBOR_FLOAT32_NEGATIVE })), Float32Negative);

    EXPECT_EQ(m_sut.deserialize<dots::float64_t>(data_t({ CBOR_FLOAT64_ZERO })), Float64Zero);
    EXPECT_EQ(m_sut.deserialize<dots::float64_t>(data_t({ CBOR_FLOAT64_POSITIVE })), Float64Positive);
    EXPECT_EQ(m_sut.deserialize<dots::float64_t>(data_t({ CBOR_FLOAT64_NEGATIVE })), Float64Negative);

    EXPECT_EQ(m_sut.deserialize<dots::property_set_t>(data_t({ CBOR_PROPERTY_SET_NONE })), PropertySetNone);
    EXPECT_EQ(m_sut.deserialize<dots::property_set_t>(data_t({ CBOR_PROPERTY_SET_ALL })), PropertySetAll);
    EXPECT_EQ(m_sut.deserialize<dots::property_set_t>(data_t({ CBOR_PROPERTY_SET_MIXED_1 })), PropertySetMixed1);

    EXPECT_EQ(m_sut.deserialize<dots::timepoint_t>(data_t({ CBOR_TIME_POINT_1 })), TimePoint1);
    EXPECT_EQ(m_sut.deserialize<dots::steady_timepoint_t>(data_t({ CBOR_STEADY_TIME_POINT_1 })), SteadyTimePoint1);

    EXPECT_EQ(m_sut.deserialize<dots::duration_t>(data_t({ CBOR_DURATION_1 })), Duration1);
    EXPECT_EQ(m_sut.deserialize<dots::duration_t>(data_t({ CBOR_DURATION_2 })), Duration2);

    EXPECT_EQ(m_sut.deserialize<dots::uuid_t>(data_t({ CBOR_UUID_1 })), Uuid1);
    EXPECT_EQ(m_sut.deserialize<dots::string_t>(data_t({ CBOR_STRING_1 })), String1);

    EXPECT_EQ(m_sut.deserialize<SerializationEnum>(data_t({ CBOR_TEST_ENUM_1 })), SerializationEnum1);
}

TEST_F(TestCborSerializer, serialize_PropertyArgument)
{
    EXPECT_EQ(m_sut.serialize(SerializationStructSimple1.int32Property), data_t({ 0x01, CBOR_INT32_POSITIVE }));
    EXPECT_EQ(m_sut.serialize(SerializationStructSimple1.stringProperty), data_t({ 0x02, CBOR_STRING_1 }));
    EXPECT_EQ(m_sut.serialize(SerializationStructSimple1.float32Property), data_t({ 0x04, CBOR_FLOAT32_POSITIVE }));
}

TEST_F(TestCborSerializer, deserialize_PropertyArgument)
{
    SerializationStructSimple serializationStructSimple;
    m_sut.deserialize(data_t({ CBOR_INT32_POSITIVE }), serializationStructSimple.int32Property);
    m_sut.deserialize(data_t({ CBOR_STRING_1 }), serializationStructSimple.stringProperty);
    m_sut.deserialize(data_t({ CBOR_FLOAT32_POSITIVE }), serializationStructSimple.float32Property);

    EXPECT_EQ(serializationStructSimple.int32Property, SerializationStructSimple1.int32Property);
    EXPECT_EQ(serializationStructSimple.stringProperty, SerializationStructSimple1.stringProperty);
    EXPECT_EQ(serializationStructSimple.float32Property, SerializationStructSimple1.float32Property);
}

TEST_F(TestCborSerializer, serialize_VectorArgument)
{
    EXPECT_EQ(m_sut.serialize(VectorBool), data_t({ 0x83, CBOR_BOOL_TRUE, CBOR_BOOL_FALSE, CBOR_BOOL_FALSE }));
    EXPECT_EQ(m_sut.serialize(VectorFloat), data_t({ 0x82, CBOR_FLOAT32_POSITIVE, CBOR_FLOAT32_NEGATIVE }));
    EXPECT_EQ(m_sut.serialize(VectorStructSimple), data_t({ 0x82, 0xA1, 0x01, CBOR_INT32_POSITIVE, 0xA1, 0x03, CBOR_BOOL_FALSE }));
}

TEST_F(TestCborSerializer, deserialize_VectorArgument)
{
    dots::vector_t<dots::bool_t> vectorBool;
    m_sut.deserialize(data_t({ 0x83, CBOR_BOOL_TRUE, CBOR_BOOL_FALSE, CBOR_BOOL_FALSE }), vectorBool);
    EXPECT_EQ(vectorBool, VectorBool);

    dots::vector_t<dots::float32_t> vectorFloat32;
    m_sut.deserialize(data_t({ 0x82, CBOR_FLOAT32_POSITIVE, CBOR_FLOAT32_NEGATIVE }), vectorFloat32);
    EXPECT_EQ(vectorFloat32, VectorFloat);

    dots::vector_t<SerializationStructSimple> vectorStructSimple;
    m_sut.deserialize(data_t({ 0x82, 0xA1, 0x01, CBOR_INT32_POSITIVE, 0xA1, 0x03, CBOR_BOOL_FALSE }), vectorStructSimple);
    EXPECT_EQ(vectorStructSimple, VectorStructSimple);
}

TEST_F(TestCborSerializer, serialize_SimpleStructArgument)
{
    data_t expectedValid = { 0xA3, 0x01, CBOR_INT32_POSITIVE, 0x02, CBOR_STRING_1, 0x04, CBOR_FLOAT32_POSITIVE };
    EXPECT_EQ(m_sut.serialize(SerializationStructSimple1, SerializationStructSimple1._validProperties()), expectedValid);

    data_t expectedAll = { 0xA3, 0x01, CBOR_INT32_POSITIVE, 0x02, CBOR_STRING_1, 0x04, CBOR_FLOAT32_POSITIVE };
    EXPECT_EQ(m_sut.serialize(SerializationStructSimple1, dots::property_set_t::All), expectedAll);

    data_t expectedSpecific = { 0xA1, 0x04, CBOR_FLOAT32_POSITIVE };
    EXPECT_EQ(m_sut.serialize(SerializationStructSimple1, SerializationStructSimple::boolProperty_p + SerializationStructSimple::float32Property_p), expectedSpecific);
}

TEST_F(TestCborSerializer, deserialize_SimpleStructArgument)
{
    SerializationStructSimple serializationStructSimple1;
    m_sut.deserialize(data_t{ 0xA3, 0x01, CBOR_INT32_POSITIVE, 0x02, CBOR_STRING_1, 0x04, CBOR_FLOAT32_POSITIVE }, serializationStructSimple1);
    EXPECT_EQ(serializationStructSimple1, SerializationStructSimple1);

    SerializationStructSimple serializationStructSimple2;
    m_sut.deserialize(data_t{ 0xA1, 0x04, CBOR_FLOAT32_POSITIVE }, serializationStructSimple2);
    EXPECT_TRUE(serializationStructSimple2._equal(SerializationStructSimple1, SerializationStructSimple::float32Property_p));
}

TEST_F(TestCborSerializer, serialize_ComplexStructArgument)
{
    data_t expectedValid1 = { 0xA4, 0x07, CBOR_TEST_ENUM_1, 0x04, CBOR_FLOAT64_NEGATIVE, 0x18, 0x19, CBOR_TIME_POINT_1, 0x0F, 0xA1, 0x03, CBOR_BOOL_FALSE };
    EXPECT_EQ(m_sut.serialize(SerializationStructComplex1, SerializationStructComplex1._validProperties()), expectedValid1);

    data_t expectedValid2 = { 0xA3, 0x03, CBOR_PROPERTY_SET_MIXED_1, 0x09, 0x82, CBOR_DURATION_1, CBOR_DURATION_2, 0x06, CBOR_UUID_1 };
    EXPECT_EQ(m_sut.serialize(SerializationStructComplex2, SerializationStructComplex2._validProperties()), expectedValid2);

    data_t expectedSpecific1 = { 0xA1, 0x18, 0x19, CBOR_TIME_POINT_1 };
    EXPECT_EQ(m_sut.serialize(SerializationStructComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p), expectedSpecific1);

    data_t expectedSpecific2 = { 0xA1, 0x09, 0x82, CBOR_DURATION_1, CBOR_DURATION_2 };
    EXPECT_EQ(m_sut.serialize(SerializationStructComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p), expectedSpecific2);
}

TEST_F(TestCborSerializer, deserialize_ComplexStructArgument)
{
    SerializationStructComplex serializationStructComplex1;
    m_sut.deserialize(data_t{ 0xA4, 0x07, CBOR_TEST_ENUM_1, 0x04, CBOR_FLOAT64_NEGATIVE, 0x18, 0x19, CBOR_TIME_POINT_1, 0x0F, 0xA1, 0x03, CBOR_BOOL_FALSE }, serializationStructComplex1);
    EXPECT_EQ(serializationStructComplex1, SerializationStructComplex1);

    SerializationStructComplex serializationStructComplex2;
    m_sut.deserialize(data_t{ 0xA3, 0x03, CBOR_PROPERTY_SET_MIXED_1, 0x09, 0x82, CBOR_DURATION_1, CBOR_DURATION_2, 0x06, CBOR_UUID_1 }, serializationStructComplex2);
    EXPECT_EQ(serializationStructComplex2, SerializationStructComplex2);

    SerializationStructComplex serializationStructComplex3;
    m_sut.deserialize(data_t{ 0xA1, 0x18, 0x19, CBOR_TIME_POINT_1 }, serializationStructComplex3);
    EXPECT_TRUE(serializationStructComplex3._equal(SerializationStructComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p));

    SerializationStructComplex serializationStructComplex4;
    m_sut.deserialize(data_t{ 0xA1, 0x09, 0x82, CBOR_DURATION_1, CBOR_DURATION_2 }, serializationStructComplex4);
    EXPECT_TRUE(serializationStructComplex4._equal(SerializationStructComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p));
}