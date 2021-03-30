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
    friend dots::io::SerializerBase<std::vector<uint8_t>, TestSerializer>;

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

    void serializePackBeginDerived()
    {
        std::fill_n(std::back_inserter(output()), 1, 0x00);
    }
    
    void serializePackElementBeginDerived(size_t/* index*/)
    {
        std::fill_n(std::back_inserter(output()), 1, 0x00);
    }

    void serializePackElementEndDerived(size_t/* index*/)
    {
        std::fill_n(std::back_inserter(output()), 1, 0x00);
    }
    
    void serializePackEndDerived()
    {
        std::fill_n(std::back_inserter(output()), 1, 0x00);
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

    void deserializePackBeginDerived()
    {
        inputData() += 1;
    }
    
    void deserializePackElementBeginDerived(size_t/* index*/)
    {
        inputData() += 1;
    }

    void deserializePackElementEndDerived(size_t/* index*/)
    {
        inputData() += 1;
    }
    
    void deserializePackEndDerived()
    {
        inputData() += 1;
    }
};

struct TestSerializerBase : ::testing::Test, TestSerializationInput
{
protected:

    using data_t = TestSerializer::data_t;
};

TEST_F(TestSerializerBase, serialize_WriteToFreshInternalBuffer)
{
    EXPECT_EQ(TestSerializer::Serialize(String1).size(), sizeof(String1));
    EXPECT_EQ(TestSerializer::Serialize(SerializationEnum1).size(), sizeof(SerializationEnum1));
    EXPECT_EQ(TestSerializer::Serialize(SerializationStructSimple1.int32Property).size(), sizeof(SerializationStructSimple1.int32Property));
    EXPECT_EQ(TestSerializer::Serialize(VectorBool).size(), sizeof(VectorBool));
    EXPECT_EQ(TestSerializer::Serialize(SerializationStructSimple1).size(), sizeof(SerializationStructSimple1));
}

TEST_F(TestSerializerBase, deserialize_ReadFromFreshExternalBuffer)
{
    std::string s;
    EXPECT_EQ(TestSerializer::Deserialize(data_t(sizeof(s), 0x00), s), sizeof(s));

    SerializationEnum e;
    EXPECT_EQ(TestSerializer::Deserialize(data_t(sizeof(e), 0x00), e), sizeof(e));

    SerializationStructSimple serializationStructSimple1;
    EXPECT_EQ(TestSerializer::Deserialize(data_t(sizeof(serializationStructSimple1.int32Property), 0x00), serializationStructSimple1.int32Property), sizeof(serializationStructSimple1.int32Property));

    dots::vector_t<dots::bool_t> v;
    EXPECT_EQ(TestSerializer::Deserialize(data_t(sizeof(v), 0x00), v), sizeof(v));

    SerializationStructSimple serializationStructSimple2;
    EXPECT_EQ(TestSerializer::Deserialize(data_t(sizeof(serializationStructSimple2), 0x00), serializationStructSimple2), sizeof(serializationStructSimple2));
}

TEST_F(TestSerializerBase, serialize_WriteToContinuousInternalBuffer)
{
    TestSerializer sut;
    
    EXPECT_EQ(sut.serialize(String1), sizeof(String1));
    EXPECT_EQ(sut.serialize(SerializationEnum1), sizeof(SerializationEnum1));
    EXPECT_EQ(sut.serialize(SerializationStructSimple1.int32Property), sizeof(SerializationStructSimple1.int32Property));
    EXPECT_EQ(sut.serialize(VectorBool), sizeof(VectorBool));
    EXPECT_EQ(sut.serialize(SerializationStructSimple1), sizeof(SerializationStructSimple1));
    EXPECT_EQ(sut.output().size(), sizeof(String1) + sizeof(SerializationEnum1) + sizeof(SerializationStructSimple1.int32Property) + sizeof(VectorBool) + sizeof(SerializationStructSimple1));
}

TEST_F(TestSerializerBase, serialize_WriteToContinuousInternalBufferFromTuple)
{
    TestSerializer sut;

    sut.serializePackBegin();
    {
        sut.serialize(String1);
        sut.serialize(SerializationEnum1);
        sut.serialize(VectorBool);
        sut.serialize(SerializationStructSimple1);
    }
    sut.serializePackEnd();
    EXPECT_EQ(sut.output().size(), sizeof(String1) + sizeof(SerializationEnum1) + sizeof(VectorBool) + sizeof(SerializationStructSimple1) + 2 + 8);
}

TEST_F(TestSerializerBase, deserialize_ReadFromContinuousExternalBuffer)
{
    TestSerializer sut;
    std::string s;
    SerializationEnum e;
    SerializationStructSimple serializationStructSimple1;
    dots::vector_t<dots::bool_t> v;
    SerializationStructSimple serializationStructSimple2;
    data_t input(sizeof(s) + sizeof(e) + sizeof(serializationStructSimple1.int32Property) + sizeof(v) + sizeof(serializationStructSimple2), 0x00);
    sut.setInput(input);

    EXPECT_TRUE(sut.inputAvailable());
    EXPECT_EQ(sut.deserialize(s), sizeof(s));

    EXPECT_TRUE(sut.inputAvailable());
    EXPECT_EQ(sut.deserialize(e), sizeof(e));

    EXPECT_TRUE(sut.inputAvailable());
    EXPECT_EQ(sut.deserialize(serializationStructSimple1.int32Property), sizeof(serializationStructSimple1.int32Property));

    EXPECT_TRUE(sut.inputAvailable());
    EXPECT_EQ(sut.deserialize(v), sizeof(v));

    EXPECT_TRUE(sut.inputAvailable());
    EXPECT_EQ(sut.deserialize(serializationStructSimple2), sizeof(serializationStructSimple2));

    EXPECT_FALSE(sut.inputAvailable());
}

TEST_F(TestSerializerBase, deserialize_ReadFromContinuousExternalBufferToTuple)
{
    TestSerializer sut;
    std::string s;
    SerializationEnum e;
    dots::vector_t<dots::bool_t> v;
    SerializationStructSimple serializationStructSimple;
    data_t input(sizeof(s) + sizeof(e) + sizeof(v) + sizeof(serializationStructSimple) + 2 + 8, 0x00);
    sut.setInput(input);
    
    sut.deserializePackBegin();
    {
        sut.deserialize(s);
        sut.deserialize(e);
        sut.deserialize(v);
        sut.deserialize(serializationStructSimple);
    }
    sut.deserializePackEnd();

    EXPECT_FALSE(sut.inputAvailable());
}

TEST_F(TestSerializerBase, deserialize_ReadFromContinuousExternalBufferToNewTuple)
{
    TestSerializer sut;
    data_t input(sizeof(std::string) + sizeof(SerializationEnum) + sizeof(dots::vector_t<dots::bool_t>) + sizeof(SerializationStructSimple) + 2 + 8, 0x00);
    sut.setInput(input);

    sut.deserializePackBegin();
    {
        sut.deserialize<std::string>();
        sut.deserialize<SerializationEnum>();
        sut.deserialize<dots::vector_t<dots::bool_t>>();
        sut.deserialize<SerializationStructSimple>();
    }
    sut.deserializePackEnd();

    EXPECT_FALSE(sut.inputAvailable());
}