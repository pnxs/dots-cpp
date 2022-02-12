#include <dots/testing/gtest/gtest.h>
#include <dots/Dispatcher.h>
#include <DotsHeader.dots.h>
#include <DotsTestStruct.dots.h>
#include <DotsUncachedTestStruct.dots.h>

namespace dots
{
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
                DotsHeader::sentTime_i{ dots::timepoint_t::Now() },
                DotsHeader::attributes_i{ instance._validProperties() },
                DotsHeader::sender_i{ sender },
                DotsHeader::removeObj_i{ remove },
            };
        }
    }
}

//TEST_F(TestDispatcher, dispatch_ThrowWhenNotSubscribed)
//{
//    dots::Dispatcher sut;
//    DotsTestStruct dts{
//        DotsTestStruct::indKeyfField_i{ 1 }
//    };
//    DotsHeader header = test_helpers::make_header(dts, 42);
//
//    ASSERT_THROW(m_sut.dispatch(header, dts), std::logic_error);
//}

struct TestDispatcher : ::testing::Test
{
    dots::Dispatcher m_sut;
};

TEST_F(TestDispatcher, addEventHandler_AllowsUsageOfVariousHandlerTypes)
{
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    static int I = 0;

    struct Foobar
    {
        void NonStaticEventHandler(const dots::Event<DotsTestStruct>&/* e*/)
        {
            ++I;
        }

        void NonStaticEventHandlerConst(const dots::Event<DotsTestStruct>&/* e*/) const
        {
            ++I;
        }

        void NonStaticEventHandlerWithBindings(int i, float f, const dots::Event<DotsTestStruct>&/* e*/)
        {
            ++I;
            I += i;
            I += static_cast<int>(f);
        }

        static void StaticEventHandler(const dots::Event<DotsTestStruct>&/* e*/)
        {
            ++I;
        }
    };

    Foobar foobar;
    Foobar* this_ = &foobar;

    // lambda
    m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/){ ++I; });

    // mutable lambda
    m_sut.addEventHandler<DotsTestStruct>([&, i = 0](const dots::Event<DotsTestStruct>&/* e*/) mutable { ++I; I += ++i; }); 

    // static member function
    m_sut.addEventHandler<DotsTestStruct>(Foobar::StaticEventHandler);

    // non-static member function
    m_sut.addEventHandler<DotsTestStruct>({ &Foobar::NonStaticEventHandler, this_ });

    // non-static const member function
    m_sut.addEventHandler<DotsTestStruct>({ &Foobar::NonStaticEventHandlerConst, this_ });

    // function pointer
    auto fPtr = &Foobar::StaticEventHandler;
    m_sut.addEventHandler<DotsTestStruct>(fPtr);

    // handler with bindings
    m_sut.addEventHandler<DotsTestStruct>({ &Foobar::NonStaticEventHandlerWithBindings, this_, 5, 2.3f });

    m_sut.dispatch(dots::Transmission{ header, dts });

    ASSERT_EQ(I, 15);
}

TEST_F(TestDispatcher, dispatch_CreateEventWhenAddedHandlerForCachedType)
{
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::timepoint_t::Now());
        }
    });
    (void)id;

    m_sut.dispatch(dots::Transmission{ header, dts });

    ASSERT_EQ(i, 1);
}

TEST_F(TestDispatcher, dispatch_UpdateEventWhenAddedHandlerForCachedType)
{
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

    dots::Dispatcher::id_t id = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::timepoint_t::Now());
        }
    });
    (void)id;

    m_sut.dispatch(dots::Transmission{ header1, dts1 });
    m_sut.dispatch(dots::Transmission{ header2, dts2 });

    ASSERT_EQ(i, 2);
}

TEST_F(TestDispatcher, dispatch_RemoveEventWhenAddedHandlerForCachedType)
{
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

    dots::Dispatcher::id_t id = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::timepoint_t::Now());
        }
    });
    (void)id;

    m_sut.dispatch(dots::Transmission{ header1, dts1 });
    m_sut.dispatch(dots::Transmission{ header2, dts2 });
    m_sut.dispatch(dots::Transmission{ header3, dts3 });

    ASSERT_EQ(i, 3);
}

TEST_F(TestDispatcher, dispatch_CreateEventWhenDynamicallyAddedHandlerForCachedType)
{
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id = m_sut.addEventHandler(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
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
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::timepoint_t::Now());
        }
    });
    (void)id;

    m_sut.dispatch(dots::Transmission{ header, dts });

    ASSERT_EQ(i, 1);
}

TEST_F(TestDispatcher, dispatch_UpdateEventWhenDynamicallAddedHandlerForCachedType)
{
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

    dots::Dispatcher::id_t id = m_sut.addEventHandler(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
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
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::timepoint_t::Now());
        }
    });
    (void)id;

    m_sut.dispatch(dots::Transmission{ header1, dts1 });
    m_sut.dispatch(dots::Transmission{ header2, dts2 });

    ASSERT_EQ(i, 2);
}

TEST_F(TestDispatcher, dispatch_RemoveEventWhenDynamicallAddedHandlerForCachedType)
{
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

    dots::Dispatcher::id_t id = m_sut.addEventHandler(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
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
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::timepoint_t::Now());
        }
    });
    (void)id;

    m_sut.dispatch(dots::Transmission{ header1, dts1 });
    m_sut.dispatch(dots::Transmission{ header2, dts2 });
    m_sut.dispatch(dots::Transmission{ header3, dts3 });

    ASSERT_EQ(i, 3);
}

