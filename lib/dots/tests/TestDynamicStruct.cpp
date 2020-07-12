#include <optional>
#include <dots/type/DynamicStruct.h>
#include <dots/io/Registry.h>
#include <dots/io/DescriptorConverter.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/PropertyInitializer.h>
#include <StructDescriptorData.dots.h>
#include <gtest/gtest.h>

using namespace dots::type;
using namespace dots::types;

struct TestDynamicStruct : ::testing::Test
{
protected:

    TestDynamicStruct() :
        m_descriptorConverter{ m_registry }
    {
        StructDescriptorData testDynamicSubSubStructData{
            StructDescriptorData::name_i{ "TestDynamicSubSubStruct" },
            StructDescriptorData::flags_i{
                DotsStructFlags::cached_i{ true }
            },
            StructDescriptorData::properties_i{ vector_t<StructPropertyData>{
                StructPropertyData{
                    StructPropertyData::name_i{ "subSubIntProperty" },
                    StructPropertyData::tag_i{ 1 },
                    StructPropertyData::isKey_i{ false },
                    StructPropertyData::type_i{ "int32" }
                },
                StructPropertyData{
                    StructPropertyData::name_i{ "subSubDoubleProperty" },
                    StructPropertyData::tag_i{ 2 },
                    StructPropertyData::isKey_i{ true },
                    StructPropertyData::type_i{ "float64" }
                }
            } }
        };
        m_testDynamicSubSubStructDescriptor = std::static_pointer_cast<Descriptor<DynamicStruct>>(m_descriptorConverter(testDynamicSubSubStructData));

        StructDescriptorData testDynamicSubStructData{
            StructDescriptorData::name_i{ "TestDynamicSubStruct" },
            StructDescriptorData::flags_i{
                DotsStructFlags::cached_i{ true },
                DotsStructFlags::substructOnly_i{ true }
            },
            StructDescriptorData::properties_i{ vector_t<StructPropertyData>{
                StructPropertyData{
                    StructPropertyData::name_i{ "subIntProperty" },
                    StructPropertyData::tag_i{ 1 },
                    StructPropertyData::isKey_i{ true },
                    StructPropertyData::type_i{ "int64" }
                },
                StructPropertyData{
                    StructPropertyData::name_i{ "subSubStructProperty" },
                    StructPropertyData::tag_i{ 2 },
                    StructPropertyData::isKey_i{ false },
                    StructPropertyData::type_i{ "TestDynamicSubSubStruct" }
                },
                StructPropertyData{
                    StructPropertyData::name_i{ "subFloatProperty" },
                    StructPropertyData::tag_i{ 3 },
                    StructPropertyData::isKey_i{ false },
                    StructPropertyData::type_i{ "float32" }
                }
            } }
        };
        m_testDynamicSubStructDescriptor = std::static_pointer_cast<Descriptor<DynamicStruct>>(m_descriptorConverter(testDynamicSubStructData));

        StructDescriptorData testDynamicStructData{
            StructDescriptorData::name_i{ "TestDynamicStruct" },
            StructDescriptorData::flags_i{
                DotsStructFlags::cached_i{ true }
            },
            StructDescriptorData::properties_i{ vector_t<StructPropertyData>{
                StructPropertyData{
                    StructPropertyData::name_i{ "intProperty" },
                    StructPropertyData::tag_i{ 1 },
                    StructPropertyData::isKey_i{ true },
                    StructPropertyData::type_i{ "int32" }
                },
                StructPropertyData{
                    StructPropertyData::name_i{ "stringProperty" },
                    StructPropertyData::tag_i{ 2 },
                    StructPropertyData::isKey_i{ false },
                    StructPropertyData::type_i{ "string" }
                },
                StructPropertyData{
                    StructPropertyData::name_i{ "boolProperty" },
                    StructPropertyData::tag_i{ 3 },
                    StructPropertyData::isKey_i{ false },
                    StructPropertyData::type_i{ "bool" }
                },
                StructPropertyData{
                    StructPropertyData::name_i{ "floatVectorProperty" },
                    StructPropertyData::tag_i{ 4 },
                    StructPropertyData::isKey_i{ false },
                    StructPropertyData::type_i{ "vector<float32>" }
                },
                StructPropertyData{
                    StructPropertyData::name_i{ "subStructProperty" },
                    StructPropertyData::tag_i{ 5 },
                    StructPropertyData::isKey_i{ false },
                    StructPropertyData::type_i{ "TestDynamicSubStruct" }
                }
            } },
        };
        m_testDynamicStructDescriptor = std::static_pointer_cast<Descriptor<DynamicStruct>>(m_descriptorConverter(testDynamicStructData));
    }

