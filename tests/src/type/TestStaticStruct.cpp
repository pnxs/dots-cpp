// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/type/StaticStruct.h>
#include <dots/type/StaticProperty.h>
#include <dots/type/StaticPropertyOffset.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/PropertyInitializer.h>
#include <dots/testing/gtest/gtest.h>
#include <TestStruct.dots.h>

using namespace dots::types;

struct TestStaticStruct : ::testing::Test
{
};

TEST_F(TestStaticStruct, PropertyOffsetsMatchActualOffsets)
{
    TestStruct sut;

    auto determine_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&sut._propertyArea()); };
    EXPECT_EQ(TestStruct::intProperty_pt::Offset(), determine_offset(sut.intProperty));
    EXPECT_EQ(TestStruct::stringProperty_pt::Offset(), determine_offset(sut.stringProperty));
    EXPECT_EQ(TestStruct::boolProperty_pt::Offset(), determine_offset(sut.boolProperty));
    EXPECT_EQ(TestStruct::floatVectorProperty_pt::Offset(), determine_offset(sut.floatVectorProperty));
    EXPECT_EQ(TestStruct::subStruct_pt::Offset(), determine_offset(sut.subStruct));

    TestSubStruct sutSub;
    auto determine_sub_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&sutSub._propertyArea()); };
    EXPECT_EQ(TestSubStruct::p1_pt::Offset(), determine_sub_offset(sutSub.p1));
    EXPECT_EQ(TestSubStruct::p2_pt::Offset(), determine_sub_offset(sutSub.p2));
    EXPECT_EQ(TestSubStruct::p3_pt::Offset(), determine_sub_offset(sutSub.p3));
}

TEST_F(TestStaticStruct, PropertiesHaveExpectedTags)
{
    EXPECT_EQ(TestStruct::intProperty_pt::Tag(), 1u);
    EXPECT_EQ(TestStruct::stringProperty_pt::Tag(), 2u);
    EXPECT_EQ(TestStruct::boolProperty_pt::Tag(), 3u);
    EXPECT_EQ(TestStruct::floatVectorProperty_pt::Tag(), 4u);
}

TEST_F(TestStaticStruct, PropertiesHaveExpectedNames)
{
    EXPECT_EQ(TestStruct::intProperty_pt::Name(), "intProperty");
    EXPECT_EQ(TestStruct::stringProperty_pt::Name(), "stringProperty");
    EXPECT_EQ(TestStruct::boolProperty_pt::Name(), "boolProperty");
    EXPECT_EQ(TestStruct::floatVectorProperty_pt::Name(), "floatVectorProperty");
}

TEST_F(TestStaticStruct, PropertiesHaveExpectedSet)
{
    EXPECT_EQ(TestStruct::intProperty_p, dots::type::PropertySet{ 0x1 << 1 });
    EXPECT_EQ(TestStruct::stringProperty_p, dots::type::PropertySet{ 0x1 << 2 });
    EXPECT_EQ(TestStruct::boolProperty_p, dots::type::PropertySet{ 0x1 << 3 });
    EXPECT_EQ(TestStruct::floatVectorProperty_p, dots::type::PropertySet{ 0x1 << 4 });
}

TEST_F(TestStaticStruct, _Descriptor_PropertyOffsetsMatchActualOffsets)
{
    TestStruct sut;

    auto determine_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&sut._propertyArea()); };
    EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[0].offset(), determine_offset(sut.intProperty));
    EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[1].offset(), determine_offset(sut.stringProperty));
    EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[2].offset(), determine_offset(sut.boolProperty));
    EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[3].offset(), determine_offset(sut.floatVectorProperty));
}

TEST_F(TestStaticStruct, _Descriptor_SizeMatchesActualSize)
{
    EXPECT_EQ(TestStruct::_Descriptor().size(), sizeof(TestStruct));
}

TEST_F(TestStaticStruct, _Descriptor_AlignmentMatchesActualAlignment)
{
    EXPECT_EQ(TestStruct::_Descriptor().alignment(), alignof(TestStruct));
}

TEST_F(TestStaticStruct, _Descriptor_FlagsHaveExpectedValues)
{
    EXPECT_TRUE(TestStruct::_Descriptor().cached());
    EXPECT_FALSE(TestStruct::_Descriptor().internal());
    EXPECT_FALSE(TestStruct::_Descriptor().persistent());
    EXPECT_FALSE(TestStruct::_Descriptor().cleanup());
    EXPECT_FALSE(TestStruct::_Descriptor().local());
    EXPECT_FALSE(TestStruct::_Descriptor().substructOnly());
}

TEST_F(TestStaticStruct, _KeyProperties)
{
    EXPECT_EQ(TestStruct::_KeyProperties(), TestStruct::intProperty_p);
}

