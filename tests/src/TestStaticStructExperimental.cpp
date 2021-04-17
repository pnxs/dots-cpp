#include <dots/type/StaticStruct.h>
#include <dots/type/StaticProperty.h>
#include <dots/type/StaticPropertyOffset.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/PropertyInitializer.h>
#include <gtest/gtest.h>

namespace dots::types
{
    namespace details
    {
        struct ProtoTestSubStruct : type::details::ProtoStaticStruct
        {
            type::details::ProtoStaticProperty<types::bool_t> p1;
            type::details::ProtoStaticProperty<types::bool_t> p2;
            type::details::ProtoStaticProperty<types::float64_t> p3;
        };
    }

    struct TestSubStruct : type::StaticStruct<TestSubStruct>
    {
        struct p1_t : type::StaticProperty<bool_t, p1_t>
        {
            using StaticProperty::StaticProperty;
            inline static constexpr auto Metadata = []()
            { 
                constexpr auto protoTestSubStruct = details::ProtoTestSubStruct{};
                return type::StaticPropertyMetadata{ "p1", 1, true, type::StaticPropertyOffset::MakeOffset(&protoTestSubStruct._propertyArea, &protoTestSubStruct.p1) };
            }();
        };

        struct p2_t : type::StaticProperty<bool_t, p2_t>
        {
            using StaticProperty::StaticProperty;
            inline static constexpr auto Metadata = []()
            { 
                constexpr auto protoTestSubStruct = details::ProtoTestSubStruct{};
                return type::StaticPropertyMetadata{ "p2", 2, false, type::StaticPropertyOffset::MakeOffset(&protoTestSubStruct._propertyArea, &protoTestSubStruct.p2) };
            }();
        };

        struct p3_t : type::StaticProperty<float64_t, p3_t>
        {
            using StaticProperty::StaticProperty;
            inline static constexpr auto Metadata = []()
            { 
                constexpr auto protoTestSubStruct = details::ProtoTestSubStruct{};
                return type::StaticPropertyMetadata{ "p3", 3, false, type::StaticPropertyOffset::MakeOffset(&protoTestSubStruct._propertyArea, &protoTestSubStruct.p3) };
            }();
        };

        using _key_properties_t = std::tuple<p1_t*>;
        using _properties_t     = std::tuple<p1_t*, p2_t*, p3_t*>;

        template <typename P>
        const P& _getProperty() const
        {
            if constexpr (std::is_same_v<P, p1_t>)
            {
                return p1;
            }
            else if constexpr (std::is_same_v<P, p2_t>)
            {
                return p2;
            }
            else if constexpr (std::is_same_v<P, p3_t>)
            {
                return p3;
            }
            else
            {
                static_assert(std::is_same_v<P, void>, "P is not a property of struct type DotsTestStruct");
            }
        }

        template <typename P>
        P& _getProperty()
        {
            return const_cast<P&>(std::as_const(*this).template _getProperty<P>());
        }

        p1_t p1;
        p2_t p2;
        p3_t p3;

        inline static property_set_t p1_p = property_set_t::FromIndex(1);
        inline static property_set_t p2_p = property_set_t::FromIndex(2);
        inline static property_set_t p3_p = property_set_t::FromIndex(3);
    };
}

namespace dots::type
{
    template <>
    struct Descriptor<types::TestSubStruct> : StructDescriptor<types::TestSubStruct>
    {
        Descriptor(key_t key) :
            StructDescriptor(key, "TestSubStruct", Cached, types::TestSubStruct::_MakePropertyDescriptors()){}
    };
}

namespace dots::types
{
    namespace details
    {
        struct ProtoTestStruct : type::details::ProtoStaticStruct
        {
            type::details::ProtoStaticProperty<types::int32_t> intProperty;
            type::details::ProtoStaticProperty<types::string_t> stringProperty;
            type::details::ProtoStaticProperty<types::bool_t> boolProperty;
            type::details::ProtoStaticProperty<types::vector_t<float32_t>> floatVectorProperty;
            type::details::ProtoStaticProperty<types::TestSubStruct> subStruct;
        };
    }