    dots::io::Registry m_registry;
    dots::io::DescriptorConverter m_descriptorConverter;
    std::shared_ptr<Descriptor<DynamicStruct>> m_testDynamicSubSubStructDescriptor;
    std::shared_ptr<Descriptor<DynamicStruct>> m_testDynamicSubStructDescriptor;
    std::shared_ptr<Descriptor<DynamicStruct>> m_testDynamicStructDescriptor;
};

TEST_F(TestDynamicStruct, PropertyOffsetsMatchExpectedOffsets)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    
    EXPECT_EQ(sut._get("intProperty").descriptor().offset(), sizeof(PropertyArea));
    EXPECT_EQ(sut._get("stringProperty").descriptor().offset(), sut._get("intProperty").descriptor().offset() + 4);
    EXPECT_EQ(sut._get("boolProperty").descriptor().offset(), sut._get("stringProperty").descriptor().offset() + sizeof(string_t));
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().offset(), sut._get("boolProperty").descriptor().offset() + 8);
    EXPECT_EQ(sut._get("subStructProperty").descriptor().offset(), sut._get("floatVectorProperty").descriptor().offset() + sizeof(vector_t<float32_t>));
}

TEST_F(TestDynamicStruct, PropertyAddressessMatchExpectedAddresses)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    const std::byte* sutAddress = reinterpret_cast<const std::byte*>(&sut._propertyArea());

    ProxyProperty<> intProperty = sut._get("intProperty");
    ProxyProperty<> stringProperty = sut._get("stringProperty");
    ProxyProperty<> boolProperty = sut._get("boolProperty");
    ProxyProperty<> floatVectorProperty = sut._get("floatVectorProperty");
    ProxyProperty<> subStructProperty = sut._get("subStructProperty");
    
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&intProperty.storage()), sutAddress + intProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&stringProperty.storage()), sutAddress + stringProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&boolProperty.storage()), sutAddress + boolProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&floatVectorProperty.storage()), sutAddress + floatVectorProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subStructProperty.storage()), sutAddress + subStructProperty.descriptor().offset());

    DynamicStruct& sutSub = subStructProperty.construct().to<DynamicStruct>();
    size_t subOffset = subStructProperty.descriptor().offset() + sizeof(DynamicStruct);
    ProxyProperty<> subIntProperty = sutSub._get("subIntProperty");
    ProxyProperty<> subSubStructProperty = sutSub._get("subSubStructProperty");
    ProxyProperty<> subFloatProperty = sutSub._get("subFloatProperty");    

    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subIntProperty.storage()), sutAddress + subOffset + subIntProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subSubStructProperty.storage()), sutAddress + subOffset + subSubStructProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subFloatProperty.storage()), sutAddress + subOffset + subFloatProperty.descriptor().offset());

    DynamicStruct& sutSubSub = subSubStructProperty.construct().to<DynamicStruct>();
    size_t subSubOffset = subSubStructProperty.descriptor().offset() + sizeof(DynamicStruct);
    ProxyProperty<> subSubIntProperty = sutSubSub._get("subSubIntProperty");
    ProxyProperty<> subSubDoubleProperty = sutSubSub._get("subSubDoubleProperty");

    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subSubIntProperty.storage()), sutAddress + subOffset + subSubOffset + subSubIntProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subSubDoubleProperty.storage()), sutAddress + subOffset + subSubOffset + subSubDoubleProperty.descriptor().offset());
}

