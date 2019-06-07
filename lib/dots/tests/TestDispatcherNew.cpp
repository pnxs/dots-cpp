#include <gtest/gtest.h>
#include <dots/io/DispatcherNew.h>
#include <DotsHeader.dots.h>
#include <DotsTestStruct.dots.h>
#include <DotsUncachedTestStruct.dots.h>

namespace
{
	namespace test_helpers
	{
		DotsHeader make_header(const dots::type::Struct& instance, uint32_t sender, bool remove = false)
		{
			return DotsHeader{
				DotsHeader::typeName_t_i{ instance._descriptor().name() },
				DotsHeader::sentTime_t_i{ pnxs::SystemNow() },
				DotsHeader::attributes_t_i{ instance._validProperties() },
				DotsHeader::sender_t_i{ sender },
				DotsHeader::removeObj_t_i{ remove },
			};
		}
	}
}

//TEST(TestDispatcherNew, dispatch_ThrowWhenNotSubscribed)
//{
//	dots::DispatcherNew sut;
//	DotsTestStruct dts{
//		DotsTestStruct::indKeyfField_t_i{ 1 }
//	};
//	DotsHeader header = test_helpers::make_header(dts, 42);
//
//	ASSERT_THROW(sut.dispatch(header, dts), std::logic_error);
//}

TEST(TestDispatcherNew, dispatch_CreateEventWhenSubscribedToCachedType)
{
	dots::DispatcherNew sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_t_i{ 1 } };
	DotsHeader header = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::SubscriptionNew subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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