TEST_F(TestDispatcher, dispatch_CreateEventFromCacheWhenAddedHandlerForCachedType)
{
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id1 = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        /* do nothing */
    });
    (void)id1;

    m_sut.dispatch(dots::Transmission{ header, dts });
    m_sut.dispatch(dots::Transmission{ header, dts });

    dots::Dispatcher::id_t id2 = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::timepoint_t::Now());
        }
    });
    (void)id2;

    ASSERT_EQ(i, 1);
}

TEST_F(TestDispatcher, dispatch_CreateEventWhenAddedHandlerForUncachedType)
{
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

    dots::Dispatcher::id_t id = m_sut.addEventHandler<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>& e)
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
            ASSERT_LE(*e.cloneInfo().localUpdateTime, dots::timepoint_t::Now());
        }
    });
    (void)id;

    m_sut.dispatch(dots::Transmission{ header1, dts1 });
    m_sut.dispatch(dots::Transmission{ header2, dts2 });

    ASSERT_EQ(i, 2);
}

TEST_F(TestDispatcher, dispatch_NoEventAfterRedundantRemoveForCachedType)
{
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader headerCreate = test_helpers::make_header(dts, 42);
    DotsHeader headerRemove = test_helpers::make_header(dts, 42, true);

    m_sut.dispatch(dots::Transmission{ headerCreate, dts });

    size_t i = 0;

    dots::Dispatcher::id_t id = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
    {
        if (e.isRemove())
        {
            ++i;
        }
    });
    (void)id;

    m_sut.dispatch(dots::Transmission{ headerRemove, dts });
    m_sut.dispatch(dots::Transmission{ headerRemove, dts });

    ASSERT_EQ(i, 1);
}

TEST_F(TestDispatcher, dispatch_ThrowWhenRemovingUncachedType)
{
    DotsUncachedTestStruct duts{
        DotsUncachedTestStruct::intKeyfField_i{ 1 }
    };
    DotsHeader header = test_helpers::make_header(duts, 42, true);

    dots::Dispatcher::id_t id1 = m_sut.addEventHandler<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>&/* e*/)
    {
        /* do nothing */
    });
    (void)id1;

    ASSERT_THROW(m_sut.dispatch(dots::Transmission{ header, duts }), std::logic_error);
}

TEST_F(TestDispatcher, dispatch_NoEventWhenNotAddedHandlerForType)
{
    DotsUncachedTestStruct duts{ DotsUncachedTestStruct::intKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(duts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id1 = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        ++i;
    });
    dots::Dispatcher::id_t id2 = m_sut.addEventHandler<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>&/* e*/)
    {
        /* do nothing */
    });
    (void)id1;
    (void)id2;

    m_sut.dispatch(dots::Transmission{ header, duts });

    ASSERT_EQ(i, 0);
}

TEST_F(TestDispatcher, dispatch_NoEventAfterExplicitRemoveHandlerForType)
{
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header1 = test_helpers::make_header(dts, 42);
    DotsHeader header2 = test_helpers::make_header(dts, 42);

    size_t i = 0;

    dots::Dispatcher::id_t id = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        ++i;
    });

    m_sut.dispatch(dots::Transmission{ header1, dts });
    m_sut.removeEventHandler(DotsTestStruct::_Descriptor(), id);
    m_sut.dispatch(dots::Transmission{ header2, dts });

    ASSERT_EQ(i, 1);
}

TEST_F(TestDispatcher, dispatch_AddEventHandlerDuringDispatch)
{
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;

    for (size_t j = 0; j < 10; ++j)
    {
        m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
        {
            ++i;
        });
    }

    m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
        {
            ++i;
        });
    });

    for (size_t j = 0; j < 10; ++j)
    {
        m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
        {
            ++i;
        });
    }

    m_sut.dispatch(dots::Transmission{ header, dts });

    ASSERT_EQ(i, 21);
}

TEST_F(TestDispatcher, dispatch_RemoveCurrentEventHandlerDuringDispatch)
{
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;
    dots::Dispatcher::id_t id;

    for (size_t j = 0; j < 10; ++j)
    {
        m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
        {
            ++i;
        });
    }

    id = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        m_sut.removeEventHandler(DotsTestStruct::_Descriptor(), id);
    });

    for (size_t j = 0; j < 10; ++j)
    {
        m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
        {
            ++i;
        });
    }

    m_sut.dispatch(dots::Transmission{ header, dts });

    ASSERT_EQ(i, 20);
}

TEST_F(TestDispatcher, dispatch_RemoveOtherEventHandlerDuringDispatch)
{
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    size_t i = 0;
    dots::Dispatcher::id_t id1;
    dots::Dispatcher::id_t id5;

    id1 = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        ++i;
    });

    m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        ++i;
        m_sut.removeEventHandler(DotsTestStruct::_Descriptor(), id1);
    });

    m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        ++i;
    });

    m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        ++i;
        m_sut.removeEventHandler(DotsTestStruct::_Descriptor(), id5);
    });

    id5 = m_sut.addEventHandler<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
    {
        ++i;
    });

    m_sut.dispatch(dots::Transmission{ header, dts });

    ASSERT_EQ(i, 4);
}

TEST_F(TestDispatcher, moveCtor_CreateEventAfterMoveContructWhenAddedHandlerForCachedType)
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
