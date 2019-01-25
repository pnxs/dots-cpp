#include "dots/io/AnyContainer.h"
#include "DotsTestStruct.dots.h"
#include "DotsUncachedTestStruct.dots.h"
#include "dots/type/Registry.h"
#include <gtest/gtest.h>

#include <iostream>

using namespace dots::type;

/**
 * Add two instances of a DotsTestStruct to an AnyContainer and
 * remove one of them.
 */
TEST(TestAnyContainer, storeAndRemove)
{
    DotsTestStruct dts;

    // SUT: Container of DotsTestStruct
    dots::AnyContainer container(dts._td());
    ASSERT_EQ(container.size(), 0u);

    dts.setIndKeyfField(1);

    DotsHeader dh;
    dh.setTypeName(dts._td()->name());
    dh.setRemoveObj(false);
    dh.setSender(0);
    dh.setSentTime(pnxs::SystemNow());
    dh.setAttributes(dts.validProperties());

    // Process a DotsTestStruct 'create' in container
    container.process(dh, &dts);

    ASSERT_EQ(container.size(), 1u);

    // Modify Key property, so that it will be another instance
    dts.setIndKeyfField(2);

    container.process(dh, &dts);

    EXPECT_EQ(container.size(), 2u);

    dts.setIndKeyfField(2); // Set field again, because the container moves the values into it.
    dh.setRemoveObj(true);

    // Remove DotsTestStruct with Key==2 from container
    container.process(dh, &dts);

    EXPECT_EQ(container.size(), 1u);
}

