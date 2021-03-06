#include <sstream>
#include <dots/type/EnumDescriptor.h>
#include <dots/type/FundamentalTypes.h>
#include <gtest/gtest.h>

namespace dots::types
{
    enum class TestEnumSimple : int32_t
    {
        enumerator2 = 2,
        enumerator3 = 3,
        enumerator5 = 5,
        enumerator7 = 7,
        enumerator11 = 11,
        enumerator13 = 13
    };

    struct TestEnumGeneric : vector_t<string_t>
    {
        using underlying_type_t = types::vector_t<string_t>;
        using types::vector_t<string_t>::vector_t;
    };
}

namespace dots::type
{
    template <>
    struct Descriptor<types::TestEnumSimple> : EnumDescriptor<types::TestEnumSimple>
    {
        Descriptor() : EnumDescriptor("TestEnumSimple", {
            EnumeratorDescriptor{ 2, "enumerator2", types::TestEnumSimple::enumerator2 },
            EnumeratorDescriptor{ 3, "enumerator3", types::TestEnumSimple::enumerator3 },
            EnumeratorDescriptor{ 5, "enumerator5", types::TestEnumSimple::enumerator5 },
            EnumeratorDescriptor{ 7, "enumerator7", types::TestEnumSimple::enumerator7 },
            EnumeratorDescriptor{ 11, "enumerator11", types::TestEnumSimple::enumerator11 },
            EnumeratorDescriptor{ 13, "enumerator13", types::TestEnumSimple::enumerator13 }
        }){}
    };

    template <>
    struct Descriptor<types::TestEnumGeneric> : EnumDescriptor<types::TestEnumGeneric>
    {
        Descriptor() : EnumDescriptor("TestEnumGeneric", {
            EnumeratorDescriptor<types::TestEnumGeneric>{ 1, "enumerator1", { "foo", "bar"} },
            EnumeratorDescriptor<types::TestEnumGeneric>{ 4, "enumerator4", { "baz", "qux" } },
            EnumeratorDescriptor<types::TestEnumGeneric>{ 6, "enumerator6", { "bla", "blubb" } },
            EnumeratorDescriptor<types::TestEnumGeneric>{ 8, "enumerator8", { "meow", "bark" } },
            EnumeratorDescriptor<types::TestEnumGeneric>{ 9, "enumerator9", { "1", "3" } },
            EnumeratorDescriptor<types::TestEnumGeneric>{ 14, "enumerator14", { "a", "b" } },
            EnumeratorDescriptor<types::TestEnumGeneric>{ 16, "enumerator16", { "alpha", "beta" } }
        }) {}
    };
}

using namespace dots::type;
using namespace dots::types;

struct TestEnumDescriptor : ::testing::Test
{
protected:

    Descriptor<TestEnumSimple> m_sutSimple;
    Descriptor<TestEnumGeneric> m_sutGeneric;
};

TEST_F(TestEnumDescriptor, underlyingDescriptor)
{
    EXPECT_EQ(m_sutSimple.underlyingDescriptor().name(), "int32");
    EXPECT_EQ(m_sutGeneric.underlyingDescriptor().name(), "vector<string>");
}

TEST_F(TestEnumDescriptor, enumerators_expectedSize)
{
    EXPECT_EQ(m_sutSimple.enumerators().size(), 6);
    EXPECT_EQ(m_sutGeneric.enumerators().size(), 7);
}

TEST_F(TestEnumDescriptor, enumeratorsTypeless_expectedSize)
{
    EXPECT_EQ(m_sutSimple.enumeratorsTypeless().size(), 6);
    EXPECT_EQ(m_sutGeneric.enumeratorsTypeless().size(), 7);
}

TEST_F(TestEnumDescriptor, enumerators_expectedElements)
{
    auto expect_eq_enumerator = [](const auto& enumerator, uint32_t tag, const std::string_view& name, const auto& value)
    {
        EXPECT_EQ(enumerator.tag(), tag);
        EXPECT_EQ(enumerator.name(), name);
        EXPECT_EQ(enumerator.value(), value);
    };

    expect_eq_enumerator(m_sutSimple.enumerators()[0], 2, "enumerator2", TestEnumSimple::enumerator2);
    expect_eq_enumerator(m_sutSimple.enumerators()[1], 3, "enumerator3", TestEnumSimple::enumerator3);
    expect_eq_enumerator(m_sutSimple.enumerators()[2], 5, "enumerator5", TestEnumSimple::enumerator5);
    expect_eq_enumerator(m_sutSimple.enumerators()[3], 7, "enumerator7", TestEnumSimple::enumerator7);
    expect_eq_enumerator(m_sutSimple.enumerators()[4], 11, "enumerator11", TestEnumSimple::enumerator11);
    expect_eq_enumerator(m_sutSimple.enumerators()[5], 13, "enumerator13", TestEnumSimple::enumerator13);

    expect_eq_enumerator(m_sutGeneric.enumerators()[0], 1, "enumerator1", TestEnumGeneric{ "foo", "bar"});
    expect_eq_enumerator(m_sutGeneric.enumerators()[1], 4, "enumerator4", TestEnumGeneric{ "baz", "qux" });
    expect_eq_enumerator(m_sutGeneric.enumerators()[2], 6, "enumerator6", TestEnumGeneric{ "bla", "blubb" });
    expect_eq_enumerator(m_sutGeneric.enumerators()[3], 8, "enumerator8", TestEnumGeneric{ "meow", "bark" });
    expect_eq_enumerator(m_sutGeneric.enumerators()[4], 9, "enumerator9", TestEnumGeneric{ "1", "3" });
    expect_eq_enumerator(m_sutGeneric.enumerators()[5], 14, "enumerator14", TestEnumGeneric{ "a", "b" });
    expect_eq_enumerator(m_sutGeneric.enumerators()[6], 16, "enumerator16", TestEnumGeneric{ "alpha", "beta" } );
}

