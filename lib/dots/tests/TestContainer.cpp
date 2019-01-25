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
    sig.connect([&](dots::CTypeless cbd) {
        timesSigCalled++;
    });

    DotsTestStruct dts;

    // SUT: Container of DotsTestStruct
    auto& container = dots::rC<DotsTestStruct>();
    ASSERT_EQ(container.size(), 0u);

    dts.setIndKeyfField(1);

    DotsHeader dh;
    dh.setTypeName(dts._td()->name());
    dh.setRemoveObj(false);
    dh.setSender(0);
    dh.setSentTime(pnxs::SystemNow());
    dh.setAttributes(dts.validProperties());

    // Process a DotsTestStruct 'create' in container
    container.process(dh, dts, sig);

    ASSERT_EQ(container.size(), 1u);

    // Modify Key property, so that it will be another instance
    dts.setIndKeyfField(2);

    container.process(dh, dts, sig);

    EXPECT_EQ(container.size(), 2u);

    dts.setIndKeyfField(2); // Set field again, because the container moves the values into it.
    dh.setRemoveObj(true);

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

            ASSERT_TRUE(testStruct.hasStringField());
            EXPECT_EQ(testStruct.stringField(), "Hello");
        };
        container.process(dh, dts, sig);
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
        DotsTestStruct removeObj(2);

        // Fill DotsHeader data
        DotsHeader dh;
        dh.setTypeName(dts._td()->name());
        dh.setRemoveObj(true);
        dh.setSender(1);
        dh.setSentTime(t2);
        dh.setAttributes(dts.validProperties());

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
            ASSERT_TRUE(testStruct.hasStringField());
            EXPECT_EQ(testStruct.stringField(), "World");
        };
        container.process(dh, removeObj, sig);
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
            ASSERT_TRUE(testStruct.hasStringField());
            EXPECT_EQ(testStruct.stringField(), "Hello");
            ASSERT_TRUE(testStruct.hasFloatField());
            EXPECT_EQ(testStruct.floatField(), 3);
        };
        container.process(dh, dts, sig);
        EXPECT_EQ(container.size(), 1u);
        EXPECT_EQ(timesSigCalled, 4u);
    }

}