TEST(TestAnyContainer, storeUpdateAndRemoveSignal)
{
    //DotsTestStruct dts;

    uint32_t timesSigCalled = 0;

    std::function<void (const dots::AnyContainerCbd& cbd)> expectCheck = [](auto&){
        FAIL();
    };

    dots::AnyContainer::signal_type sig;
    sig.connect([&](const dots::AnyContainerCbd& cbd) {
        timesSigCalled++;
        expectCheck(cbd);
    });

    dots::AnyContainer container(DotsTestStruct::_td());
    ASSERT_EQ(container.size(), 0u);

    auto t1 = pnxs::SystemNow();

    {
        DotsTestStruct dts;

        // Add 1st element
        dts.setIndKeyfField(1);
        dts.setStringField("Hello");

        // Fill DotsHeader data
        DotsHeader dh;
        dh.setTypeName(dts._td()->name());
        dh.setRemoveObj(false);
        dh.setSender(0);
        dh.setSentTime(t1);
        dh.setAttributes(dts.validProperties());

        // Expect call of signal-handler with:
        // data is heap pointer
        expectCheck = [&](const dots::AnyContainerCbd &cbd)
        {
            auto testStruct = reinterpret_cast<const DotsTestStruct *>(cbd.element.data);

            ASSERT_TRUE(cbd.receivedData != nullptr);
            ASSERT_TRUE(cbd.element.data != nullptr);

            EXPECT_EQ(cbd.element.information.lastOperation(), dots::Mt::create);
            EXPECT_EQ(cbd.element.information.lastUpdateFrom(), 0);
            EXPECT_EQ(cbd.element.information.created(), t1);
            EXPECT_EQ(cbd.element.information.createdFrom(), 0);
            EXPECT_EQ(cbd.element.information.modified(), t1);
            //EXPECT_EQ(cbd.element.information.localUpdateTime, ); // Is not good testable, because the current time is used.
            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.td, dts._td());
            EXPECT_EQ(cbd.mt, dots::Mt::create);

            ASSERT_TRUE(testStruct->hasStringField());
            EXPECT_EQ(testStruct->stringField(), "Hello");
        };
        container.process(dh, &dts, &sig);
        ASSERT_EQ(container.size(), 1u);
        EXPECT_EQ(timesSigCalled, 1u);
    }

    auto t2 = pnxs::SystemNow();

    {
        DotsTestStruct dts;

        // Add 2nd
        dts.setIndKeyfField(2);
        dts.setStringField("World");

        // Fill DotsHeader data
        DotsHeader dh;
        dh.setTypeName(dts._td()->name());
        dh.setRemoveObj(false);
        dh.setSender(1);
        dh.setSentTime(t2);
        dh.setAttributes(dts.validProperties());

        expectCheck = [&](const dots::AnyContainerCbd &cbd)
        {
            ASSERT_TRUE(cbd.receivedData != nullptr);
            ASSERT_TRUE(cbd.element.data != nullptr);

            EXPECT_EQ(cbd.element.information.lastOperation(), dots::Mt::create);
            EXPECT_EQ(cbd.element.information.lastUpdateFrom(), 1);
            EXPECT_EQ(cbd.element.information.created(), t2);
            EXPECT_EQ(cbd.element.information.createdFrom(), 1);
            EXPECT_EQ(cbd.element.information.modified(), t2);
            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.td, dts._td());
            EXPECT_EQ(cbd.mt, dots::Mt::create);
        };
        container.process(dh, &dts, &sig);
        EXPECT_EQ(container.size(), 2u);
        EXPECT_EQ(timesSigCalled, 2u);
    }

    {
        DotsTestStruct dts;

        // Remove
        DotsTestStruct removeObj(2);

        // Fill DotsHeader data
        DotsHeader dh;
        dh.setTypeName(dts._td()->name());
        dh.setRemoveObj(true);
        dh.setSender(1);
        dh.setSentTime(t2);
        dh.setAttributes(dts.validProperties());

        expectCheck = [&](const dots::AnyContainerCbd &cbd)
        {
            ASSERT_TRUE(cbd.receivedData != nullptr);
            ASSERT_TRUE(cbd.element.data != nullptr);
            EXPECT_EQ(cbd.element.information.lastOperation(), dots::Mt::remove);
            EXPECT_EQ(cbd.element.information.lastUpdateFrom(), 1);
            EXPECT_EQ(cbd.element.information.created(), t2);
            EXPECT_EQ(cbd.element.information.createdFrom(), 1);
            EXPECT_EQ(cbd.element.information.modified(), t2);
            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.td, dts._td());
            EXPECT_EQ(cbd.mt, dots::Mt::remove);

            auto testStruct = reinterpret_cast<const DotsTestStruct *>(cbd.element.data);
            ASSERT_TRUE(testStruct->hasStringField());
            EXPECT_EQ(testStruct->stringField(), "World");
        };
        container.process(dh, &removeObj, &sig);
        EXPECT_EQ(container.size(), 1u);
        EXPECT_EQ(timesSigCalled, 3u);
    }

    auto t3 = pnxs::SystemNow();

    {
        DotsTestStruct dts;

        // Update 1st
        dts.setIndKeyfField(1);
        dts.setFloatField(3);

        // Fill DotsHeader data
        DotsHeader dh;
        dh.setTypeName(dts._td()->name());
        dh.setRemoveObj(false);
        dh.setSender(2);
        dh.setSentTime(t3);
        dh.setAttributes(dts.validProperties());

        expectCheck = [&](const dots::AnyContainerCbd &cbd)
        {
            ASSERT_TRUE(cbd.receivedData != nullptr);
            ASSERT_TRUE(cbd.element.data != nullptr);
            EXPECT_EQ(cbd.element.information.lastOperation(), dots::Mt::update);
            EXPECT_EQ(cbd.element.information.lastUpdateFrom(), 2);
            EXPECT_EQ(cbd.element.information.created(), t1);
            EXPECT_EQ(cbd.element.information.createdFrom(), 0);
            EXPECT_EQ(cbd.element.information.modified(), t3);
            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.td, dts._td());
            EXPECT_EQ(cbd.mt, dots::Mt::update);

            auto testStruct = reinterpret_cast<const DotsTestStruct *>(cbd.element.data);
            ASSERT_TRUE(testStruct->hasStringField());
            EXPECT_EQ(testStruct->stringField(), "Hello");
            ASSERT_TRUE(testStruct->hasFloatField());
            EXPECT_EQ(testStruct->floatField(), 3);
        };
        container.process(dh, &dts, &sig);
        EXPECT_EQ(container.size(), 1u);
        EXPECT_EQ(timesSigCalled, 4u);
    }

}

