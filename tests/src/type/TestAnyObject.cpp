// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/AnyObject.h>
#include <dots/type/Registry.h>
#include <dots/serialization/CborSerializer.h>
#include <dots/serialization/RapidJsonSerializer.h>
#include <SerializationStructSimple.dots.h>

using dots::type::AnyObject;
using dots::types::SerializationStructSimple;

TEST(TestAnyObject, defaultConstructed_IsEmpty)
{
    AnyObject any;
    EXPECT_TRUE(any.empty());
    EXPECT_TRUE(any.typeName().empty());
    EXPECT_TRUE(any.payload().empty());
}

TEST(TestAnyObject, valueSemantics_EqualityAndOrdering)
{
    AnyObject a{ "Foo", { 0x01, 0x02 } };
    AnyObject b{ "Foo", { 0x01, 0x02 } };
    AnyObject c{ "Foo", { 0x01, 0x03 } };
    AnyObject d{ "Bar", { 0x01, 0x02 } };

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);

    EXPECT_LT(a, c);            // same name, payload differs
    EXPECT_LT(d, a);            // "Bar" < "Foo"

    EXPECT_EQ(std::hash<AnyObject>{}(a), std::hash<AnyObject>{}(b));
}

TEST(TestAnyObject, cborRoundTrip_PreservesTypeNameAndPayload)
{
    AnyObject any{ "Ping", { 0xA1, 0x01, 0x18, 0x2A } };

    std::vector<uint8_t> encoded = dots::to_cbor(any);
    AnyObject decoded = dots::from_cbor<AnyObject>(encoded);

    EXPECT_EQ(decoded, any);
    EXPECT_EQ(decoded.typeName(), "Ping");
    EXPECT_EQ(decoded.payload(), (std::vector<uint8_t>{ 0xA1, 0x01, 0x18, 0x2A }));
}

TEST(TestAnyObject, cborWireFormat_IsOpaqueEnvelope)
{
    AnyObject any{ "Ping", { 0xA1, 0x01, 0x18, 0x2A } };

    std::vector<uint8_t> encoded = dots::to_cbor(any);

    // [ "Ping", h'A101182A' ]
    const std::vector<uint8_t> expected{
        0x82,                               // array(2)
        0x64, 'P', 'i', 'n', 'g',           // text(4) "Ping"
        0x44, 0xA1, 0x01, 0x18, 0x2A        // bytes(4)
    };
    EXPECT_EQ(encoded, expected);
}

TEST(TestAnyObject, toAny_fromAny_RoundTripsThroughRegistry)
{
    SerializationStructSimple original{
        .int32Property = 42,
        .stringProperty = "hello",
        .boolProperty = true,
        .float32Property = 3.5f
    };

    // wrap -> serialize the envelope -> deserialize the envelope (opaque, no registry)
    AnyObject any = dots::to_any(original);
    EXPECT_EQ(any.typeName(), "SerializationStructSimple");

    AnyObject transported = dots::from_cbor<AnyObject>(dots::to_cbor(any));
    EXPECT_EQ(transported, any);

    // recover the concrete struct via the registry
    dots::type::Registry registry;
    dots::type::AnyStruct recovered = dots::from_any(transported, registry);

    EXPECT_TRUE(recovered->_equal(original));
}

TEST(TestAnyObject, fromAny_UnknownType_Throws)
{
    AnyObject any{ "ThisTypeIsNotRegistered", { 0xA0 } };
    dots::type::Registry registry{ std::nullopt, dots::type::Registry::StaticTypePolicy::FundamentalOnly };

    EXPECT_THROW(dots::from_any(any, registry), std::logic_error);
}

TEST(TestAnyObject, json_PlainCall_IsOpaque)
{
    using serializer_t = dots::serialization::RapidJsonSerializer<>;

    AnyObject any{ "Ping", { 0xA1, 0x01, 0x18, 0x2A } };
    std::string json = serializer_t::Serialize(any);

    EXPECT_EQ(json, "\"Ping#a101182a\"");

    AnyObject decoded = serializer_t::Deserialize<AnyObject>(json);
    EXPECT_EQ(decoded, any);
}

TEST(TestAnyObject, json_RegistryOverload_ExpandsInline)
{
    using serializer_t = dots::serialization::RapidJsonSerializer<>;

    SerializationStructSimple original{
        .int32Property = 42,
        .stringProperty = "hello",
        .boolProperty = true
    };

    dots::type::Registry registry;
    AnyObject any = dots::to_any(original);

    // expand mode: contained object is inlined under @type / value
    std::string json = serializer_t::Serialize(any, registry);
    EXPECT_NE(json.find("\"@type\":\"SerializationStructSimple\""), std::string::npos);
    EXPECT_NE(json.find("\"stringProperty\":\"hello\""), std::string::npos);

    // and round-trips back through the registry overload
    AnyObject roundTripped = serializer_t::Deserialize<AnyObject>(json, registry);
    EXPECT_EQ(roundTripped.typeName(), "SerializationStructSimple");

    dots::type::AnyStruct recovered = dots::from_any(roundTripped, registry);
    EXPECT_TRUE(recovered->_equal(original));
}

TEST(TestAnyObject, json_ExpandUnknownType_Throws)
{
    using serializer_t = dots::serialization::RapidJsonSerializer<>;

    AnyObject any{ "NopeNotRegistered", { 0xA0 } };
    dots::type::Registry registry{ std::nullopt, dots::type::Registry::StaticTypePolicy::FundamentalOnly };

    EXPECT_THROW(serializer_t::Serialize(any, registry), std::logic_error);
}

TEST(TestAnyObject, descriptor_HasAnyType)
{
    const dots::type::Descriptor<dots::types::any_t>& descriptor = dots::type::Descriptor<dots::types::any_t>::Instance();
    EXPECT_EQ(descriptor.type(), dots::type::Type::Any);
    EXPECT_EQ(descriptor.name(), "any");
    EXPECT_TRUE(descriptor.usesDynamicMemory());
}
