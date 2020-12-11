#include <gtest/gtest.h>
#include <dots/io/Dispatcher.h>
#include <DotsHeader.dots.h>
#include <DotsTestStruct.dots.h>
#include <DotsUncachedTestStruct.dots.h>

namespace dots
{
    using io::Dispatcher;
    using io::Event;
    using io::Transmission;
}

namespace
{
    namespace test_helpers
    {
        DotsHeader make_header(const dots::type::Struct& instance, uint32_t sender, bool remove = false)
        {
            return DotsHeader{
                DotsHeader::typeName_i{ instance._descriptor().name() },
                DotsHeader::sentTime_i{ dots::types::timepoint_t::Now() },
                DotsHeader::attributes_i{ instance._validProperties() },
                DotsHeader::sender_i{ sender },
                DotsHeader::removeObj_i{ remove },
            };
        }
    }
}

//TEST(TestDispatcher, dispatch_ThrowWhenNotSubscribed)
//{
//    dots::Dispatcher sut;
//    DotsTestStruct dts{
//        DotsTestStruct::indKeyfField_i{ 1 }
//    };
//    DotsHeader header = test_helpers::make_header(dts, 42);
//
//    ASSERT_THROW(sut.dispatch(header, dts), std::logic_error);
//}

TEST(TestDispatcher, dispatch_CreateEventWhenAddedHandlerForCachedType)
{
    dots::Dispatcher sut;
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id = sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
    {
        ++i;

        if (i == 1)
        {
            ASSERT_EQ(e.mt(), DotsMt::create);
            ASSERT_TRUE(e.isCreate());

            ASSERT_TRUE(e.transmitted()._equal(dts));
            ASSERT_TRUE(e.updated()._equal(dts));

            ASSERT_EQ(e.cloneInfo().lastOperation, DotsMt::create);

            ASSERT_EQ(e.cloneInfo().createdFrom, *header.sender);
            ASSERT_EQ(e.cloneInfo().created, *header.sentTime);

            ASSERT_EQ(e.cloneInfo().lastUpdateFrom, e.cloneInfo().createdFrom);
            ASSERT_EQ(e.cloneInfo().modified, e.cloneInfo().created);

            ASSERT_GE(*e.cloneInfo().localUpdateTime, *header.sentTime);
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::types::timepoint_t::Now());
        }
    });
    (void)id;

    sut.dispatch(dots::Transmission{ header, dts });

    ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, dispatch_UpdateEventWhenAddedHandlerForCachedType)
{
    dots::Dispatcher sut;
    DotsTestStruct dts1{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" }
    };
    DotsTestStruct dts2{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::floatField_i{ 2.7183f }
    };
    DotsTestStruct dts3{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" },
        DotsTestStruct::floatField_i{ 2.7183f }
    };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21);

    size_t i = 0;

    dots::Dispatcher::id_t id = sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
    {
        ++i;

        if (i == 2)
        {
            ASSERT_EQ(e.mt(), DotsMt::update);
            ASSERT_TRUE(e.isUpdate());

            ASSERT_TRUE(e.transmitted()._equal(dts2));
            ASSERT_TRUE(e.updated()._equal(dts3));

            ASSERT_EQ(e.cloneInfo().lastOperation, DotsMt::update);

            ASSERT_EQ(e.cloneInfo().createdFrom, *header1.sender);
            ASSERT_EQ(e.cloneInfo().created, *header1.sentTime);

            ASSERT_EQ(e.cloneInfo().lastUpdateFrom, *header2.sender);
            ASSERT_EQ(e.cloneInfo().modified, *header2.sentTime);

            ASSERT_GE(*e.cloneInfo().localUpdateTime, *header2.sentTime);
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::types::timepoint_t::Now());
        }
    });
    (void)id;

    sut.dispatch(dots::Transmission{ header1, dts1 });
    sut.dispatch(dots::Transmission{ header2, dts2 });

    ASSERT_EQ(i, 2);
}

