#include "dots/io/Container.h"
#include "DotsTestStruct.dots.h"
#include "DotsUncachedTestStruct.dots.h"
#include "dots/type/Registry.h"
#include <gtest/gtest.h>

#include <iostream>

using namespace dots::type;

/**
 * Add two instances of a DotsTestStruct to an Container and
 * remove one of them.
 */
TEST(TestContainer, storeAndRemove)
{
    uint32_t timesSigCalled = 0;
    dots::rC<DotsTestStruct>().clear();

    dots::ContainerBase::signal_type sig;
    sig.connect([&](dots::CTypeless /*cbd*/) {
        timesSigCalled++;
    });

    DotsTestStruct dts;

    // SUT: Container of DotsTestStruct
    auto& container = dots::rC<DotsTestStruct>();
    ASSERT_EQ(container.size(), 0u);

    dts.indKeyfField(1);

    DotsHeader dh;
    dh.typeName(dts._Descriptor().name());
    dh.removeObj(false);
    dh.sender(0);
    dh.sentTime(pnxs::SystemNow());
    dh.attributes(dts._validProperties());

    // Process a DotsTestStruct 'create' in container
    container.process(dh, dts, sig);

    ASSERT_EQ(container.size(), 1u);

    // Modify Key property, so that it will be another instance
    dts.indKeyfField = 2;

    container.process(dh, dts, sig);

    EXPECT_EQ(container.size(), 2u);

    dts.indKeyfField = 2; // Set field again, because the container moves the values into it.
    dh.removeObj = true;

    // Remove DotsTestStruct with Key==2 from container
    container.process(dh, dts, sig);

    EXPECT_EQ(container.size(), 1u);

    EXPECT_EQ(timesSigCalled, 3);
}

TEST(TestContainer, storeUpdateAndRemoveSignal)
{
    uint32_t timesSigCalled = 0;
    dots::rC<DotsTestStruct>().clear();

    std::function<void (const DotsTestStruct::Cbd& cbd)> expectCheck = [](auto&){
        FAIL();
    };

    dots::ContainerBase::signal_type sig;
    sig.connect([&](dots::CTypeless cbd) {
        timesSigCalled++;
        expectCheck(*(dots::Cbd<DotsTestStruct>*)(cbd));
    });

    auto& container = dots::rC<DotsTestStruct>();
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
        expectCheck = [&](const DotsTestStruct::Cbd& cbd)
        {
            auto& testStruct = cbd();

            /*
            EXPECT_EQ(cbd.information.lastOperation(), dots::Mt::create);
            EXPECT_EQ(cbd.information.lastUpdateFrom(), 0);
            EXPECT_EQ(cbd.information.created(), t1);
            EXPECT_EQ(cbd.information.createdFrom(), 0);
            EXPECT_EQ(cbd.information.modified(), t1);
            //EXPECT_EQ(cbd.information.localUpdateTime, ); // Is not good testable, because the current time is used.
             */
            EXPECT_EQ(cbd.header, dh);

            EXPECT_EQ(cbd.mt, dots::Mt::create);

            ASSERT_TRUE(testStruct.stringField.isValid());
            EXPECT_EQ(testStruct.stringField, "Hello");
        };
        container.process(dh, dts, sig);
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

        expectCheck = [&](const DotsTestStruct::Cbd& cbd)
        {
            /*
            EXPECT_EQ(cbd.information.lastOperation(), dots::Mt::create);
            EXPECT_EQ(cbd.information.lastUpdateFrom(), 1);
            EXPECT_EQ(cbd.information.created(), t2);
            EXPECT_EQ(cbd.information.createdFrom(), 1);
            EXPECT_EQ(cbd.information.modified(), t2);
             */

            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.mt, dots::Mt::create);
        };
        container.process(dh, dts, sig);
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

        expectCheck = [&](const DotsTestStruct::Cbd& cbd)
        {
            /*
            EXPECT_EQ(cbd.information.lastOperation(), dots::Mt::remove);
            EXPECT_EQ(cbd.information.lastUpdateFrom(), 1);
            EXPECT_EQ(cbd.information.created(), t2);
            EXPECT_EQ(cbd.information.createdFrom(), 1);
            EXPECT_EQ(cbd.information.modified(), t2);
             */

            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.mt, dots::Mt::remove);

            auto& testStruct = cbd();
            ASSERT_TRUE(testStruct.stringField.isValid());
            EXPECT_EQ(testStruct.stringField, "World");
        };
        container.process(dh, removeObj, sig);
        EXPECT_EQ(container.size(), 1u);
        EXPECT_EQ(timesSigCalled, 3u);
    }

    auto t3 = pnxs::SystemNow();

    {
        DotsTestStruct dts;

        // Update 1st
        dts.indKeyfField(1);
        dts.floatField(3.0f);

        // Fill DotsHeader data
        DotsHeader dh;
        dh.typeName(dts._Descriptor().name());
        dh.removeObj(false);
        dh.sender(2);
        dh.sentTime(t3);
        dh.attributes(dts._validProperties());

        expectCheck = [&](const DotsTestStruct::Cbd& cbd)
        {
            /*
            EXPECT_EQ(cbd.information.lastOperation(), dots::Mt::update);
            EXPECT_EQ(cbd.information.lastUpdateFrom(), 2);
            EXPECT_EQ(cbd.information.created(), t1);
            EXPECT_EQ(cbd.information.createdFrom(), 0);
            EXPECT_EQ(cbd.information.modified(), t3);
             */

            EXPECT_EQ(cbd.header, dh);
            EXPECT_EQ(cbd.mt, dots::Mt::update);

            auto& testStruct = cbd();
            ASSERT_TRUE(testStruct.stringField.isValid());
            EXPECT_EQ(testStruct.stringField, "Hello");
            ASSERT_TRUE(testStruct.floatField.isValid());
            EXPECT_EQ(testStruct.floatField, 3.0f);
        };
        container.process(dh, dts, sig);
        EXPECT_EQ(container.size(), 1u);
        EXPECT_EQ(timesSigCalled, 4u);
    }

}