TEST_F(TestDynamicStruct, FlatPropertyDescriptorOffsetsMatchExpectedOffsets)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    const std::byte* sutAddress = reinterpret_cast<const std::byte*>(&sut._propertyArea());

    const property_descriptor_container_t& flatPropertyDescriptors = sut._descriptor().flatPropertyDescriptors();
    ASSERT_EQ(flatPropertyDescriptors.size(), 10);

    ProxyProperty<> intProperty = sut._get("intProperty");
    ProxyProperty<> stringProperty = sut._get("stringProperty");
    ProxyProperty<> boolProperty = sut._get("boolProperty");
    ProxyProperty<> floatVectorProperty = sut._get("floatVectorProperty");
    ProxyProperty<> subStructProperty = sut._get("subStructProperty");
    
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&intProperty.storage()), sutAddress + flatPropertyDescriptors[0].offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&stringProperty.storage()), sutAddress + flatPropertyDescriptors[1].offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&boolProperty.storage()), sutAddress + flatPropertyDescriptors[2].offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&floatVectorProperty.storage()), sutAddress + flatPropertyDescriptors[3].offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subStructProperty.storage()), sutAddress + flatPropertyDescriptors[4].offset());

    DynamicStruct& sutSub = subStructProperty.construct().to<DynamicStruct>();
    ProxyProperty<> subIntProperty = sutSub._get("subIntProperty");
    ProxyProperty<> subSubStructProperty = sutSub._get("subSubStructProperty");
    ProxyProperty<> subFloatProperty = sutSub._get("subFloatProperty");    

    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subIntProperty.storage()), sutAddress + flatPropertyDescriptors[5].offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subSubStructProperty.storage()), sutAddress + flatPropertyDescriptors[6].offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subFloatProperty.storage()), sutAddress + flatPropertyDescriptors[9].offset());

    DynamicStruct& sutSubSub = subSubStructProperty.construct().to<DynamicStruct>();
    ProxyProperty<> subSubIntProperty = sutSubSub._get("subSubIntProperty");
    ProxyProperty<> subSubDoubleProperty = sutSubSub._get("subSubDoubleProperty");

    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subSubIntProperty.storage()), sutAddress + flatPropertyDescriptors[7].offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subSubDoubleProperty.storage()), sutAddress + flatPropertyDescriptors[8].offset());
}

TEST_F(TestDynamicStruct, GetPropertyReturnsSubProperty)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };

    EXPECT_EQ(sut._get("intProperty").descriptor().name(), "intProperty");
    EXPECT_EQ(sut._get("stringProperty").descriptor().name(), "stringProperty");
    EXPECT_EQ(sut._get("boolProperty").descriptor().name(), "boolProperty");
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().name(), "floatVectorProperty");
    EXPECT_EQ(sut._get("subStructProperty").descriptor().name(), "subStructProperty");

    EXPECT_EQ(sut._get("subStructProperty.subIntProperty").descriptor().name(), "subIntProperty");
    EXPECT_EQ(sut._get("subStructProperty.subSubStructProperty").descriptor().name(), "subSubStructProperty");
    EXPECT_EQ(sut._get("subStructProperty.subFloatProperty").descriptor().name(), "subFloatProperty");

    EXPECT_EQ(sut._get("subStructProperty.subSubStructProperty.subSubIntProperty").descriptor().name(), "subSubIntProperty");
    EXPECT_EQ(sut._get("subStructProperty.subSubStructProperty.subSubDoubleProperty").descriptor().name(), "subSubDoubleProperty");
}

