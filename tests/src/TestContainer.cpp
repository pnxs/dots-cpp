#include <gtest/gtest.h>
#include <dots/io/Container.h>
#include <DotsHeader.dots.h>
#include <DotsTestStruct.dots.h>

namespace dots
{
    using io::Container;
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

TEST(TestContainer, ctor_EmptyAfterDefaultConstruction)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts{ DotsTestStruct::indKeyfField_i{ 1 } };

    ASSERT_TRUE(sut.empty());
    ASSERT_EQ(sut.size(), 0);
    ASSERT_EQ(sut.find(dts), nullptr);
    ASSERT_THROW(sut.get(dts), std::logic_error);
}

TEST(TestContainer, insert_CreateInstanceWhenEmpty)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" },
        DotsTestStruct::floatField_i{ 3.1415f }
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
    ASSERT_LE(*cloneInfo.localUpdateTime, dots::types::timepoint_t::Now());

    ASSERT_EQ(cloneInfo.lastUpdateFrom, cloneInfo.createdFrom);
    ASSERT_EQ(cloneInfo.modified, cloneInfo.created);
}

TEST(TestContainer, insert_UpdateSameInstanceWhenNotEmpty)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts1{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" },
        DotsTestStruct::floatField_i{ 3.1415f }
    };
    DotsTestStruct dts2{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::floatField_i{ 2.7183f },
        DotsTestStruct::enumField_i{ DotsTestEnum::value1 }
    };
    DotsTestStruct dts3{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" },
        DotsTestStruct::floatField_i{ 2.7183f },
        DotsTestStruct::enumField_i{ DotsTestEnum::value1 }
    };
    DotsHeader header1 = test_helpers::make_header(dts1, 42);
    DotsHeader header2 = test_helpers::make_header(dts2, 21);

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
    ASSERT_LE(*cloneInfo.localUpdateTime, dots::types::timepoint_t::Now());
}

TEST(TestContainer, insert_CreateDifferentInstanceWhenNotEmpty)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts1{
        DotsTestStruct::indKeyfField_i{ 1 },
        DotsTestStruct::stringField_i{ "foo" },
        DotsTestStruct::floatField_i{ 3.1415f }
    };
    DotsTestStruct dts2{
        DotsTestStruct::indKeyfField_i{ 2 },
        DotsTestStruct::floatField_i{ 2.7183f },
        DotsTestStruct::enumField_i{ DotsTestEnum::value1 }
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
    ASSERT_LE(*cloneInfo.localUpdateTime, dots::types::timepoint_t::Now());

    ASSERT_EQ(cloneInfo.lastUpdateFrom, cloneInfo.createdFrom);
    ASSERT_EQ(cloneInfo.modified, cloneInfo.created);
}

TEST(TestContainer, remove_EmptyNodeWhenEmpty)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts{    DotsTestStruct::indKeyfField_i{ 1 } };
    DotsHeader header = test_helpers::make_header(dts, 42);

    ASSERT_TRUE(sut.remove(header, dts).empty());
}

TEST(TestContainer, remove_EmptyNodeWhenNotContained)
{
    dots::Container<DotsTestStruct> sut;
    DotsTestStruct dts1{ DotsTestStruct::indKeyfField_i{ 1 } };
    DotsTestStruct dts2{ DotsTestStruct::indKeyfField_i{ 2 } };
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
    DotsHeader header3 = test_helpers::make_header(dts2, 73, true);

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
    ASSERT_LE(*cloneInfo.localUpdateTime, dots::types::timepoint_t::Now());
}