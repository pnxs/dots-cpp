// SPDX-License-Identifier: LGPL-3.0-only
#include <dots/testing/gtest/gtest.h>
#include <dots/serialization/CborSerializer.h>
#include <dots/serialization/AsciiSerialization.h>
#include <dots/serialization/ExperimentalCborSerializer.h>
#include <dots/serialization/JsonSerializer.h>
#include <dots/serialization/RapidJsonSerializer.h>
#include <dots/serialization/StringSerializer.h>
#include <dots/io/DescriptorConverter.h>
#include <ExampleType.dots.h>
#include <RouteType.dots.h>
#include <TimeType.dots.h>
#include <StatusType.dots.h>
#include <TypedStatusType.dots.h>
#include <RefScalarKeys.dots.h>
#include <RefTreeNode.dots.h>
#include <SerializationStructComplex.dots.h>

using namespace dots;
using namespace dots::types;
namespace
{
    const std::vector<uint8_t> ExampleKey{0x81, 0x64, 'e', 't', 'h', '0'};
    const std::vector<uint8_t> ExampleRef{0x82, 0x6b, 'E', 'x', 'a', 'm', 'p', 'l', 'e', 'T', 'y', 'p', 'e',
                                       0x81, 0x64, 'e', 't', 'h', '0'};
}

TEST(TestInstanceRef, goldenVectors)
{
    ExampleType example{.name = "eth0", .enabled = true};
    EXPECT_EQ(canonical_key(example), ExampleKey);
    auto ref = to_instance_ref(example);
    EXPECT_EQ(to_cbor(ref), ExampleRef);
    EXPECT_EQ(from_cbor<instance_ref_t>(ExampleRef), ref);
    auto typed = to_typed_instance_ref(example);
    EXPECT_EQ(to_cbor(typed), ExampleKey);
    EXPECT_EQ(from_cbor<typed_instance_ref_t<ExampleType>>(ExampleKey), typed);
    RouteType route{.name = "r1", .via = "192.0.2.1", .parent = "eth0"};
    EXPECT_EQ(canonical_key(route), (std::vector<uint8_t>{0x82, 0x64, 'e', 't', 'h', '0', 0x62, 'r', '1'}));
    EXPECT_EQ(canonical_key(TimeType{}), (std::vector<uint8_t>{0x80}));
    EXPECT_EQ(to_cbor(to_typed_instance_ref(TimeType{})), (std::vector<uint8_t>{0x80}));
    StatusType status{.subject = ref, .applied = true};
    auto nested = ExampleRef;
    nested.insert(nested.begin(), 0x81);
    EXPECT_EQ(canonical_key(status), nested);
}

TEST(TestInstanceRef, partialKeysAreOnlyAllowedAsStandaloneArtifacts)
{
    EXPECT_EQ(canonical_key(ExampleType{}), (std::vector<uint8_t>{0x81, 0xf6}));
    EXPECT_THROW(to_instance_ref(ExampleType{}), std::invalid_argument);
    EXPECT_THROW(to_cbor(instance_ref_t{}), std::invalid_argument);
    EXPECT_THROW(to_cbor(typed_instance_ref_t<ExampleType>{}), std::invalid_argument);
    // Existing models may have legacy key types; canonical identity rejects them.
    EXPECT_THROW(canonical_key(SerializationStructComplex{}), std::invalid_argument);
}

TEST(TestInstanceRef, rejectsMalformedKeys)
{
    const std::vector<std::vector<uint8_t>> invalid{
        {}, {0xa0}, {0x81}, {0x81,0xf6}, {0x81,0xf9,0,0}, {0x81,0xa0}, {0x81,0xc0,0},
        {0x9f,0xff}, {0x98,0}, {0x81,0x18,0x17}, {0x81,0x19,0,0xff},
        {0x81,0x1a,0,0,0xff,0xff}, {0x81,0x1b,0,0,0,0,0xff,0xff,0xff,0xff},
        {0x81,0x61,0xff}, {0x81,0x62,0xc0,0x80}, {0x81,0x63,0xed,0xa0,0x80},
        {0x81,0x78,1,'x'}, {0x81,0x63,'x'}, {0x81,0x82,0x61,'T',0x81,0xf6}, {0x80,0}
    };
    for (const auto& key : invalid) EXPECT_THROW((instance_ref_t{"ExampleType", key}), std::invalid_argument);
    auto nonCanonicalEnvelope = ExampleRef;
    nonCanonicalEnvelope[0] = 0x98;
    nonCanonicalEnvelope.insert(nonCanonicalEnvelope.begin() + 1, 2);
    EXPECT_THROW(from_cbor<instance_ref_t>(nonCanonicalEnvelope), std::invalid_argument);
    for (size_t size = 0; size < ExampleRef.size(); ++size)
        EXPECT_THROW(from_cbor<instance_ref_t>(ExampleRef.data(), size), std::exception);
}

