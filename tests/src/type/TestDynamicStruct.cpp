// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <optional>
#include <dots/type/DynamicStruct.h>
#include <dots/type/Registry.h>
#include <dots/io/DescriptorConverter.h>
#include <dots/type/FundamentalTypes.h>
#include <StructDescriptorData.dots.h>
#include <dots/testing/gtest/gtest.h>

using namespace dots::type;
using namespace dots::types;

struct TestDynamicStruct : ::testing::Test
{
protected:

    TestDynamicStruct() :
        m_descriptorConverter{ m_registry }
    {
        StructDescriptorData testDynamicSubSubStructData{
            .name = "TestDynamicSubSubStruct",
            .properties = vector_t<StructPropertyData>{
                StructPropertyData{
                    .name = "subSubIntProperty",
                    .tag = 1,
                    .isKey = false,
                    .type = "int32"
                },
                StructPropertyData{
                    .name = "subSubDoubleProperty",
                    .tag = 2,
                    .isKey = true,
                    .type = "float64"
                }
            },
            .flags = DotsStructFlags{
                .cached = true
            },
        };
        m_testDynamicSubSubStructDescriptor = static_cast<Descriptor<DynamicStruct>*>(&m_descriptorConverter(testDynamicSubSubStructData));

        StructDescriptorData testDynamicSubStructData{
            .name = "TestDynamicSubStruct",
            .properties = vector_t<StructPropertyData>{
                StructPropertyData{
                    .name = "subIntProperty",
                    .tag = 1,
                    .isKey = true,
                    .type = "int64"
                },
                StructPropertyData{
                    .name = "subSubStructProperty",
                    .tag = 2,
                    .isKey = false,
                    .type = "TestDynamicSubSubStruct"
                },
                StructPropertyData{
                    .name = "subFloatProperty",
                    .tag = 3,
                    .isKey = false,
                    .type = "float32"
                }
            },
            .flags = DotsStructFlags{
                .cached = true,
                .substructOnly = true
            },
        };
        m_testDynamicSubStructDescriptor = static_cast<Descriptor<DynamicStruct>*>(&m_descriptorConverter(testDynamicSubStructData));

        StructDescriptorData testDynamicStructData{
            .name = "TestDynamicStruct",
            .properties = vector_t<StructPropertyData>{
                StructPropertyData{
                    .name = "intProperty",
                    .tag = 1,
                    .isKey = true,
                    .type = "int32"
                },
                StructPropertyData{
                    .name = "stringProperty",
                    .tag = 2,
                    .isKey = false,
                    .type = "string"
                },
                StructPropertyData{
                    .name = "boolProperty",
                    .tag = 3,
                    .isKey = false,
                    .type = "bool"
                },
                StructPropertyData{
                    .name = "floatVectorProperty",
                    .tag = 4,
                    .isKey = false,
                    .type = "vector<float32>"
                },
                StructPropertyData{
                    .name = "subStructProperty",
                    .tag = 5,
                    .isKey = false,
                    .type = "TestDynamicSubStruct"
                },
                StructPropertyData{
                    .name = "structVectorProperty",
                    .tag = 6,
                    .isKey = false,
                    .type = "vector<TestDynamicSubStruct>"
                },
            },
            .flags = DotsStructFlags{
                .cached = true
            },
        };
        m_testDynamicStructDescriptor = static_cast<Descriptor<DynamicStruct>*>(&m_descriptorConverter(testDynamicStructData));
    }

    Registry m_registry;
    dots::io::DescriptorConverter m_descriptorConverter;
    Descriptor<DynamicStruct>* m_testDynamicSubSubStructDescriptor;
    Descriptor<DynamicStruct>* m_testDynamicSubStructDescriptor;
    Descriptor<DynamicStruct>* m_testDynamicStructDescriptor;
};

