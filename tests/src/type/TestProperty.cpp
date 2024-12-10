// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <string>
#include <type_traits>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/Property.h>
#include <dots/type/FundamentalTypes.h>
#include <TestStruct.dots.h>

using namespace dots::type;

struct TestProperty : ::testing::Test
{
protected:

    TestProperty() :
        m_sut(m_propertyArea.stringProperty),
        m_sutLhs(m_propertyAreaLhs.stringProperty),
        m_sutRhs(m_propertyAreaRhs.stringProperty) {}

    TestStruct m_propertyArea;
    TestStruct m_propertyAreaLhs;
    TestStruct m_propertyAreaRhs;

    TestStruct::stringProperty_pt& m_sut;
    TestStruct::stringProperty_pt& m_sutLhs;
    TestStruct::stringProperty_pt& m_sutRhs;
};

TEST_F(TestProperty, isValid_InvalidWithoutValue)
{
    EXPECT_FALSE(m_sut.isValid());
    EXPECT_EQ(m_sut, dots::invalid);
    EXPECT_EQ(dots::invalid, m_sut);
    EXPECT_THROW(m_sut.value(), std::runtime_error);
}

TEST_F(TestProperty, isValid_ValidWithValue)
{
    m_sut.emplace();

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_NE(m_sut, dots::invalid);
    EXPECT_NE(dots::invalid, m_sut);
    EXPECT_NO_THROW(m_sut.value());
}

TEST_F(TestProperty, emplace_EqualValueAfterExplicitConstruction)
{
    m_sut.emplace(std::string{ "foo" });

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(m_sut.value(), std::string{ "foo" });
}

TEST_F(TestProperty, emplace_EqualValueAfterImplicitConstruction)
{
    m_sut.emplace("foo");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProperty, emplace_EqualValueAfterEmplaceConstruction)
{
    m_sut.emplace(std::string{ "barfoo" }, 3u, 3u);

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(m_sut.value(), "foo");
}

TEST_F(TestProperty, emplace_EqualValueAfterConstructionFromOther)
{
    m_sutRhs.emplace("foo");
    m_sutLhs = m_sutRhs;

    EXPECT_TRUE(m_sutLhs.isValid());
    EXPECT_EQ(m_sutLhs.value(), "foo");
}

TEST_F(TestProperty, emplace_InvalidAfterConstructionFromInvalidOther)
{
    m_sutLhs = m_sutRhs;
    EXPECT_FALSE(m_sutLhs.isValid());
}

TEST_F(TestProperty, destroy_InvalidAfterDestroy)
{
    {
        m_sut.emplace("foo");
        m_sut.reset();

        EXPECT_FALSE(m_sut.isValid());
    }

    {
        m_sut.emplace("foo");
        m_sut = dots::invalid;

        EXPECT_FALSE(m_sut.isValid());
    }
}

TEST_F(TestProperty, valueOrDefault_ValueOnValid)
{
    m_sut.emplace("foo");
    std::string value = m_sut.valueOrDefault("bar");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(value, "foo");
}

TEST_F(TestProperty, valueOrDefault_DefaultOnValid)
{
    std::string value = m_sut.valueOrDefault("bar");

    EXPECT_FALSE(m_sut.isValid());
    EXPECT_EQ(value, "bar");
}

TEST_F(TestProperty, valueOrEmplace_EmplaceOnInvalid)
{
    std::string& value = m_sut.valueOrEmplace("foo");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(value, "foo");
}

TEST_F(TestProperty, valueOrEmplace_ValueOnValid)
{
    m_sut.emplace("foo");

    std::string& value = m_sut.valueOrEmplace("bar");

    EXPECT_TRUE(m_sut.isValid());
    EXPECT_EQ(value, "foo");
}

TEST_F(TestProperty, swap_OppositeValuesAfterSwapValid)
{
    m_sutLhs.emplace("foo");
    m_sutRhs.emplace("bar");

    m_sutLhs.swap(m_sutRhs);

    EXPECT_EQ(m_sutLhs.value(), "bar");
    EXPECT_EQ(m_sutRhs.value(), "foo");
}

