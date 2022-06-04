#include <sstream>
#include <dots/type/EnumDescriptor.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/testing/gtest/gtest.h>

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
        using underlying_type_t = dots::vector_t<string_t>;
        using dots::vector_t<string_t>::vector_t;
    };
}

namespace dots::type
{
    template <>
    struct Descriptor<types::TestEnumSimple> : EnumDescriptor<>
    {
        Descriptor(key_t key) : EnumDescriptor(key, "TestEnumSimple", {
            EnumeratorDescriptor{ 2, "enumerator2", types::TestEnumSimple::enumerator2 },
            EnumeratorDescriptor{ 3, "enumerator3", types::TestEnumSimple::enumerator3 },
            EnumeratorDescriptor{ 5, "enumerator5", types::TestEnumSimple::enumerator5 },
            EnumeratorDescriptor{ 7, "enumerator7", types::TestEnumSimple::enumerator7 },
            EnumeratorDescriptor{ 11, "enumerator11", types::TestEnumSimple::enumerator11 },
            EnumeratorDescriptor{ 13, "enumerator13", types::TestEnumSimple::enumerator13 }
        }){}

        static auto& Instance()
        {
            return InitInstance<types::TestEnumSimple>();
        }
    };
}

using namespace dots::type;
using namespace dots::types;

struct TestEnumDescriptor : ::testing::Test
{
protected:

    std::shared_ptr<Descriptor<TestEnumSimple>> m_sutSimple = make_descriptor<Descriptor<TestEnumSimple>>();
};

TEST_F(TestEnumDescriptor, underlyingDescriptor)
{
    EXPECT_EQ(m_sutSimple->underlyingDescriptor().name(), "int32");
}

TEST_F(TestEnumDescriptor, enumerators_expectedSize)
{
    EXPECT_EQ(m_sutSimple->enumerators().size(), 6);
}

TEST_F(TestEnumDescriptor, enumeratorsTypeless_expectedSize)
{
    EXPECT_EQ(m_sutSimple->enumeratorsTypeless().size(), 6);
}

TEST_F(TestEnumDescriptor, enumerators_expectedElements)
{
    auto expect_eq_enumerator = [](const auto& enumerator, uint32_t tag, std::string_view name, const auto& value)
    {
        EXPECT_EQ(enumerator.tag(), tag);
        EXPECT_EQ(enumerator.name(), name);
        EXPECT_EQ(enumerator.template value<TestEnumSimple>(), value);
    };

    expect_eq_enumerator(m_sutSimple->enumerators()[0], 2, "enumerator2", TestEnumSimple::enumerator2);
    expect_eq_enumerator(m_sutSimple->enumerators()[1], 3, "enumerator3", TestEnumSimple::enumerator3);
    expect_eq_enumerator(m_sutSimple->enumerators()[2], 5, "enumerator5", TestEnumSimple::enumerator5);
    expect_eq_enumerator(m_sutSimple->enumerators()[3], 7, "enumerator7", TestEnumSimple::enumerator7);
    expect_eq_enumerator(m_sutSimple->enumerators()[4], 11, "enumerator11", TestEnumSimple::enumerator11);
    expect_eq_enumerator(m_sutSimple->enumerators()[5], 13, "enumerator13", TestEnumSimple::enumerator13);
}

TEST_F(TestEnumDescriptor, enumeratorsTypeless_expectedElements)
{
    auto expect_eq_enumerator_simple = [&](const EnumeratorDescriptor& enumerator, uint32_t tag, std::string_view name, const TestEnumSimple& value)
    {
        EXPECT_EQ(enumerator.tag(), tag);
        EXPECT_EQ(enumerator.name(), name);
        EXPECT_TRUE(enumerator.underlyingDescriptor().equal(enumerator.valueTypeless(), Typeless::From(value)));
    };

    expect_eq_enumerator_simple(m_sutSimple->enumeratorsTypeless()[0], 2, "enumerator2", TestEnumSimple::enumerator2);
    expect_eq_enumerator_simple(m_sutSimple->enumeratorsTypeless()[1], 3, "enumerator3", TestEnumSimple::enumerator3);
    expect_eq_enumerator_simple(m_sutSimple->enumeratorsTypeless()[2], 5, "enumerator5", TestEnumSimple::enumerator5);
    expect_eq_enumerator_simple(m_sutSimple->enumeratorsTypeless()[3], 7, "enumerator7", TestEnumSimple::enumerator7);
    expect_eq_enumerator_simple(m_sutSimple->enumeratorsTypeless()[4], 11, "enumerator11", TestEnumSimple::enumerator11);
    expect_eq_enumerator_simple(m_sutSimple->enumeratorsTypeless()[5], 13, "enumerator13", TestEnumSimple::enumerator13);
}

TEST_F(TestEnumDescriptor, enumeratorFromTag)
{
    EXPECT_EQ(m_sutSimple->enumeratorFromTag(2).name(), "enumerator2");
    EXPECT_EQ(m_sutSimple->enumeratorFromTag(7).value<TestEnumSimple>(), TestEnumSimple::enumerator7);

    EXPECT_THROW(m_sutSimple->enumeratorFromTag(1), std::logic_error);
}

TEST_F(TestEnumDescriptor, enumeratorFromName)
{
    EXPECT_EQ(m_sutSimple->enumeratorFromName("enumerator3").tag(), 3u);
    EXPECT_EQ(m_sutSimple->enumeratorFromName("enumerator5").value<TestEnumSimple>(), TestEnumSimple::enumerator5);

    EXPECT_THROW(m_sutSimple->enumeratorFromName("enumerator4"), std::logic_error);
}

TEST_F(TestEnumDescriptor, enumeratorFromValue)
{
    EXPECT_EQ(m_sutSimple->enumeratorFromValue(TestEnumSimple::enumerator11).tag(), 11u);
    EXPECT_EQ(m_sutSimple->enumeratorFromValue(TestEnumSimple::enumerator13).name(), "enumerator13");

    EXPECT_THROW(m_sutSimple->enumeratorFromValue(static_cast<TestEnumSimple>(6)), std::logic_error);
}
