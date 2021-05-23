#include <dots/testing/gtest/gtest.h>

#include <SerializationStructSimple.dots.h>
#include <SerializationStructComplex.dots.h>

using namespace dots::type::literals;

struct SerializerTestBaseDataDecoded
{
    //
    // fundamental types
    //

    dots::bool_t boolFalse{ false };
    dots::bool_t boolTrue{ true };

    dots::int8_t int8Zero{ 0 };
    dots::int8_t int8Positive{ 42 };
    dots::int8_t int8Negative{ -42 };

    dots::uint8_t uint8Zero{ 0u };
    dots::uint8_t uint8Positive1{ 42u };
    dots::uint8_t uint8Positive2{ 170u };

    dots::int16_t int16Zero{ 0 };
    dots::int16_t int16Positive{ 12345 };
    dots::int16_t int16Negative{ -12345 };

    dots::uint16_t uint16Zero{ 0u };
    dots::uint16_t uint16Positive1{ 12345u };
    dots::uint16_t uint16Positive2{ 45113u };

    dots::int32_t int32Zero{ 0 };
    dots::int32_t int32Positive{ 12345789 };
    dots::int32_t int32Negative{ -12345789 };

    dots::uint32_t uint32Zero{ 0u };
    dots::uint32_t uint32Positive1{ 12345789u };
    dots::uint32_t uint32Positive2{ 2159829437u };

    dots::int64_t int64Zero{ 0 };
    dots::int64_t int64Positive{ 12345678910111213 };
    dots::int64_t int64Negative{ -12345678910111213 };

    dots::uint64_t uint64Zero{ 0u };
    dots::uint64_t uint64Positive1{ 12345678910111213u };
    dots::uint64_t uint64Positive2{ 18434398394799440403u };

    dots::float32_t float32Zero{ 0.0f };
    dots::float32_t float32Positive{ 3.1415f };
    dots::float32_t float32Negative{ -2.7183f };

    dots::float64_t float64Zero{ 0.0f };
    dots::float64_t float64Positive{ 3.14159265359 };
    dots::float64_t float64Negative{ -2.71828182846 };

    dots::property_set_t propertySetNone{ dots::property_set_t::None };
    dots::property_set_t propertySetAll{ dots::property_set_t::All };
    dots::property_set_t propertySetMixed1{ 0b10101010111111110000000001010101 };

    dots::timepoint_t timePoint1{ dots::timepoint_t::FromString("2020-03-11T21:07:57.500+00:00") };
    dots::steady_timepoint_t steadyTimePoint1{ dots::steady_timepoint_t::FromString("P3DT18H11M42.125S") };

    dots::duration_t duration1{ 123.456 };
    dots::duration_t duration2{ 5min + 42s + 73ms };

    dots::uuid_t uuid1{ dots::uuid_t::FromString("8c96148e-58bd-11eb-ae93-0242ac130002") };

    dots::string_t string1{ "foobar" };
    dots::string_t string2{ "\"foo\" bar baz" };
    dots::string_t string3{ "foo \"bar\" baz" };
    dots::string_t string4{ "foo bar \"baz\"" };
    dots::string_t string5{ u8"foo\\ \u0062\u0061\u0072\u00A9\n b\\az" };

    //
    // enum types
    //

    SerializationEnum serializationEnum1{ SerializationEnum::baz };

    //
    // struct types
    //

    SerializationStructSimple serializationStructSimple1{
        SerializationStructSimple::int32Property_i{ int32Positive },
        SerializationStructSimple::stringProperty_i{ string1 },
        SerializationStructSimple::float32Property_i{ float32Positive }
    };

    SerializationStructComplex serializationStructComplex1{
        SerializationStructComplex::enumProperty_i{ serializationEnum1 },
        SerializationStructComplex::float64Property_i{ float64Negative },
        SerializationStructComplex::timepointProperty_i{ timePoint1 },
        SerializationStructComplex::structSimpleProperty_i{
            SerializationStructSimple::boolProperty_i{ boolFalse }
        }
    };