TEST(TestDispatcher, dispatch_RemoveEventWhenAddedHandlerForCachedType)
{
    dots::Dispatcher sut;
    DotsTestStruct dts1{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" }
    };
    DotsTestStruct dts2{
        DotsTestStruct::indKeyfField_i{ 2 }
    };
    DotsTestStruct dts3{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::floatField_i{ 2.7183f }
    };
    DotsTestStruct dts4{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" },
        DotsTestStruct::floatField_i{ 2.7183f }
    };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21);
    DotsHeader header3 = test_helpers::make_header(dts3, 42, true);

    size_t i = 0;

    dots::Dispatcher::id_t id = sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
    {
        ++i;

        if (i == 3)
        {
            ASSERT_EQ(e.mt(), DotsMt::remove);
            ASSERT_TRUE(e.isRemove());

            ASSERT_TRUE(e.transmitted()._equal(dts3));
            ASSERT_TRUE(e.updated()._equal(dts4));

            ASSERT_EQ(e.cloneInfo().lastOperation, DotsMt::remove);

            ASSERT_EQ(e.cloneInfo().createdFrom, *header1.sender);
            ASSERT_EQ(e.cloneInfo().created, *header1.sentTime);

            ASSERT_EQ(e.cloneInfo().lastUpdateFrom, *header3.sender);
            ASSERT_EQ(e.cloneInfo().modified, *header3.sentTime);

            ASSERT_GE(*e.cloneInfo().localUpdateTime, *header3.sentTime);
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::types::timepoint_t::Now());
        }
    });
    (void)id;

    sut.dispatch(dots::Transmission{ header1, dts1 });
    sut.dispatch(dots::Transmission{ header2, dts2 });
    sut.dispatch(dots::Transmission{ header3, dts3 });

    ASSERT_EQ(i, 3);
}

TEST(TestDispatcher, dispatch_CreateEventWhenDynamicallyAddedHandlerForCachedType)
{
    dots::Dispatcher sut;
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id = sut.addEventHandler(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
    {
        ++i;

        if (i == 1)
        {
            ASSERT_EQ(e.mt(), DotsMt::create);
            ASSERT_TRUE(e.isCreate());

            ASSERT_TRUE(e.transmitted()._equal(dts));
            ASSERT_TRUE(e.updated()._equal(dts));

            ASSERT_EQ(e.cloneInfo().lastOperation, DotsMt::create);

            ASSERT_EQ(e.cloneInfo().createdFrom, *header.sender);
            ASSERT_EQ(e.cloneInfo().created, *header.sentTime);

            ASSERT_EQ(e.cloneInfo().lastUpdateFrom, e.cloneInfo().createdFrom);
            ASSERT_EQ(e.cloneInfo().modified, e.cloneInfo().created);

            ASSERT_GE(*e.cloneInfo().localUpdateTime, *header.sentTime);
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::types::timepoint_t::Now());
        }
    });
    (void)id;

    sut.dispatch(dots::Transmission{ header, dts });

    ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, dispatch_UpdateEventWhenDynamicallAddedHandlerForCachedType)
{
    dots::Dispatcher sut;
    DotsTestStruct dts1{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" }
    };
    DotsTestStruct dts2{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::floatField_i{ 2.7183f }
    };
    DotsTestStruct dts3{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" },
        DotsTestStruct::floatField_i{ 2.7183f }
    };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21);

    size_t i = 0;

    dots::Dispatcher::id_t id = sut.addEventHandler(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
    {
        ++i;

        if (i == 2)
        {
            ASSERT_EQ(e.mt(), DotsMt::update);
            ASSERT_TRUE(e.isUpdate());

            ASSERT_TRUE(e.transmitted()._equal(dts2));
            ASSERT_TRUE(e.updated()._equal(dts3));

            ASSERT_EQ(e.cloneInfo().lastOperation, DotsMt::update);

            ASSERT_EQ(e.cloneInfo().createdFrom, *header1.sender);
            ASSERT_EQ(e.cloneInfo().created, *header1.sentTime);

            ASSERT_EQ(e.cloneInfo().lastUpdateFrom, *header2.sender);
            ASSERT_EQ(e.cloneInfo().modified, *header2.sentTime);

            ASSERT_GE(*e.cloneInfo().localUpdateTime, *header2.sentTime);
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::types::timepoint_t::Now());
        }
    });
    (void)id;

    sut.dispatch(dots::Transmission{ header1, dts1 });
    sut.dispatch(dots::Transmission{ header2, dts2 });

    ASSERT_EQ(i, 2);
}

