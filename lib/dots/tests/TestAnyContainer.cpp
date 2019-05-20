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
    dots::AnyContainer container(&dts._Descriptor());
    ASSERT_EQ(container.size(), 0u);

    dts.indKeyfField(1);

    DotsHeader dh;
    dh.typeName(dts._Descriptor().name());
    dh.removeObj(false);
    dh.sender(0);
    dh.sentTime(pnxs::SystemNow());
    dh.attributes(dts._validProperties());

    // Process a DotsTestStruct 'create' in container
    container.process(dh, dts);

    ASSERT_EQ(container.size(), 1u);

    // Modify Key property, so that it will be another instance
    dts.indKeyfField = 2;

    container.process(dh, dts);

    EXPECT_EQ(container.size(), 2u);

    dts.indKeyfField = 2; // Set field again, because the container moves the values into it.
    dh.removeObj = true;

    // Remove DotsTestStruct with Key==2 from container
    container.process(dh, dts);

    EXPECT_EQ(container.size(), 1u);
}



TEST(TestAnyContainer, storeUpdateAndRemoveSignal)
{
    uint32_t timesSigCalled = 0;

    std::function<void (const dots::AnyContainerCbd& cbd)> expectCheck = [](auto&){
        FAIL();
    };

    dots::AnyContainer::signal_type sig;
    sig.connect([&](const dots::AnyContainerCbd& cbd) {
        timesSigCalled++;
        expectCheck(cbd);
    });

    dots::AnyContainer container(&DotsTestStruct::_Descriptor());
    ASSERT_EQ(container.size(), 0u);

    auto t1 = pnxs::SystemNow();


    {
        DotsTestStruct dts;

        // Add 1st element
        dts.indKeyfField(1);
        dts.stringField("Hello");

        // Fill DotsHeader data
        DotsHeader dh;
        dh.typeName(dts._Descriptor().name());
        dh.removeObj(false);
        dh.sender(0);
        dh.sentTime(t1);
        dh.attributes(dts._validProperties());

        // Expect call of signal-handler with:
        // data is heap pointer
        expectCheck = [&](const dots::AnyContainerCbd &cbd)
        {
            auto testStruct = reinterpret_cast<const DotsTestStruct *>(cbd.element.data);

            ASSERT_TRUE(cbd.receivedData != nullptr);
            ASSERT_TRUE(cbd.element.data != nullptr);

            EXPECT_EQ(cbd.element.information.lastOperation, dots::Mt::create);
            EXPECT_EQ(cbd.element.information.lastUpdateFrom, 0u);
            EXPECT_EQ(cbd.element.information.created, t1);
            EXPECT_EQ(cbd.element.information.createdFrom, 0u);
            EXPECT_EQ(cbd.element.information.modified, t1);
            //EXPECT_EQ(cbd.element.information.localUpdateTime, ); // Is not good testable, because the current time is used.
            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.td, &dts._Descriptor());
            EXPECT_EQ(cbd.mt, dots::Mt::create);

            ASSERT_TRUE(testStruct->stringField.isValid());
            EXPECT_EQ(testStruct->stringField, "Hello");
        };
        container.process(dh, dts, &sig);
        ASSERT_EQ(container.size(), 1u);
        EXPECT_EQ(timesSigCalled, 1u);
    }

    auto t2 = pnxs::SystemNow();

    {
        DotsTestStruct dts;

        // Add 2nd
        dts.indKeyfField(2);
        dts.stringField("World");

        // Fill DotsHeader data
        DotsHeader dh;
        dh.typeName(dts._Descriptor().name());
        dh.removeObj(false);
        dh.sender(1);
        dh.sentTime(t2);
        dh.attributes(dts._validProperties());

        expectCheck = [&](const dots::AnyContainerCbd &cbd)
        {
            ASSERT_TRUE(cbd.receivedData != nullptr);
            ASSERT_TRUE(cbd.element.data != nullptr);

            EXPECT_EQ(cbd.element.information.lastOperation, dots::Mt::create);
            EXPECT_EQ(cbd.element.information.lastUpdateFrom, 1u);
            EXPECT_EQ(cbd.element.information.created, t2);
            EXPECT_EQ(cbd.element.information.createdFrom, 1u);
            EXPECT_EQ(cbd.element.information.modified, t2);
            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.td, &dts._Descriptor());
            EXPECT_EQ(cbd.mt, dots::Mt::create);
        };
        container.process(dh, dts, &sig);
        EXPECT_EQ(container.size(), 2u);
        EXPECT_EQ(timesSigCalled, 2u);
    }

    {
        DotsTestStruct dts;

        // Remove
        DotsTestStruct removeObj;
        removeObj.indKeyfField = 2;

        // Fill DotsHeader data
        DotsHeader dh;
        dh.typeName(dts._Descriptor().name());
        dh.removeObj(true);
        dh.sender(1);
        dh.sentTime(t2);
        dh.attributes(dts._validProperties());


        expectCheck = [&](const dots::AnyContainerCbd &cbd)
        {
            ASSERT_TRUE(cbd.receivedData != nullptr);
            ASSERT_TRUE(cbd.element.data != nullptr);
            EXPECT_EQ(cbd.element.information.lastOperation, dots::Mt::remove);
            EXPECT_EQ(cbd.element.information.lastUpdateFrom, 1u);
            EXPECT_EQ(cbd.element.information.created, t2);
            EXPECT_EQ(cbd.element.information.createdFrom, 1u);
            EXPECT_EQ(cbd.element.information.modified, t2);
            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.td, &dts._Descriptor());
            EXPECT_EQ(cbd.mt, dots::Mt::remove);

            auto testStruct = reinterpret_cast<const DotsTestStruct *>(cbd.element.data);
            ASSERT_TRUE(testStruct->stringField.isValid());
            EXPECT_EQ(testStruct->stringField, "World");
        };
        container.process(dh, removeObj, &sig);
        EXPECT_EQ(container.size(), 1u);
        EXPECT_EQ(timesSigCalled, 3u);
    }

    auto t3 = pnxs::SystemNow();

    {
        DotsTestStruct dts;

        // Update 1st
        dts.indKeyfField(1);
        dts.floatField(3);

        // Fill DotsHeader data
        DotsHeader dh;
        dh.typeName(dts._Descriptor().name());
        dh.removeObj(false);
        dh.sender(2);
        dh.sentTime(t3);
        dh.attributes(dts._validProperties());

        expectCheck = [&](const dots::AnyContainerCbd &cbd)
        {
            ASSERT_TRUE(cbd.receivedData != nullptr);
            ASSERT_TRUE(cbd.element.data != nullptr);
            EXPECT_EQ(cbd.element.information.lastOperation, dots::Mt::update);
            EXPECT_EQ(cbd.element.information.lastUpdateFrom, 2u);
            EXPECT_EQ(cbd.element.information.created, t1);
            EXPECT_EQ(cbd.element.information.createdFrom, 0u);
            EXPECT_EQ(cbd.element.information.modified, t3);
            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.td, &dts._Descriptor());
            EXPECT_EQ(cbd.mt, dots::Mt::update);

            auto testStruct = reinterpret_cast<const DotsTestStruct *>(cbd.element.data);
            ASSERT_TRUE(testStruct->stringField.isValid());
            EXPECT_EQ(testStruct->stringField, "Hello");
            ASSERT_TRUE(testStruct->floatField.isValid());
            EXPECT_EQ(testStruct->floatField, 3.0f);
        };
        container.process(dh, dts, &sig);
        EXPECT_EQ(container.size(), 1u);
        EXPECT_EQ(timesSigCalled, 4u);
    }

}


