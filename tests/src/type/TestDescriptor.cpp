#include <string>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/Descriptor.h>
#include <DotsTestStruct.dots.h>
#include <DotsTestVectorStruct.dots.h>

using namespace dots::type;
using namespace dots::types;

struct TestDescriptor : ::testing::Test
{
protected:
    std::shared_ptr<Descriptor<int>> m_sutInt = make_descriptor<Descriptor<int>>();
    std::shared_ptr<Descriptor<vector_t<DotsHeader>>> m_sutVectorDotsHeader = make_descriptor<Descriptor<vector_t<DotsHeader>>>();
    std::shared_ptr<Descriptor<vector_t<uint8_t>>> m_sutVectorByte = make_descriptor<Descriptor<vector_t<uint8_t>>>();
    std::shared_ptr<Descriptor<vector_t<vector_t<uint8_t>>>> m_sutVectorVectorByte = make_descriptor<Descriptor<vector_t<vector_t<uint8_t>>>>();
};

TEST_F(TestDescriptor, isFundamental)
{
    EXPECT_TRUE(m_sutInt->isFundamentalType());
    EXPECT_TRUE(m_sutVectorByte->isFundamentalType());
    EXPECT_TRUE(m_sutVectorVectorByte->isFundamentalType());

    EXPECT_FALSE(m_sutVectorDotsHeader->isFundamentalType());

}

TEST_F(TestDescriptor, usesDynamicMemory)
{
    EXPECT_FALSE(Descriptor<bool_t>::Instance().usesDynamicMemory());

    EXPECT_FALSE(Descriptor<int8_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<uint8_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<int16_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<uint16_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<int32_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<uint32_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<int64_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<uint64_t>::Instance().usesDynamicMemory());

    EXPECT_FALSE(Descriptor<float32_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<float64_t>::Instance().usesDynamicMemory());

    EXPECT_FALSE(Descriptor<property_set_t>::Instance().usesDynamicMemory());

    EXPECT_FALSE(Descriptor<timepoint_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<steady_timepoint_t>::Instance().usesDynamicMemory());
    EXPECT_FALSE(Descriptor<duration_t>::Instance().usesDynamicMemory());

    EXPECT_FALSE(Descriptor<uuid_t>::Instance().usesDynamicMemory());
    EXPECT_TRUE(Descriptor<string_t>::Instance().usesDynamicMemory());
    
    EXPECT_TRUE(Descriptor<vector_t<bool_t>>::Instance().usesDynamicMemory());
    EXPECT_TRUE(Descriptor<vector_t<int8_t>>::Instance().usesDynamicMemory());

    EXPECT_FALSE(Descriptor<DotsMt>::Instance().usesDynamicMemory());
    EXPECT_TRUE(Descriptor<vector_t<DotsMt>>::Instance().usesDynamicMemory());

    EXPECT_TRUE(Descriptor<DotsHeader>::Instance().usesDynamicMemory());
    EXPECT_TRUE(Descriptor<vector_t<DotsHeader>>::Instance().usesDynamicMemory());
}

#define EXPECT_DYNAMIC_MEMORY_USAGE                                                                                   \
[](auto value, size_t dynamicMemoryUsage)                                                                             \
{                                                                                                                     \
    EXPECT_EQ(Descriptor<decltype(value)>::Instance().dynamicMemoryUsage(Typeless::From(value)), dynamicMemoryUsage); \
}

TEST_F(TestDescriptor, dynamicMemoryUsage)
{
    EXPECT_DYNAMIC_MEMORY_USAGE(bool_t{ true }, 0);

    EXPECT_DYNAMIC_MEMORY_USAGE(int8_t{ -42 }, 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(uint8_t{ 42 }, 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(int16_t{ -12345 }, 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(uint16_t{ 12345 }, 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(int32_t{ -12345789 }, 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(uint32_t{ 12345789 }, 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(int64_t{ -12345678910111213 }, 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(uint64_t{ 12345678910111213 }, 0);

    EXPECT_DYNAMIC_MEMORY_USAGE(float32_t{ 3.1415f }, 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(float64_t{ -2.71828182846 }, 0);

    EXPECT_DYNAMIC_MEMORY_USAGE(property_set_t{ 0b10101010111111110000000001010101 }, 0);

    EXPECT_DYNAMIC_MEMORY_USAGE(timepoint_t::FromString("2020-03-11T21:07:57.500+00:00"), 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(steady_timepoint_t::FromString("P3DT18H11M42.125S"), 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(duration_t{ 123.456 }, 0);

    EXPECT_DYNAMIC_MEMORY_USAGE(uuid_t::FromString("8c96148e-58bd-11eb-ae93-0242ac130002"), 0);
    EXPECT_DYNAMIC_MEMORY_USAGE(string_t{ "foobar"}, 7);

    EXPECT_DYNAMIC_MEMORY_USAGE(DotsTestEnum::value1, 0);

    EXPECT_DYNAMIC_MEMORY_USAGE(
        DotsTestVectorStruct{
            DotsTestVectorStruct::stringList_i{ vector_t<string_t>{ "foo", "bar", "baz", "qux" } }
        },
        4 * sizeof(string_t) + 4 * 4
    );

    EXPECT_DYNAMIC_MEMORY_USAGE(vector_t<string_t>{ "foo", "bar", "baz", "qux" }, 4 * sizeof(string_t) + 4 * 4);
    EXPECT_DYNAMIC_MEMORY_USAGE(vector_t<bool_t>{ true, false, true }, 3 * sizeof(bool_t));
    EXPECT_DYNAMIC_MEMORY_USAGE(vector_t<float32_t>{ 3.1415f, -2.7183f }, 2 * sizeof(float32_t));
    EXPECT_DYNAMIC_MEMORY_USAGE(vector_t<DotsTestEnum>{ DotsTestEnum::value1, DotsTestEnum::value2, DotsTestEnum::value3 }, 3 * sizeof(DotsMt));

    {
        vector_t<DotsTestSubStruct> subStructVector{
            DotsTestSubStruct{
                DotsTestSubStruct::flag1_i{ false },
            },
            DotsTestSubStruct{
                DotsTestSubStruct::flag1_i{ true },
            }
        };
        constexpr size_t SubStructVectorDynSize = 2 * sizeof(DotsTestSubStruct);
        EXPECT_DYNAMIC_MEMORY_USAGE(subStructVector, SubStructVectorDynSize);

        vector_t<string_t> stringVector{
            "foo", "bar", "baz", "qux"
        };
        constexpr size_t StringVectorDynSize = 4 * sizeof(string_t) + 4 * 4;
        EXPECT_DYNAMIC_MEMORY_USAGE(stringVector, StringVectorDynSize);

        DotsTestVectorStruct vectorStruct1{
            DotsTestVectorStruct::subStructList_i{ subStructVector }
        };
        constexpr size_t VectorStruct1DynSize = SubStructVectorDynSize;
        EXPECT_DYNAMIC_MEMORY_USAGE(vectorStruct1, VectorStruct1DynSize);

        DotsTestVectorStruct vectorStruct2{
            DotsTestVectorStruct::stringList_i{ stringVector }
        };
        constexpr size_t VectorStruct2DynSize = StringVectorDynSize;
        EXPECT_DYNAMIC_MEMORY_USAGE(vectorStruct2, VectorStruct2DynSize);

        EXPECT_DYNAMIC_MEMORY_USAGE(
            vector_t<DotsTestVectorStruct>{
                vectorStruct1,
                vectorStruct2
            },
            2 * sizeof(DotsTestVectorStruct) + VectorStruct1DynSize + VectorStruct2DynSize
        );
    } 
}