TEST_F(TestStaticStruct, ctor_Initializer)
{
    TestStruct sut{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };

    EXPECT_EQ(sut.intProperty, 1);
    EXPECT_EQ(sut.stringProperty, "foo");
    EXPECT_FALSE(sut.boolProperty.isValid());
    EXPECT_EQ(sut.floatVectorProperty, vector_t<float32_t>({ 3.1415f, 2.7183f }));
}

TEST_F(TestStaticStruct, ctor_InitializeFromCompatibleProperty)
{
    TestSubStruct subStruct{
        TestSubStruct::p1_i{ true }
    };

    TestStruct sut{
        TestStruct::boolProperty_i{ subStruct.p1 }
    };

    EXPECT_FALSE(sut.intProperty.isValid());
    EXPECT_FALSE(sut.stringProperty.isValid());
    EXPECT_EQ(sut.boolProperty, true);
    EXPECT_FALSE(sut.floatVectorProperty.isValid());
}

TEST_F(TestStaticStruct, ctor_Copy)
{
    TestStruct sutOther{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    TestStruct sutThis{ sutOther };

    EXPECT_EQ(sutThis.intProperty, sutOther.intProperty);
    EXPECT_EQ(sutThis.stringProperty, sutOther.stringProperty);
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, sutOther.floatVectorProperty);
}

TEST_F(TestStaticStruct, ctor_Move)
{
    TestStruct sutOther{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    TestStruct sutThis{ std::move(sutOther) };

    EXPECT_EQ(sutThis.intProperty, 1);
    EXPECT_EQ(sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>({ 3.1415f, 2.7183f }));

    EXPECT_FALSE(sutOther.intProperty.isValid());
    EXPECT_FALSE(sutOther.stringProperty.isValid());
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_FALSE(sutOther.floatVectorProperty.isValid());
}

TEST_F(TestStaticStruct, assignment_Copy)
{
    TestStruct sutOther{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    TestStruct sutThis = sutOther;

    EXPECT_EQ(sutThis.intProperty, sutOther.intProperty);
    EXPECT_EQ(sutThis.stringProperty, sutOther.stringProperty);
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, sutOther.floatVectorProperty);
}

TEST_F(TestStaticStruct, assignment_Move)
{
    TestStruct sutOther{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    TestStruct sutThis = std::move(sutOther);

    EXPECT_EQ(sutThis.intProperty, 1);
    EXPECT_EQ(sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>({ 3.1415f, 2.7183f }));

    EXPECT_FALSE(sutOther.intProperty.isValid());
    EXPECT_FALSE(sutOther.stringProperty.isValid());
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_FALSE(sutOther.floatVectorProperty.isValid());
}

TEST_F(TestStaticStruct, assignment_FromValidCompatibleProperty)
{
    TestSubStruct subStruct{
        TestSubStruct::p1_i{ true }
    };

    TestStruct sut{
        TestStruct::boolProperty_i{ false }
    };

    EXPECT_EQ(sut.boolProperty, false);

    sut.boolProperty = subStruct.p1;
    EXPECT_EQ(sut.boolProperty, true);
}

TEST_F(TestStaticStruct, assignment_FromInvalidCompatibleProperty)
{
    TestSubStruct subStruct;

    TestStruct sut{
        TestStruct::boolProperty_i{ true }
    };

    EXPECT_EQ(sut.boolProperty, true);

    sut.boolProperty = subStruct.p1;
    EXPECT_FALSE(sut.boolProperty.isValid());
}

TEST_F(TestStaticStruct, assign_CompleteAssign)
{
    TestStruct sutThis{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutOther{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 2.7183f } }
    };

    sutThis._assign(sutOther);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });
}

TEST_F(TestStaticStruct, assign_PartialAssign)
{
    TestStruct sutThis{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutOther{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" }
    };

    sutThis._assign(sutOther, ~TestStruct::floatVectorProperty_p);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_FALSE(sutThis.floatVectorProperty.isValid());
}

TEST_F(TestStaticStruct, copy_CompleteCopy)
{
    TestStruct sutThis{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutOther{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 2.7183f } }
    };

    sutThis._copy(sutOther);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });
}

TEST_F(TestStaticStruct, copy_PartialCopy)
{
    TestStruct sutThis{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::boolProperty_i{ true },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutOther{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" }
    };

    sutThis._copy(sutOther, TestStruct::stringProperty_p + TestStruct::boolProperty_p);

    EXPECT_EQ(sutThis.intProperty, 1);
    EXPECT_EQ(sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestStaticStruct, merge_CompleteMerge)
{
    TestStruct sutThis{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutOther{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" }
    };

    sutThis._merge(sutOther);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestStaticStruct, merge_PartialMerge)
{
    TestStruct sutThis{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutOther{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 2.7183f } }
    };

    sutThis._merge(sutOther, ~TestStruct::stringProperty_p);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });
}