TEST_F(TestDynamicStruct, PropertyOffsetsMatchExpectedOffsets)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };

    EXPECT_EQ(sut._get("intProperty").descriptor().offset(), sizeof(PropertyArea));
    EXPECT_EQ(sut._get("stringProperty").descriptor().offset(), sut._get("intProperty").descriptor().offset() + 4);
    EXPECT_EQ(sut._get("boolProperty").descriptor().offset(), sut._get("stringProperty").descriptor().offset() + sizeof(string_t));
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().offset(), sut._get("boolProperty").descriptor().offset() + sizeof(bool_t) + (sizeof(void*) == 4 ? 3 :7));
    EXPECT_EQ(sut._get("subStructProperty").descriptor().offset(), sut._get("floatVectorProperty").descriptor().offset() + sizeof(vector_t<float32_t>));
    EXPECT_EQ(sut._get("structVectorProperty").descriptor().offset(), sut._get("subStructProperty").descriptor().offset() + sut._get("subStructProperty").descriptor().valueDescriptor().size());
    EXPECT_EQ(sut._descriptor().size(), sizeof(DynamicStruct) + sut._get("structVectorProperty").descriptor().offset() + sizeof(vector_t<DynamicStruct>));
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
    ProxyProperty<> structVectorProperty = sut._get("structVectorProperty");

    EXPECT_EQ(reinterpret_cast<const std::byte*>(&intProperty.storage()), sutAddress + intProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&stringProperty.storage()), sutAddress + stringProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&boolProperty.storage()), sutAddress + boolProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&floatVectorProperty.storage()), sutAddress + floatVectorProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subStructProperty.storage()), sutAddress + subStructProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&structVectorProperty.storage()), sutAddress + structVectorProperty.descriptor().offset());

    DynamicStruct& sutSub = subStructProperty.emplace().to<DynamicStruct>();
    size_t subOffset = subStructProperty.descriptor().offset() + sizeof(DynamicStruct);
    ProxyProperty<> subIntProperty = sutSub._get("subIntProperty");
    ProxyProperty<> subSubStructProperty = sutSub._get("subSubStructProperty");
    ProxyProperty<> subFloatProperty = sutSub._get("subFloatProperty");

    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subIntProperty.storage()), sutAddress + subOffset + subIntProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subSubStructProperty.storage()), sutAddress + subOffset + subSubStructProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subFloatProperty.storage()), sutAddress + subOffset + subFloatProperty.descriptor().offset());

    DynamicStruct& sutSubSub = subSubStructProperty.emplace().to<DynamicStruct>();
    size_t subSubOffset = subSubStructProperty.descriptor().offset() + sizeof(DynamicStruct);
    ProxyProperty<> subSubIntProperty = sutSubSub._get("subSubIntProperty");
    ProxyProperty<> subSubDoubleProperty = sutSubSub._get("subSubDoubleProperty");

    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subSubIntProperty.storage()), sutAddress + subOffset + subSubOffset + subSubIntProperty.descriptor().offset());
    EXPECT_EQ(reinterpret_cast<const std::byte*>(&subSubDoubleProperty.storage()), sutAddress + subOffset + subSubOffset + subSubDoubleProperty.descriptor().offset());
}

TEST_F(TestDynamicStruct, GetPropertyReturnsSubProperty)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };

    EXPECT_EQ(sut._get("intProperty").descriptor().name(), "intProperty");
    EXPECT_EQ(sut._get("stringProperty").descriptor().name(), "stringProperty");
    EXPECT_EQ(sut._get("boolProperty").descriptor().name(), "boolProperty");
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().name(), "floatVectorProperty");
    EXPECT_EQ(sut._get("subStructProperty").descriptor().name(), "subStructProperty");
    EXPECT_EQ(sut._get("structVectorProperty").descriptor().name(), "structVectorProperty");

    EXPECT_EQ(sut._get("subStructProperty.subIntProperty").descriptor().name(), "subIntProperty");
    EXPECT_EQ(sut._get("subStructProperty.subSubStructProperty").descriptor().name(), "subSubStructProperty");
    EXPECT_EQ(sut._get("subStructProperty.subFloatProperty").descriptor().name(), "subFloatProperty");

    EXPECT_EQ(sut._get("subStructProperty.subSubStructProperty.subSubIntProperty").descriptor().name(), "subSubIntProperty");
    EXPECT_EQ(sut._get("subStructProperty.subSubStructProperty.subSubDoubleProperty").descriptor().name(), "subSubDoubleProperty");
}