    struct TestStruct : type::StaticStruct<TestStruct>
    {
        struct intProperty_t : type::StaticProperty<int32_t, intProperty_t>
        {
            using StaticProperty::StaticProperty;
            inline static constexpr auto Metadata = []()
            { 
                constexpr auto protoTestStruct = details::ProtoTestStruct{};
                return type::StaticPropertyMetadata{ "intProperty", 1, true, type::StaticPropertyOffset::MakeOffset(&protoTestStruct._propertyArea, &protoTestStruct.intProperty) };
            }();
        };

        struct stringProperty_t : type::StaticProperty<string_t, stringProperty_t>
        {
            using StaticProperty::StaticProperty;
            inline static constexpr auto Metadata = []()
            { 
                constexpr auto protoTestStruct = details::ProtoTestStruct{};
                return type::StaticPropertyMetadata{ "stringProperty", 2, false, type::StaticPropertyOffset::MakeOffset(&protoTestStruct._propertyArea, &protoTestStruct.stringProperty) };
            }();
        };

        struct boolProperty_t : type::StaticProperty<bool_t, boolProperty_t>
        {
            using StaticProperty::StaticProperty;
            inline static constexpr auto Metadata = []()
            { 
                constexpr auto protoTestStruct = details::ProtoTestStruct{};
                return type::StaticPropertyMetadata{ "boolProperty", 3, false, type::StaticPropertyOffset::MakeOffset(&protoTestStruct._propertyArea, &protoTestStruct.boolProperty) };
            }();
        };

        struct floatVectorProperty_t : type::StaticProperty<vector_t<float32_t>, floatVectorProperty_t>
        {
            using StaticProperty::StaticProperty;
            inline static constexpr auto Metadata = []()
            { 
                constexpr auto protoTestStruct = details::ProtoTestStruct{};
                return type::StaticPropertyMetadata{ "floatVectorProperty", 4, false, type::StaticPropertyOffset::MakeOffset(&protoTestStruct._propertyArea, &protoTestStruct.floatVectorProperty) };
            }();
        };

        struct subStruct_t : type::StaticProperty<TestSubStruct, subStruct_t>
        {
            using StaticProperty::StaticProperty;
            inline static constexpr auto Metadata = []()
            { 
                constexpr auto protoTestStruct = details::ProtoTestStruct{};
                return type::StaticPropertyMetadata{ "subStruct", 5, false, type::StaticPropertyOffset::MakeOffset(&protoTestStruct._propertyArea, &protoTestStruct.subStruct) };
            }();
        };

        using _key_properties_t = std::tuple<intProperty_t*>;
        using _properties_t     = std::tuple<intProperty_t*, stringProperty_t*, boolProperty_t*, floatVectorProperty_t*, subStruct_t*>;

        template <typename P>
        const P& _getProperty() const
        {
            if constexpr (std::is_same_v<P, intProperty_t>)
            {
                return intProperty;
            }
            else if constexpr (std::is_same_v<P, stringProperty_t>)
            {
                return stringProperty;
            }
            else if constexpr (std::is_same_v<P, boolProperty_t>)
            {
                return boolProperty;
            }
            else if constexpr (std::is_same_v<P, floatVectorProperty_t>)
            {
                return floatVectorProperty;
            }
            else if constexpr (std::is_same_v<P, subStruct_t>)
            {
                return subStruct;
            }
            else
            {
                static_assert(std::is_same_v<P, void>, "P is not a property of struct type DotsTestStruct");
            }
        }

        template <typename P>
        P& _getProperty()
        {
            return const_cast<P&>(std::as_const(*this).template _getProperty<P>());
        }

        intProperty_t intProperty;
        stringProperty_t stringProperty;
        boolProperty_t boolProperty;
        floatVectorProperty_t floatVectorProperty;
        subStruct_t subStruct;

        inline static property_set_t intProperty_p = property_set_t::FromIndex(1);
        inline static property_set_t stringProperty_p = property_set_t::FromIndex(2);
        inline static property_set_t boolProperty_p = property_set_t::FromIndex(3);
        inline static property_set_t floatVectorProperty_p = property_set_t::FromIndex(4);
        inline static property_set_t subStruct_p = property_set_t::FromIndex(5);
    };
}