TEST_F(TestDynamicStruct, GetPropertyReturnsSubPropertyWithExpectedValues)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } },
        DynamicStruct::property_i<DynamicStruct>{ "subStructProperty",
            DynamicStruct{ *m_testDynamicSubStructDescriptor,
                DynamicStruct::property_i<int64_t>{ "subIntProperty", 42 },
                DynamicStruct::property_i<DynamicStruct>{ "subSubStructProperty",
                    DynamicStruct{ *m_testDynamicSubSubStructDescriptor,
                        DynamicStruct::property_i<float64_t>{ "subSubDoubleProperty", 21.0 }
                    }
                }
            }
        }
    };

    EXPECT_TRUE(sut._get("intProperty").isValid());
    EXPECT_TRUE(sut._get("stringProperty").isValid());
    EXPECT_TRUE(sut._get("floatVectorProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subIntProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subSubStructProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subSubStructProperty.subSubDoubleProperty").isValid());

    EXPECT_EQ(sut._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sut._get<string_t>("stringProperty"), "foo");
    EXPECT_EQ(sut._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 3.1415f });
    EXPECT_EQ(sut._get<int64_t>("subStructProperty.subIntProperty"), int64_t{ 42 });
    EXPECT_EQ(sut._get<float64_t>("subStructProperty.subSubStructProperty.subSubDoubleProperty"), 21.0);

    EXPECT_FALSE(sut._get("boolProperty").isValid());
    EXPECT_FALSE(sut._get("subStructProperty.subFloatProperty").isValid());
    EXPECT_FALSE(sut._get("subStructProperty.subSubStructProperty.subSubIntProperty").isValid());
}

TEST_F(TestDynamicStruct, GetPropertyAllowsImplicitConstructionOfPath)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    EXPECT_FALSE(sut._get("subStructProperty.subSubStructProperty.subSubIntProperty").isValid());
    EXPECT_FALSE(sut._get("subStructProperty.subSubStructProperty").isValid());
    EXPECT_FALSE(sut._get("subStructProperty").isValid());

    sut._get<int64_t>("subStructProperty.subSubStructProperty.subSubIntProperty").construct(42);

    EXPECT_TRUE(sut._get("subStructProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subSubStructProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subSubStructProperty.subSubIntProperty").isValid());
    EXPECT_EQ(sut._get<int32_t>("subStructProperty.subSubStructProperty.subSubIntProperty"), 42);
}

TEST_F(TestDynamicStruct, PropertiesHaveExpectedTags)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    
    EXPECT_EQ(sut._get("intProperty").descriptor().tag(), 1);
    EXPECT_EQ(sut._get("stringProperty").descriptor().tag(), 2);
    EXPECT_EQ(sut._get("boolProperty").descriptor().tag(), 3);
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().tag(), 4);
    EXPECT_EQ(sut._get("subStructProperty").descriptor().tag(), 5);
}

TEST_F(TestDynamicStruct, PropertiesHaveExpectedNames)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    
    EXPECT_EQ(sut._get("intProperty").descriptor().name(), "intProperty");
    EXPECT_EQ(sut._get("stringProperty").descriptor().name(), "stringProperty");
    EXPECT_EQ(sut._get("boolProperty").descriptor().name(), "boolProperty");
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().name(), "floatVectorProperty");
    EXPECT_EQ(sut._get("subStructProperty").descriptor().name(), "subStructProperty");
}

TEST_F(TestDynamicStruct, PropertiesHaveExpectedSet)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    
    EXPECT_EQ(sut._get("intProperty").descriptor().set(), PropertySet{ 0x1 << 1 });
    EXPECT_EQ(sut._get("stringProperty").descriptor().set(), PropertySet{ 0x1 << 2 });
    EXPECT_EQ(sut._get("boolProperty").descriptor().set(), PropertySet{ 0x1 << 3 });
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().set(), PropertySet{ 0x1 << 4 });
    EXPECT_EQ(sut._get("subStructProperty").descriptor().set(), PropertySet{ 0x1 << 5 });
}