TEST_F(TestDynamicStruct, GetPropertyAllowsTypedDefaultConstruction)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };

    EXPECT_EQ(sut._get<dots::int32_t>("intProperty").emplace(), 0);
    EXPECT_EQ(sut._get<dots::string_t>("stringProperty").emplace(), "");
    EXPECT_EQ(sut._get<dots::bool_t>("boolProperty").emplace(), false);
    EXPECT_EQ(sut._get<dots::vector_t<dots::float32_t>>("floatVectorProperty").emplace(), dots::vector_t<dots::float32_t>{});
    // proxy properties currently do not support dynamic types
    //EXPECT_EQ(sut._get<DynamicStruct>("subStructProperty").emplace()._validProperties(), dots::property_set_t{ dots::property_set_t::None });
    EXPECT_EQ(sut._get<vector_t<DynamicStruct>>("structVectorProperty").emplace(), vector_t<DynamicStruct>{});

    EXPECT_EQ(sut._get<dots::int64_t>("subStructProperty.subIntProperty").emplace(), 0);
    // proxy properties currently do not support dynamic types
    //EXPECT_EQ(sut._get<DynamicStruct>("subStructProperty.subSubStructProperty").emplace(), dots::property_set_t{ dots::property_set_t::None });
    EXPECT_EQ(sut._get<dots::float32_t>("subStructProperty.subFloatProperty").emplace(), 0.0f);

    EXPECT_EQ(sut._get<dots::int32_t>("subStructProperty.subSubStructProperty.subSubIntProperty").emplace(), 0);
    EXPECT_EQ(sut._get<dots::float64_t>("subStructProperty.subSubStructProperty.subSubDoubleProperty").emplace(), 0.0);
}

TEST_F(TestDynamicStruct, GetPropertyAllowsTypelessDefaultConstruction)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };

    EXPECT_EQ(sut._get("intProperty").emplace().to<dots::int32_t>(), 0);
    EXPECT_EQ(sut._get("stringProperty").emplace().to<dots::string_t>(), "");
    EXPECT_EQ(sut._get("boolProperty").emplace().to<dots::bool_t>(), false);
    EXPECT_EQ(sut._get("floatVectorProperty").emplace().to<dots::vector_t<dots::float32_t>>(), dots::vector_t<dots::float32_t>{});
    EXPECT_EQ(sut._get("subStructProperty").emplace().to<DynamicStruct>()._validProperties(), dots::property_set_t{ dots::property_set_t::None });
    EXPECT_EQ(sut._get("structVectorProperty").emplace().to<vector_t<DynamicStruct>>(), vector_t<DynamicStruct>{});

    EXPECT_EQ(sut._get("subStructProperty.subIntProperty").emplace().to<dots::int64_t>(), 0);
    EXPECT_EQ(sut._get("subStructProperty.subSubStructProperty").emplace().to<DynamicStruct>()._validProperties(), dots::property_set_t{ dots::property_set_t::None });
    EXPECT_EQ(sut._get("subStructProperty.subFloatProperty").emplace().to<dots::float32_t>(), 0.0f);

    EXPECT_EQ(sut._get("subStructProperty.subSubStructProperty.subSubIntProperty").emplace().to<dots::int32_t>(), 0);
    EXPECT_EQ(sut._get("subStructProperty.subSubStructProperty.subSubDoubleProperty").emplace().to<dots::float64_t>(), 0.0);
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
        },
        DynamicStruct::property_i<vector_t<DynamicStruct>>{ "structVectorProperty",
            vector_t<DynamicStruct>{
                DynamicStruct{ *m_testDynamicSubStructDescriptor,
                    DynamicStruct::property_i<int64_t>{ "subIntProperty", 7 }
                },
                DynamicStruct{ *m_testDynamicSubStructDescriptor,
                    DynamicStruct::property_i<float32_t>{ "subFloatProperty", 11.0f }
                },
            }
        },
    };

    EXPECT_TRUE(sut._get("intProperty").isValid());
    EXPECT_TRUE(sut._get("stringProperty").isValid());
    EXPECT_TRUE(sut._get("floatVectorProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subIntProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subSubStructProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subSubStructProperty.subSubDoubleProperty").isValid());
    EXPECT_TRUE(sut._get("structVectorProperty").isValid());

    EXPECT_EQ(sut._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sut._get<string_t>("stringProperty"), "foo");
    EXPECT_EQ(sut._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>{ 3.1415f });
    EXPECT_EQ(sut._get<int64_t>("subStructProperty.subIntProperty"), int64_t{ 42 });
    EXPECT_EQ(sut._get<float64_t>("subStructProperty.subSubStructProperty.subSubDoubleProperty"), 21.0);

     vector_t<DynamicStruct> structVectorExpected{
        DynamicStruct{ *m_testDynamicSubStructDescriptor,
            DynamicStruct::property_i<int64_t>{ "subIntProperty", 7 }
        },
        DynamicStruct{ *m_testDynamicSubStructDescriptor,
            DynamicStruct::property_i<float32_t>{ "subFloatProperty", 11.0f }
        },
    };
    EXPECT_EQ(sut._get<vector_t<DynamicStruct>>("structVectorProperty"), structVectorExpected);

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

    sut._get<int64_t>("subStructProperty.subSubStructProperty.subSubIntProperty").emplace(42);

    EXPECT_TRUE(sut._get("subStructProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subSubStructProperty").isValid());
    EXPECT_TRUE(sut._get("subStructProperty.subSubStructProperty.subSubIntProperty").isValid());
    EXPECT_EQ(sut._get<int32_t>("subStructProperty.subSubStructProperty.subSubIntProperty"), 42);
}

