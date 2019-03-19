#include "dots/type/StructDescriptor.h"
#include "dots/type/VectorDescriptor.h"
#include "dots/type/Registry.h"
#include "DotsTestVectorStruct.dots.h"
#include "DotsTestStruct.dots.h"
#include "StructDescriptorData.dots.h"
#include <gtest/gtest.h>

using namespace dots::type;


TEST(TestVectorDescriptor, construct)
{
    DotsTestVectorStruct vectorStruct;
    //auto testVectorType = type::get(vectorStruct.intList());

    auto newVector = VectorDescriptor::createDescriptor("int32");
    ASSERT_TRUE(newVector != nullptr);

    DotsTestVectorStruct ts;
    auto& intList = ts.intList();

    intList.push_back(1);
    intList.push_back(2);
    intList.push_back(4);
    intList.push_back(8);

    EXPECT_EQ(ts.intList->size(), 4u);

    auto vectorInstance = newVector->New();

    newVector->swap(vectorInstance, &ts.intList);

    EXPECT_EQ(ts.intList->size(), 0u);
    ASSERT_EQ(newVector->get_size(vectorInstance), 4u);

    EXPECT_EQ(newVector->vtd()->to_string(newVector->get_data(vectorInstance, 0)), "1");
    EXPECT_EQ(newVector->vtd()->to_string(newVector->get_data(vectorInstance, 1)), "2");
    EXPECT_EQ(newVector->vtd()->to_string(newVector->get_data(vectorInstance, 2)), "4");
    EXPECT_EQ(newVector->vtd()->to_string(newVector->get_data(vectorInstance, 3)), "8");

    newVector->Delete(vectorInstance);
}

TEST(TestVectorDescriptor, constructSubStruct)
{
    DotsTestVectorStruct vectorStruct;
    //auto testVectorType = type::get(vectorStruct.subStructList());

    auto newVector = VectorDescriptor::createDescriptor("DotsTestSubStruct");
    ASSERT_TRUE(newVector != nullptr);

    DotsTestVectorStruct ts;
    auto& ssList = ts.subStructList();

    DotsTestSubStruct s;
    s.flag1(true);

    ssList.push_back(s);
    s.flag1 = false;
    ssList.push_back(s);
    ssList.push_back(s);
    s.flag1 = true;
    ssList.push_back(s);

    ASSERT_EQ(ssList.size(), 4u);

    auto vectorInstance = newVector->New();

    newVector->swap(vectorInstance, &ts.subStructList);

    ts.intList();
    EXPECT_EQ(ts.intList->size(), 0u);
    ASSERT_EQ(newVector->get_size(vectorInstance), 4u);

    ASSERT_TRUE(dynamic_cast<const StructDescriptor*>(newVector->vtd()) != nullptr);

    auto subStructDesc = dynamic_cast<const StructDescriptor*>(newVector->vtd());

    ASSERT_EQ(subStructDesc->properties().size(), 1u);

    auto& prop = subStructDesc->properties()[0];
    EXPECT_EQ(prop.td()->to_string(prop.address(newVector->get_data(vectorInstance, 0))), "true");
    EXPECT_EQ(prop.td()->to_string(prop.address(newVector->get_data(vectorInstance, 1))), "false");
    EXPECT_EQ(prop.td()->to_string(prop.address(newVector->get_data(vectorInstance, 2))), "false");
    EXPECT_EQ(prop.td()->to_string(prop.address(newVector->get_data(vectorInstance, 3))), "true");

    newVector->Delete(vectorInstance);
}

TEST(TestVectorDescriptor, registerDynamic)
{
    // For this thest the following type must not exist (because it will be dynamically registered in this test).
    //ASSERT_FALSE(rttr::type::get_by_name("dots::Vector<dots::types::DotsTestStruct>").is_valid());

    auto newVector = VectorDescriptor::createDescriptor("DotsTestStruct");
    EXPECT_TRUE(newVector != nullptr);

    // Now it should exist.
    //EXPECT_TRUE(rttr::type::get_by_name("dots::Vector<dots::types::DotsTestStruct>").is_valid());

    //listRttrTypes_if([](auto& t) { return t.is_array(); });
    //listRttrTypes_if();

    EXPECT_EQ(newVector->name(), "vector<DotsTestStruct>");
    //EXPECT_EQ(newVector->name(), "dots::Vector<dots::types::DotsTestStruct>");

    // Test access via VectorDescriptor
    DotsTestStruct testStruct;
    testStruct.indKeyfField(1);

    auto vectorInstance = newVector->New();

    newVector->resize(vectorInstance, 2);
    {
        DotsTestStruct *vtp = (DotsTestStruct *) newVector->get_data(vectorInstance, 0);
        vtp->indKeyfField(1);

        vtp = (DotsTestStruct *) newVector->get_data(vectorInstance, 1);
        vtp->indKeyfField(2);
    }

    EXPECT_EQ(newVector->get_size(vectorInstance), 2u);

    newVector->Delete(vectorInstance);
}

TEST(TestVectorDescriptor, registerDynamicWithoutVectorDescriptor)
{
    {
        //auto testSd = Registry::toStructDescriptorData(type::get<DotsTestSubStruct>());
        auto testSd = DotsTestSubStruct::_Descriptor().descriptorData();
        testSd.name = "DotsTestSubStructX2";
        StructDescriptor::createFromStructDescriptorData(testSd);
    }

    // Create dynamically registered type "DotsTestStructX" from DotsTestStruct
    auto sd = DotsTestVectorStruct::_Descriptor().descriptorData();

    sd.name = "DotsTestVectorStructX2";
    sd.properties->at(1).type = "vector<DotsTestSubStructX2>";

    //auto sdx = StructDescriptor::createFromStructDescriptorData(sd);

}