TEST_F(TestDynamicStruct, _descriptor_SizeMatchesAllocateSize)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    const PropertyDescriptor& lastPropertyDescriptor = sut._get("subStructProperty").descriptor();

    // note: this expectation only holds true if the last property has an 8-byte alignment
    EXPECT_EQ(m_testDynamicStructDescriptor->size(), sizeof(DynamicStruct) + lastPropertyDescriptor.offset() + lastPropertyDescriptor.valueDescriptor().size());
}

TEST_F(TestDynamicStruct, _descriptor_AlignmentMatchesActualAlignment)
{
    EXPECT_EQ(m_testDynamicStructDescriptor->alignment(), alignof(DynamicStruct));
}

TEST_F(TestDynamicStruct, _descriptor_FlagsHaveExpectedValues)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    const StructDescriptor<>& descriptor = sut._descriptor();

    EXPECT_TRUE(descriptor.cached());
    EXPECT_FALSE(descriptor.internal());
    EXPECT_FALSE(descriptor.persistent());
    EXPECT_FALSE(descriptor.cleanup());
    EXPECT_FALSE(descriptor.local());
    EXPECT_FALSE(descriptor.substructOnly());
}

TEST_F(TestDynamicStruct, _keyProperties)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    const StructDescriptor<>& descriptor = sut._descriptor();

    EXPECT_EQ(descriptor.keyProperties(), sut._get("intProperty").descriptor().set());
}

TEST_F(TestDynamicStruct, ctor_Initializer)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    
    EXPECT_EQ(sut._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sut._get<string_t>("stringProperty"), "foo");
    EXPECT_FALSE(sut._get("boolProperty").isValid());
    EXPECT_EQ(sut._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>({ 3.1415f, 2.7183f }));
}

TEST_F(TestDynamicStruct, ctor_Copy)
{
    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    DynamicStruct sutThis{ sutOther };

    EXPECT_EQ(sutThis._get("intProperty"), sutOther._get("intProperty"));
    EXPECT_EQ(sutThis._get("stringProperty"), sutOther._get("stringProperty"));
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get("floatVectorProperty"), sutOther._get("floatVectorProperty"));
}

TEST_F(TestDynamicStruct, ctor_Move)
{
    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    DynamicStruct sutThis{ std::move(sutOther) };

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "foo");
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>({ 3.1415f, 2.7183f }));
}

TEST_F(TestDynamicStruct, assignment_Copy)
{
    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    DynamicStruct sutThis = sutOther;

    EXPECT_EQ(sutThis._get("intProperty"), sutOther._get("intProperty"));
    EXPECT_EQ(sutThis._get("stringProperty"), sutOther._get("stringProperty"));
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get("floatVectorProperty"), sutOther._get("floatVectorProperty"));
}

TEST_F(TestDynamicStruct, assignment_Move)
{
    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    DynamicStruct sutThis = std::move(sutOther);

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "foo");
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>({ 3.1415f, 2.7183f }));

    EXPECT_FALSE(sutOther._get("intProperty").isValid());
    EXPECT_FALSE(sutOther._get("stringProperty").isValid());
    EXPECT_FALSE(sutOther._get("floatVectorProperty").isValid());
}

TEST_F(TestDynamicStruct, assign_CompleteAssign)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } }
    };

    sutThis._assign(sutOther);

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 2);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "bar");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 2.7183f });
}

TEST_F(TestDynamicStruct, assign_PartialAssign)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" }
    };

    sutThis._assign(sutOther, ~sutThis._get("floatVectorProperty").descriptor().set());

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 2);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "bar");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_FALSE(sutThis._get("floatVectorProperty").isValid());
}

