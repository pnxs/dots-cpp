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