TEST(TestInstanceRef, valueSemanticsAndText)
{
    instance_ref_t a{"ExampleType", ExampleKey};
    instance_ref_t b{"OtherType", ExampleKey};
    EXPECT_NE(a, b);
    EXPECT_LT(a, b);
    EXPECT_EQ(std::hash<instance_ref_t>{}(a), std::hash<instance_ref_t>{}(instance_ref_t::FromString(a.toString())));
    EXPECT_EQ(instance_ref_t::FromString(a.toString()), a);
    EXPECT_THROW(instance_ref_t::FromString("T#8"), std::invalid_argument);
    EXPECT_THROW(instance_ref_t::FromString("T#xx"), std::invalid_argument);
    EXPECT_THROW(instance_ref_t::FromString("T"), std::invalid_argument);
    EXPECT_THROW((typed_instance_ref_t<ExampleType>{b}), std::invalid_argument);
}

TEST(TestInstanceRef, generatedPropertiesVectorsAndTextSerializers)
{
    auto typed = to_typed_instance_ref(ExampleType{.name = "eth0"});
    TypedStatusType original{.subject = typed, .related = {typed}, .others = {typed}};
    EXPECT_EQ(canonical_key(original), (std::vector<uint8_t>{0x81, 0x81, 0x64, 'e', 't', 'h', '0'}));
    EXPECT_EQ(from_cbor<TypedStatusType>(to_cbor(original)), original);
    EXPECT_EQ(serialization::ExperimentalCborSerializer::Deserialize<TypedStatusType>(
        serialization::ExperimentalCborSerializer::Serialize(original)), original);
    EXPECT_EQ(serialization::JsonSerializer::Deserialize<TypedStatusType>(
        serialization::JsonSerializer::Serialize(original)), original);
    EXPECT_EQ(serialization::RapidJsonSerializer<serialization::DefaultRapidJsonSerializerFormat>::Deserialize<TypedStatusType>(
        serialization::RapidJsonSerializer<serialization::DefaultRapidJsonSerializerFormat>::Serialize(original)), original);
    EXPECT_EQ(serialization::StringSerializer::Deserialize<TypedStatusType>(
        serialization::StringSerializer::Serialize(original)), original);
}

TEST(TestInstanceRef, materializesOnlyKeys)
{
    type::Registry registry;
    auto ref = to_instance_ref(ExampleType{.name = "eth0", .enabled = true});
    auto instance = from_instance_ref(ref, registry);
    EXPECT_EQ(instance->_validProperties(), ExampleType::_Descriptor().keyProperties());
    EXPECT_EQ(to_instance_ref(*instance), ref);
    EXPECT_THROW(from_instance_ref(instance_ref_t{"ExampleType", {0x80}}, registry), std::invalid_argument);
    EXPECT_THROW(from_instance_ref(instance_ref_t{"ExampleType", {0x81,0x01}}, registry), std::exception);
}

TEST(TestInstanceRef, dynamicDescriptorExchange)
{
    type::Registry registry{std::nullopt, type::Registry::StaticTypePolicy::FundamentalOnly};
    io::DescriptorConverter converter{registry};
    converter(converter(ExampleType::_Descriptor()));
    const auto& descriptor = converter(converter(TypedStatusType::_Descriptor()));
    EXPECT_EQ(descriptor.propertyDescriptors()[0].valueDescriptor().name(), "instance_ref<ExampleType>");
    auto typed = to_typed_instance_ref(ExampleType{.name = "eth0"});
    TypedStatusType original{.subject = typed, .related = {typed}, .others = {typed}};
    type::AnyStruct dynamic{descriptor};
    from_cbor(to_cbor(original), *dynamic);
    EXPECT_EQ(to_cbor(*dynamic), to_cbor(original));
    EXPECT_EQ(canonical_key(*dynamic), canonical_key(original));
    type::AnyStruct copy{*dynamic};
    EXPECT_EQ(to_cbor(*copy), to_cbor(original));
    EXPECT_TRUE(copy->_equal(*dynamic));
    EXPECT_GT(descriptor.dynamicMemoryUsage(type::Typeless::From(*dynamic)), 0u);
}