TEST_F(TestStaticStruct, merge_PartialMergeSubStruct)
{
    TestStruct sutThis{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } },
        TestStruct::subStruct_i{
            TestSubStruct::p2_i(true)
        }
    };

    TestStruct sutOther{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 2.7183f } },
        TestStruct::subStruct_i{
            TestSubStruct::p1_i(true)
        }
    };

    sutThis._merge(sutOther, ~TestStruct::stringProperty_p);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });
    EXPECT_EQ(sutThis.subStruct->p1, true);
    EXPECT_EQ(sutThis.subStruct->p2, true);
}

TEST_F(TestStaticStruct, swap_CompleteSwap)
{
    TestStruct sutThis{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutOther{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" }
    };

    sutThis._swap(sutOther);

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(sutThis.stringProperty, "bar");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_FALSE(sutThis.floatVectorProperty.isValid());

    EXPECT_EQ(sutOther.intProperty, 1);
    EXPECT_EQ(sutOther.stringProperty, "foo");
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_EQ(sutOther.floatVectorProperty, vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestStaticStruct, swap_PartialSwap)
{
    TestStruct sutThis{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutOther{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 2.7183f } }
    };

    sutThis._swap(sutOther, TestStruct::floatVectorProperty_p);

    EXPECT_EQ(sutThis.intProperty, 1);
    EXPECT_EQ(sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });

    EXPECT_EQ(sutOther.intProperty, 2);
    EXPECT_EQ(sutOther.stringProperty, "bar");
    EXPECT_FALSE(sutOther.boolProperty.isValid());
    EXPECT_EQ(sutOther.floatVectorProperty, vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestStaticStruct, clear_CompleteClear)
{
    TestStruct sut{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    sut._clear();

    EXPECT_FALSE(sut.intProperty.isValid());
    EXPECT_FALSE(sut.stringProperty.isValid());
    EXPECT_FALSE(sut.boolProperty.isValid());
    EXPECT_FALSE(sut.floatVectorProperty.isValid());
}

TEST_F(TestStaticStruct, clear_PartialClear)
{
    TestStruct sut{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    sut._clear(~TestStruct::stringProperty_p);

    EXPECT_FALSE(sut.intProperty.isValid());
    EXPECT_EQ(sut.stringProperty, "foo");
    EXPECT_FALSE(sut.boolProperty.isValid());
    EXPECT_FALSE(sut.floatVectorProperty.isValid());
}

TEST_F(TestStaticStruct, equal)
{
    TestStruct sutLhs{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutRhs{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    EXPECT_TRUE(sutLhs._equal(sutLhs));
    EXPECT_TRUE(sutRhs._equal(sutRhs));
    EXPECT_FALSE(sutLhs._equal(sutRhs));

    EXPECT_TRUE(sutLhs._equal(sutRhs, TestStruct::intProperty_p));
    EXPECT_TRUE(sutLhs._equal(sutRhs, TestStruct::floatVectorProperty_p));
    EXPECT_FALSE(sutLhs._equal(sutRhs, TestStruct::intProperty_p + TestStruct::stringProperty_p));
}

TEST_F(TestStaticStruct, same)
{
    TestStruct sut1{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sut2{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "bar" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sut3{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 2.7183f } }
    };

    EXPECT_TRUE(sut1._same(sut1));
    EXPECT_TRUE(sut1._same(sut2));
    EXPECT_FALSE(sut1._same(sut3));
}

TEST_F(TestStaticStruct, less_lessEqual_greater_greaterEqual)
{
    TestStruct sutLhs{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutRhs{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 2.7183f } }
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

TEST_F(TestStaticStruct, diffProperties)
{
    TestStruct sutLhs{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "bar" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
    };

    TestStruct sutRhs{
        TestStruct::intProperty_i{ 2 },
        TestStruct::stringProperty_i{ "foo" },
        TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 2.7183f } }
    };

    EXPECT_EQ(sutLhs._diffProperties(sutRhs), TestStruct::stringProperty_p + TestStruct::floatVectorProperty_p);
    EXPECT_EQ(sutLhs._diffProperties(sutRhs, (~TestStruct::floatVectorProperty_p)), TestStruct::stringProperty_p);
}

TEST_F(TestStaticStruct, assertProperties)
{
    TestStruct sut{
        TestStruct::intProperty_i{ 1 },
        TestStruct::stringProperty_i{ "foo" }
    };

    EXPECT_NO_THROW(sut._assertHasProperties(TestStruct::intProperty_p));
    EXPECT_THROW(sut._assertHasProperties<false>(TestStruct::intProperty_p), std::logic_error);

    EXPECT_NO_THROW(sut._assertHasProperties(TestStruct::intProperty_p + TestStruct::stringProperty_p));
    EXPECT_NO_THROW(sut._assertHasProperties<false>(TestStruct::intProperty_p + TestStruct::stringProperty_p));

    EXPECT_THROW(sut._assertHasProperties(TestStruct::intProperty_p + TestStruct::floatVectorProperty_p), std::logic_error);
    EXPECT_THROW(sut._assertHasProperties<false>(TestStruct::intProperty_p + TestStruct::floatVectorProperty_p), std::logic_error);
}