TEST(TestDispatcher, dispatch_RemoveEventWhenDynamicallAddedHandlerForCachedType)
{
    dots::Dispatcher sut;
    DotsTestStruct dts1{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" }
    };
    DotsTestStruct dts2{
        DotsTestStruct::indKeyfField_i{ 2 }
    };
    DotsTestStruct dts3{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::floatField_i{ 2.7183f }
    };
    DotsTestStruct dts4{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" },
        DotsTestStruct::floatField_i{ 2.7183f }
    };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21);
    DotsHeader header3 = test_helpers::make_header(dts3, 42, true);

    size_t i = 0;

    dots::Dispatcher::id_t id = sut.addEventHandler(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
    {
        ++i;

        if (i == 3)
        {
            ASSERT_EQ(e.mt(), DotsMt::remove);
            ASSERT_TRUE(e.isRemove());

            ASSERT_TRUE(e.transmitted()._equal(dts3));
            ASSERT_TRUE(e.updated()._equal(dts4));

            ASSERT_EQ(e.cloneInfo().lastOperation, DotsMt::remove);

            ASSERT_EQ(e.cloneInfo().createdFrom, *header1.sender);
            ASSERT_EQ(e.cloneInfo().created, *header1.sentTime);

            ASSERT_EQ(e.cloneInfo().lastUpdateFrom, *header3.sender);
            ASSERT_EQ(e.cloneInfo().modified, *header3.sentTime);

            ASSERT_GE(*e.cloneInfo().localUpdateTime, *header3.sentTime);
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::types::timepoint_t::Now());
        }
    });
    (void)id;

    sut.dispatch(dots::Transmission{ header1, dts1 });
    sut.dispatch(dots::Transmission{ header2, dts2 });
    sut.dispatch(dots::Transmission{ header3, dts3 });

    ASSERT_EQ(i, 3);
}

TEST(TestDispatcher, dispatch_CreateEventFromCacheWhenAddedHandlerForCachedType)
{
    dots::Dispatcher sut;
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id1 = sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        /* do nothing */
    });
    (void)id1;

    sut.dispatch(dots::Transmission{ header, dts });
    sut.dispatch(dots::Transmission{ header, dts });

    dots::Dispatcher::id_t id2 = sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
    {
        ++i;

        if (i == 1)
        {
            ASSERT_EQ(e.mt(), DotsMt::create);
            ASSERT_TRUE(e.isCreate());

            ASSERT_TRUE(e.transmitted()._equal(dts));
            ASSERT_TRUE(e.updated()._equal(dts));

            ASSERT_EQ(e.cloneInfo().lastOperation, DotsMt::update);

            ASSERT_EQ(e.cloneInfo().createdFrom, *header.sender);
            ASSERT_EQ(e.cloneInfo().created, *header.sentTime);

            ASSERT_EQ(e.cloneInfo().lastUpdateFrom, e.cloneInfo().createdFrom);
            ASSERT_EQ(e.cloneInfo().modified, e.cloneInfo().created);

            ASSERT_GE(*e.cloneInfo().localUpdateTime, *header.sentTime);
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::types::timepoint_t::Now());
        }
    });
    (void)id2;

    ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, dispatch_CreateEventWhenAddedHandlerForUncachedType)
{
    dots::Dispatcher sut;
    DotsUncachedTestStruct dts1{
        DotsUncachedTestStruct::intKeyfField_i{ 1 },
        DotsUncachedTestStruct::value_i{ "foo" },
    };
    DotsUncachedTestStruct dts2{
        DotsUncachedTestStruct::intKeyfField_i{ 1 },
    };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21);

    size_t i = 0;

    dots::Dispatcher::id_t id = sut.addEventHandler<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>& e)
    {
        ++i;

        if (i == 2)
        {
            ASSERT_EQ(e.mt(), DotsMt::create);
            ASSERT_TRUE(e.isCreate());

            ASSERT_TRUE(e.transmitted()._equal(dts2));
            ASSERT_TRUE(e.updated()._equal(dts2));

            ASSERT_EQ(e.cloneInfo().lastOperation, DotsMt::create);

            ASSERT_EQ(e.cloneInfo().createdFrom, *header2.sender);
            ASSERT_EQ(e.cloneInfo().created, *header2.sentTime);

            ASSERT_FALSE(e.cloneInfo().lastUpdateFrom.isValid());
            ASSERT_FALSE(e.cloneInfo().modified.isValid());

            ASSERT_GE(*e.cloneInfo().localUpdateTime, *header2.sentTime);
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::types::timepoint_t::Now());
        }
    });
    (void)id;

    sut.dispatch(dots::Transmission{ header1, dts1 });
    sut.dispatch(dots::Transmission{ header2, dts2 });

    ASSERT_EQ(i, 2);
}