    SerializationStructComplex serializationStructComplex2{
        SerializationStructComplex::propertySetProperty_i{ propertySetMixed1 },
        SerializationStructComplex::durationVectorProperty_i{ dots::vector_t<dots::duration_t>{ duration1, duration2 } },
        SerializationStructComplex::uuidProperty_i{ uuid1 }
    };

    //
    // property types
    //

    const SerializationStructSimple::int32Property_t& serializationStructSimple1_int32Property = serializationStructSimple1.int32Property;
    const SerializationStructSimple::stringProperty_t& serializationStructSimple1_stringProperty = serializationStructSimple1.stringProperty;
    const SerializationStructSimple::float32Property_t& serializationStructSimple1_float32Property = serializationStructSimple1.float32Property;

    const SerializationStructComplex::enumProperty_t& serializationStructComplex1_enumProperty = serializationStructComplex1.enumProperty;
    const SerializationStructComplex::float64Property_t& serializationStructComplex1_float64Property = serializationStructComplex1.float64Property;
    const SerializationStructComplex::timepointProperty_t& serializationStructComplex1_timepointProperty = serializationStructComplex1.timepointProperty;
    const SerializationStructComplex::structSimpleProperty_t& serializationStructComplex1_structSimpleProperty = serializationStructComplex1.structSimpleProperty;

    const SerializationStructComplex::propertySetProperty_t& serializationStructComplex2_propertySetProperty = serializationStructComplex2.propertySetProperty;
    const SerializationStructComplex::durationVectorProperty_t& serializationStructComplex2_durationVectorProperty = serializationStructComplex2.durationVectorProperty;
    const SerializationStructComplex::uuidProperty_t& serializationStructComplex2_uuidProperty = serializationStructComplex2.uuidProperty;

    //
    // vector types
    //

    dots::vector_t<dots::bool_t> vectorBool{ boolTrue, boolFalse, boolFalse };
    dots::vector_t<dots::float32_t> vectorFloat{ float32Positive, float32Negative };

    dots::vector_t<SerializationStructSimple> vectorStructSimple{
        SerializationStructSimple{
            SerializationStructSimple::int32Property_i{ int32Positive }
        },
        SerializationStructSimple{
            SerializationStructSimple::boolProperty_i{ boolFalse }
        }
    };
};

template <typename Serializer>
struct SerializerTestBaseDataEncoded
{
    using serializer_t = Serializer;
    using data_t = typename serializer_t::data_t;
    using value_t = typename data_t::value_type;

    template <typename T>
    using is_partial_data = std::disjunction<std::is_same<std::decay_t<T>, data_t>, std::is_convertible<std::decay_t<T>, value_t>>;

    template <typename... Ts, std::enable_if_t<std::conjunction_v<is_partial_data<Ts>...>, int> = 0>
    static data_t Concat(Ts&&... ts)
    {
        data_t data;

        auto append = [&data](auto&& partialData)
        {
            using decayed_t = std::decay_t<decltype(partialData)>;

            if constexpr (std::is_same_v<decayed_t, data_t>)
            {
                std::copy(partialData.begin(), partialData.end(), std::back_inserter(data));
            }
            else/* if constexpr (std::is_same_v<decayed_t, value_t>)*/
            {
                data.emplace_back(static_cast<value_t>(partialData));
            }
        };

        (append(std::forward<Ts>(ts)), ...);

        return data;
    }
};

template <typename TEncoded>
struct SerializerTestBase : ::testing::Test
{
protected:

    using decoded_t = SerializerTestBaseDataDecoded;
    using encoded_t = TEncoded;

    using serializer_t = typename encoded_t::serializer_t;

    using data_t = typename encoded_t::data_t;
    using value_t = typename data_t::value_type;

    static const decoded_t& Decoded()
    {
        static decoded_t Decoded;
        return Decoded;
    }