namespace dots::type
{
    template <>
    struct Descriptor<types::TestStruct> : StructDescriptor<types::TestStruct>
    {
        Descriptor(key_t key) :
            StructDescriptor(key, "TestStruct", Cached, types::TestStruct::_MakePropertyDescriptors()){}
    };
}

using namespace dots::types;

struct TestStaticStructExperimental : ::testing::Test
{
protected:
};

TEST_F(TestStaticStructExperimental, PropertyOffsetsMatchActualOffsets)
{
    TestStruct sut;

    auto determine_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&sut._propertyArea()); };
    EXPECT_EQ(TestStruct::intProperty_t::Offset(), determine_offset(sut.intProperty));
    EXPECT_EQ(TestStruct::stringProperty_t::Offset(), determine_offset(sut.stringProperty));
    EXPECT_EQ(TestStruct::boolProperty_t::Offset(), determine_offset(sut.boolProperty));
    EXPECT_EQ(TestStruct::floatVectorProperty_t::Offset(), determine_offset(sut.floatVectorProperty));
    EXPECT_EQ(TestStruct::subStruct_t::Offset(), determine_offset(sut.subStruct));

    TestSubStruct sutSub;
    auto determine_sub_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&sutSub._propertyArea()); };
    EXPECT_EQ(TestSubStruct::p1_t::Offset(), determine_sub_offset(sutSub.p1));
    EXPECT_EQ(TestSubStruct::p2_t::Offset(), determine_sub_offset(sutSub.p2));
    EXPECT_EQ(TestSubStruct::p3_t::Offset(), determine_sub_offset(sutSub.p3));
}

TEST_F(TestStaticStructExperimental, PropertiesHaveExpectedTags)
{
    EXPECT_EQ(TestStruct::intProperty_t::Tag(), 1u);
    EXPECT_EQ(TestStruct::stringProperty_t::Tag(), 2u);
    EXPECT_EQ(TestStruct::boolProperty_t::Tag(), 3u);
    EXPECT_EQ(TestStruct::floatVectorProperty_t::Tag(), 4u);
}

TEST_F(TestStaticStructExperimental, PropertiesHaveExpectedNames)
{
    EXPECT_EQ(TestStruct::intProperty_t::Name(), "intProperty");
    EXPECT_EQ(TestStruct::stringProperty_t::Name(), "stringProperty");
    EXPECT_EQ(TestStruct::boolProperty_t::Name(), "boolProperty");
    EXPECT_EQ(TestStruct::floatVectorProperty_t::Name(), "floatVectorProperty");
}

TEST_F(TestStaticStructExperimental, PropertiesHaveExpectedSet)
{
    EXPECT_EQ(TestStruct::intProperty_p, dots::type::PropertySet{ 0x1 << 1 });
    EXPECT_EQ(TestStruct::stringProperty_p, dots::type::PropertySet{ 0x1 << 2 });
    EXPECT_EQ(TestStruct::boolProperty_p, dots::type::PropertySet{ 0x1 << 3 });
    EXPECT_EQ(TestStruct::floatVectorProperty_p, dots::type::PropertySet{ 0x1 << 4 });
}

TEST_F(TestStaticStructExperimental, _Descriptor_PropertyOffsetsMatchActualOffsets)
{
    TestStruct sut;

    auto determine_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&sut._propertyArea()); };
    EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[0].offset(), determine_offset(sut.intProperty));
    EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[1].offset(), determine_offset(sut.stringProperty));
    EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[2].offset(), determine_offset(sut.boolProperty));
    EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[3].offset(), determine_offset(sut.floatVectorProperty));
}

TEST_F(TestStaticStructExperimental, _Descriptor_SizeMatchesActualSize)
{
    EXPECT_EQ(TestStruct::_Descriptor().size(), sizeof(TestStruct));
}

TEST_F(TestStaticStructExperimental, _Descriptor_AlignmentMatchesActualAlignment)
{
    EXPECT_EQ(TestStruct::_Descriptor().alignment(), alignof(TestStruct));
}