TEST_F(TestProperty, swap_OppositeValuesAfterSwapInvalid)
{
    m_sutLhs.emplace("foo");
    m_sutLhs.swap(m_sutRhs);

    EXPECT_FALSE(m_sutLhs.isValid());
    EXPECT_TRUE(m_sutRhs.isValid());
    EXPECT_EQ(m_sutRhs.value(), "foo");
}

TEST_F(TestProperty, swap_OppositeValuesAfterInvalidSwap)
{
    m_sutRhs.emplace("bar");
    m_sutLhs.swap(m_sutRhs);

    EXPECT_FALSE(m_sutRhs.isValid());
    EXPECT_TRUE(m_sutLhs.isValid());
    EXPECT_EQ(m_sutLhs.value(), "bar");
}

// WORKAROUND
//
// The following tests do not build with MSVC versions 19.40 and latest
// (currently 14.42.34433), because syntactic constructs in the form of
// 'TestStruct{}.stringProperty' result in an 'internal compiler
// error'.
//
// This is most likely due to a compiler bug related to union classes,
// which are used for the storage of property values. An isolated
// version of the issue can be inspected in the Compiler Explorer (see
// [1]), which shows that everything works as intended for GCC and
// Clang, as well as MSVC versions up to 19.39.
//
// Note that we did not yet report the issue and it is unclear whether
// it is being worked on by the MSVC team.
//
// References:
//
// - [1] https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIM1ykrgAyeAyYAHI%2BAEaYxBIAbKQADqgKhE4MHt6%2B/oGp6Y4CoeFRLLHxXEl2mA6ZQgRMxATZPn4Btpj2RQwNTQQlkTFxibaNza25HQrjA2FD5SNVAJS2qF7EyOwc5gDMYcjeWADUJrtuM8RhwGfYJhoAgnsHR5in5wQAnsmYAPoExCYhAUt3uT0ezDYCmSTC2xwImBmYJMAHYrI9jpjjpcvA5jugfCxPv8wVjTmjSVjUQARM7o8EPMkIljJAwI95uL4/SFvY4AFVIx2iqE872p2II6BAIDwCn%2BVwAbngxLRiVgcXVovRfgqzm4%2BaDdncMViNQQJSQmMA/gRKZjUfSyWSvAxMnanQ73U6sQSWET/vjCZ86V7vfzjgqxF5MCGTd6abHGVSUbTdvT3czWUx2XquYxWG8DUb3WaLYDrSTzgLjlQxEpQXHyY6wy63Y2yZ722Hff7zT3g2nQ96%2BRGozHB137SnE0Oy1abRBlscGKhMKotslzSBA37gxSU02E49ZzNLRWCBA0AwZnPz%2BYEkuV2uN%2BbPdPJ7eF6fyzb7/fjqgBAIHEj6ruumCboe74MmGxwAH7fvO/yLsuYEvrOnZJrBmIAPQ4ccCCGOg9DoMca4IsQzC0KqGHQSeRA/iSZgJABPyAgxYrHJeAg3ohd7MccxAIAooHPhBr5ooJmAEBsDDHAAVEBsp0uStIfnxv4CagbHZiQnEQBpTEJP%2BQkiahYmQQ6UkyZRClKSCaaqe6CYTjBpoAri5oAGIis5FIfnhhm2ucYTBdgQpNImsFBXqMxSiAlzXLcEXEFFyapumdGPAqqB4KR/CoNETSLsi/lYccV43kwXhEP%2BBVFcQnEIjM0o%2Bagb7UgAdA1aWYhAOV5cs9WRa5NIcKstCcAArLwfgcFopCoJwbjWNY2LrJsbx7DwpAEJo42rAA1iAuy7J1p0XZdV1JJNHCSLN%2B2LZwvAKCAGi7ftqxwLASBoCydBxOQlB/ckAPxAqyDJMkOpcAAnL8uzwwAakIXAor88OqAk0g0LQFGvRA0SPdEYRNJ8nA7STzDEJ8ADy0TaLUe3cLwf1sIItMMKqj1YNEXjAG4KqvSzpBYCwhjAOI828PgxBM3gCqIo9a61DV2w7aFXSPbQeDRICNMeFgj0AngLAU7wivEMKSjUpg4tGDrRifXwBjAAoSN4JgADutPcubMiCCIYjsFIAfyEoaiPbogQGE7piWNY%2Bi669kCrNpPTC7wqCW1c6rwKsNR1M4ECuJMfiBCE8xlBUegFBkAhl7XaT1wwgzV0snTdPUsyN4Ehc9H0zRt8MlRjP0vdj0PVcjxIBcbVss/6NND3S09HDHFjkjHCwCgQxGcPnbDxwo2jnVHxAuCEHp23LLwzNaMsR0nWdV2vxdN2cPdpBzQtS0cC9b0PrSy%2BjARAIB1gEGSDVIG3F/r0GIBEAsnBVAAA4EgAFpsYVVjsAY4B8uCdQWpgfADE8p6H4IHUQ4hQ4UPDiodQq9o6kC9oCZI5sJrL2/o9P%2BtMapQPNKgKgG8sE7z3gqA%2BiNj6oxRGfLiHg4FxFOGYXYXBb5AIfk/U65036vyXndFev9nq2EAffA6eizAGKzkY0xj9SCW3SM4SQQA%3D%3D
#if !defined(_MSC_FULL_VER) || _MSC_FULL_VER < 194000000
TEST_F(TestProperty, CompareEqualityOfPropertyWithProperty)
{
    // invalid lhs, invalid rhs
    {
        const auto& invalidProperty1 = TestStruct{}.stringProperty;
        const auto& invalidProperty2 = TestStruct{}.stringProperty;

        EXPECT_TRUE(invalidProperty1 == invalidProperty2);
        EXPECT_FALSE(invalidProperty1 != invalidProperty2);
    }

    // valid lhs, invalid rhs
    {
        const auto& validProperty = TestStruct{ .stringProperty = "a" }.stringProperty;
        const auto& invalidProperty = TestStruct{}.stringProperty;

        EXPECT_FALSE(validProperty == invalidProperty);
        EXPECT_TRUE(validProperty != invalidProperty);
    }

    // invalid lhs, valid rhs
    {
        const auto& invalidProperty = TestStruct{}.stringProperty;
        const auto& validProperty = TestStruct{ .stringProperty = "a" }.stringProperty;

        EXPECT_FALSE(invalidProperty == validProperty);
        EXPECT_TRUE(invalidProperty != validProperty);
    }

    // valid lhs, valid rhs, unequal values
    {
        const auto& unequalProperty1 = TestStruct{ .stringProperty = "a" }.stringProperty;
        const auto& unequalProperty2 = TestStruct{ .stringProperty = "b" }.stringProperty;

        EXPECT_FALSE(unequalProperty1 == unequalProperty2);
        EXPECT_TRUE(unequalProperty1 != unequalProperty2);
    }

    // valid lhs, valid rhs, equal values
    {
        const auto& equalProperty1 = TestStruct{ .stringProperty = "a" }.stringProperty;
        const auto& equalProperty2 = TestStruct{ .stringProperty = "a" }.stringProperty;

        EXPECT_TRUE(equalProperty1 == equalProperty2);
        EXPECT_FALSE(equalProperty1 != equalProperty2);
    }
}

