#include <dots/type/StaticStruct.h>
#include <dots/type/StaticProperty.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/PropertyInitializer.h>
#include <gtest/gtest.h>

namespace dots::types
{
    struct TestSubStruct : type::StaticStruct<TestSubStruct>
    {
        struct p1_t : type::StaticProperty<int32_t, p1_t>
        {
            static constexpr auto Metadata = type::PropertyMetadata<types::int32_t>{ "p1", 1, true};
        };

        struct p2_t : type::StaticProperty<int32_t, p2_t>
        {
            static constexpr auto Metadata = type::PropertyMetadata<types::int32_t>{ "p2", 2, false, p1_t::Metadata };
        };

        using p1_i = type::PropertyInitializer<p1_t>;
        using p2_i = type::PropertyInitializer<p2_t>;

        using _key_properties_t = std::tuple<p1_t*>;
        using _properties_t     = std::tuple<p1_t*, p2_t*>;

        TestSubStruct() = default;
        TestSubStruct(const TestSubStruct& other) = default;
        TestSubStruct(TestSubStruct&& other) = default;
        ~TestSubStruct() = default;

        TestSubStruct& operator = (const TestSubStruct& sutRhs) = default;
        TestSubStruct& operator = (TestSubStruct&& sutRhs) = default;

        template <typename... PropertyInitializers, std::enable_if_t<sizeof...(PropertyInitializers) >= 1 && std::conjunction_v<type::is_property_initializer_t<std::remove_pointer_t<std::decay_t<PropertyInitializers>>>...>, int> = 0>
        explicit TestSubStruct(PropertyInitializers&&... propertyInitializers)
        {
            (_getProperty<typename std::remove_pointer_t<std::decay_t<PropertyInitializers>>::property_t>().template construct<false>(std::forward<decltype(propertyInitializers)>(propertyInitializers).value), ...);
        }

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
    };

    struct TestStruct : type::StaticStruct<TestStruct>
    {
        struct intProperty_t : type::StaticProperty<int32_t, intProperty_t>
        {
        	static constexpr auto Metadata = type::PropertyMetadata<types::int32_t>{ "intProperty", 1, true };
        };
    	
        struct stringProperty_t : type::StaticProperty<string_t, stringProperty_t>
        {
        	static constexpr auto Metadata = type::PropertyMetadata<types::string_t>{ "stringProperty", 2, false, intProperty_t::Metadata };
        };

    	struct boolProperty_t : type::StaticProperty<bool_t, boolProperty_t>
        {
    		static constexpr auto Metadata = type::PropertyMetadata<types::bool_t>{ "boolProperty", 3, false, stringProperty_t::Metadata };
        };
    	
        struct floatVectorProperty_t : type::StaticProperty<vector_t<types::float32_t>, floatVectorProperty_t>
        {
        	static constexpr auto Metadata = type::PropertyMetadata<types::vector_t<types::float32_t>>{ "floatVectorProperty", 4, false, boolProperty_t::Metadata };
        };

        struct subStruct_t : type::StaticProperty<TestSubStruct, subStruct_t>
        {
            static constexpr auto Metadata = type::PropertyMetadata<TestSubStruct>{ "subStruct", 5, false, floatVectorProperty_t::Metadata };
        };

    	using intProperty_i = type::PropertyInitializer<intProperty_t>;
    	using stringProperty_i = type::PropertyInitializer<stringProperty_t>;
    	using boolProperty_i = type::PropertyInitializer<boolProperty_t>;
    	using floatVectorProperty_i = type::PropertyInitializer<floatVectorProperty_t>;
        using subStruct_i = type::PropertyInitializer<subStruct_t, TestSubStruct>;
    	
        using _key_properties_t = std::tuple<intProperty_t*>;
        using _properties_t     = std::tuple<intProperty_t*, stringProperty_t*, boolProperty_t*, floatVectorProperty_t*, subStruct_t*>;

		TestStruct() = default;
		TestStruct(const TestStruct& other) = default;
		TestStruct(TestStruct&& other) = default;
		~TestStruct() = default;

		TestStruct& operator = (const TestStruct& sutRhs) = default;
		TestStruct& operator = (TestStruct&& sutRhs) = default;

    	template <typename... PropertyInitializers, std::enable_if_t<sizeof...(PropertyInitializers) >= 1 && std::conjunction_v<type::is_property_initializer_t<std::remove_pointer_t<std::decay_t<PropertyInitializers>>>...>, int> = 0>
		explicit TestStruct(PropertyInitializers&&... propertyInitializers)
		{
			(_getProperty<typename std::remove_pointer_t<std::decay_t<PropertyInitializers>>::property_t>().template construct<false>(std::forward<decltype(propertyInitializers)>(propertyInitializers).value), ...);
		}

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
    };
}

namespace dots::type
{
    template <>
    struct Descriptor<types::TestSubStruct> : StructDescriptor<types::TestSubStruct>
    {
        Descriptor() :
            StructDescriptor("TestSubStruct", Cached, types::TestSubStruct::_MakePropertyDescriptors()){}
    };

	template <>
	struct Descriptor<types::TestStruct> : StructDescriptor<types::TestStruct>
	{
		Descriptor() :
			StructDescriptor("TestStruct", Cached, types::TestStruct::_MakePropertyDescriptors()){}
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

    sutThis._merge(sutOther, ~TestStruct::stringProperty_t::Set());

    EXPECT_EQ(sutThis.intProperty, 2);
    EXPECT_EQ(sutThis.stringProperty, "foo");
    EXPECT_FALSE(sutThis.boolProperty.isValid());
    EXPECT_EQ(sutThis.floatVectorProperty, vector_t<float32_t>{ 2.7183f });

    EXPECT_TRUE(sutThis.subStruct->p1.isValid());
    EXPECT_TRUE(sutThis.subStruct->p2.isValid());

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

TEST_F(TestStaticStruct, assertProperties)
{
	TestStruct sut{
		TestStruct::intProperty_i{ 1 },
		TestStruct::stringProperty_i{ "foo" }
	};
	
	EXPECT_NO_THROW(sut._assertHasProperties(TestStruct::intProperty_t::Set()));
	EXPECT_THROW(sut._assertHasProperties<false>(TestStruct::intProperty_t::Set()), std::logic_error);

	EXPECT_NO_THROW(sut._assertHasProperties(TestStruct::intProperty_t::Set() + TestStruct::stringProperty_t::Set()));
	EXPECT_NO_THROW(sut._assertHasProperties<false>(TestStruct::intProperty_t::Set() + TestStruct::stringProperty_t::Set()));

	EXPECT_THROW(sut._assertHasProperties(TestStruct::intProperty_t::Set() + TestStruct::floatVectorProperty_t::Set()), std::logic_error);
	EXPECT_THROW(sut._assertHasProperties<false>(TestStruct::intProperty_t::Set() + TestStruct::floatVectorProperty_t::Set()), std::logic_error);
}