TEST_F(TestStaticStructExperimental, _Descriptor_FlagsHaveExpectedValues)
{
    EXPECT_TRUE(TestStruct::_Descriptor().cached());
    EXPECT_FALSE(TestStruct::_Descriptor().internal());
    EXPECT_FALSE(TestStruct::_Descriptor().persistent());
    EXPECT_FALSE(TestStruct::_Descriptor().cleanup());
    EXPECT_FALSE(TestStruct::_Descriptor().local());
    EXPECT_FALSE(TestStruct::_Descriptor().substructOnly());
}

TEST_F(TestStaticStructExperimental, _KeyProperties)
{
    EXPECT_EQ(TestStruct::_KeyProperties(), TestStruct::intProperty_p);
}

TEST_F(TestStaticStructExperimental, ctor_Initializer)
{
    TestStruct sut{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f, 2.7183f }
    };

    EXPECT_EQ(sut.intProperty, 1);
    EXPECT_EQ(*sut.stringProperty, "foo");
    EXPECT_FALSE(sut.boolProperty.isValid());
    EXPECT_EQ(sut.floatVectorProperty, vector_t<float32_t>({ 3.1415f, 2.7183f }));
}

TEST_F(TestStaticStructExperimental, ctor_Copy)
{
    TestStruct sutOther{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f, 2.7183f }
    };
    TestStruct sutThis{ sutOther };

    EXPECT_EQ(sutThis.intProperty, sutOther.intProperty);
    EXPECT_EQ(sutThis.stringProperty, sutOther.stringProperty);
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, sutOther.floatVectorProperty);
}

TEST_F(TestStaticStructExperimental, ctor_Move)
{
    TestStruct sutOther{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f, 2.7183f }
    };
    TestStruct sutThis{ std::move(sutOther) };

    EXPECT_EQ(sutThis.intProperty, 1);
    EXPECT_EQ(*sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>({ 3.1415f, 2.7183f }));

    EXPECT_FALSE(sutOther.intProperty.isValid());
    EXPECT_FALSE(sutOther.stringProperty.isValid());
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_FALSE(sutOther.floatVectorProperty.isValid());
}

TEST_F(TestStaticStructExperimental, assignment_Copy)
{
    TestStruct sutOther{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f, 2.7183f }
    };
    TestStruct sutThis = sutOther;

    EXPECT_EQ(sutThis.intProperty, sutOther.intProperty);
    EXPECT_EQ(sutThis.stringProperty, sutOther.stringProperty);
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, sutOther.floatVectorProperty);
}

TEST_F(TestStaticStructExperimental, assignment_Move)
{
    TestStruct sutOther{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f, 2.7183f }
    };
    TestStruct sutThis = std::move(sutOther);

    EXPECT_EQ(sutThis.intProperty, 1);
    EXPECT_EQ(*sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>({ 3.1415f, 2.7183f }));

    EXPECT_FALSE(sutOther.intProperty.isValid());
    EXPECT_FALSE(sutOther.stringProperty.isValid());
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_FALSE(sutOther.floatVectorProperty.isValid());
}

TEST_F(TestStaticStructExperimental, assign_CompleteAssign)
{
    TestStruct sutThis{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutOther{
        .intProperty = 2,
        .stringProperty = "bar",
        .floatVectorProperty = { 2.7183f }
    };

    sutThis._assign(sutOther);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(*sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });
}