    static const encoded_t& Encoded()
    {
        static encoded_t Encoded;
        return Encoded;
    }
};

TYPED_TEST_SUITE_P(SerializerTestBase);

TYPED_TEST_P(SerializerTestBase, serialize_TypedArgument)
{
    using base_t = SerializerTestBase<TypeParam>;

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().boolFalse), base_t::Encoded().boolFalse);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().boolTrue), base_t::Encoded().boolTrue);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int8Zero), base_t::Encoded().int8Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int8Positive),  base_t::Encoded().int8Positive);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int8Negative), base_t::Encoded().int8Negative);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint8Zero), base_t::Encoded().uint8Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint8Positive1), base_t::Encoded().uint8Positive1);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint8Positive2), base_t::Encoded().uint8Positive2);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int16Zero), base_t::Encoded().int16Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int16Positive), base_t::Encoded().int16Positive);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int16Negative), base_t::Encoded().int16Negative);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint16Zero), base_t::Encoded().uint16Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint16Positive1), base_t::Encoded().uint16Positive1);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint16Positive2), base_t::Encoded().uint16Positive2);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int32Zero), base_t::Encoded().int32Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int32Positive), base_t::Encoded().int32Positive);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int32Negative), base_t::Encoded().int32Negative);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint32Zero), base_t::Encoded().uint32Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint32Positive1), base_t::Encoded().uint32Positive1);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint32Positive2), base_t::Encoded().uint32Positive2);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int64Zero), base_t::Encoded().int64Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int64Positive), base_t::Encoded().int64Positive);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().int64Negative), base_t::Encoded().int64Negative);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint64Zero), base_t::Encoded().uint64Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint64Positive1), base_t::Encoded().uint64Positive1);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uint64Positive2), base_t::Encoded().uint64Positive2);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().float32Zero), base_t::Encoded().float32Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().float32Positive), base_t::Encoded().float32Positive);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().float32Negative), base_t::Encoded().float32Negative);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().float64Zero), base_t::Encoded().float64Zero);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().float64Positive), base_t::Encoded().float64Positive);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().float64Negative), base_t::Encoded().float64Negative);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().propertySetNone), base_t::Encoded().propertySetNone);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().propertySetAll), base_t::Encoded().propertySetAll);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().propertySetMixed1), base_t::Encoded().propertySetMixed1);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().timePoint1), base_t::Encoded().timePoint1);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().steadyTimePoint1), base_t::Encoded().steadyTimePoint1);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().duration1), base_t::Encoded().duration1);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().duration2), base_t::Encoded().duration2);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().uuid1), base_t::Encoded().uuid1);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().string1), base_t::Encoded().string1);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationEnum1), base_t::Encoded().serializationEnum1);
}

