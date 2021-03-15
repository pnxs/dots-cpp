#include <gtest/gtest.h>
#include <dots/io/serialization/SerializerBase.h>
#include <io/serialization/data/serialization_data.h>

struct TestSerializer : dots::io::SerializerBase<std::vector<uint8_t>, TestSerializer>
{
    using data_t = std::vector<uint8_t>;

    TestSerializer() = default;
    TestSerializer(const TestSerializer& other) = default;
    TestSerializer(TestSerializer&& other) = default;
    ~TestSerializer() = default;

    TestSerializer& operator = (const TestSerializer& rhs) = default;
    TestSerializer& operator = (TestSerializer&& rhs) = default;

protected:

    friend TypeVisitor<TestSerializer>;

    template <typename T>
    bool visitStructBeginDerived(const T& instance, dots::property_set_t&/* includedProperties*/)
    {
        std::fill_n(std::back_inserter(output()), sizeof(instance), 0x00);
        return false;
    }

    template <typename T>
    bool visitPropertyBeginDerived(const T&/* property*/, bool/* first*/)
    {
        std::fill_n(std::back_inserter(output()), sizeof(typename T::value_t), 0x00);
        return false;
    }

    template <typename T>
    bool visitVectorBeginDerived(const dots::vector_t<T>& vector, const dots::type::Descriptor<dots::vector_t<T>>&/* descriptor*/)
    {
        std::fill_n(std::back_inserter(output()), sizeof(vector), 0x00);
        return false;
    }

    template <typename T>
    void visitEnumDerived(const T& value, const dots::type::EnumDescriptor<T>&/* descriptor*/)
    {
        std::fill_n(std::back_inserter(output()), sizeof(value), 0x00);
    }

    template <typename T>
    void visitFundamentalTypeDerived(const T& value, const dots::type::Descriptor<T>&/* descriptor*/)
    {
        std::fill_n(std::back_inserter(output()), sizeof(value), 0x00);
    }

    template <typename T>
    bool visitStructBeginDerived(T& instance, dots::property_set_t&/* includedProperties*/)
    {
        inputData() += sizeof(instance);
        return false;
    }

    template <typename T>
    bool visitPropertyBeginDerived(T&/* property*/, bool/* first*/)
    {
        inputData() += sizeof(typename T::value_t);
        return false;
    }

    template <typename T>
    bool visitVectorBeginDerived(dots::vector_t<T>& vector, const dots::type::Descriptor<dots::vector_t<T>>&/* descriptor*/)
    {
        inputData() += sizeof(vector);
        return false;
    }

    template <typename T>
    void visitEnumDerived(T& value, const dots::type::EnumDescriptor<T>&/* descriptor*/)
    {
        inputData() += sizeof(value);
    }

    template <typename T>
    void visitFundamentalTypeDerived(T& value, const dots::type::Descriptor<T>&/* descriptor*/)
    {
        inputData() += sizeof(value);
    }
};

struct TestSerializerBase : ::testing::Test, TestSerializationInput
{
protected:

    using data_t = TestSerializer::data_t;
};

TEST_F(TestSerializerBase, serialize_WriteToFreshInternalBuffer)
{
    EXPECT_EQ(TestSerializer{}.serialize(String1).size(), sizeof(String1));
    EXPECT_EQ(TestSerializer{}.serialize(SerializationEnum1).size(), sizeof(SerializationEnum1));
    EXPECT_EQ(TestSerializer{}.serializeProperty(SerializationStructSimple1.int32Property).size(), sizeof(SerializationStructSimple1.int32Property));
    EXPECT_EQ(TestSerializer{}.serializeVector(VectorBool).size(), sizeof(VectorBool));
    EXPECT_EQ(TestSerializer{}.serializeStruct(SerializationStructSimple1).size(), sizeof(SerializationStructSimple1));
}

TEST_F(TestSerializerBase, deserialize_ReadFromFreshExternalBuffer)
{
    std::string s;
    EXPECT_EQ(TestSerializer{}.deserialize(data_t(sizeof(s), 0x00), s), sizeof(s));

    SerializationEnum e;
    EXPECT_EQ(TestSerializer{}.deserialize(data_t(sizeof(e), 0x00), e), sizeof(e));

    SerializationStructSimple serializationStructSimple1;
    EXPECT_EQ(TestSerializer{}.deserializeProperty(data_t(sizeof(serializationStructSimple1.int32Property), 0x00), serializationStructSimple1.int32Property), sizeof(serializationStructSimple1.int32Property));

    dots::vector_t<dots::bool_t> v;
    EXPECT_EQ(TestSerializer{}.deserializeVector(data_t(sizeof(v), 0x00), v), sizeof(v));

    SerializationStructSimple serializationStructSimple2;
    EXPECT_EQ(TestSerializer{}.deserializeStruct(data_t(sizeof(serializationStructSimple2), 0x00), serializationStructSimple2), sizeof(serializationStructSimple2));
}