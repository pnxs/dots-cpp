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
using ::testing::Pointee;
using ::testing::Eq;
using ::testing::SafeMatcherCast;
using ::testing::MatcherCast;

using ::testing::ElementsAreArray;

template<typename F, class T>
void expect_publish(F& mock, const T& data)
{
    EXPECT_CALL(mock, publish(T::_td(), MatcherCast<const void*>(SafeMatcherCast<const T*>(Pointee(Eq(data)))), data.validProperties(), false));
}

TEST(TestDistributedTypeId, createId)
{
    dots::MockPublisher mockPublisher;
    dots::onPublishObject = &mockPublisher;

    dots::DistributedTypeId dtid(true); // Master

    const Descriptor* p1 = DotsTestStruct::_td();
    const Descriptor* p2 = DotsTestSubStruct::_td();
    const Descriptor* p3 = dots::type::EnumDescriptorInit<DotsTestEnum>::_td();

    expect_publish(mockPublisher, DotsTypes(1).setName(p1->name()));
    expect_publish(mockPublisher, DotsTypes(2).setName(p2->name()));
    expect_publish(mockPublisher, DotsTypes(3).setName(p3->name()));

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