TEST(TestInstanceRef, scalarBoundariesUseShortestEncoding)
{
    RefScalarKeys keys{.signedKey = std::numeric_limits<int64_t>::min(),
                       .unsignedKey = std::numeric_limits<uint64_t>::max(), .boolKey = true};
    const std::vector<uint8_t> expected{0x83, 0x1b, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                      0xf5, 0x3b, 0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    EXPECT_EQ(canonical_key(keys), expected);
    EXPECT_EQ(to_instance_ref(keys).key(), expected);
}

TEST(TestInstanceRef, selfReference)
{
    RefTreeNode parent{.name = "root"};
    RefTreeNode child{.name = "child", .parent = to_typed_instance_ref(parent)};
    EXPECT_EQ(from_cbor<RefTreeNode>(to_cbor(child)), child);
}

TEST(TestInstanceRef, opaqueTypedTargetsAndEmptySingletons)
{
    type::Registry registry{std::nullopt, type::Registry::StaticTypePolicy::FundamentalOnly};
    io::DescriptorConverter converter{registry};
    auto schema = converter(TypedStatusType::_Descriptor());
    schema.name = "OpaqueStatus";
    (*schema.properties)[0].type = "instance_ref<UnknownTarget>";
    auto& descriptor = converter(schema);
    EXPECT_EQ(descriptor.propertyDescriptors()[0].valueDescriptor().name(), "instance_ref<UnknownTarget>");
    EXPECT_EQ(registry.findType("UnknownTarget"), nullptr);
    type::AnyStruct opaque{descriptor};
    from_cbor(std::vector<uint8_t>{0xa1, 1, 0x81, 0x18, 42}, *opaque);
    EXPECT_EQ(canonical_key(*opaque), (std::vector<uint8_t>{0x81, 0x81, 0x18, 42}));

    StructDescriptorData singleton{.name = "EmptySingleton", .properties = vector_t<StructPropertyData>{}};
    converter(singleton);
    instance_ref_t ref{"EmptySingleton", {0x80}};
    EXPECT_EQ(to_instance_ref(*from_instance_ref(ref, registry)), ref);

    schema.name = "InvalidReferenceTarget";
    (*schema.properties)[0].type = "instance_ref<int32>";
    EXPECT_THROW(converter(schema), std::logic_error);
}

TEST(TestInstanceRef, unknownReferencePropertyCanBeSkipped)
{
    std::vector<uint8_t> bytes{0xa2, 0x18, 0x1f};
    bytes.insert(bytes.end(), ExampleRef.begin(), ExampleRef.end());
    bytes.insert(bytes.end(), {1, 0x64, 'e', 't', 'h', '0'});
    EXPECT_EQ(from_cbor<ExampleType>(bytes), (ExampleType{.name = "eth0"}));
}

TEST(TestInstanceRef, textOutputAndTypedTargetValidation)
{
    auto typed = to_typed_instance_ref(ExampleType{.name = "eth0"});
    TypedStatusType instance{.subject = typed};
    auto ascii = to_ascii(&instance._descriptor(), &instance);
    EXPECT_NE(ascii.find("ExampleType#816465746830"), std::string::npos);
    EXPECT_THROW((serialization::JsonSerializer::Deserialize<typed_instance_ref_t<ExampleType>>(
        "\"OtherType#80\"")), std::invalid_argument);
    EXPECT_THROW((serialization::RapidJsonSerializer<serialization::DefaultRapidJsonSerializerFormat>::Deserialize<typed_instance_ref_t<ExampleType>>(
        "\"OtherType#80\"")), std::invalid_argument);
}