TEST_F(TestProperty, CompareEqualityOfPropertyWithValue)
{
    // invalid property
    {
        const auto& invalidProperty = TestStruct{}.stringProperty;
        dots::string_t value = "a";

        EXPECT_TRUE(invalidProperty != value);
        EXPECT_TRUE(value != invalidProperty);

        EXPECT_FALSE(invalidProperty == value);
        EXPECT_FALSE(value == invalidProperty);
    }

    // valid property, unequal values
    {
        const auto& unequalProperty = TestStruct{ .stringProperty = "a" }.stringProperty;
        dots::string_t unequalValue = "b";

        EXPECT_TRUE(unequalProperty != unequalValue);
        EXPECT_TRUE(unequalValue != unequalProperty);

        EXPECT_FALSE(unequalProperty == unequalValue);
        EXPECT_FALSE(unequalValue == unequalProperty);
    }

    // valid property, equal values
    {
        const auto& equalProperty = TestStruct{ .stringProperty = "a" }.stringProperty;
        dots::string_t equalValue = "a";

        EXPECT_TRUE(equalProperty == equalValue);
        EXPECT_TRUE(equalValue == equalProperty);

        EXPECT_FALSE(equalProperty != equalValue);
        EXPECT_FALSE(equalValue != equalProperty);
    }
}