TYPED_TEST_P(SerializerTestBase, deserialize_TypedArgument)
{
    using base_t = SerializerTestBase<TypeParam>;

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::bool_t>(base_t::Encoded().boolFalse), base_t::Decoded().boolFalse);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::bool_t>(base_t::Encoded().boolTrue), base_t::Decoded().boolTrue);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int8_t>(base_t::Encoded().int8Zero), base_t::Decoded().int8Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int8_t>(base_t::Encoded().int8Positive), base_t::Decoded().int8Positive);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int8_t>(base_t::Encoded().int8Negative), base_t::Decoded().int8Negative);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint8_t>(base_t::Encoded().uint8Zero), base_t::Decoded().uint8Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint8_t>(base_t::Encoded().uint8Positive1), base_t::Decoded().uint8Positive1);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint8_t>(base_t::Encoded().uint8Positive2), base_t::Decoded().uint8Positive2);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int16_t>(base_t::Encoded().int16Zero), base_t::Decoded().int16Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int16_t>(base_t::Encoded().int16Positive), base_t::Decoded().int16Positive);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int16_t>(base_t::Encoded().int16Negative), base_t::Decoded().int16Negative);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint16_t>(base_t::Encoded().uint16Zero), base_t::Decoded().uint16Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint16_t>(base_t::Encoded().uint16Positive1), base_t::Decoded().uint16Positive1);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint16_t>(base_t::Encoded().uint16Positive2), base_t::Decoded().uint16Positive2);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int32_t>(base_t::Encoded().int32Zero), base_t::Decoded().int32Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int32_t>(base_t::Encoded().int32Positive), base_t::Decoded().int32Positive);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int32_t>(base_t::Encoded().int32Negative), base_t::Decoded().int32Negative);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint32_t>(base_t::Encoded().uint32Zero), base_t::Decoded().uint32Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint32_t>(base_t::Encoded().uint32Positive1), base_t::Decoded().uint32Positive1);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint32_t>(base_t::Encoded().uint32Positive2), base_t::Decoded().uint32Positive2);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int64_t>(base_t::Encoded().int64Zero), base_t::Decoded().int64Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int64_t>(base_t::Encoded().int64Positive), base_t::Decoded().int64Positive);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::int64_t>(base_t::Encoded().int64Negative), base_t::Decoded().int64Negative);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint64_t>(base_t::Encoded().uint64Zero), base_t::Decoded().uint64Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint64_t>(base_t::Encoded().uint64Positive1), base_t::Decoded().uint64Positive1);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uint64_t>(base_t::Encoded().uint64Positive2), base_t::Decoded().uint64Positive2);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::float32_t>(base_t::Encoded().float32Zero), base_t::Decoded().float32Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::float32_t>(base_t::Encoded().float32Positive), base_t::Decoded().float32Positive);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::float32_t>(base_t::Encoded().float32Negative), base_t::Decoded().float32Negative);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::float64_t>(base_t::Encoded().float64Zero), base_t::Decoded().float64Zero);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::float64_t>(base_t::Encoded().float64Positive), base_t::Decoded().float64Positive);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::float64_t>(base_t::Encoded().float64Negative), base_t::Decoded().float64Negative);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::property_set_t>(base_t::Encoded().propertySetNone), base_t::Decoded().propertySetNone);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::property_set_t>(base_t::Encoded().propertySetAll), base_t::Decoded().propertySetAll);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::property_set_t>(base_t::Encoded().propertySetMixed1), base_t::Decoded().propertySetMixed1);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::timepoint_t>(base_t::Encoded().timePoint1), base_t::Decoded().timePoint1);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::steady_timepoint_t>(base_t::Encoded().steadyTimePoint1), base_t::Decoded().steadyTimePoint1);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::duration_t>(base_t::Encoded().duration1), base_t::Decoded().duration1);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::duration_t>(base_t::Encoded().duration2), base_t::Decoded().duration2);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::uuid_t>(base_t::Encoded().uuid1), base_t::Decoded().uuid1);
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::string_t>(base_t::Encoded().string1), base_t::Decoded().string1);

    EXPECT_EQ(base_t::serializer_t::template Deserialize<SerializationEnum>(base_t::Encoded().serializationEnum1), base_t::Decoded().serializationEnum1);
}

TYPED_TEST_P(SerializerTestBase, serialize_PropertyArgument)
{
    using base_t = SerializerTestBase<TypeParam>;

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructSimple1_int32Property), base_t::Encoded().serializationStructSimple1_int32Property);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructSimple1_stringProperty), base_t::Encoded().serializationStructSimple1_stringProperty);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructSimple1_float32Property), base_t::Encoded().serializationStructSimple1_float32Property);
}

TYPED_TEST_P(SerializerTestBase, deserialize_PropertyArgument)
{
    using base_t = SerializerTestBase<TypeParam>;

    SerializationStructSimple serializationProperties;
    base_t::serializer_t::Deserialize(base_t::Encoded().int32Positive, serializationProperties.int32Property);
    base_t::serializer_t::Deserialize(base_t::Encoded().string1, serializationProperties.stringProperty);
    base_t::serializer_t::Deserialize(base_t::Encoded().float32Positive, serializationProperties.float32Property);

    EXPECT_EQ(serializationProperties.int32Property, base_t::Decoded().serializationStructSimple1_int32Property);
    EXPECT_EQ(serializationProperties.stringProperty, base_t::Decoded().serializationStructSimple1_stringProperty);
    EXPECT_EQ(serializationProperties.float32Property, base_t::Decoded().serializationStructSimple1_float32Property);
}

