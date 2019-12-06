#include "dots/io/DistributedTypeId.h"

#include "DotsTestStruct.dots.h"
#include "DotsTestSubStruct.dots.h"
#include "DotsTestEnum.dots.h"
#include "DotsTypes.dots.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <dots/io/Transceiver.h>

#include "MockPublisher.h"

using namespace dots::type;
using ::testing::Truly;

using ::testing::ElementsAreArray;

template<typename F, class T>
void expect_publish(F& mock, const T& instance)
{
    EXPECT_CALL(mock, publish(&T::_Descriptor(), Truly([&instance](const NewStruct& published){ return published._equal(instance); }), instance._validProperties(), false));
}

TEST(TestDistributedTypeId, createId)
{
    dots::MockPublisher mockPublisher;
    dots::onPublishObject = &mockPublisher;

    dots::DistributedTypeId dtid(true); // Master

    const NewDescriptor<>* p1 = &DotsTestStruct::_Descriptor();
    const NewDescriptor<>* p2 = &DotsTestSubStruct::_Descriptor();
    const NewDescriptor<>* p3 = &NewDescriptor<DotsTestEnum>::Instance();

    DotsTypes t1;
    t1.id = 1;
    t1.name = p1->name();
    expect_publish(mockPublisher, t1);

    DotsTypes t2;
    t2.id = 2;
    t2.name = p2->name();
    expect_publish(mockPublisher, t2);

    DotsTypes t3;
    t3.id = 3;
    t3.name = p3->name();
    expect_publish(mockPublisher, t3);

    EXPECT_EQ(dtid.createTypeId(p1), 1);
    EXPECT_EQ(dtid.createTypeId(p2), 2);
    EXPECT_EQ(dtid.createTypeId(p3), 3);

    // Find by TypeId
    EXPECT_TRUE(dtid.findDescriptor(0) == nullptr);
    EXPECT_TRUE(dtid.findDescriptor(1) == p1);
    EXPECT_TRUE(dtid.findDescriptor(2) == p2);
    EXPECT_TRUE(dtid.findDescriptor(3) == p3);
    EXPECT_TRUE(dtid.findDescriptor(4) == nullptr);

    // Find by Name
    EXPECT_TRUE(dtid.findDescriptor(p1->name()) == p1);
    EXPECT_TRUE(dtid.findDescriptor(p2->name()) == p2);
    EXPECT_TRUE(dtid.findDescriptor(p3->name()) == p3);

    // Find TypeId
    EXPECT_TRUE(dtid.findDescriptor(p1->name()) == p1);
    EXPECT_TRUE(dtid.findDescriptor(p2->name()) == p2);
    EXPECT_TRUE(dtid.findDescriptor(p3->name()) == p3);

    // Find StructDescriptor
    EXPECT_TRUE(dtid.findStructDescriptor(1) == p1);
    EXPECT_TRUE(dtid.findStructDescriptor(2) == p2);
    EXPECT_TRUE(dtid.findStructDescriptor(3) == nullptr); // DotsTestEnum is not an StructDescriptor

    dtid.removeTypeId(2);
    EXPECT_TRUE(dtid.findDescriptor(2) == nullptr);
}