TEST_F(TestProperty, CompareEqualityOfPropertyWithInvalid)
{
    // invalid property
    {
        const auto& invalidProperty = TestStruct{}.stringProperty;

        EXPECT_TRUE(invalidProperty == dots::invalid);
        EXPECT_TRUE(dots::invalid == invalidProperty);

        EXPECT_FALSE(invalidProperty != dots::invalid);
        EXPECT_FALSE(dots::invalid != invalidProperty);
    }

    // valid property
    {
        const auto& validProperty = TestStruct{ .stringProperty = "a" }.stringProperty;

        EXPECT_TRUE(validProperty != dots::invalid);
        EXPECT_TRUE(dots::invalid != validProperty);

        EXPECT_FALSE(validProperty == dots::invalid);
        EXPECT_FALSE(dots::invalid == validProperty);
    }
}

TEST_F(TestProperty, CompareOrderingOfPropertyWithProperty)
{
    // invalid lhs, invalid rhs
    {
        const auto& invalidProperty1 = TestStruct{}.stringProperty;
        const auto& invalidProperty2 = TestStruct{}.stringProperty;

        EXPECT_TRUE(invalidProperty1 <= invalidProperty2);
        EXPECT_TRUE(invalidProperty1 >= invalidProperty2);

        EXPECT_FALSE(invalidProperty1 < invalidProperty2);
        EXPECT_FALSE(invalidProperty1 > invalidProperty2);
    }

    // valid lhs, invalid rhs
    {
        const auto& validProperty = TestStruct{ .stringProperty = "a" }.stringProperty;
        const auto& invalidProperty = TestStruct{}.stringProperty;

        EXPECT_TRUE(validProperty > invalidProperty);
        EXPECT_TRUE(validProperty >= invalidProperty);

        EXPECT_FALSE(validProperty < invalidProperty);
        EXPECT_FALSE(validProperty <= invalidProperty);
    }

    // invalid lhs, valid rhs
    {
        const auto& invalidProperty = TestStruct{}.stringProperty;
        const auto& validProperty = TestStruct{ .stringProperty = "a" }.stringProperty;

        EXPECT_TRUE(invalidProperty < validProperty);
        EXPECT_TRUE(invalidProperty <= validProperty);

        EXPECT_FALSE(invalidProperty > validProperty);
        EXPECT_FALSE(invalidProperty >= validProperty);
    }

    // valid lhs, valid rhs, unequal values
    {
        const auto& lesserProperty = TestStruct{ .stringProperty = "a" }.stringProperty;
        const auto& greaterProperty = TestStruct{ .stringProperty = "b" }.stringProperty;

        EXPECT_TRUE(lesserProperty < greaterProperty);
        EXPECT_TRUE(lesserProperty <= greaterProperty);

        EXPECT_FALSE(lesserProperty > greaterProperty);
        EXPECT_FALSE(lesserProperty >= greaterProperty);
    }

    // valid lhs, valid rhs, unequal values, flipped
    {
        const auto& greaterProperty = TestStruct{ .stringProperty = "b" }.stringProperty;
        const auto& lesserProperty = TestStruct{ .stringProperty = "a" }.stringProperty;

        EXPECT_TRUE(greaterProperty > lesserProperty);
        EXPECT_TRUE(greaterProperty >= lesserProperty);

        EXPECT_FALSE(greaterProperty < lesserProperty);
        EXPECT_FALSE(greaterProperty <= lesserProperty);
    }

    // valid lhs, valid rhs, equal values
    {
        const auto& equalProperty1 = TestStruct{ .stringProperty = "a" }.stringProperty;
        const auto& equalProperty2 = TestStruct{ .stringProperty = "a" }.stringProperty;

        EXPECT_TRUE(equalProperty1 <= equalProperty2);
        EXPECT_TRUE(equalProperty1 >= equalProperty2);

        EXPECT_FALSE(equalProperty1 < equalProperty2);
        EXPECT_FALSE(equalProperty1 > equalProperty2);
    }
}

