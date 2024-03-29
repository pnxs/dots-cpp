// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/testing/gtest/gtest.h>
#include <dots/Container.h>
#include <DotsHeader.dots.h>
#include <DotsTestStruct.dots.h>

namespace
{
    namespace test_helpers
    {
        DotsHeader make_header(const dots::type::Struct& instance, uint32_t sender, bool remove = false, dots::property_set_t includeProperties = dots::property_set_t::All)
        {
            return DotsHeader{
                .typeName = instance._descriptor().name(),
                .sentTime = dots::timepoint_t::Now(),
                .attributes = includeProperties == dots::property_set_t::All ? instance._validProperties() : includeProperties,
                .sender = sender,
                .removeObj = remove,
            };
        }

        std::pair<DotsHeader, DotsTestStruct> make_instance(DotsTestStruct instance, uint32_t sender, bool remove = false, dots::property_set_t includeProperties = dots::property_set_t::All)
        {
            DotsHeader header = make_header(instance, sender, remove, includeProperties);
            return std::make_pair(std::move(header), std::move(instance));
        }
    }
}

TEST(TestContainer, ctor_EmptyAfterDefaultConstruction)
{
    {
        dots::Container<DotsTestStruct> sut;
        DotsTestStruct dts{ .indKeyfField = 1 };

        ASSERT_TRUE(sut.empty());
        ASSERT_EQ(sut.size(), 0);
        ASSERT_EQ(sut.find(dts), nullptr);
        ASSERT_THROW(sut.get(dts), std::logic_error);
    }

    {
        dots::Container<DotsTestSubStruct> sut;

        ASSERT_TRUE(sut.empty());
        ASSERT_EQ(sut.size(), 0);
        ASSERT_EQ(sut.find(), nullptr);
        ASSERT_THROW(sut.get(), std::logic_error);
    }
}

TEST(TestContainer, insert_CreateInstanceWhenEmpty)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts{
        .stringField = "foo",
        .indKeyfField = 1,
        .floatField = 3.1415f
    };
    DotsHeader header = test_helpers::make_header(dts, 42);

    const auto& [created, cloneInfo] = sut.insert(header, dts);

    ASSERT_FALSE(sut.empty());
    ASSERT_EQ(sut.size(), 1);
    ASSERT_EQ(sut.find(dts), &*created);
    ASSERT_TRUE(created->_equal(dts));
    ASSERT_NO_THROW(sut.get(dts));

    ASSERT_EQ(cloneInfo.lastOperation, DotsMt::create);

    ASSERT_EQ(cloneInfo.createdFrom, *header.sender);
    ASSERT_EQ(cloneInfo.created, *header.sentTime);

    ASSERT_GE(*cloneInfo.localUpdateTime, *header.sentTime);
    ASSERT_LE(*cloneInfo.localUpdateTime, dots::timepoint_t::Now());

    ASSERT_EQ(cloneInfo.lastUpdateFrom, cloneInfo.createdFrom);
    ASSERT_EQ(cloneInfo.modified, cloneInfo.created);
}

TEST(TestContainer, insert_UpdateSameInstanceWhenNotEmpty)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts1{
        .stringField = "foo",
        .indKeyfField = 1,
        .floatField = 3.1415f
    };
    DotsTestStruct dts2{
        .indKeyfField = 1,
        .floatField = 2.7183f,
        .enumField = DotsTestEnum::value1
    };
    DotsTestStruct dts3{
        .indKeyfField = 1,
        .floatField = 2.7183f,
        .enumField = DotsTestEnum::value1
    };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21, false, dts2._validProperties() + DotsTestStruct::stringField_p);

    sut.insert(header1, dts1);
    const auto& [updated, cloneInfo] = sut.insert(header2, dts2);

    ASSERT_FALSE(sut.empty());
    ASSERT_EQ(sut.size(), 1);
    ASSERT_EQ(sut.find(dts3), &*updated);
    ASSERT_TRUE(updated->_equal(dts3));
    ASSERT_NO_THROW(sut.get(dts2));

    ASSERT_EQ(cloneInfo.lastOperation, DotsMt::update);

    ASSERT_EQ(cloneInfo.createdFrom, *header1.sender);
    ASSERT_EQ(cloneInfo.created, *header1.sentTime);

    ASSERT_EQ(cloneInfo.lastUpdateFrom, *header2.sender);
    ASSERT_EQ(cloneInfo.modified, *header2.sentTime);

    ASSERT_GE(*cloneInfo.localUpdateTime, *header2.sentTime);
    ASSERT_LE(*cloneInfo.localUpdateTime, dots::timepoint_t::Now());
}

TEST(TestContainer, insert_CreateDifferentInstanceWhenNotEmpty)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts1{
        .stringField = "foo",
        .indKeyfField = 1,
        .floatField = 3.1415f
    };
    DotsTestStruct dts2{
        .indKeyfField = 2,
        .floatField = 2.7183f,
        .enumField = DotsTestEnum::value1
    };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21);

    sut.insert(header1, dts1);
    const auto& [created, cloneInfo] = sut.insert(header2, dts2);

    ASSERT_FALSE(sut.empty());
    ASSERT_EQ(sut.size(), 2);
    ASSERT_EQ(sut.find(dts2), &*created);
    ASSERT_TRUE(created->_equal(dts2));
    ASSERT_NO_THROW(sut.get(dts2));

    ASSERT_EQ(cloneInfo.lastOperation, DotsMt::create);

    ASSERT_EQ(cloneInfo.createdFrom, *header2.sender);
    ASSERT_EQ(cloneInfo.created, *header2.sentTime);

    ASSERT_GE(*cloneInfo.localUpdateTime, *header2.sentTime);
    ASSERT_LE(*cloneInfo.localUpdateTime, dots::timepoint_t::Now());

    ASSERT_EQ(cloneInfo.lastUpdateFrom, cloneInfo.createdFrom);
    ASSERT_EQ(cloneInfo.modified, cloneInfo.created);
}