TEST_F(TestDynamicStruct, NestedAccessViaPropertyPathGet)
{
    DynamicStruct sut1{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<DynamicStruct>{ "subStructProperty",
            DynamicStruct{ *m_testDynamicSubStructDescriptor,
                DynamicStruct::property_i<int64_t>{ "subIntProperty", 42 },
                DynamicStruct::property_i<float32_t>{ "subFloatProperty", 3.1415f }
            }
        }
    };

    EXPECT_EQ(ProxyProperty<int64_t>(sut1, sut1._path("subStructProperty.subIntProperty")), int64_t{ 42 });
    EXPECT_EQ(ProxyProperty<float32_t>(sut1, sut1._path("subStructProperty.subFloatProperty")), float32_t{ 3.1415f });

    DynamicStruct sut2{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<DynamicStruct>{ "subStructProperty",
            DynamicStruct{ *m_testDynamicSubStructDescriptor,
                DynamicStruct::property_i<int64_t>{ "subIntProperty", 21 },
                DynamicStruct::property_i<float32_t>{ "subFloatProperty", 2.7183f }
            }
        }
    };

    EXPECT_EQ(ProxyProperty<int64_t>(sut2, sut2._path("subStructProperty.subIntProperty")), int64_t{ 21 });
    EXPECT_EQ(ProxyProperty<float32_t>(sut2, sut2._path("subStructProperty.subFloatProperty")), float32_t{ 2.7183f });

    DynamicStruct sut3{ *m_testDynamicSubStructDescriptor,
        DynamicStruct::property_i<int64_t>{ "subIntProperty", 23 },
        DynamicStruct::property_i<float32_t>{ "subFloatProperty", 6.6261f }
    };

    EXPECT_EQ(ProxyProperty<int64_t>(sut3, sut3._path("subIntProperty")), int64_t{ 23 });
    EXPECT_EQ(ProxyProperty<float32_t>(sut3, sut3._path("subFloatProperty")), float32_t{ 6.6261f });
}

TEST_F(TestDynamicStruct, NestedAccessViaPropertyPathList)
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

    EXPECT_TRUE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[0]).isValid());
    EXPECT_TRUE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[1]).isValid());
    EXPECT_TRUE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[3]).isValid());
    EXPECT_TRUE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[4]).isValid());
    EXPECT_TRUE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[5]).isValid());
    EXPECT_TRUE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[6]).isValid());
    EXPECT_TRUE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[8]).isValid());

    EXPECT_EQ(ProxyProperty<int32_t>(sut, m_testDynamicStructDescriptor->propertyPaths()[0]), 1);
    EXPECT_EQ(ProxyProperty<string_t>(sut, m_testDynamicStructDescriptor->propertyPaths()[1]), "foo");
    EXPECT_EQ(ProxyProperty<vector_t<float32_t>>(sut, m_testDynamicStructDescriptor->propertyPaths()[3]), vector_t<float32_t>{ 3.1415f });
    EXPECT_EQ(ProxyProperty<int64_t>(sut, m_testDynamicStructDescriptor->propertyPaths()[5]), int64_t{ 42 });
    EXPECT_EQ(ProxyProperty<float64_t>(sut, m_testDynamicStructDescriptor->propertyPaths()[8]), 21.0);

    EXPECT_FALSE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[2]).isValid());
    EXPECT_FALSE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[7]).isValid());
    EXPECT_FALSE(ProxyProperty<>(sut, m_testDynamicStructDescriptor->propertyPaths()[9]).isValid());
}