TEST(TestAnyContainer, find)
{
    DotsTestStruct dts;
    dts.indKeyfField(1);

    dots::AnyContainer container(&dts._Descriptor());

    DotsHeader dh;
    dh.typeName(dts._Descriptor().name());
    dh.removeObj(false);
    dh.sender(0);
    dh.sentTime(pnxs::SystemNow());
    dh.attributes(dts._validProperties());

    container.process(dh, dts);

    dts.indKeyfField = 2;
    container.process(dh, dts);

    // dts is now empty, because the content was moved

    {
        DotsTestStruct dts2;
        dts2.indKeyfField(1);

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
    dots::AnyContainer container(&DotsTestStruct::_Descriptor());
    ASSERT_EQ(container.size(), 0u);

    DotsHeader dh;
    dh.typeName(DotsTestStruct::_Descriptor().name());
    dh.removeObj(false);
    dh.sender(0);
    dh.sentTime(pnxs::SystemNow());


    // Add DotsTestStruct with Key=1 to container
    {
        DotsTestStruct dts;
        dts.indKeyfField(1);

        dh.attributes(dts._validProperties());
        container.process(dh, dts);
    }
    ASSERT_EQ(container.size(), 1u);

    // Check for correct properties of DotsTestStruct
    {
        DotsTestStruct f;
        f.indKeyfField(1);

        auto iter = container.find({&f, pnxs::TimePoint()});
        EXPECT_TRUE(iter != container.end());
        auto result = static_cast<const DotsTestStruct*>(iter->data);
        EXPECT_EQ(result->_validProperties(), DotsTestStruct::indKeyfField_t::Set());
        //EXPECT_EQ(result->validProperties(), DotsTestStruct::PropSet(DotsTestStruct::Att::indKeyfField) + DotsTestStruct::PropSet(DotsTestStruct::Att::stringField));
    }

    // Update DotsTestStruct with Key=1 to container
    {
        DotsTestStruct dts;
        dts.indKeyfField(1);
        dts.stringField("Hello");


        dh.attributes = dts._validProperties();
        container.process(dh, dts);
    }
    ASSERT_EQ(container.size(), 1u);

    // Check for correct properties of DotsTestStruct
    {
        DotsTestStruct f;
        f.indKeyfField(1);

        auto iter = container.find({&f, pnxs::TimePoint()});
        EXPECT_TRUE(iter != container.end());
        auto result = static_cast<const DotsTestStruct*>(iter->data);
        EXPECT_EQ(result->_validProperties(), DotsTestStruct::indKeyfField_t::Set() + DotsTestStruct::stringField_t::Set());
        EXPECT_EQ(result->stringField, "Hello");
    }

    // Update DotsTestStruct with Key=1 to container
    {
        DotsTestStruct dts;
        dts.indKeyfField(1);
        dts.floatField(3);

        dh.attributes = dts._validProperties();
        container.process(dh, dts);
    }
    ASSERT_EQ(container.size(), 1u);

    // Check for correct properties of DotsTestStruct
    {
        DotsTestStruct f;
        f.indKeyfField(1);

        auto iter = container.find({&f, pnxs::TimePoint()});
        EXPECT_TRUE(iter != container.end());
        auto result = static_cast<const DotsTestStruct*>(iter->data);
        EXPECT_EQ(result->_validProperties(),
                  DotsTestStruct::indKeyfField_t::Set() +
                  DotsTestStruct::stringField_t::Set() +
                  DotsTestStruct::floatField_t::Set());
        EXPECT_EQ(result->stringField, "Hello");
        EXPECT_EQ(result->floatField, 3.0f);
    }

    // Update DotsTestStruct with Key=1 to container
    {
        DotsTestStruct dts;
        dts.indKeyfField(1);

        // Set Float-Field to Valid, but it's not set. We've expect, that the property will be set to not-set in container
        dh.attributes = dts._validProperties() + DotsTestStruct::floatField_t::Set();
        container.process(dh, dts);
    }
    ASSERT_EQ(container.size(), 1u);

    // Check for correct properties of DotsTestStruct
    {
        DotsTestStruct f;
        f.indKeyfField(1);

        auto iter = container.find({&f, pnxs::TimePoint()});
        EXPECT_TRUE(iter != container.end());
        auto result = static_cast<const DotsTestStruct*>(iter->data);
        EXPECT_EQ(result->_validProperties(),
                  DotsTestStruct::indKeyfField_t::Set() +
                      DotsTestStruct::stringField_t::Set());
        EXPECT_EQ(result->stringField, "Hello");
    }
}

/**
 * Tests, that an uncached type does not create any instance in the container.
 */
TEST(TestAnyContainer, uncachedType)
{
    DotsUncachedTestStruct dts;

    // SUT: Container of DotsTestStruct
    dots::AnyContainer container(&dts._Descriptor());
    ASSERT_EQ(container.size(), 0u);

    dts.intKeyfField(1);

    DotsHeader dh;
    dh.typeName(dts._Descriptor().name());
    dh.removeObj(false);
    dh.sender(0);
    dh.sentTime(pnxs::SystemNow());
    dh.attributes(dts._validProperties());

    // Process a DotsTestStruct 'create' in container
    container.process(dh, dts);

    ASSERT_EQ(container.size(), 0u);


    dts.intKeyfField = 1; // Set field again, because the container moves the values into it.
    dh.removeObj = true;

    container.process(dh, dts);

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
    dots::AnyContainer container(&dts._Descriptor());
    ASSERT_EQ(container.size(), 0u);

    dts.intKeyfField(1);

    auto t1 = pnxs::SystemNow();

    DotsHeader dh;
    dh.typeName(dts._Descriptor().name());
    dh.removeObj(false);
    dh.sender(0);
    dh.sentTime(t1);
    dh.attributes(dts._validProperties());

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

        EXPECT_EQ(cbd.element.information.lastOperation, dots::Mt::create);
        EXPECT_EQ(cbd.element.information.lastUpdateFrom, 0u);
        EXPECT_EQ(cbd.element.information.created, t1);
        EXPECT_EQ(cbd.element.information.createdFrom, 0u);
        EXPECT_EQ(cbd.element.information.modified, t1);
        //EXPECT_EQ(cbd.element.information.localUpdateTime, ); // Is not good testable, because the current time is used.
        EXPECT_EQ(cbd.header, dh);
        EXPECT_EQ(cbd.td, &dts._Descriptor());
        EXPECT_EQ(cbd.mt, dots::Mt::create);

        ASSERT_TRUE(testStruct->intKeyfField.isValid());
        EXPECT_EQ(testStruct->intKeyfField, 1);
    };

    // Process a DotsTestStruct 'create' in container
    container.process(dh, dts, &sig);
    ASSERT_EQ(container.size(), 0u);

    dts.intKeyfField = 1; // Set field again, because the container moves the values into it.
    dh.removeObj = true;

    container.process(dh, dts, &sig);

    EXPECT_EQ(container.size(), 0u);
}
