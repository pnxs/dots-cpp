#pragma once
#include <dots/testing/gtest/gtest.h>
#include <io/serialization/TestSerializerBase.h>

template <typename TEncoded>
struct StringSerializerTestBase : SerializerTestBase<TEncoded>
{
};

TYPED_TEST_SUITE_P(StringSerializerTestBase);

TYPED_TEST_P(StringSerializerTestBase, deserialize_FromUnescapedStringArgument)
{
    using base_t = StringSerializerTestBase<TypeParam>;
    EXPECT_EQ(base_t::serializer_t::template Deserialize<dots::string_t>(base_t::Encoded().string5Unescaped), base_t::Decoded().string5);

    SerializationStructSimple structSimple1;
    base_t::serializer_t::template Deserialize(base_t::Encoded().string5Unescaped, structSimple1.stringProperty);
    EXPECT_EQ(*structSimple1.stringProperty, base_t::Decoded().string5);

    SerializationStructSimple structSimple2;
    EXPECT_THROW(base_t::serializer_t::template Deserialize(base_t::Encoded().structSimple_String5Unescaped, structSimple2), std::runtime_error);
}

TYPED_TEST_P(StringSerializerTestBase, serialize_TupleToContinuousInternalBuffer)
{
    using base_t = StringSerializerTestBase<TypeParam>;
    typename base_t::serializer_t sut;

    sut.serializeTupleBegin();
    {
        sut.serialize(base_t::Decoded().string1);
        sut.serialize(base_t::Decoded().enum1);
        sut.serialize(base_t::Decoded().vectorBool);
        sut.serialize(base_t::Decoded().structSimple1);
    }
    sut.serializeTupleEnd();

    EXPECT_EQ(sut.output(), base_t::Encoded().serializationTuple1WithBeginEnd);
}

TYPED_TEST_P(StringSerializerTestBase, deserialize_TupleFromContinuousExternalBuffer)
{
    using base_t = StringSerializerTestBase<TypeParam>;
    typename base_t::serializer_t sut;

    sut.setInput(base_t::Encoded().serializationTuple1WithBeginEnd);
    EXPECT_TRUE(sut.inputAvailable());

    sut.deserializeTupleBegin();
    {
        EXPECT_EQ(sut.template deserialize<std::string>(), base_t::Decoded().string1);
        EXPECT_EQ(sut.template deserialize<SerializationEnum>(), base_t::Decoded().enum1);
        EXPECT_EQ(sut.template deserialize<dots::vector_t<dots::bool_t>>(), base_t::Decoded().vectorBool);
        EXPECT_EQ(sut.template deserialize<SerializationStructSimple>(), base_t::Decoded().structSimple1);
    }
    sut.deserializeTupleEnd();

    EXPECT_FALSE(sut.inputAvailable());
}

TYPED_TEST_P(StringSerializerTestBase, serialize_WithOutputStyle)
{
    using base_t = StringSerializerTestBase<TypeParam>;

    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::Minimal }), base_t::Encoded().structComplex_MinimalStyle);
    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::Compact }), base_t::Encoded().structComplex_CompactStyle);
    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::SingleLine }), base_t::Encoded().structComplex_SingleLineStyle);
    EXPECT_EQ(base_t::serializer_t::template Serialize(base_t::Decoded().structComplex1, dots::io::StringSerializerOptions{ dots::io::StringSerializerOptions::MultiLine }), base_t::Encoded().structComplex_MultiLineStyle);
}

TYPED_TEST_P(StringSerializerTestBase, deserialize_WithInputPolicy)
{
    using base_t = StringSerializerTestBase<TypeParam>;
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

REGISTER_TYPED_TEST_SUITE_P(StringSerializerTestBase,
    deserialize_FromUnescapedStringArgument,
    serialize_TupleToContinuousInternalBuffer,
    deserialize_TupleFromContinuousExternalBuffer,
    serialize_WithOutputStyle,
    deserialize_WithInputPolicy
);