TEST(TestAnyContainer, find)
{
    DotsTestStruct dts;
    dts.setIndKeyfField(1);

    dots::AnyContainer container(dts._td());

    DotsHeader dh;
    dh.setTypeName(dts._td()->name());
    dh.setRemoveObj(false);
    dh.setSender(0);
    dh.setSentTime(pnxs::SystemNow());
    dh.setAttributes(dts.validProperties());

    container.process(dh, &dts);

    dts.setIndKeyfField(2);
    container.process(dh, &dts);

    // dts is now empty, because the content was moved

    {
        DotsTestStruct dts2;
        dts2.setIndKeyfField(1);

        auto iter = container.find({&dts2, pnxs::TimePoint()});
        EXPECT_TRUE(iter != container.end());
    }
}

/**
 * Insert one instance, update existing properties, update 'not-set' properties
 */
TEST(TestAnyContainer, updateInstance)
{
    // SUT: Container of DotsTestStruct
    dots::AnyContainer container(DotsTestStruct::_td());
    ASSERT_EQ(container.size(), 0u);

    DotsHeader dh;
    dh.setTypeName(DotsTestStruct::_td()->name());
    dh.setRemoveObj(false);
    dh.setSender(0);
    dh.setSentTime(pnxs::SystemNow());


    // Add DotsTestStruct with Key=1 to container
    {
        DotsTestStruct dts;
        dts.setIndKeyfField(1);

        dh.setAttributes(dts.validProperties());
        container.process(dh, &dts);
    }
    ASSERT_EQ(container.size(), 1u);

    // Check for correct properties of DotsTestStruct
    {
        DotsTestStruct f;
        f.setIndKeyfField(1);

        auto iter = container.find({&f, pnxs::TimePoint()});
        EXPECT_TRUE(iter != container.end());
        auto result = static_cast<const DotsTestStruct*>(iter->data);
        EXPECT_EQ(result->validProperties(), DotsTestStruct::PropSet(DotsTestStruct::Att::indKeyfField));
        //EXPECT_EQ(result->validProperties(), DotsTestStruct::PropSet(DotsTestStruct::Att::indKeyfField) + DotsTestStruct::PropSet(DotsTestStruct::Att::stringField));
    }

    // Update DotsTestStruct with Key=1 to container
    {
        DotsTestStruct dts;
        dts.setIndKeyfField(1);
        dts.setStringField("Hello");


        dh.setAttributes(dts.validProperties());
        container.process(dh, &dts);
    }
    ASSERT_EQ(container.size(), 1u);

    // Check for correct properties of DotsTestStruct
    {
        DotsTestStruct f;
        f.setIndKeyfField(1);

        auto iter = container.find({&f, pnxs::TimePoint()});
        EXPECT_TRUE(iter != container.end());
        auto result = static_cast<const DotsTestStruct*>(iter->data);
        EXPECT_EQ(result->validProperties(), DotsTestStruct::PropSet(DotsTestStruct::Att::indKeyfField) + DotsTestStruct::PropSet(DotsTestStruct::Att::stringField));
        EXPECT_EQ(result->stringField(), "Hello");
    }

    // Update DotsTestStruct with Key=1 to container
    {
        DotsTestStruct dts;
        dts.setIndKeyfField(1);
        dts.setFloatField(3);

        dh.setAttributes(dts.validProperties());
        container.process(dh, &dts);
    }
    ASSERT_EQ(container.size(), 1u);

    // Check for correct properties of DotsTestStruct
    {
        DotsTestStruct f;
        f.setIndKeyfField(1);

        auto iter = container.find({&f, pnxs::TimePoint()});
        EXPECT_TRUE(iter != container.end());
        auto result = static_cast<const DotsTestStruct*>(iter->data);
        EXPECT_EQ(result->validProperties(),
                  DotsTestStruct::PropSet(DotsTestStruct::Att::indKeyfField) +
                  DotsTestStruct::PropSet(DotsTestStruct::Att::stringField) +
                  DotsTestStruct::PropSet(DotsTestStruct::Att::floatField));
        EXPECT_EQ(result->stringField(), "Hello");
        EXPECT_EQ(result->floatField(), 3);
    }

    // Update DotsTestStruct with Key=1 to container
    {
        DotsTestStruct dts;
        dts.setIndKeyfField(1);

        // Set Float-Field to Valid, but it's not set. We've expect, that the property will be set to not-set in container
        dh.setAttributes(dts.validProperties() + DotsTestStruct::PropSet(DotsTestStruct::Att::floatField));
        container.process(dh, &dts);
    }
    ASSERT_EQ(container.size(), 1u);

    // Check for correct properties of DotsTestStruct
    {
        DotsTestStruct f;
        f.setIndKeyfField(1);

        auto iter = container.find({&f, pnxs::TimePoint()});
        EXPECT_TRUE(iter != container.end());
        auto result = static_cast<const DotsTestStruct*>(iter->data);
        EXPECT_EQ(result->validProperties(),
                  DotsTestStruct::PropSet(DotsTestStruct::Att::indKeyfField) +
                      DotsTestStruct::PropSet(DotsTestStruct::Att::stringField));
        EXPECT_EQ(result->stringField(), "Hello");
    }
}

