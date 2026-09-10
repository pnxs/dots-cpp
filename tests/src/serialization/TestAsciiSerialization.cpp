// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/serialization/AsciiSerialization.h>
#include <dots/serialization/CborSerializer.h>
#include "StructDescriptorData.dots.h"
#include "DotsTestStruct.dots.h"
#include "SerializationStructSimple.dots.h"
#include "SerializationStructAny.dots.h"
#include "dots/type/Registry.h"
#include <dots/testing/gtest/gtest.h>

#include <iostream>

using namespace dots::type;


TEST(TestAsciiSerialization, serialize)
{
    StructDescriptorData sd{
        .name = "aName"
    };

    auto& properties = sd.properties.emplace();
    auto& documentation = sd.documentation.emplace();

    StructPropertyData pd{
        .name = "aProperty" ,
        .tag = 1 ,
        .isKey = false ,
        .type = "type" ,
    };

    properties.push_back(pd);
    pd.name = "anotherProperty";
    pd.tag = 2;
    properties.push_back(pd);

    documentation.description.emplace("aDescription");
    documentation.comment.emplace("aComment");

    std::string expectedOutput = "{\n"
            "    name=aName\n"
            "    properties=[\n"
            "        {\n"
            "            name=aProperty\n"
            "            tag=1\n"
            "            isKey=0\n"
            "            type=type\n"
            "        },\n"
            "        {\n"
            "            name=anotherProperty\n"
            "            tag=2\n"
            "            isKey=0\n"
            "            type=type\n"
            "        },\n"
            "    ]\n"
            "    documentation={\n"
            "        description=aDescription\n"
            "        comment=aComment\n"
            "    }\n"
            "}\n";

    EXPECT_EQ(dots::to_ascii(&sd._Descriptor(), &sd, PropertySet::All), expectedOutput);
}

TEST(TestAsciiSerialization, serializeSingleLine)
{
    StructDescriptorData sd{
        .name = "aName"
    };

    auto& properties = sd.properties.emplace();
    auto& documentation = sd.documentation.emplace();

    StructPropertyData pd{
        .name = "aProperty" ,
        .tag = 1 ,
        .isKey = false ,
        .type = "type" ,
    };

    properties.push_back(pd);
    pd.name = "anotherProperty";
    pd.tag = 2;
    properties.push_back(pd);

    documentation.description.emplace("aDescription");
    documentation.comment.emplace("aComment");

    dots::ToAsciiOptions options;
    options.singleLine = true;

    std::string expectedOutput = "<name:aName properties:[<name:aProperty tag:1 isKey:0 type:type >, <name:anotherProperty tag:2 isKey:0 type:type >] documentation:<description:aDescription comment:aComment >>";

    EXPECT_EQ(dots::to_ascii(&sd._Descriptor(), &sd, PropertySet::All, options), expectedOutput);
}

TEST(TestAsciiSerialization, serializeSingleLineWithEnums)
{
    DotsTestStruct ts{
        .indKeyfField = 42 ,
        .enumField = DotsTestEnum::value3 ,
        .tp = TimePoint() ,
        .uuid = dots::uuid_t{ dots::uuid_t::value_t{} }
    };

    dots::ToAsciiOptions options;
    options.singleLine = true;

    //std::string expectedOutput = "<name:aName properties:[<name:aProperty tag:1 isKey:0 type:type >, <name:anotherProperty tag:2 isKey:0 type:type >] documentation:<description:aDescription comment:aComment >>";
    std::string expectedOutput = "<indKeyfField:42 enumField:value3 tp:0.000000 uuid:00000000-0000-0000-0000-000000000000 >";

    //std::cout << "Ascii: '" << dots::to_ascii(&ts._Descriptor(), &ts, PropertySet::All, options) << "'";

    EXPECT_EQ(dots::to_ascii(&ts._Descriptor(), &ts, PropertySet::All, options), expectedOutput);
}

TEST(TestAsciiSerialization, anyField_ExpandedWithRegistry)
{
    using dots::types::SerializationStructSimple;
    using dots::types::SerializationStructAny;

    SerializationStructSimple inner{
        .int32Property = 42,
        .stringProperty = "hello",
        .boolProperty = true
    };

    SerializationStructAny holder{
        .int32Property = 7,
        .anyProperty = dots::to_any(inner)
    };

    dots::type::Registry registry;

    dots::ToAsciiOptions options;
    options.singleLine = true;
    options.registry = &registry;

    std::string out = dots::to_ascii(&holder._Descriptor(), &holder, PropertySet::All, options);

    // the `any` field is expanded into the contained object via from_any()
    EXPECT_NE(out.find("@type:SerializationStructSimple"), std::string::npos);
    EXPECT_NE(out.find("stringProperty:hello"), std::string::npos);
    EXPECT_NE(out.find("int32Property:42"), std::string::npos);
}

TEST(TestAsciiSerialization, anyField_OpaqueWithoutRegistry)
{
    using dots::types::SerializationStructSimple;
    using dots::types::SerializationStructAny;

    SerializationStructSimple inner{
        .int32Property = 1,
        .stringProperty = "x"
    };

    SerializationStructAny holder{
        .int32Property = 7,
        .anyProperty = dots::to_any(inner)
    };

    dots::ToAsciiOptions options;
    options.singleLine = true;
    // no registry -> opaque "typeName#<hex>" form, and must not throw

    std::string out = dots::to_ascii(&holder._Descriptor(), &holder, PropertySet::All, options);

    EXPECT_NE(out.find("SerializationStructSimple#"), std::string::npos);
}

struct TraceColorSchema: dots::ToAsciiColorSchema
{
    const char* string() const override { return "\33[1;35m"; }
    const char* integer() const override { return "\33[1;31m"; }
    const char* floatingPoint() const override { return "\33[1;31m"; }
    const char* enumValue() const override { return "\33[1;33m"; }
    const char* attribute() const override { return "\33[0;37m"; }
    const char* typeName() const override { return "\33[1;34m"; }
    const char* allOff() const override { return "\33[0m"; }
    const char* create() const override { return "\33[1;32m"; }
    const char* update() const override { return "\33[1;33m"; }
    const char* remove() const override { return "\33[1;31m"; }
    const char* timestamp() const override { return "\33[1;30m"; }
    const char* highlight() const override { return "\33[43m"; }
};

TEST(TestAsciiSerialization, serializeSingleLineColored)
{
    StructDescriptorData sd{
        .name = "aName"
    };

    auto& properties = sd.properties.emplace();
    auto& documentation = sd.documentation.emplace();

    StructPropertyData pd{
        .name = "aProperty" ,
        .tag = 1 ,
        .isKey = false ,
        .type = "type" ,
    };

    properties.push_back(pd);
    pd.name = "anotherProperty";
    pd.tag = 2;
    properties.push_back(pd);

    documentation.description.emplace("aDescription");
    documentation.comment.emplace("aComment");

    dots::ToAsciiOptions options;
    TraceColorSchema cs;
    options.singleLine = true;
    options.cs = &cs;

    std::cout << "\nAscii: " << dots::to_ascii(&sd._Descriptor(), &sd, PropertySet::All, options) << "\n";
}
