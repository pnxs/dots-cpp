#include <dots/type/StaticStruct.h>
#include <dots/type/StaticProperty.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/TPropertyInitializer.h>
#include <gtest/gtest.h>

namespace dots::types
{
    struct TestStruct : type::StaticStruct<TestStruct>
    {
        struct intProperty_t : type::StaticProperty<int32_t, intProperty_t>
        {
			inline static auto Descriptor = type::PropertyDescriptor<int32_t>("intProperty", 1, true);
        };
    	
        struct stringProperty_t : type::StaticProperty<string_t, stringProperty_t>
        {
			inline static auto Descriptor = dots::type::PropertyDescriptor<string_t>("stringProperty", intProperty_t::Descriptor, 2, false);
        };

    	struct boolProperty_t : type::StaticProperty<bool_t, boolProperty_t>
        {
			inline static auto Descriptor = dots::type::PropertyDescriptor<bool_t>("boolProperty", stringProperty_t::Descriptor, 3, false);
        };
    	
        struct floatVectorProperty_t : type::StaticProperty<vector_t<float32_t>, floatVectorProperty_t>
        {
			inline static auto Descriptor = type::PropertyDescriptor<vector_t<float32_t>>("floatVectorProperty", boolProperty_t::Descriptor, 4, false);
        };

    	using intProperty_i = type::TPropertyInitializer<intProperty_t>;
    	using stringProperty_i = type::TPropertyInitializer<stringProperty_t>;
    	using boolProperty_i = type::TPropertyInitializer<boolProperty_t>;
    	using floatVectorProperty_i = type::TPropertyInitializer<floatVectorProperty_t>;
    	
        using _key_properties_t = std::tuple<intProperty_t*>;
        using _properties_t     = std::tuple<intProperty_t*, stringProperty_t*, boolProperty_t*, floatVectorProperty_t*>;

		using StaticStruct::StaticStruct;
		TestStruct(const TestStruct& other) = default;
		TestStruct(TestStruct&& other) = default;
		~TestStruct() = default;

		TestStruct& operator = (const TestStruct& sutRhs) = default;
		TestStruct& operator = (TestStruct&& sutRhs) = default;        

        intProperty_t intProperty;
        stringProperty_t stringProperty;
    	boolProperty_t boolProperty;
        floatVectorProperty_t floatVectorProperty;
    };
}

namespace dots::type
{
	template <>
	struct Descriptor<types::TestStruct> : StructDescriptor<types::TestStruct>
	{
		Descriptor() :
			StructDescriptor("TestStruct", Cached, {
        		nullptr,
	            &types::TestStruct::intProperty_t::Descriptor,
	            &types::TestStruct::stringProperty_t::Descriptor,
				&types::TestStruct::boolProperty_t::Descriptor,
	            &types::TestStruct::floatVectorProperty_t::Descriptor
			}){}
	};
}

using namespace dots::types;

struct TestStaticStruct : ::testing::Test
{
protected:
};

TEST_F(TestStaticStruct, PropertyOffsetsMatchActualOffsets)
{
	TestStruct sut;
	
	auto determine_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&sut._propertyArea()); };
	EXPECT_EQ(TestStruct::intProperty_t::Offset(), determine_offset(sut.intProperty));
	EXPECT_EQ(TestStruct::stringProperty_t::Offset(), determine_offset(sut.stringProperty));
	EXPECT_EQ(TestStruct::boolProperty_t::Offset(), determine_offset(sut.boolProperty));
	EXPECT_EQ(TestStruct::floatVectorProperty_t::Offset(), determine_offset(sut.floatVectorProperty));
}

TEST_F(TestStaticStruct, PropertiesHaveExpectedTags)
{
	EXPECT_EQ(TestStruct::intProperty_t::Tag(), 1);
	EXPECT_EQ(TestStruct::stringProperty_t::Tag(), 2);
	EXPECT_EQ(TestStruct::boolProperty_t::Tag(), 3);
	EXPECT_EQ(TestStruct::floatVectorProperty_t::Tag(), 4);
}

TEST_F(TestStaticStruct, PropertiesHaveExpectedNames)
{
	EXPECT_EQ(TestStruct::intProperty_t::Name(), "intProperty");
	EXPECT_EQ(TestStruct::stringProperty_t::Name(), "stringProperty");
	EXPECT_EQ(TestStruct::boolProperty_t::Name(), "boolProperty");
	EXPECT_EQ(TestStruct::floatVectorProperty_t::Name(), "floatVectorProperty");
}

