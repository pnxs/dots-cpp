// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <string>
#include <type_traits>
#include <dots/testing/gtest/gtest.h>
#include <dots/type/Descriptor.h>
#include <dots/type/StaticDescriptor.h>

using namespace dots::type;

struct TestStaticDescriptor : ::testing::Test
{
protected:

    template <typename T>
    using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
    using int_storage_t = storage_t<int>;
    using string_storage_t = storage_t<std::string>;

    std::shared_ptr<Descriptor<int>> m_sutInt = make_descriptor<Descriptor<int>>();
    std::shared_ptr<Descriptor<std::string>> m_sutString = make_descriptor<Descriptor<std::string>>();
    std::shared_ptr<Descriptor<uint8_t>> m_sutByte = make_descriptor<Descriptor<uint8_t>>();
};

TEST_F(TestStaticDescriptor, size)
{
    EXPECT_EQ(m_sutInt->size(), sizeof(int));
    EXPECT_EQ(m_sutString->size(), sizeof(std::string));
}

TEST_F(TestStaticDescriptor, alignment)
{
    EXPECT_EQ(m_sutInt->alignment(), alignof(int));
    EXPECT_EQ(m_sutString->alignment(), alignof(std::string));
}

TEST_F(TestStaticDescriptor, name)
{
    EXPECT_EQ(m_sutInt->name(), "int32");
    EXPECT_EQ(m_sutString->name(), "string");
}

TEST_F(TestStaticDescriptor, construct_default)
{
    int_storage_t i;
    string_storage_t s;

    m_sutInt->construct(Typeless::From(i));
    m_sutString->construct(Typeless::From(s));

    EXPECT_EQ(reinterpret_cast<int&>(i), int{});
    EXPECT_EQ(reinterpret_cast<std::string&>(s), std::string{});
}

TEST_F(TestStaticDescriptor, construct_copy)
{
    int_storage_t i;
    string_storage_t s;

    int iOther = 42;
    std::string sOther = "foo";

    m_sutInt->construct(Typeless::From(i), Typeless::From(iOther));
    m_sutString->construct(Typeless::From(s), Typeless::From(sOther));

    EXPECT_EQ(reinterpret_cast<int&>(i), 42);
    EXPECT_EQ(reinterpret_cast<std::string&>(s), "foo");
}

TEST_F(TestStaticDescriptor, construct_move)
{
    int_storage_t i;
    string_storage_t s;

    int iOther = 42;
    std::string sOther = "foo";

    m_sutInt->construct(Typeless::From(i), Typeless::From(iOther));
    m_sutString->construct(Typeless::From(s), Typeless::From(sOther));

    EXPECT_EQ(reinterpret_cast<int&>(i), 42);
    EXPECT_EQ(reinterpret_cast<std::string&>(s), "foo");
}

TEST_F(TestStaticDescriptor, assign_copy)
{
    int i = 42;
    std::string s = "foo";

    int iRhs = 73;
    std::string sRhs = "baz";

    m_sutInt->assign(Typeless::From(i), Typeless::From(iRhs));
    m_sutString->construct(Typeless::From(s), Typeless::From(sRhs));

    EXPECT_EQ(i, 73);
    EXPECT_EQ(s, "baz");
}

TEST_F(TestStaticDescriptor, assign_move)
{
    int i = 42;
    std::string s = "foo";

    int iRhs = 73;
    std::string sRhs = "baz";

    m_sutInt->assign(Typeless::From(i), Typeless::From(iRhs));
    m_sutString->construct(Typeless::From(s), Typeless::From(sRhs));

    EXPECT_EQ(i, 73);
    EXPECT_EQ(s, "baz");
}

TEST_F(TestStaticDescriptor, swap)
{
    int iLhs = 42;
    std::string sLhs = "foo";

    int iRhs = 73;
    std::string sRhs = "baz";

    m_sutInt->swap(Typeless::From(iLhs), Typeless::From(iRhs));
    m_sutString->swap(Typeless::From(sLhs), Typeless::From(sRhs));

    EXPECT_EQ(iLhs, 73);
    EXPECT_EQ(sLhs, "baz");

    EXPECT_EQ(iRhs, 42);
    EXPECT_EQ(sRhs, "foo");
}

TEST_F(TestStaticDescriptor, equal)
{
    int i1 = 42;
    int i2 = 42;
    int i3 = 21;

    std::string s1 = "foo";
    std::string s2 = "foo";
    std::string s3 = "bar";

    EXPECT_TRUE(m_sutInt->equal(Typeless::From(i1), Typeless::From(i2)));
    EXPECT_TRUE(m_sutString->equal(Typeless::From(s1), Typeless::From(s2)));

    EXPECT_FALSE(m_sutInt->equal(Typeless::From(i1), Typeless::From(i3)));
    EXPECT_FALSE(m_sutString->equal(Typeless::From(s1), Typeless::From(s3)));
}

TEST_F(TestStaticDescriptor, less)
{
    EXPECT_TRUE(m_sutInt->less<int>(21, 42));
    EXPECT_TRUE(m_sutString->less<std::string>("bar", "foo"));

    EXPECT_FALSE(m_sutInt->less<int>(21, 21));
    EXPECT_FALSE(m_sutString->less<std::string>("bar", "bar"));

    EXPECT_FALSE(m_sutInt->less<int>(42, 21));
    EXPECT_FALSE(m_sutString->less<std::string>("foo", "bar"));
}

TEST_F(TestStaticDescriptor, lessEqual)
{
    EXPECT_TRUE(m_sutInt->lessEqual<int>(21, 42));
    EXPECT_TRUE(m_sutString->lessEqual<std::string>("bar", "foo"));

    EXPECT_TRUE(m_sutInt->lessEqual<int>(21, 21));
    EXPECT_TRUE(m_sutString->lessEqual<std::string>("bar", "bar"));

    EXPECT_FALSE(m_sutInt->lessEqual<int>(42, 21));
    EXPECT_FALSE(m_sutString->lessEqual<std::string>("foo", "bar"));
}

TEST_F(TestStaticDescriptor, greater)
{
    EXPECT_FALSE(m_sutInt->greater<int>(21, 42));
    EXPECT_FALSE(m_sutString->greater<std::string>("bar", "foo"));

    EXPECT_FALSE(m_sutInt->greater<int>(21, 21));
    EXPECT_FALSE(m_sutString->greater<std::string>("bar", "bar"));

    EXPECT_TRUE(m_sutInt->greater<int>(42, 21));
    EXPECT_TRUE(m_sutString->greater<std::string>("foo", "bar"));
}

TEST_F(TestStaticDescriptor, greaterEqual)
{
    EXPECT_FALSE(m_sutInt->greaterEqual<int>(21, 42));
    EXPECT_FALSE(m_sutString->greaterEqual<std::string>("bar", "foo"));

    EXPECT_TRUE(m_sutInt->greaterEqual<int>(21, 21));
    EXPECT_TRUE(m_sutString->greaterEqual<std::string>("bar", "bar"));

    EXPECT_TRUE(m_sutInt->greaterEqual<int>(42, 21));
    EXPECT_TRUE(m_sutString->greaterEqual<std::string>("foo", "bar"));
}
