#pragma once
#include <dots/testing/gtest/gtest.h>
#include <io/serialization/TestSerializerBase.h>

template <typename TEncoded>
struct TestStringSerializerBase : TestSerializerBase<TEncoded>
{
};

TYPED_TEST_SUITE_P(TestStringSerializerBase);

TYPED_TEST_P(TestStringSerializerBase, deserialize_FromUnescapedStringArgument)
{
    using base_t = TestStringSerializerBase<TypeParam>;
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::string_t>(base_t::Encoded().string5Unescaped), base_t::Decoded().string5);

    SerializationStructSimple structSimple1;
    base_t::serializer_t::template Deserialize(base_t::Encoded().string5Unescaped, structSimple1.stringProperty);
    EXPECT_EQ(*structSimple1.stringProperty, base_t::Decoded().string5);

    SerializationStructSimple structSimple2;
    EXPECT_THROW(base_t::serializer_t::template Deserialize(base_t::Encoded().structSimple_String5Unescaped, structSimple2), std::runtime_error);
}

TYPED_TEST_P(TestStringSerializerBase, serialize_WithOutputStyle)
{
    using base_t = TestStringSerializerBase<TypeParam>;

    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::Minimal }), base_t::Encoded().structComplex_MinimalStyle);
    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::Compact }), base_t::Encoded().structComplex_CompactStyle);
    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::SingleLine }), base_t::Encoded().structComplex_SingleLineStyle);
    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::MultiLine }), base_t::Encoded().structComplex_MultiLineStyle);
}

TYPED_TEST_P(TestStringSerializerBase, deserialize_WithInputPolicy)
{
    using base_t = TestStringSerializerBase<TypeParam>;
    using serializer_traits_t = typename base_t::serializer_t::traits_t;

    if constexpr (serializer_traits_t::UserTypeNames)
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
            EXPECT_THROW(base_t::serializer_t::template Deserialize(base_t::Encoded().structComplex_StrictPolicy1, serializationStruct, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::SingleLine, dots::io::StringSerializerOptions::Strict }), std::runtime_error);
            EXPECT_THROW(base_t::serializer_t::template Deserialize(base_t::Encoded().structComplex_StrictPolicy2, serializationStruct, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::SingleLine, dots::io::StringSerializerOptions::Strict }), std::runtime_error);
        }
    }
}

REGISTER_TYPED_TEST_SUITE_P(TestStringSerializerBase,
    deserialize_FromUnescapedStringArgument,
    serialize_WithOutputStyle,
    deserialize_WithInputPolicy
);