TYPED_TEST_P(SerializerTestBase, serialize_VectorArgument)
{
    using base_t = SerializerTestBase<TypeParam>;

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().vectorBool), base_t::Encoded().vectorBool);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().vectorFloat), base_t::Encoded().vectorFloat);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().vectorStructSimple), base_t::Encoded().vectorStructSimple);
}

TYPED_TEST_P(SerializerTestBase, deserialize_VectorArgument)
{
    using base_t = SerializerTestBase<TypeParam>;

    dots::vector_t<dots::bool_t> vectorBool;
    base_t::serializer_t::Deserialize(base_t::Encoded().vectorBool, vectorBool);
    EXPECT_EQ(vectorBool, base_t::Decoded().vectorBool);

    dots::vector_t<dots::float32_t> vectorFloat32;
    base_t::serializer_t::Deserialize(base_t::Encoded().vectorFloat, vectorFloat32);
    EXPECT_EQ(vectorFloat32, base_t::Decoded().vectorFloat);

    dots::vector_t<SerializationStructSimple> vectorStructSimple;
    base_t::serializer_t::Deserialize(base_t::Encoded().vectorStructSimple, vectorStructSimple);
    EXPECT_EQ(vectorStructSimple, base_t::Decoded().vectorStructSimple);
}

TYPED_TEST_P(SerializerTestBase, serialize_SimpleStructArgument)
{
    using base_t = SerializerTestBase<TypeParam>;
    
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructSimple1), base_t::Encoded().serializationStructSimple1_Valid);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructSimple1, dots::property_set_t::All), base_t::Encoded().serializationStructSimple1_All);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructSimple1, SerializationStructSimple::boolProperty_p + SerializationStructSimple::float32Property_p), base_t::Encoded().serializationStructSimple1_Specific);
}

TYPED_TEST_P(SerializerTestBase, deserialize_SimpleStructArgument)
{
    using base_t = SerializerTestBase<TypeParam>;

    SerializationStructSimple serializationStructSimple1;
    base_t::serializer_t::Deserialize(base_t::Encoded().serializationStructSimple1_Valid, serializationStructSimple1);
    EXPECT_EQ(serializationStructSimple1, base_t::Decoded().serializationStructSimple1);

    SerializationStructSimple serializationStructSimple2;
    base_t::serializer_t::Deserialize(base_t::Encoded().serializationStructSimple1_Specific, serializationStructSimple2);
    EXPECT_TRUE(serializationStructSimple2._equal(base_t::Decoded().serializationStructSimple1, SerializationStructSimple::float32Property_p));
}

TYPED_TEST_P(SerializerTestBase, serialize_ComplexStructArgument)
{
    using base_t = SerializerTestBase<TypeParam>;
    
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructComplex1), base_t::Encoded().serializationStructComplex1_Valid);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p), base_t::Encoded().serializationStructComplex1_Specific);

    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructComplex2), base_t::Encoded().serializationStructComplex2_Valid);
    EXPECT_EQ(base_t::serializer_t::Serialize(base_t::Decoded().serializationStructComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p), base_t::Encoded().serializationStructComplex2_Specific);
}