TEST_F(TestDynamicStruct, PropertiesHaveExpectedTags)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };

    EXPECT_EQ(sut._get("intProperty").descriptor().tag(), 1u);
    EXPECT_EQ(sut._get("stringProperty").descriptor().tag(), 2u);
    EXPECT_EQ(sut._get("boolProperty").descriptor().tag(), 3u);
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().tag(), 4u);
    EXPECT_EQ(sut._get("subStructProperty").descriptor().tag(), 5u);
    EXPECT_EQ(sut._get("structVectorProperty").descriptor().tag(), 6u);
}

TEST_F(TestDynamicStruct, PropertiesHaveExpectedNames)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };

    EXPECT_EQ(sut._get("intProperty").descriptor().name(), "intProperty");
    EXPECT_EQ(sut._get("stringProperty").descriptor().name(), "stringProperty");
    EXPECT_EQ(sut._get("boolProperty").descriptor().name(), "boolProperty");
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().name(), "floatVectorProperty");
    EXPECT_EQ(sut._get("subStructProperty").descriptor().name(), "subStructProperty");
    EXPECT_EQ(sut._get("structVectorProperty").descriptor().name(), "structVectorProperty");
}

TEST_F(TestDynamicStruct, PropertiesHaveExpectedSet)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };

    EXPECT_EQ(sut._get("intProperty").descriptor().set(), PropertySet{ 0x1 << 1 });
    EXPECT_EQ(sut._get("stringProperty").descriptor().set(), PropertySet{ 0x1 << 2 });
    EXPECT_EQ(sut._get("boolProperty").descriptor().set(), PropertySet{ 0x1 << 3 });
    EXPECT_EQ(sut._get("floatVectorProperty").descriptor().set(), PropertySet{ 0x1 << 4 });
    EXPECT_EQ(sut._get("subStructProperty").descriptor().set(), PropertySet{ 0x1 << 5 });
    EXPECT_EQ(sut._get("structVectorProperty").descriptor().set(), PropertySet{ 0x1 << 6 });
}

TEST_F(TestDynamicStruct, _descriptor_SizeMatchesAllocateSize)
{
    DynamicStruct sut{ *m_testDynamicStructDescriptor };
    const PropertyDescriptor& lastPropertyDescriptor = sut._get("structVectorProperty").descriptor();

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
    const StructDescriptor& descriptor = sut._descriptor();

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
    EXPECT_EQ(sut._keyProperties(), sut._get("intProperty").descriptor().set());
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

TEST_F(TestDynamicStruct, constructViaDescriptor_Default)
{
    std::aligned_storage_t<sizeof(DynamicStruct), alignof(DynamicStruct)> storage;
    DynamicStruct& sutThis = reinterpret_cast<DynamicStruct&>(storage);
    m_testDynamicStructDescriptor->construct(sutThis);

    EXPECT_FALSE(sutThis._get("intProperty").isValid());
    EXPECT_FALSE(sutThis._get("stringProperty").isValid());
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_FALSE(sutThis._get("floatVectorProperty").isValid());

    m_testDynamicStructDescriptor->destruct(sutThis);
}

TEST_F(TestDynamicStruct, constructViaDescriptor_Copy)
{
    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    std::aligned_storage_t<sizeof(DynamicStruct), alignof(DynamicStruct)> storage;
    DynamicStruct& sutThis = reinterpret_cast<DynamicStruct&>(storage);
    m_testDynamicStructDescriptor->construct(sutThis, sutOther);

    EXPECT_EQ(sutThis._get("intProperty"), sutOther._get("intProperty"));
    EXPECT_EQ(sutThis._get("stringProperty"), sutOther._get("stringProperty"));
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get("floatVectorProperty"), sutOther._get("floatVectorProperty"));

    EXPECT_TRUE(sutOther._get("intProperty").isValid());
    EXPECT_TRUE(sutOther._get("stringProperty").isValid());
    EXPECT_FALSE(sutOther._get("boolProperty").isValid());
    EXPECT_TRUE(sutOther._get("floatVectorProperty").isValid());

    m_testDynamicStructDescriptor->destruct(sutThis);
}

