#include <string>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/Descriptor.h>

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