TEST(TestDispatcherNew, dispatch_UpdateEventWhenSubscribedToCachedType)
{
	dots::DispatcherNew sut;
	DotsTestStruct dts1{ 
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::stringField_t_i{ "foo" }
	};
	DotsTestStruct dts2{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::floatField_t_i{ 2.7183f }
	};
	DotsTestStruct dts3{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::stringField_t_i{ "foo" },
		DotsTestStruct::floatField_t_i{ 2.7183f }
	};
	DotsHeader header1 = test_helpers::make_header(dts1, 42);
	DotsHeader header2 = test_helpers::make_header(dts2, 21);

	size_t i = 0;

	dots::SubscriptionNew subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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

TEST(TestDispatcherNew, dispatch_RemoveEventWhenSubscribedToCachedType)
{
	dots::DispatcherNew sut;
	DotsTestStruct dts1{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::stringField_t_i{ "foo" }
	};
	DotsTestStruct dts2{
		DotsTestStruct::indKeyfField_t_i{ 2 }
	};
	DotsTestStruct dts3{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::floatField_t_i{ 2.7183f }
	};
	DotsTestStruct dts4{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::stringField_t_i{ "foo" },
		DotsTestStruct::floatField_t_i{ 2.7183f }
	};
	DotsHeader header1 = test_helpers::make_header(dts1, 42);
	DotsHeader header2 = test_helpers::make_header(dts2, 21);
	DotsHeader header3 = test_helpers::make_header(dts3, 42, true);

	size_t i = 0;

	dots::SubscriptionNew subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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

TEST(TestDispatcherNew, dispatch_CreateEventWhenDynamicallySubscribedToCachedType)
{
	dots::DispatcherNew sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_t_i{ 1 } };
	DotsHeader header = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::SubscriptionNew subscription = sut.subscribe(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
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

TEST(TestDispatcherNew, dispatch_UpdateEventWhenDynamicallSubscribedToCachedType)
{
	dots::DispatcherNew sut;
	DotsTestStruct dts1{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::stringField_t_i{ "foo" }
	};
	DotsTestStruct dts2{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::floatField_t_i{ 2.7183f }
	};
	DotsTestStruct dts3{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::stringField_t_i{ "foo" },
		DotsTestStruct::floatField_t_i{ 2.7183f }
	};
	DotsHeader header1 = test_helpers::make_header(dts1, 42);
	DotsHeader header2 = test_helpers::make_header(dts2, 21);

	size_t i = 0;

	dots::SubscriptionNew subscription = sut.subscribe(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
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

TEST(TestDispatcherNew, dispatch_RemoveEventWhenDynamicallSubscribedToCachedType)
{
	dots::DispatcherNew sut;
	DotsTestStruct dts1{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::stringField_t_i{ "foo" }
	};
	DotsTestStruct dts2{
		DotsTestStruct::indKeyfField_t_i{ 2 }
	};
	DotsTestStruct dts3{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::floatField_t_i{ 2.7183f }
	};
	DotsTestStruct dts4{
		DotsTestStruct::indKeyfField_t_i{ 1 },
		DotsTestStruct::stringField_t_i{ "foo" },
		DotsTestStruct::floatField_t_i{ 2.7183f }
	};
	DotsHeader header1 = test_helpers::make_header(dts1, 42);
	DotsHeader header2 = test_helpers::make_header(dts2, 21);
	DotsHeader header3 = test_helpers::make_header(dts3, 42, true);

	size_t i = 0;

	dots::SubscriptionNew subscription = sut.subscribe(DotsTestStruct::_Descriptor(), [&](const dots::Event<>& e)
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

TEST(TestDispatcherNew, dispatch_CreateEventFromCacheWhenSubscribingToCachedType)
{
	dots::DispatcherNew sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_t_i{ 1 } };
	DotsHeader header = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::SubscriptionNew subscription1 = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
	{
		/* do nothing */
	});	

	sut.dispatch(header, dts);

	dots::SubscriptionNew subscription2 = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
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

TEST(TestDispatcherNew, dispatch_CreateEventWhenSubscribedToUncachedType)
{
	dots::DispatcherNew sut;
	DotsUncachedTestStruct dts1{
		DotsUncachedTestStruct::intKeyfField_t_i{ 1 },
		DotsUncachedTestStruct::value_t_i{ "foo" },
	};
	DotsUncachedTestStruct dts2{
		DotsUncachedTestStruct::intKeyfField_t_i{ 1 },
	};
	DotsHeader header1 = test_helpers::make_header(dts1, 42);
	DotsHeader header2 = test_helpers::make_header(dts2, 21);

	size_t i = 0;

	dots::SubscriptionNew subscription = sut.subscribe<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>& e)
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

TEST(TestDispatcherNew, dispatch_ThrowWhenRemovingUncachedType)
{
	dots::DispatcherNew sut;
	DotsUncachedTestStruct duts{
		DotsUncachedTestStruct::intKeyfField_t_i{ 1 }
	};
	DotsHeader header = test_helpers::make_header(duts, 42, true);

	dots::SubscriptionNew subscription1 = sut.subscribe<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>&/* e*/)
	{
		/* do nothing */
	});

	ASSERT_THROW(sut.dispatch(header, duts), std::logic_error);
}

TEST(TestDispatcherNew, dispatch_NoEventWhenNotSubscribedToType)
{
	dots::DispatcherNew sut;
	DotsUncachedTestStruct duts{ DotsUncachedTestStruct::intKeyfField_t_i{ 1 } };
	DotsHeader header = test_helpers::make_header(duts, 42);

	size_t i = 0;

	dots::SubscriptionNew subscription1 = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
	{
		++i;
	});
	dots::SubscriptionNew subscription2 = sut.subscribe<DotsUncachedTestStruct>([&](const dots::Event<DotsUncachedTestStruct>&/* e*/)
	{
		/* do nothing */
	});

	sut.dispatch(header, duts);

	ASSERT_EQ(i, 0);
}

TEST(TestDispatcherNew, dispatch_NoEventAfterExplicitUnubscribeFromType)
{
	dots::DispatcherNew sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_t_i{ 1 } };
	DotsHeader header1 = test_helpers::make_header(dts, 42);
	DotsHeader header2 = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::SubscriptionNew subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
	{
		++i;
	});

	sut.dispatch(header1, dts);
	subscription.unsubscribe();
	sut.dispatch(header2, dts);

	ASSERT_EQ(i, 1);
}

TEST(TestDispatcherNew, dispatch_NoEventAfterImplicitUnubscribeFromType)
{
	dots::DispatcherNew sut;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_t_i{ 1 } };
	DotsHeader header1 = test_helpers::make_header(dts, 42);
	DotsHeader header2 = test_helpers::make_header(dts, 42);

	size_t i = 0;

	{
		dots::SubscriptionNew subscription = sut.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>&/* e*/)
		{
			++i;
		});

		sut.dispatch(header1, dts);
	}

	sut.dispatch(header2, dts);

	ASSERT_EQ(i, 1);
}

TEST(TestDispatcherNew, moveCtor_CreateEventAfterMoveContructWhenSubscribedToCachedType)
{
	dots::DispatcherNew dispatcher;
	DotsTestStruct dts{ DotsTestStruct::indKeyfField_t_i{ 1 } };
	DotsHeader header = test_helpers::make_header(dts, 42);

	size_t i = 0;

	dots::SubscriptionNew subscription = dispatcher.subscribe<DotsTestStruct>([&](const dots::Event<DotsTestStruct>& e)
	{
		++i;
	});

	dots::DispatcherNew sut{ std::move(dispatcher) };
	sut.dispatch(header, dts);

	ASSERT_EQ(i, 1);
}