TEST_F(TestStaticStructExperimental, assign_PartialAssign)
{
    TestStruct sutThis{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutOther{
        .intProperty = 2,
        .stringProperty = "bar"
    };

    sutThis._assign(sutOther, ~TestStruct::floatVectorProperty_p);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(*sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_FALSE(sutThis.floatVectorProperty.isValid());
}

TEST_F(TestStaticStructExperimental, copy_CompleteCopy)
{
    TestStruct sutThis{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutOther{
        .intProperty = 2,
        .stringProperty = "bar",
        .floatVectorProperty = { 2.7183f }
    };

    sutThis._copy(sutOther);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(*sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });
}

TEST_F(TestStaticStructExperimental, copy_PartialCopy)
{
    TestStruct sutThis{
        .intProperty = 1,
        .stringProperty = "foo",
        .boolProperty = true,
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutOther{
        .intProperty = 2,
        .stringProperty = "bar"
    };

    sutThis._copy(sutOther, TestStruct::stringProperty_p + TestStruct::boolProperty_p);

    EXPECT_EQ(sutThis.intProperty, 1);
    EXPECT_EQ(*sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestStaticStructExperimental, merge_CompleteMerge)
{
    TestStruct sutThis{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutOther{
        .intProperty = 2,
        .stringProperty = "bar"
    };

    sutThis._merge(sutOther);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(*sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestStaticStructExperimental, merge_PartialMerge)
{
    TestStruct sutThis{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutOther{
        .intProperty = 2,
        .stringProperty = "bar",
        .floatVectorProperty = { 2.7183f }
    };

    sutThis._merge(sutOther, ~TestStruct::stringProperty_p);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(*sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });
}

TEST_F(TestStaticStructExperimental, merge_PartialMergeSubStruct)
{
    TestStruct sutThis{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f },
        .subStruct = TestSubStruct{
            .p2 = true
        }
    };

    TestStruct sutOther{
        .intProperty = 2,
        .stringProperty = "bar",
        .floatVectorProperty = { 2.7183f },
        .subStruct = TestSubStruct{
            .p1 = true
        }
    };

    sutThis._merge(sutOther, ~TestStruct::stringProperty_p);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(*sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });
    EXPECT_EQ(sutThis.subStruct->p1, true);
    EXPECT_EQ(sutThis.subStruct->p2, true);
}

TEST_F(TestStaticStructExperimental, swap_CompleteSwap)
{
    TestStruct sutThis{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutOther{
        .intProperty = 2,
        .stringProperty = "bar"
    };

    sutThis._swap(sutOther);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(*sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_FALSE(sutThis.floatVectorProperty.isValid());

    EXPECT_EQ(sutOther.intProperty, 1);
    EXPECT_EQ(*sutOther.stringProperty, "foo");
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_EQ(sutOther.floatVectorProperty, vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestStaticStructExperimental, swap_PartialSwap)
{
    TestStruct sutThis{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutOther{
        .intProperty = 2,
        .stringProperty = "bar",
        .floatVectorProperty = { 2.7183f }
    };

    sutThis._swap(sutOther, TestStruct::floatVectorProperty_p);

    EXPECT_EQ(sutThis.intProperty, 1);
    EXPECT_EQ(*sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });

    EXPECT_EQ(sutOther.intProperty, 2);
    EXPECT_EQ(*sutOther.stringProperty, "bar");
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_EQ(sutOther.floatVectorProperty, vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestStaticStructExperimental, clear_CompleteClear)
{
    TestStruct sut{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    sut._clear();

    EXPECT_FALSE(sut.intProperty.isValid());
    EXPECT_FALSE(sut.stringProperty.isValid());
    EXPECT_FALSE(sut.boolProperty.isValid());
    EXPECT_FALSE(sut.floatVectorProperty.isValid());
}

TEST_F(TestStaticStructExperimental, clear_PartialClear)
{
    TestStruct sut{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    sut._clear(~TestStruct::stringProperty_p);

    EXPECT_FALSE(sut.intProperty.isValid());
    EXPECT_EQ(*sut.stringProperty, "foo");
    EXPECT_FALSE(sut.boolProperty.isValid());
    EXPECT_FALSE(sut.floatVectorProperty.isValid());
}

TEST_F(TestStaticStructExperimental, equal)
{
    TestStruct sutLhs{
        .intProperty = 2,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutRhs{
        .intProperty = 2,
        .stringProperty = "bar",
        .floatVectorProperty = { 3.1415f }
    };

    EXPECT_TRUE(sutLhs._equal(sutLhs));
    EXPECT_TRUE(sutRhs._equal(sutRhs));
    EXPECT_FALSE(sutLhs._equal(sutRhs));

    EXPECT_TRUE(sutLhs._equal(sutRhs, TestStruct::intProperty_p));
    EXPECT_TRUE(sutLhs._equal(sutRhs, TestStruct::floatVectorProperty_p));
    EXPECT_FALSE(sutLhs._equal(sutRhs, TestStruct::intProperty_p + TestStruct::stringProperty_p));
}

TEST_F(TestStaticStructExperimental, same)
{
    TestStruct sut1{
        .intProperty = 1,
        .stringProperty = "foo",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sut2{
        .intProperty = 1,
        .stringProperty = "bar",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sut3{
        .intProperty = 2,
        .stringProperty = "foo",
        .floatVectorProperty = { 2.7183f }
    };

    EXPECT_TRUE(sut1._same(sut1));
    EXPECT_TRUE(sut1._same(sut2));
    EXPECT_FALSE(sut1._same(sut3));
}

TEST_F(TestStaticStructExperimental, less_lessEqual_greater_greaterEqual)
{
    TestStruct sutLhs{
        .intProperty = 2,
        .stringProperty = "bar",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutRhs{
        .intProperty = 2,
        .stringProperty = "foo",
        .floatVectorProperty = { 2.7183f }
    };

    EXPECT_TRUE(sutLhs._less(sutRhs));
    EXPECT_FALSE(sutRhs._less(sutLhs));
    EXPECT_TRUE(sutRhs._less(sutLhs, TestStruct::floatVectorProperty_p));
    EXPECT_FALSE(sutLhs._less(sutLhs));
    EXPECT_FALSE(sutLhs._less(sutLhs, TestStruct::boolProperty_p));

    EXPECT_TRUE(sutLhs._lessEqual(sutRhs));
    EXPECT_FALSE(sutRhs._lessEqual(sutLhs));
    EXPECT_TRUE(sutRhs._lessEqual(sutLhs, TestStruct::floatVectorProperty_p));
    EXPECT_TRUE(sutLhs._lessEqual(sutLhs));
    EXPECT_TRUE(sutLhs._lessEqual(sutLhs, TestStruct::boolProperty_p));

    EXPECT_FALSE(sutLhs._greater(sutRhs));
    EXPECT_TRUE(sutRhs._greater(sutLhs));
    EXPECT_FALSE(sutRhs._greater(sutLhs, TestStruct::floatVectorProperty_p));
    EXPECT_FALSE(sutLhs._greater(sutLhs));
    EXPECT_FALSE(sutLhs._greater(sutLhs, TestStruct::boolProperty_p));

    EXPECT_FALSE(sutLhs._greaterEqual(sutRhs));
    EXPECT_TRUE(sutRhs._greaterEqual(sutLhs));
    EXPECT_FALSE(sutRhs._greaterEqual(sutLhs, TestStruct::floatVectorProperty_p));
    EXPECT_TRUE(sutLhs._greaterEqual(sutLhs));
    EXPECT_TRUE(sutLhs._greaterEqual(sutLhs, TestStruct::boolProperty_p));
}

TEST_F(TestStaticStructExperimental, diffProperties)
{
    TestStruct sutLhs{
        .intProperty = 2,
        .stringProperty = "bar",
        .floatVectorProperty = { 3.1415f }
    };

    TestStruct sutRhs{
        .intProperty = 2,
        .stringProperty = "foo",
        .floatVectorProperty = { 2.7183f }
    };

    EXPECT_EQ(sutLhs._diffProperties(sutRhs), TestStruct::stringProperty_p + TestStruct::floatVectorProperty_p);
    EXPECT_EQ(sutLhs._diffProperties(sutRhs, (~TestStruct::floatVectorProperty_p)), TestStruct::stringProperty_p);
}

TEST_F(TestStaticStructExperimental, assertProperties)
{
    TestStruct sut{
        .intProperty = 1,
        .stringProperty = "foo"
    };

    EXPECT_NO_THROW(sut._assertHasProperties(TestStruct::intProperty_p));
    EXPECT_THROW(sut._assertHasProperties<false>(TestStruct::intProperty_p), std::logic_error);

    EXPECT_NO_THROW(sut._assertHasProperties(TestStruct::intProperty_p + TestStruct::stringProperty_p));
    EXPECT_NO_THROW(sut._assertHasProperties<false>(TestStruct::intProperty_p + TestStruct::stringProperty_p));

    EXPECT_THROW(sut._assertHasProperties(TestStruct::intProperty_p + TestStruct::floatVectorProperty_p), std::logic_error);
    EXPECT_THROW(sut._assertHasProperties<false>(TestStruct::intProperty_p + TestStruct::floatVectorProperty_p), std::logic_error);
}