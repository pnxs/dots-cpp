#pragma once
#include <SerializationStructSimple.dots.h>
#include <SerializationStructComplex.dots.h>

using namespace dots::type::literals;

struct TestSerializationInput
{
    static constexpr dots::bool_t BoolFalse{ false };
    static constexpr dots::bool_t BoolTrue{ true };

    static constexpr dots::int8_t Int8Zero{ 0 };
    static constexpr dots::int8_t Int8Positive{ 42 };
    static constexpr dots::int8_t Int8Negative{ -42 };

    static constexpr dots::uint8_t Uint8Zero{ 0u };
    static constexpr dots::uint8_t Uint8Positive1{ 42u };
    static constexpr dots::uint8_t Uint8Positive2{ 170u };

    static constexpr dots::int16_t Int16Zero{ 0 };
    static constexpr dots::int16_t Int16Positive{ 12345 };
    static constexpr dots::int16_t Int16Negative{ -12345 };

    static constexpr dots::uint16_t Uint16Zero{ 0u };
    static constexpr dots::uint16_t Uint16Positive1{ 12345u };
    static constexpr dots::uint16_t Uint16Positive2{ 45113u };

    static constexpr dots::int32_t Int32Zero{ 0 };
    static constexpr dots::int32_t Int32Positive{ 12345789 };
    static constexpr dots::int32_t Int32Negative{ -12345789 };

    static constexpr dots::uint32_t Uint32Zero{ 0u };
    static constexpr dots::uint32_t Uint32Positive1{ 12345789u };
    static constexpr dots::uint32_t Uint32Positive2{ 2159829437u };

    static constexpr dots::int64_t Int64Zero{ 0 };
    static constexpr dots::int64_t Int64Positive{ 12345678910111213 };
    static constexpr dots::int64_t Int64Negative{ -12345678910111213 };

    static constexpr dots::uint64_t Uint64Zero{ 0u };
    static constexpr dots::uint64_t Uint64Positive1{ 12345678910111213u };
    static constexpr dots::uint64_t Uint64Positive2{ 18434398394799440403u };

    static constexpr dots::float32_t Float32Zero{ 0.0f };
    static constexpr dots::float32_t Float32Positive{ 3.1415f };
    static constexpr dots::float32_t Float32Negative{ -2.7183f };

    static constexpr dots::float64_t Float64Zero{ 0.0f };
    static constexpr dots::float64_t Float64Positive{ 3.14159265359 };
    static constexpr dots::float64_t Float64Negative{ -2.71828182846 };

    static constexpr dots::property_set_t PropertySetNone{ dots::property_set_t::None };
    static constexpr dots::property_set_t PropertySetAll{ dots::property_set_t::All };
    static constexpr dots::property_set_t PropertySetMixed1{ 0b10101010111111110000000001010101 };

    inline static dots::timepoint_t TimePoint1{ dots::timepoint_t::FromString("2020-03-11T21:07:57.500+00:00") };
    inline static dots::steady_timepoint_t SteadyTimePoint1{ dots::steady_timepoint_t::FromString("P3DT18H11M42.125S") };

    static constexpr dots::duration_t Duration1{ 123.456 };
    static constexpr dots::duration_t Duration2{ 5min + 42s + 73ms };

    inline static dots::uuid_t Uuid1{ dots::uuid_t::FromString("8c96148e-58bd-11eb-ae93-0242ac130002") };

    inline static dots::string_t String1{ "foobar" };
    inline static dots::string_t String2{ "\"foo\" bar baz" };
    inline static dots::string_t String3{ "foo \"bar\" baz" };
    inline static dots::string_t String4{ "foo bar \"baz\"" };
    inline static dots::string_t String5{ u8"foo\\ \u0062\u0061\u0072\u00A9\n b\\az" };

    inline static SerializationEnum SerializationEnum1{ SerializationEnum::baz };

    /*inline static dots::vector_t<dots::bool_t> VectorBool{ BoolTrue, BoolFalse, BoolFalse };
    inline static dots::vector_t<dots::float32_t> VectorFloat{ Float32Positive, Float32Negative };

    inline static dots::vector_t<SerializationStructSimple> VectorStructSimple{
        SerializationStructSimple{
            SerializationStructSimple::int32Property_i{ Int32Positive }
        },
        SerializationStructSimple{
            SerializationStructSimple::boolProperty_i{ BoolFalse }
        }
    };

    inline static SerializationStructSimple SerializationStructSimple1{
        SerializationStructSimple::int32Property_i{ Int32Positive },
        SerializationStructSimple::stringProperty_i{ String1 },
        SerializationStructSimple::float32Property_i{ Float32Positive }
    };

    inline static SerializationStructComplex SerializationStructComplex1{
        SerializationStructComplex::enumProperty_i{ SerializationEnum1 },
        SerializationStructComplex::float64Property_i{ Float64Negative },
        SerializationStructComplex::timepointProperty_i{ TimePoint1 },
        SerializationStructComplex::structSimpleProperty_i{
            SerializationStructSimple::boolProperty_i{ BoolFalse }
        }
    };

    inline static SerializationStructComplex SerializationStructComplex2{
        SerializationStructComplex::propertySetProperty_i{ PropertySetMixed1 },
        SerializationStructComplex::durationVectorProperty_i{ dots::vector_t<dots::duration_t>{ Duration1, Duration2 } },
        SerializationStructComplex::uuidProperty_i{ Uuid1 }
    };*/

    inline static dots::vector_t<dots::bool_t> VectorBool{ BoolTrue, BoolFalse, BoolFalse };
    inline static dots::vector_t<dots::float32_t> VectorFloat{ Float32Positive, Float32Negative };

    dots::vector_t<SerializationStructSimple> VectorStructSimple{
        SerializationStructSimple{
            SerializationStructSimple::int32Property_i{ Int32Positive }
        },
        SerializationStructSimple{
            SerializationStructSimple::boolProperty_i{ BoolFalse }
        }
    };

    SerializationStructSimple SerializationStructSimple1{
        SerializationStructSimple::int32Property_i{ Int32Positive },
        SerializationStructSimple::stringProperty_i{ String1 },
        SerializationStructSimple::float32Property_i{ Float32Positive }
    };

    SerializationStructComplex SerializationStructComplex1{
        SerializationStructComplex::enumProperty_i{ SerializationEnum1 },
        SerializationStructComplex::float64Property_i{ Float64Negative },
        SerializationStructComplex::timepointProperty_i{ TimePoint1 },
        SerializationStructComplex::structSimpleProperty_i{
            SerializationStructSimple::boolProperty_i{ BoolFalse }
        }
    };

    SerializationStructComplex SerializationStructComplex2{
        SerializationStructComplex::propertySetProperty_i{ PropertySetMixed1 },
        SerializationStructComplex::durationVectorProperty_i{ dots::vector_t<dots::duration_t>{ Duration1, Duration2 } },
        SerializationStructComplex::uuidProperty_i{ Uuid1 }
    };
};