TEST(TestContainer, remove_EmptyNodeWhenEmpty)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts{    .indKeyfField = 1 };
    DotsHeader header = test_helpers::make_header(dts, 42);

    ASSERT_TRUE(sut.remove(header, dts).empty());
}

TEST(TestContainer, remove_EmptyNodeWhenNotContained)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts1{ .indKeyfField = 1 };
    DotsTestStruct dts2{ .indKeyfField = 2 };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21);

    sut.insert(header1, dts1);

    ASSERT_EQ(sut.find(dts2), nullptr);
    ASSERT_TRUE(sut.remove(header2, dts2).empty());
}

TEST(TestContainer, remove_RemoveWhenContained)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts1{
        .stringField = "foo",
        .indKeyfField = 1
    };
    DotsTestStruct dts2{
        .indKeyfField = 2
    };
    DotsTestStruct dts3{
        .indKeyfField = 1,
        .floatField = 2.7183f
    };
    DotsTestStruct dts4{
        .stringField = "foo",
        .indKeyfField = 1,
        .floatField = 2.7183f
    };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21);
    DotsHeader header3 = test_helpers::make_header(dts3, 73, true);

    sut.insert(header1, dts1);
    sut.insert(header2, dts2);
    dots::Container<DotsTestStruct>::node_t removedNode = sut.remove(header3, dts3);
    const dots::type::Struct& removed = removedNode.key();
    const DotsCloneInformation& cloneInfo = removedNode.mapped();

    ASSERT_EQ(sut.size(), 1);
    ASSERT_EQ(sut.find(dts1), nullptr);
    ASSERT_EQ(sut.find(dts3), nullptr);
    ASSERT_TRUE(removed._equal(dts4));
    ASSERT_THROW(sut.get(dts4), std::logic_error);

    ASSERT_EQ(cloneInfo.lastOperation, DotsMt::remove);

    ASSERT_EQ(cloneInfo.createdFrom, *header1.sender);
    ASSERT_EQ(cloneInfo.created, *header1.sentTime);

    ASSERT_EQ(cloneInfo.lastUpdateFrom, *header3.sender);
    ASSERT_EQ(cloneInfo.modified, *header3.sentTime);

    ASSERT_GE(*cloneInfo.localUpdateTime, *header3.sentTime);
    ASSERT_LE(*cloneInfo.localUpdateTime, dots::timepoint_t::Now());
}

TEST(TestContainer, begin_end_IterationYieldsExpectedInstances)
{
    dots::Container<DotsTestStruct> sut;
    std::vector<DotsTestStruct> expected;

    auto [header1, dts1] = test_helpers::make_instance(DotsTestStruct{ .stringField = "foo", .indKeyfField = 1 }, 42);
    auto [header2, dts2] = test_helpers::make_instance(DotsTestStruct{ .stringField = "bar", .indKeyfField = 2 }, 42);
    auto [header3, dts3] = test_helpers::make_instance(DotsTestStruct{ .stringField = "baz", .indKeyfField = 3 }, 42);
    auto [header4, dts4] = test_helpers::make_instance(DotsTestStruct{ .stringField = "qux", .indKeyfField = 4 }, 42);

    expected.emplace_back(sut.insert(header1, dts1).first.to<DotsTestStruct>());
    expected.emplace_back(sut.insert(header2, dts2).first.to<DotsTestStruct>());
    expected.emplace_back(sut.insert(header3, dts3).first.to<DotsTestStruct>());
    expected.emplace_back(sut.insert(header4, dts4).first.to<DotsTestStruct>());

    size_t i = 0;

    for (const auto& [instance, cloneInfo] : sut)
    {
        (void)instance;
        (void)cloneInfo;
        ++i;
    }

    ASSERT_EQ(i, expected.size());

    auto itExpected = expected.begin();

    for (const auto& [instance, cloneInfo]: sut)
    {
        EXPECT_EQ(instance.to<DotsTestStruct>(), *itExpected++);
    }
}

TEST(TestContainer, forEach_IterationYieldsExpectedInstances)
{
    dots::Container<DotsTestStruct> sut;
    std::vector<DotsTestStruct> expected;

    auto [header1, dts1] = test_helpers::make_instance(DotsTestStruct{ .stringField = "foo", .indKeyfField = 1 }, 42);
    auto [header2, dts2] = test_helpers::make_instance(DotsTestStruct{ .stringField = "bar", .indKeyfField = 2 }, 42);
    auto [header3, dts3] = test_helpers::make_instance(DotsTestStruct{ .stringField = "baz", .indKeyfField = 3 }, 42);
    auto [header4, dts4] = test_helpers::make_instance(DotsTestStruct{ .stringField = "qux", .indKeyfField = 4 }, 42);

    expected.emplace_back(sut.insert(header1, dts1).first.to<DotsTestStruct>());
    expected.emplace_back(sut.insert(header2, dts2).first.to<DotsTestStruct>());
    expected.emplace_back(sut.insert(header3, dts3).first.to<DotsTestStruct>());
    expected.emplace_back(sut.insert(header4, dts4).first.to<DotsTestStruct>());

    size_t i = 0;

    sut.forEach([&](const DotsTestStruct&/* instance*/)
    {
        ++i;
    });

    ASSERT_EQ(i, expected.size());

    auto itExpected = expected.begin();

    sut.forEach([&](auto& instance)
    {
        EXPECT_EQ(instance, *itExpected++);
    });
}