TEST(TestDispatcher, dispatch_NoEventAfterRedundantRemoveForCachedType)
{
    dots::Dispatcher sut;
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader headerCreate = test_helpers::make_header(dts, 42);
    DotsHeader headerRemove = test_helpers::make_header(dts, 42, true);

    sut.dispatch(dots::Transmission{ headerCreate, dts });

    size_t i = 0;

    dots::Dispatcher::id_t id = sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
    {
        if (e.isRemove())
        {
            ++i;
        }
    });
    (void)id;

    sut.dispatch(dots::Transmission{ headerRemove, dts });
    sut.dispatch(dots::Transmission{ headerRemove, dts });

    ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, dispatch_ThrowWhenRemovingUncachedType)
{
    dots::Dispatcher sut;
    DotsUncachedTestStruct duts{
        DotsUncachedTestStruct::intKeyfField_i{ 1 }
    };
    DotsHeader header = test_helpers::make_header(duts, 42, true);

    dots::Dispatcher::id_t id1 = sut.addEventHandler<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>&/* e*/)
    {
        /* do nothing */
    });
    (void)id1;

    ASSERT_THROW(sut.dispatch(dots::Transmission{ header, duts }), std::logic_error);
}

TEST(TestDispatcher, dispatch_NoEventWhenNotAddedHandlerForType)
{
    dots::Dispatcher sut;
    DotsUncachedTestStruct duts{ DotsUncachedTestStruct::intKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(duts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id1 = sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        ++i;
    });
    dots::Dispatcher::id_t id2 = sut.addEventHandler<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>&/* e*/)
    {
        /* do nothing */
    });
    (void)id1;
    (void)id2;

    sut.dispatch(dots::Transmission{ header, duts });

    ASSERT_EQ(i, 0);
}

TEST(TestDispatcher, dispatch_NoEventAfterExplicitRemoveHandlerForType)
{
    dots::Dispatcher sut;
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header1 = test_helpers::make_header(dts, 42);
    DotsHeader header2 = test_helpers::make_header(dts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id = sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        ++i;
    });

    sut.dispatch(dots::Transmission{ header1, dts });
    sut.removeEventHandler(DotsTestStruct::_Descriptor(), id);
    sut.dispatch(dots::Transmission{ header2, dts });

    ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, moveCtor_CreateEventAfterMoveContructWhenAddedHandlerForCachedType)
{
    dots::Dispatcher dispatcher;
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id = dispatcher.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& /*e*/)
    {
        ++i;
    });
    (void)id;

    dots::Dispatcher sut{ std::move(dispatcher) };
    sut.dispatch(dots::Transmission{ header, dts });

    ASSERT_EQ(i, 1);
}