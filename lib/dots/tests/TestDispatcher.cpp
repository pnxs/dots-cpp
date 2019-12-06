#include <gtest/gtest.h>
#include <dots/io/Dispatcher.h>
#include <DotsHeader.dots.h>
#include <DotsTestStruct.dots.h>
#include <DotsUncachedTestStruct.dots.h>

namespace
{
	namespace test_helpers
	{
		DotsHeader make_header(const dots::type::NewStruct& instance, uint32_t sender, bool remove = false)
		{
			return DotsHeader{
				DotsHeader::typeName_i{ instance._descriptor().name() },
				DotsHeader::sentTime_i{ pnxs::SystemNow() },
				DotsHeader::attributes_i{ instance._validProperties() },
				DotsHeader::sender_i{ sender },
				DotsHeader::removeObj_i{ remove },
			};
		}
	}
}

//TEST(TestDispatcher, dispatch_ThrowWhenNotSubscribed)
//{
//	dots::Dispatcher sut;
//	DotsTestStruct dts{
//		DotsTestStruct::indKeyfField_i{ 1 }
//	};
//	DotsHeader header = test_helpers::make_header(dts, 42);
//
//	ASSERT_THROW(sut.dispatch(header, dts), std::logic_error);
//}

TEST(TestDispatcher, dispatch_CreateEventWhenSubscribedToCachedType)
{
	dots::Dispatcher sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
	DotsHeader header = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::Subscription subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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

			ASSERT_FALSE(e.cloneInfo().lastUpdateFrom.isValid());
			ASSERT_FALSE(e.cloneInfo().modified.isValid());

			ASSERT_GE(*e.cloneInfo().localUpdateTime, *header.sentTime);
			ASSERT_LE(*e.cloneInfo().localUpdateTime, pnxs::SystemNow());
		}
	});

	sut.dispatch(header, dts);

	ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, dispatch_UpdateEventWhenSubscribedToCachedType)
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

	dots::Subscription subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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
			ASSERT_LE(*e.cloneInfo().localUpdateTime, pnxs::SystemNow());
		}
	});

	sut.dispatch(header1, dts1);
	sut.dispatch(header2, dts2);

	ASSERT_EQ(i, 2);
}

TEST(TestDispatcher, dispatch_RemoveEventWhenSubscribedToCachedType)
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

	dots::Subscription subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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
			ASSERT_LE(*e.cloneInfo().localUpdateTime, pnxs::SystemNow());
		}
	});

	sut.dispatch(header1, dts1);
	sut.dispatch(header2, dts2);
	sut.dispatch(header3, dts3);

	ASSERT_EQ(i, 3);
}

TEST(TestDispatcher, dispatch_CreateEventWhenDynamicallySubscribedToCachedType)
{
	dots::Dispatcher sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
	DotsHeader header = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::Subscription subscription = sut.subscribe(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
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

			ASSERT_FALSE(e.cloneInfo().lastUpdateFrom.isValid());
			ASSERT_FALSE(e.cloneInfo().modified.isValid());

			ASSERT_GE(*e.cloneInfo().localUpdateTime, *header.sentTime);
			ASSERT_LE(*e.cloneInfo().localUpdateTime, pnxs::SystemNow());
		}
	});

	sut.dispatch(header, dts);

	ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, dispatch_UpdateEventWhenDynamicallSubscribedToCachedType)
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

	dots::Subscription subscription = sut.subscribe(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
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
			ASSERT_LE(*e.cloneInfo().localUpdateTime, pnxs::SystemNow());
		}
	});

	sut.dispatch(header1, dts1);
	sut.dispatch(header2, dts2);

	ASSERT_EQ(i, 2);
}

TEST(TestDispatcher, dispatch_RemoveEventWhenDynamicallSubscribedToCachedType)
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

	dots::Subscription subscription = sut.subscribe(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
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
			ASSERT_LE(*e.cloneInfo().localUpdateTime, pnxs::SystemNow());
		}
	});

	sut.dispatch(header1, dts1);
	sut.dispatch(header2, dts2);
	sut.dispatch(header3, dts3);

	ASSERT_EQ(i, 3);
}

TEST(TestDispatcher, dispatch_CreateEventFromCacheWhenSubscribingToCachedType)
{
	dots::Dispatcher sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
	DotsHeader header = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::Subscription subscription1 = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
	{
		/* do nothing */
	});	

	sut.dispatch(header, dts);

	dots::Subscription subscription2 = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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

			ASSERT_FALSE(e.cloneInfo().lastUpdateFrom.isValid());
			ASSERT_FALSE(e.cloneInfo().modified.isValid());

			ASSERT_GE(*e.cloneInfo().localUpdateTime, *header.sentTime);
			ASSERT_LE(*e.cloneInfo().localUpdateTime, pnxs::SystemNow());
		}
	});	

	ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, dispatch_CreateEventWhenSubscribedToUncachedType)
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

	dots::Subscription subscription = sut.subscribe<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>& e)
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
			ASSERT_LE(*e.cloneInfo().localUpdateTime, pnxs::SystemNow());
		}
	});

	sut.dispatch(header1, dts1);
	sut.dispatch(header2, dts2);

	ASSERT_EQ(i, 2);
}

TEST(TestDispatcher, dispatch_ThrowWhenRemovingUncachedType)
{
	dots::Dispatcher sut;
	DotsUncachedTestStruct duts{
		DotsUncachedTestStruct::intKeyfField_i{ 1 }
	};
	DotsHeader header = test_helpers::make_header(duts, 42, true);

	dots::Subscription subscription1 = sut.subscribe<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>&/* e*/)
	{
		/* do nothing */
	});

	ASSERT_THROW(sut.dispatch(header, duts), std::logic_error);
}

TEST(TestDispatcher, dispatch_NoEventWhenNotSubscribedToType)
{
	dots::Dispatcher sut;
	DotsUncachedTestStruct duts{ DotsUncachedTestStruct::intKeyfField_i{ 1 } };
	DotsHeader header = test_helpers::make_header(duts, 42);

	size_t i = 0;

	dots::Subscription subscription1 = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
	{
		++i;
	});
	dots::Subscription subscription2 = sut.subscribe<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>&/* e*/)
	{
		/* do nothing */
	});

	sut.dispatch(header, duts);

	ASSERT_EQ(i, 0);
}

TEST(TestDispatcher, dispatch_NoEventAfterExplicitUnubscribeFromType)
{
	dots::Dispatcher sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
	DotsHeader header1 = test_helpers::make_header(dts, 42);
	DotsHeader header2 = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::Subscription subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
	{
		++i;
	});

	sut.dispatch(header1, dts);
	subscription.unsubscribe();
	sut.dispatch(header2, dts);

	ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, dispatch_NoEventAfterImplicitUnubscribeFromType)
{
	dots::Dispatcher sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
	DotsHeader header1 = test_helpers::make_header(dts, 42);
	DotsHeader header2 = test_helpers::make_header(dts, 42);

	size_t i = 0;

	{
		dots::Subscription subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
		{
			++i;
		});

		sut.dispatch(header1, dts);
	}

	sut.dispatch(header2, dts);

	ASSERT_EQ(i, 1);
}

TEST(TestDispatcher, moveCtor_CreateEventAfterMoveContructWhenSubscribedToCachedType)
{
	dots::Dispatcher dispatcher;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };
	DotsHeader header = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::Subscription subscription = dispatcher.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& /*e*/)
	{
		++i;
	});

	dots::Dispatcher sut{ std::move(dispatcher) };
	sut.dispatch(header, dts);

	ASSERT_EQ(i, 1);
}