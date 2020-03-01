#include <dots/io/serialization/AsciiSerialization.h>
#include "StructDescriptorData.dots.h"
#include "DotsTestStruct.dots.h"
#include "dots/io/Registry.h"
#include <gtest/gtest.h>

#include <iostream>

using namespace dots::type;


TEST(TestAsciiSerialization, serialize)
{
    StructDescriptorData sd;
    sd.name("aName");

    auto& properties = sd.properties();
    auto& documentation = sd.documentation();

    StructPropertyData pd;
    pd.name("aProperty");
    pd.tag(1);
    pd.type("type");
    pd.isKey(false);

    properties.push_back(pd);
    pd.name = "anotherProperty";
    pd.tag = 2;
    properties.push_back(pd);

    documentation.description("aDescription");
    documentation.comment("aComment");

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
    StructDescriptorData sd;
    sd.name("aName");

    auto& properties = sd.properties();
    auto& documentation = sd.documentation();

    StructPropertyData pd;
    pd.name("aProperty");
    pd.tag(1);
    pd.type("type");
    pd.isKey(false);

    properties.push_back(pd);
    pd.name = "anotherProperty";
    pd.tag = 2;
    properties.push_back(pd);

    documentation.description("aDescription");
    documentation.comment("aComment");
    
    dots::ToAsciiOptions options;
    options.singleLine = true;

    std::string expectedOutput = "<name:aName properties:[<name:aProperty tag:1 isKey:0 type:type >, <name:anotherProperty tag:2 isKey:0 type:type >] documentation:<description:aDescription comment:aComment >>";

    EXPECT_EQ(dots::to_ascii(&sd._Descriptor(), &sd, PropertySet::All, options), expectedOutput);
}

TEST(TestAsciiSerialization, serializeSingleLineWithEnums)
{
    DotsTestStruct ts;
    ts.indKeyfField(42);
    ts.enumField(DotsTestEnum::value3);
    ts.tp(dots::type::TimePoint(0));
    ts.uuid(dots::uuid());

    dots::ToAsciiOptions options;
    options.singleLine = true;

    //std::string expectedOutput = "<name:aName properties:[<name:aProperty tag:1 isKey:0 type:type >, <name:anotherProperty tag:2 isKey:0 type:type >] documentation:<description:aDescription comment:aComment >>";
    std::string expectedOutput = "<indKeyfField:42 enumField:value3 tp:0.000000 uuid:00000000-0000-0000-0000-000000000000 >";

    //std::cout << "Ascii: '" << dots::to_ascii(&ts._Descriptor(), &ts, PropertySet::All, options) << "'";

    EXPECT_EQ(dots::to_ascii(&ts._Descriptor(), &ts, PropertySet::All, options), expectedOutput);
}

struct TraceColorSchema: public dots::ToAsciiColorSchema
{
    const char* string() const override { return "\33[1;35m"; };
    const char* integer() const override { return "\33[1;31m"; };
    const char* floatingPoint() const override { return "\33[1;31m"; };
    const char* enumValue() const override { return "\33[1;33m"; };
    const char* attribute() const override { return "\33[0;37m"; };
    const char* typeName() const override { return "\33[1;34m"; };
    const char* allOff() const override { return "\33[0m"; }
    const char* create() const override { return "\33[1;32m"; }
    const char* update() const override { return "\33[1;33m"; }
    const char* remove() const override { return "\33[1;31m"; }
    const char* timestamp() const override { return "\33[1;30m"; }
    const char* highlight() const override { return "\33[43m"; }
};

TEST(TestAsciiSerialization, serializeSingleLineColored)
{
    StructDescriptorData sd;
    sd.name("aName");

    auto& properties = sd.properties();
    auto& documentation = sd.documentation();

    StructPropertyData pd;
    pd.name("aProperty");
    pd.tag(1);
    pd.type("type");
    pd.isKey(false);

    properties.push_back(pd);
    pd.name = "anotherProperty";
    pd.tag = 2;
    properties.push_back(pd);

    documentation.description("aDescription");
    documentation.comment("aComment");

    dots::ToAsciiOptions options;
    TraceColorSchema cs;
    options.singleLine = true;
    options.cs = &cs;

    std::cout << "\nAscii: " << dots::to_ascii(&sd._Descriptor(), &sd, PropertySet::All, options) << "\n";
}