TEST_F(TestEnumDescriptor, enumeratorsTypeless_expectedElements)
{
    auto expect_eq_enumerator_simple = [&](const EnumeratorDescriptor<>& enumerator, uint32_t tag, const std::string_view& name, const TestEnumSimple& value)
    {
        EXPECT_EQ(enumerator.tag(), tag);
        EXPECT_EQ(enumerator.name(), name);
        EXPECT_TRUE(enumerator.underlyingDescriptor().equal(enumerator.valueTypeless(), Typeless::From(value)));
    };

    expect_eq_enumerator_simple(m_sutSimple.enumeratorsTypeless()[0], 2, "enumerator2", TestEnumSimple::enumerator2);
    expect_eq_enumerator_simple(m_sutSimple.enumeratorsTypeless()[1], 3, "enumerator3", TestEnumSimple::enumerator3);
    expect_eq_enumerator_simple(m_sutSimple.enumeratorsTypeless()[2], 5, "enumerator5", TestEnumSimple::enumerator5);
    expect_eq_enumerator_simple(m_sutSimple.enumeratorsTypeless()[3], 7, "enumerator7", TestEnumSimple::enumerator7);
    expect_eq_enumerator_simple(m_sutSimple.enumeratorsTypeless()[4], 11, "enumerator11", TestEnumSimple::enumerator11);
    expect_eq_enumerator_simple(m_sutSimple.enumeratorsTypeless()[5], 13, "enumerator13", TestEnumSimple::enumerator13);

    auto expect_eq_enumerator_generic = [&](const EnumeratorDescriptor<>& enumerator, uint32_t tag, const std::string_view& name, const TestEnumGeneric& value)
    {
        EXPECT_EQ(enumerator.tag(), tag);
        EXPECT_EQ(enumerator.name(), name);
        EXPECT_TRUE(enumerator.underlyingDescriptor().equal(enumerator.valueTypeless(), Typeless::From(value)));
    };

    expect_eq_enumerator_generic(m_sutGeneric.enumeratorsTypeless()[0], 1, "enumerator1", TestEnumGeneric{ "foo", "bar"});
    expect_eq_enumerator_generic(m_sutGeneric.enumeratorsTypeless()[1], 4, "enumerator4", TestEnumGeneric{ "baz", "qux" });
    expect_eq_enumerator_generic(m_sutGeneric.enumeratorsTypeless()[2], 6, "enumerator6", TestEnumGeneric{ "bla", "blubb" });
    expect_eq_enumerator_generic(m_sutGeneric.enumeratorsTypeless()[3], 8, "enumerator8", TestEnumGeneric{ "meow", "bark" });
    expect_eq_enumerator_generic(m_sutGeneric.enumeratorsTypeless()[4], 9, "enumerator9", TestEnumGeneric{ "1", "3" });
    expect_eq_enumerator_generic(m_sutGeneric.enumeratorsTypeless()[5], 14, "enumerator14", TestEnumGeneric{ "a", "b" });
    expect_eq_enumerator_generic(m_sutGeneric.enumeratorsTypeless()[6], 16, "enumerator16", TestEnumGeneric{ "alpha", "beta" });
}

TEST_F(TestEnumDescriptor, enumeratorFromTag)
{
    EXPECT_EQ(m_sutSimple.enumeratorFromTag(2).name(), "enumerator2");
    EXPECT_EQ(m_sutSimple.enumeratorFromTag(7).value(), TestEnumSimple::enumerator7);

    EXPECT_EQ(m_sutGeneric.enumeratorFromTag(1).name(), "enumerator1");
    EXPECT_EQ(m_sutGeneric.enumeratorFromTag(8).value(), TestEnumGeneric({ "meow", "bark" }));

    EXPECT_THROW(m_sutSimple.enumeratorFromTag(1), std::logic_error);
    EXPECT_THROW(m_sutGeneric.enumeratorFromTag(2), std::logic_error);
}

TEST_F(TestEnumDescriptor, enumeratorFromName)
{
    EXPECT_EQ(m_sutSimple.enumeratorFromName("enumerator3").tag(), 3);
    EXPECT_EQ(m_sutSimple.enumeratorFromName("enumerator5").value(), TestEnumSimple::enumerator5);

    EXPECT_EQ(m_sutGeneric.enumeratorFromName("enumerator4").tag(), 4);
    EXPECT_EQ(m_sutGeneric.enumeratorFromName("enumerator9").value(), TestEnumGeneric({ "1", "3" }));

    EXPECT_THROW(m_sutSimple.enumeratorFromName("enumerator4"), std::logic_error);
    EXPECT_THROW(m_sutGeneric.enumeratorFromName("enumerator3"), std::logic_error);
}

TEST_F(TestEnumDescriptor, enumeratorFromValue)
{
    EXPECT_EQ(m_sutSimple.enumeratorFromValue(TestEnumSimple::enumerator11).tag(), 11);
    EXPECT_EQ(m_sutSimple.enumeratorFromValue(TestEnumSimple::enumerator13).name(), "enumerator13");

    EXPECT_EQ(m_sutGeneric.enumeratorFromValue(TestEnumGeneric{ "bla", "blubb" }).tag(), 6);
    EXPECT_EQ(m_sutGeneric.enumeratorFromValue(TestEnumGeneric{ "a", "b" }).name(), "enumerator14");

    EXPECT_THROW(m_sutSimple.enumeratorFromValue(static_cast<TestEnumSimple>(6)), std::logic_error);
    EXPECT_THROW(m_sutGeneric.enumeratorFromValue(TestEnumGeneric({ "1", "2" })), std::logic_error);
}