/**
 * Tests, that an uncached type does not create any instance in the container.
 */
TEST(TestAnyContainer, uncachedType)
{
    DotsUncachedTestStruct dts;

    // SUT: Container of DotsTestStruct
    dots::AnyContainer container(dts._td());
    ASSERT_EQ(container.size(), 0u);

    dts.setIntKeyfField(1);

    DotsHeader dh;
    dh.setTypeName(dts._td()->name());
    dh.setRemoveObj(false);
    dh.setSender(0);
    dh.setSentTime(pnxs::SystemNow());
    dh.setAttributes(dts.validProperties());

    // Process a DotsTestStruct 'create' in container
    container.process(dh, &dts);

    ASSERT_EQ(container.size(), 0u);


    dts.setIntKeyfField(1); // Set field again, because the container moves the values into it.
    dh.setRemoveObj(true);

    container.process(dh, &dts);

    EXPECT_EQ(container.size(), 0u);
}

/**
 * Tests, that an uncached type does not create any instance in the container,
 * test also if signal is called correctly.
 */
TEST(TestAnyContainer, uncachedTypeSignal)
{
    DotsUncachedTestStruct dts;

    // SUT: Container of DotsTestStruct
    dots::AnyContainer container(dts._td());
    ASSERT_EQ(container.size(), 0u);

    dts.setIntKeyfField(1);

    auto t1 = pnxs::SystemNow();

    DotsHeader dh;
    dh.setTypeName(dts._td()->name());
    dh.setRemoveObj(false);
    dh.setSender(0);
    dh.setSentTime(t1);
    dh.setAttributes(dts.validProperties());

    uint32_t timesSigCalled = 0;

    std::function<void (const dots::AnyContainerCbd& cbd)> expectCheck = [](auto&){
        FAIL();
    };

    dots::AnyContainer::signal_type sig;
    sig.connect([&](const dots::AnyContainerCbd& cbd) {
        timesSigCalled++;
        expectCheck(cbd);
    });


    // Expect call of signal-handler with:
    // data is heap pointer
    expectCheck = [&](const dots::AnyContainerCbd &cbd)
    {
        auto testStruct = reinterpret_cast<const DotsUncachedTestStruct*>(cbd.element.data);

        ASSERT_TRUE(cbd.receivedData != nullptr);
        ASSERT_TRUE(cbd.element.data != nullptr);

        EXPECT_EQ(cbd.element.information.lastOperation(), dots::Mt::create);
        EXPECT_EQ(cbd.element.information.lastUpdateFrom(), 0);
        EXPECT_EQ(cbd.element.information.created(), t1);
        EXPECT_EQ(cbd.element.information.createdFrom(), 0);
        EXPECT_EQ(cbd.element.information.modified(), t1);
        //EXPECT_EQ(cbd.element.information.localUpdateTime, ); // Is not good testable, because the current time is used.
        EXPECT_EQ(cbd.header, dh);
        EXPECT_EQ(cbd.td, dts._td());
        EXPECT_EQ(cbd.mt, dots::Mt::create);

        ASSERT_TRUE(testStruct->hasIntKeyfField());
        EXPECT_EQ(testStruct->intKeyfField(), 1);
    };

    // Process a DotsTestStruct 'create' in container
    container.process(dh, &dts, &sig);
    ASSERT_EQ(container.size(), 0u);

    dts.setIntKeyfField(1); // Set field again, because the container moves the values into it.
    dh.setRemoveObj(true);

    container.process(dh, &dts, &sig);

    EXPECT_EQ(container.size(), 0u);
}