TEST_F(TestDynamicStruct, assign_CompleteMoveAssign)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } },
        DynamicStruct::property_i<DynamicStruct>{ "subStructProperty", DynamicStruct{ *m_testDynamicSubStructDescriptor,
                DynamicStruct::property_i<int64_t>{ "subIntProperty", int64_t{ 42 } }
            }
        }
    };

    sutThis._assign(std::move(sutOther));

    EXPECT_FALSE(sutThis._get("intProperty").isValid());
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "bar");
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 2.7183f });
    EXPECT_TRUE(sutThis._get("subStructProperty").isValid());
    EXPECT_EQ(sutThis._get<int64_t>("subStructProperty.subIntProperty"), int64_t{ 42 });

    EXPECT_FALSE(sutOther._get("intProperty").isValid());
    EXPECT_FALSE(sutOther._get("stringProperty").isValid());
    EXPECT_FALSE(sutOther._get("floatVectorProperty").isValid());
    EXPECT_FALSE(sutOther._get("subStructProperty").isValid());
}

TEST_F(TestDynamicStruct, assign_PartialMoveAssign)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } },
        DynamicStruct::property_i<DynamicStruct>{ "subStructProperty", DynamicStruct{ *m_testDynamicSubStructDescriptor,
                DynamicStruct::property_i<int64_t>{ "subIntProperty", int64_t{ 42 } }
            }
        }
    };

    sutThis._assign(std::move(sutOther), ~sutThis._get("floatVectorProperty").descriptor().set());

    EXPECT_FALSE(sutThis._get("intProperty").isValid());
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "bar");
    EXPECT_FALSE(sutThis._get("floatVectorProperty").isValid());
    EXPECT_EQ(sutThis._get<int64_t>("subStructProperty.subIntProperty"), int64_t{ 42 });

    EXPECT_FALSE(sutOther._get("intProperty").isValid());
    EXPECT_FALSE(sutOther._get("stringProperty").isValid());
    EXPECT_FALSE(sutOther._get("subStructProperty").isValid());
}

TEST_F(TestDynamicStruct, assign_DirectSubStructMove)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } },
        DynamicStruct::property_i<DynamicStruct>{ "subStructProperty", DynamicStruct{ *m_testDynamicSubStructDescriptor,
                DynamicStruct::property_i<float32_t>{ "subFloatProperty", 23.0f }
            }
        }
    };

    DynamicStruct sutSubOther{ *m_testDynamicSubStructDescriptor,
        DynamicStruct::property_i<int64_t>{ "subIntProperty", 42 },
        DynamicStruct::property_i<DynamicStruct>{ "subSubStructProperty", DynamicStruct{ *m_testDynamicSubSubStructDescriptor,
                DynamicStruct::property_i<float64_t>{ "subSubDoubleProperty", 21.0 }
            }
        }
    };

    sutThis._get("subStructProperty").assign(Typeless::From(std::move(sutSubOther)));

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "foo");
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 3.1415f });
    EXPECT_EQ(sutThis._get<int64_t>("subStructProperty.subIntProperty"), int64_t{ 42 });
    EXPECT_EQ(sutThis._get<float64_t>("subStructProperty.subSubStructProperty.subSubDoubleProperty"), 21.0);
    EXPECT_FALSE(sutThis._get("subStructProperty.subFloatProperty").isValid());

    EXPECT_FALSE(sutSubOther._get("subIntProperty").isValid());
    EXPECT_FALSE(sutSubOther._get("subSubStructProperty").isValid());
}

TEST_F(TestDynamicStruct, copy_CompleteCopy)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } }
    };

    sutThis._copy(sutOther);

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 2);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "bar");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 2.7183f });
}

TEST_F(TestDynamicStruct, copy_PartialCopy)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" }
    };

    sutThis._copy(sutOther, ~sutThis._get("floatVectorProperty").descriptor().set());

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 2);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "bar");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestDynamicStruct, merge_CompleteMerge)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" }
    };

    sutThis._merge(sutOther);

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 2);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "bar");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestDynamicStruct, merge_PartialMerge)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } }
    };

    sutThis._merge(sutOther, ~sutThis._get("stringProperty").descriptor().set());

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 2);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "foo");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 2.7183f });
}