TEST_F(TestProperty, CompareOrderingOfPropertyWithValue)
{
    // invalid property
    {
        const auto& invalidProperty = TestStruct{}.stringProperty;
        dots::string_t value = "a";

        EXPECT_TRUE(invalidProperty < value);
        EXPECT_TRUE(invalidProperty <= value);
        EXPECT_TRUE(value > invalidProperty);
        EXPECT_TRUE(value >= invalidProperty);

        EXPECT_FALSE(value < invalidProperty);
        EXPECT_FALSE(value <= invalidProperty);
        EXPECT_FALSE(invalidProperty > value);
        EXPECT_FALSE(invalidProperty >= value);
    }

    // valid property, unequal values
    {
        const auto& lesserProperty = TestStruct{ .stringProperty = "a" }.stringProperty;
        dots::string_t greaterValue = "b";

        EXPECT_TRUE(lesserProperty < greaterValue);
        EXPECT_TRUE(lesserProperty <= greaterValue);
        EXPECT_TRUE(greaterValue > lesserProperty);
        EXPECT_TRUE(greaterValue >= lesserProperty);

        EXPECT_FALSE(greaterValue < lesserProperty);
        EXPECT_FALSE(greaterValue <= lesserProperty);
        EXPECT_FALSE(lesserProperty > greaterValue);
        EXPECT_FALSE(lesserProperty >= greaterValue);
    }

    // valid property, unequal values, flipped
    {
        const auto& greaterProperty = TestStruct{ .stringProperty = "b" }.stringProperty;
        dots::string_t lesserValue = "a";

        EXPECT_TRUE(lesserValue < greaterProperty);
        EXPECT_TRUE(lesserValue <= greaterProperty);
        EXPECT_TRUE(greaterProperty > lesserValue);
        EXPECT_TRUE(greaterProperty >= lesserValue);

        EXPECT_FALSE(greaterProperty < lesserValue);
        EXPECT_FALSE(greaterProperty <= lesserValue);
        EXPECT_FALSE(lesserValue > greaterProperty);
        EXPECT_FALSE(lesserValue >= greaterProperty);
    }

    // valid property, equal values
    {
        const auto& equalProperty = TestStruct{ .stringProperty = "a" }.stringProperty;
        dots::string_t equalValue = "a";

        EXPECT_TRUE(equalProperty <= equalValue);
        EXPECT_TRUE(equalValue <= equalProperty);
        EXPECT_TRUE(equalProperty >= equalValue);
        EXPECT_TRUE(equalValue >= equalProperty);

        EXPECT_FALSE(equalValue < equalProperty);
        EXPECT_FALSE(equalProperty < equalValue);
        EXPECT_FALSE(equalProperty > equalValue);
        EXPECT_FALSE(equalValue > equalProperty);
    }
}

TEST_F(TestProperty, CompareOrderingOfPropertyWithInvalid)
{
    // invalid property
    {
        const auto& invalidProperty = TestStruct{}.stringProperty;

        EXPECT_TRUE(invalidProperty <= dots::invalid);
        EXPECT_TRUE(dots::invalid <= invalidProperty);
        EXPECT_TRUE(invalidProperty >= dots::invalid);
        EXPECT_TRUE(dots::invalid >= invalidProperty);

        EXPECT_FALSE(invalidProperty < dots::invalid);
        EXPECT_FALSE(dots::invalid < invalidProperty);
        EXPECT_FALSE(invalidProperty > dots::invalid);
        EXPECT_FALSE(dots::invalid > invalidProperty);
    }

    // valid property
    {
        const auto& validProperty = TestStruct{ .stringProperty = "a" }.stringProperty;

        EXPECT_TRUE(dots::invalid < validProperty);
        EXPECT_TRUE(dots::invalid <= validProperty);
        EXPECT_TRUE(validProperty > dots::invalid);
        EXPECT_TRUE(validProperty >= dots::invalid);

        EXPECT_FALSE(validProperty < dots::invalid);
        EXPECT_FALSE(validProperty <= dots::invalid);
        EXPECT_FALSE(dots::invalid > validProperty);
        EXPECT_FALSE(dots::invalid >= validProperty);
    }
}
#endif