TYPED_TEST_P(SerializerTestBase, deserialize_ComplexStructArgument)
{
    using base_t = SerializerTestBase<TypeParam>;

    SerializationStructComplex serializationStructComplex1_Valid;
    base_t::serializer_t::Deserialize(base_t::Encoded().serializationStructComplex1_Valid, serializationStructComplex1_Valid);
    EXPECT_EQ(serializationStructComplex1_Valid, base_t::Decoded().serializationStructComplex1);

    SerializationStructComplex serializationStructComplex1_Specific;
    base_t::serializer_t::Deserialize(base_t::Encoded().serializationStructComplex1_Specific, serializationStructComplex1_Specific);
    EXPECT_TRUE(serializationStructComplex1_Specific._equal(base_t::Decoded().serializationStructComplex1, SerializationStructComplex::timepointProperty_p + SerializationStructComplex::propertySetProperty_p));

    SerializationStructComplex serializationStructComplex2_Valid;
    base_t::serializer_t::Deserialize(base_t::Encoded().serializationStructComplex2_Valid, serializationStructComplex2_Valid);
    EXPECT_EQ(serializationStructComplex2_Valid, base_t::Decoded().serializationStructComplex2);

    SerializationStructComplex serializationStructComplex_Specific;
    base_t::serializer_t::Deserialize(base_t::Encoded().serializationStructComplex2_Specific, serializationStructComplex_Specific);
    EXPECT_TRUE(serializationStructComplex_Specific._equal(base_t::Decoded().serializationStructComplex2, SerializationStructComplex::enumProperty_p + SerializationStructComplex::durationVectorProperty_p));
}

TYPED_TEST_P(SerializerTestBase, serialize_WriteTupleToContinuousInternalBuffer)
{
    using base_t = SerializerTestBase<TypeParam>;

    typename base_t::serializer_t sut;
    
    {
        EXPECT_EQ(sut.serialize(base_t::Decoded().string1), base_t::Encoded().string1.size());
        EXPECT_EQ(sut.serialize(base_t::Decoded().serializationEnum1), base_t::Encoded().serializationEnum1.size());
        EXPECT_EQ(sut.serialize(base_t::Decoded().vectorBool), base_t::Encoded().vectorBool.size());
        EXPECT_EQ(sut.serialize(base_t::Decoded().serializationStructSimple1), base_t::Encoded().serializationStructSimple1_Valid.size());
    }

    EXPECT_EQ(sut.output(), base_t::Encoded().serializationTuple1);
}

TYPED_TEST_P(SerializerTestBase, deserialize_ReadTupleFromContinuousExternalBuffer)
{
    using base_t = SerializerTestBase<TypeParam>;

    typename base_t::serializer_t sut;
    sut.setInput(base_t::Encoded().serializationTuple1);

    EXPECT_TRUE(sut.inputAvailable());
    
    {
        EXPECT_EQ(sut.template deserialize<std::string>(), base_t::Decoded().string1);
        EXPECT_EQ(sut.lastDeserializeSize(), base_t::Encoded().string1.size());

        EXPECT_EQ(sut.template deserialize<SerializationEnum>(), base_t::Decoded().serializationEnum1);
        EXPECT_EQ(sut.lastDeserializeSize(), base_t::Encoded().serializationEnum1.size());

        EXPECT_EQ(sut.template deserialize<dots::vector_t<dots::bool_t>>(), base_t::Decoded().vectorBool);
        EXPECT_EQ(sut.lastDeserializeSize(), base_t::Encoded().vectorBool.size());

        EXPECT_EQ(sut.template deserialize<SerializationStructSimple>(), base_t::Decoded().serializationStructSimple1);
        EXPECT_EQ(sut.lastDeserializeSize(), base_t::Encoded().serializationStructSimple1_Valid.size());
    }

    EXPECT_FALSE(sut.inputAvailable());
}

REGISTER_TYPED_TEST_SUITE_P(SerializerTestBase, 
    serialize_TypedArgument,
    deserialize_TypedArgument,
    serialize_PropertyArgument,
    deserialize_PropertyArgument,
    serialize_VectorArgument,
    deserialize_VectorArgument,
    serialize_SimpleStructArgument,
    deserialize_SimpleStructArgument,
    serialize_ComplexStructArgument,
    deserialize_ComplexStructArgument,
    serialize_WriteTupleToContinuousInternalBuffer,
    deserialize_ReadTupleFromContinuousExternalBuffer
);