TEST_F(TestDynamicStruct, merge_PartialMergeSubStruct)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } },
        DynamicStruct::property_i<DynamicStruct>{ "subStructProperty", DynamicStruct{ *m_testDynamicSubStructDescriptor,
                DynamicStruct::property_i<float32_t>{ "subFloatProperty", 21.0f }
            }
        }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } },
        DynamicStruct::property_i<DynamicStruct>{ "subStructProperty", DynamicStruct{ *m_testDynamicSubStructDescriptor,
                DynamicStruct::property_i<int64_t>{ "subIntProperty", 42 }
            }
        }
    };

    sutThis._merge(sutOther, ~sutThis._get("stringProperty").descriptor().set());

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 2);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "foo");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 2.7183f });

    ProxyProperty<DynamicStruct> subStructProperty = sutThis._get<DynamicStruct>("subStructProperty");
    auto& sutThisSub = subStructProperty.value();
    EXPECT_EQ(sutThisSub._get<int64_t>("subIntProperty"), int64_t{ 42 });
    EXPECT_EQ(sutThisSub._get<float32_t>("subFloatProperty"), 21.0f);
}

TEST_F(TestDynamicStruct, swap_CompleteSwap)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" }
    };

    sutThis._swap(sutOther);

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 2);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "bar");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_FALSE(sutThis._get("floatVectorProperty").isValid());

    EXPECT_EQ(sutOther._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sutOther._get<string_t>("stringProperty"), "foo");
    EXPECT_FALSE(sutOther._get("boolProperty").isValid());
    EXPECT_EQ(sutOther._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestDynamicStruct, swap_PartialSwap)
{
    DynamicStruct sutThis{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } }
    };

    sutThis._swap(sutOther, sutThis._get("floatVectorProperty").descriptor().set());

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "foo");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 2.7183f });

    EXPECT_EQ(sutOther._get<int32_t>("intProperty"), 2);
    EXPECT_EQ(sutOther._get<string_t>("stringProperty"), "bar");
    EXPECT_FALSE(sutOther._get("boolProperty").isValid());
    EXPECT_EQ(sutOther._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 3.1415f });
}

TEST_F(TestDynamicStruct, clear_CompleteClear)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    sut._clear();

    EXPECT_FALSE(sut._get("intProperty").isValid());
    EXPECT_FALSE(sut._get("stringProperty").isValid());
    EXPECT_FALSE(sut._get("boolProperty").isValid());
    EXPECT_FALSE(sut._get("floatVectorProperty").isValid());
}

TEST_F(TestDynamicStruct, clear_PartialClear)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    sut._clear(~sut._get("stringProperty").descriptor().set());

    EXPECT_FALSE(sut._get("intProperty").isValid());
    EXPECT_EQ(sut._get<string_t>("stringProperty"), "foo");
    EXPECT_FALSE(sut._get("boolProperty").isValid());
    EXPECT_FALSE(sut._get("floatVectorProperty").isValid());
}

