#pragma once
#include <dots/serialization/TextSerializer.h>
#include <dots/testing/gtest/gtest.h>
#include <serialization/TestSerializer.h>

using dots::serialization::TextOptions;

template <typename TEncoded>
struct TestTextSerializer : TestSerializer<TEncoded>
{
};

TYPED_TEST_SUITE_P(TestTextSerializer);

TYPED_TEST_P(TestTextSerializer, deserialize_FromUnescapedStringArgument)
{
    using base_t = TestTextSerializer<TypeParam>;
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::string_t>(base_t::Encoded().string5Unescaped), base_t::Decoded().string5);

    SerializationStructSimple structSimple1;
    base_t::serializer_t::template Deserialize(base_t::Encoded().string5Unescaped, structSimple1.stringProperty);
    EXPECT_EQ(*structSimple1.stringProperty, base_t::Decoded().string5);

    SerializationStructSimple structSimple2;
    EXPECT_THROW(base_t::serializer_t::template Deserialize(base_t::Encoded().structSimple_String5Unescaped, structSimple2), std::runtime_error);
}

TYPED_TEST_P(TestTextSerializer, serialize_WithOutputStyle)
{
    using base_t = TestTextSerializer<TypeParam>;

    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, TextOptions{ TextOptions::Minimal }), base_t::Encoded().structComplex_MinimalStyle);
    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, TextOptions{ TextOptions::Compact }), base_t::Encoded().structComplex_CompactStyle);
    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, TextOptions{ TextOptions::SingleLine }), base_t::Encoded().structComplex_SingleLineStyle);
    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, TextOptions{ TextOptions::MultiLine }), base_t::Encoded().structComplex_MultiLineStyle);
}

TYPED_TEST_P(TestTextSerializer, deserialize_WithInputPolicy)
{
    using base_t = TestTextSerializer<TypeParam>;
    using serializer_format_t = typename base_t::serializer_t::format_t;

    if constexpr (serializer_format_t::writer_t::format_t::ObjectFormat == dots::serialization::TextFormat::ObjectFormat::WithTypeName)
    {
        {
            SerializationStructComplex serializationStruct;
            base_t::serializer_t::template Deserialize(base_t::Encoded().structComplex_RelaxedPolicy1, serializationStruct);
            EXPECT_TRUE(serializationStruct._equal(base_t::Decoded().structComplex1, SerializationStructComplex::enumProperty_p));
        }

        {
            SerializationStructComplex serializationStruct;
            base_t::serializer_t::template Deserialize(base_t::Encoded().structComplex_RelaxedPolicy2, serializationStruct);
            EXPECT_TRUE(serializationStruct._equal(base_t::Decoded().structComplex1, SerializationStructComplex::enumProperty_p));
        }

        {
            SerializationStructComplex serializationStruct;
            base_t::serializer_t::template Deserialize(base_t::Encoded().structComplex_RelaxedPolicy3, serializationStruct);
            EXPECT_TRUE(serializationStruct._equal(base_t::Decoded().structComplex3, SerializationStructComplex::uint32Property_p));
        }

        {
            SerializationStructComplex serializationStruct;
            base_t::serializer_t::template Deserialize(base_t::Encoded().structComplex_RelaxedPolicy4, serializationStruct);
            EXPECT_TRUE(serializationStruct._equal(base_t::Decoded().structComplex3, SerializationStructComplex::uint32Property_p));
        }

        {
            SerializationStructComplex serializationStruct;
            EXPECT_THROW(base_t::serializer_t::template Deserialize(base_t::Encoded().structComplex_StrictPolicy1, serializationStruct, TextOptions{ TextOptions::SingleLine, TextOptions::Strict }), std::runtime_error);
            EXPECT_THROW(base_t::serializer_t::template Deserialize(base_t::Encoded().structComplex_StrictPolicy2, serializationStruct, TextOptions{ TextOptions::SingleLine, TextOptions::Strict }), std::runtime_error);
            EXPECT_THROW(base_t::serializer_t::template Deserialize(base_t::Encoded().structComplex_StrictPolicy3, serializationStruct, TextOptions{ TextOptions::SingleLine, TextOptions::Strict }), std::runtime_error);
        }
    }
}

REGISTER_TYPED_TEST_SUITE_P(TestTextSerializer,
    deserialize_FromUnescapedStringArgument,
    serialize_WithOutputStyle,
    deserialize_WithInputPolicy
);