TEST_F(TestStaticStruct, PropertiesHaveExpectedSet)
{
	EXPECT_EQ(TestStruct::intProperty_t::Set(), dots::type::PropertySet{ 0x1 << 1 });
	EXPECT_EQ(TestStruct::stringProperty_t::Set(), dots::type::PropertySet{ 0x1 << 2 });
	EXPECT_EQ(TestStruct::boolProperty_t::Set(), dots::type::PropertySet{ 0x1 << 3 });
	EXPECT_EQ(TestStruct::floatVectorProperty_t::Set(), dots::type::PropertySet{ 0x1 << 4 });
}

TEST_F(TestStaticStruct, _Descriptor_PropertyOffsetsMatchActualOffsets)
{
	TestStruct sut;
	
	auto determine_offset = [&](const auto& property) { return reinterpret_cast<size_t>(&property) - reinterpret_cast<size_t>(&sut._propertyArea()); };
	EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[1]->offset(), determine_offset(sut.intProperty));
	EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[2]->offset(), determine_offset(sut.stringProperty));
	EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[3]->offset(), determine_offset(sut.boolProperty));
	EXPECT_EQ(TestStruct::_Descriptor().propertyDescriptors()[4]->offset(), determine_offset(sut.floatVectorProperty));
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
	EXPECT_EQ(TestStruct::_KeyProperties(), TestStruct::intProperty_t::Set());
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

	sutThis._assign(sutOther, ~TestStruct::floatVectorProperty_t::Set());

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
		TestStruct::floatVectorProperty_i{ vector_t<float32_t>{ 3.1415f } }
	};

	TestStruct sutOther{
		TestStruct::intProperty_i{ 2 },
		TestStruct::stringProperty_i{ "bar" }
	};

	sutThis._copy(sutOther, ~TestStruct::floatVectorProperty_t::Set());

	EXPECT_EQ(sutThis.intProperty, 2);
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

	sutThis._merge(sutOther, ~TestStruct::stringProperty_t::Set());

	EXPECT_EQ(sutThis.intProperty, 2);
	EXPECT_EQ(sutThis.stringProperty, "foo");
	EXPECT_FALSE(sutThis.boolProperty.isValid());
	EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });
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

	sutThis._swap(sutOther, TestStruct::floatVectorProperty_t::Set());

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

	sut._clear(~TestStruct::stringProperty_t::Set());

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
	
	EXPECT_TRUE(sutLhs._equal(sutRhs, TestStruct::intProperty_t::Set()));
	EXPECT_TRUE(sutLhs._equal(sutRhs, TestStruct::floatVectorProperty_t::Set()));
	EXPECT_FALSE(sutLhs._equal(sutRhs, TestStruct::intProperty_t::Set() + TestStruct::stringProperty_t::Set()));
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
	EXPECT_TRUE(sutRhs._less(sutLhs, TestStruct::floatVectorProperty_t::Set()));
	EXPECT_FALSE(sutLhs._less(sutLhs));
	EXPECT_FALSE(sutLhs._less(sutLhs, TestStruct::boolProperty_t::Set()));

	EXPECT_TRUE(sutLhs._lessEqual(sutRhs));
	EXPECT_FALSE(sutRhs._lessEqual(sutLhs));
	EXPECT_TRUE(sutRhs._lessEqual(sutLhs, TestStruct::floatVectorProperty_t::Set()));
	EXPECT_TRUE(sutLhs._lessEqual(sutLhs));
	EXPECT_TRUE(sutLhs._lessEqual(sutLhs, TestStruct::boolProperty_t::Set()));

	EXPECT_FALSE(sutLhs._greater(sutRhs));
	EXPECT_TRUE(sutRhs._greater(sutLhs));
	EXPECT_FALSE(sutRhs._greater(sutLhs, TestStruct::floatVectorProperty_t::Set()));
	EXPECT_FALSE(sutLhs._greater(sutLhs));
	EXPECT_FALSE(sutLhs._greater(sutLhs, TestStruct::boolProperty_t::Set()));

	EXPECT_FALSE(sutLhs._greaterEqual(sutRhs));
	EXPECT_TRUE(sutRhs._greaterEqual(sutLhs));
	EXPECT_FALSE(sutRhs._greaterEqual(sutLhs, TestStruct::floatVectorProperty_t::Set()));
	EXPECT_TRUE(sutLhs._greaterEqual(sutLhs));
	EXPECT_TRUE(sutLhs._greaterEqual(sutLhs, TestStruct::boolProperty_t::Set()));
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
	
	EXPECT_EQ(sutLhs._diffProperties(sutRhs), TestStruct::stringProperty_t::Set() + TestStruct::floatVectorProperty_t::Set());
	EXPECT_EQ(sutLhs._diffProperties(sutRhs, (~TestStruct::floatVectorProperty_t::Set())), TestStruct::stringProperty_t::Set());
}