TEST_F(TestDynamicStruct, equal)
{
    DynamicStruct sutLhs{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutRhs{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    EXPECT_TRUE(sutLhs._equal(sutLhs));
    EXPECT_TRUE(sutRhs._equal(sutRhs));
    EXPECT_FALSE(sutLhs._equal(sutRhs));
    
    EXPECT_TRUE(sutLhs._equal(sutRhs, sutLhs._get("intProperty").descriptor().set()));
    EXPECT_TRUE(sutLhs._equal(sutRhs, sutLhs._get("floatVectorProperty").descriptor().set()));
    EXPECT_FALSE(sutLhs._equal(sutRhs, sutLhs._get("intProperty").descriptor().set() + sutLhs._get("stringProperty").descriptor().set()));
}

TEST_F(TestDynamicStruct, same)
{
    DynamicStruct sut1{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sut2{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sut3{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } }
    };

    EXPECT_TRUE(sut1._same(sut1));
    EXPECT_TRUE(sut1._same(sut2));
    EXPECT_FALSE(sut1._same(sut3));
}

TEST_F(TestDynamicStruct, less_lessEqual_greater_greaterEqual)
{
    DynamicStruct sutLhs{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutRhs{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } }
    };
    
    EXPECT_TRUE(sutLhs._less(sutRhs));
    EXPECT_FALSE(sutRhs._less(sutLhs));
    EXPECT_TRUE(sutRhs._less(sutLhs, sutLhs._get("floatVectorProperty").descriptor().set()));
    EXPECT_FALSE(sutLhs._less(sutLhs));
    EXPECT_FALSE(sutLhs._less(sutLhs, sutLhs._get("boolProperty").descriptor().set()));

    EXPECT_TRUE(sutLhs._lessEqual(sutRhs));
    EXPECT_FALSE(sutRhs._lessEqual(sutLhs));
    EXPECT_TRUE(sutRhs._lessEqual(sutLhs, sutLhs._get("floatVectorProperty").descriptor().set()));
    EXPECT_TRUE(sutLhs._lessEqual(sutLhs));
    EXPECT_TRUE(sutLhs._lessEqual(sutLhs, sutLhs._get("boolProperty").descriptor().set()));

    EXPECT_FALSE(sutLhs._greater(sutRhs));
    EXPECT_TRUE(sutRhs._greater(sutLhs));
    EXPECT_FALSE(sutRhs._greater(sutLhs, sutLhs._get("floatVectorProperty").descriptor().set()));
    EXPECT_FALSE(sutLhs._greater(sutLhs));
    EXPECT_FALSE(sutLhs._greater(sutLhs, sutLhs._get("boolProperty").descriptor().set()));

    EXPECT_FALSE(sutLhs._greaterEqual(sutRhs));
    EXPECT_TRUE(sutRhs._greaterEqual(sutLhs));
    EXPECT_FALSE(sutRhs._greaterEqual(sutLhs, sutLhs._get("floatVectorProperty").descriptor().set()));
    EXPECT_TRUE(sutLhs._greaterEqual(sutLhs));
    EXPECT_TRUE(sutLhs._greaterEqual(sutLhs, sutLhs._get("boolProperty").descriptor().set()));
}

TEST_F(TestDynamicStruct, diffProperties)
{
    DynamicStruct sutLhs{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutRhs{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 2.7183f } }
    };
    
    EXPECT_EQ(sutLhs._diffProperties(sutRhs), sutLhs._get("stringProperty").descriptor().set() + sutLhs._get("floatVectorProperty").descriptor().set());
    EXPECT_EQ(sutLhs._diffProperties(sutRhs, (~sutLhs._get("floatVectorProperty").descriptor().set())), sutLhs._get("stringProperty").descriptor().set());
}

TEST_F(TestDynamicStruct, assertProperties)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" }
    };
    
    EXPECT_NO_THROW(sut._assertHasProperties(sut._get("intProperty").descriptor().set()));
    EXPECT_THROW(sut._assertHasProperties<false>(sut._get("intProperty").descriptor().set()), std::logic_error);

    EXPECT_NO_THROW(sut._assertHasProperties(sut._get("intProperty").descriptor().set() + sut._get("stringProperty").descriptor().set()));
    EXPECT_NO_THROW(sut._assertHasProperties<false>(sut._get("intProperty").descriptor().set() + sut._get("stringProperty").descriptor().set()));

    EXPECT_THROW(sut._assertHasProperties(sut._get("intProperty").descriptor().set() + sut._get("floatVectorProperty").descriptor().set()), std::logic_error);
    EXPECT_THROW(sut._assertHasProperties<false>(sut._get("intProperty").descriptor().set() + sut._get("floatVectorProperty").descriptor().set()), std::logic_error);
}