TEST_F(TestDynamicStruct, constructViaDescriptor_Move)
{
    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    std::aligned_storage_t<sizeof(DynamicStruct), alignof(DynamicStruct)> storage;
    DynamicStruct& sutThis = reinterpret_cast<DynamicStruct&>(storage);
    m_testDynamicStructDescriptor->construct(sutThis, std::move(sutOther));

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "foo");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>({ 3.1415f, 2.7183f }));

    EXPECT_FALSE(sutOther._get("intProperty").isValid());
    EXPECT_FALSE(sutOther._get("stringProperty").isValid());
    EXPECT_FALSE(sutOther._get("boolProperty").isValid());
    EXPECT_FALSE(sutOther._get("floatVectorProperty").isValid());

    m_testDynamicStructDescriptor->destruct(sutThis);
}

TEST_F(TestDynamicStruct, constructInPlaceViaDescriptor_Default)
{
    auto storage = std::make_unique<std::byte[]>(m_testDynamicStructDescriptor->size());
    DynamicStruct& sutThis = reinterpret_cast<DynamicStruct&>(*storage.get());
    m_testDynamicStructDescriptor->construct(sutThis);

    EXPECT_FALSE(sutThis._get("intProperty").isValid());
    EXPECT_FALSE(sutThis._get("stringProperty").isValid());
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_FALSE(sutThis._get("floatVectorProperty").isValid());

    m_testDynamicStructDescriptor->destruct(sutThis);
}

TEST_F(TestDynamicStruct, constructInPlaceViaDescriptor_Copy)
{
    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    auto storage = std::make_unique<std::byte[]>(m_testDynamicStructDescriptor->size());
    DynamicStruct& sutThis = reinterpret_cast<DynamicStruct&>(*storage.get());
    m_testDynamicStructDescriptor->construct(sutThis, sutOther);

    EXPECT_EQ(sutThis._get("intProperty"), sutOther._get("intProperty"));
    EXPECT_EQ(sutThis._get("stringProperty"), sutOther._get("stringProperty"));
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get("floatVectorProperty"), sutOther._get("floatVectorProperty"));

    EXPECT_TRUE(sutOther._get("intProperty").isValid());
    EXPECT_TRUE(sutOther._get("stringProperty").isValid());
    EXPECT_FALSE(sutOther._get("boolProperty").isValid());
    EXPECT_TRUE(sutOther._get("floatVectorProperty").isValid());

    m_testDynamicStructDescriptor->destruct(sutThis);
}

TEST_F(TestDynamicStruct, constructInPlaceViaDescriptor_Move)
{
    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 1 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "foo" },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f, 2.7183f } }
    };
    auto storage = std::make_unique<std::byte[]>(m_testDynamicStructDescriptor->size());
    DynamicStruct& sutThis = reinterpret_cast<DynamicStruct&>(*storage.get());
    m_testDynamicStructDescriptor->construct(sutThis, std::move(sutOther));

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 1);
    EXPECT_EQ(sutThis._get<string_t>("stringProperty"), "foo");
    EXPECT_FALSE(sutThis._get("boolProperty").isValid());
    EXPECT_EQ(sutThis._get<vector_t<float32_t>>("floatVectorProperty"), vector_t<float32_t>({ 3.1415f, 2.7183f }));

    EXPECT_FALSE(sutOther._get("intProperty").isValid());
    EXPECT_FALSE(sutOther._get("stringProperty").isValid());
    EXPECT_FALSE(sutOther._get("boolProperty").isValid());
    EXPECT_FALSE(sutOther._get("floatVectorProperty").isValid());

    m_testDynamicStructDescriptor->destruct(sutThis);
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

    sutThis._get("subStructProperty").emplace(Typeless::From(std::move(sutSubOther)));

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
        DynamicStruct::property_i<bool_t>{ "boolProperty", true },
        DynamicStruct::property_i<vector_t<float32_t>>{ "floatVectorProperty", vector_t<float32_t>{ 3.1415f } }
    };

    DynamicStruct sutOther{ *m_testDynamicStructDescriptor,
        DynamicStruct::property_i<int32_t>{ "intProperty", 2 },
        DynamicStruct::property_i<string_t>{ "stringProperty", "bar" }
    };

    sutThis._copy(sutOther, sutThis._get("stringProperty").descriptor().set() + sutThis._get("boolProperty").descriptor().set());

    EXPECT_EQ(sutThis._get<int32_t>("intProperty"), 1);
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
