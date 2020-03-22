#include <string>
#include <type_traits>
#include <gtest/gtest.h>
#include <dots/type/Descriptor.h>
#include <dots/type/StaticDescriptor.h>

struct Composite
{
	Composite() = default;
	Composite(int i, std::string s) : i(i), s(std::move(s)) {}
	Composite(const Composite& other) = default;
	Composite(Composite&& other) = default;
	~Composite() = default;

	Composite& operator = (const Composite& rhs) = default;
	Composite& operator = (Composite&& rhs) = default;

	bool operator == (const Composite& rhs) const
	{
		return i == rhs.i && s == rhs.s;
	}

	bool operator < (const Composite& rhs) const
	{
		return i < rhs.i || (i == rhs.i && s < rhs.s);
	}

	int i = 0;
	std::string s;
};

std::ostream& operator << (std::ostream& os, const Composite& composite)
{
	os << std::to_string(composite.i) + "," + composite.s;
	return os;
}

std::istream& operator >> (std::istream& is, Composite& composite)
{
	is >> composite.i;
	char c;
	is >> c;
	is >> composite.s;
	
	return is;
}

namespace dots::type
{
	template <>
	struct Descriptor<int> : StaticDescriptor<int>
	{
		Descriptor() : StaticDescriptor(Type::int32, "int32") {}
	};
	
	template <>
	struct Descriptor<std::string> : StaticDescriptor<std::string>
	{
		Descriptor() : StaticDescriptor(Type::string, "string") {}
	};

	template <>
	struct Descriptor<Composite> : StaticDescriptor<Composite>
	{
		Descriptor() : StaticDescriptor(Type::Struct, "Composite") {}
	};

    template <>
    struct Descriptor<uint8_t> : StaticDescriptor<uint8_t>
    {
        Descriptor() : StaticDescriptor(Type::uint8, "uint8") {}
    };
}

using namespace dots::type;

struct TestStaticDescriptor : ::testing::Test
{
protected:

	template <typename T>
	using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
	using int_storage_t = storage_t<int>;
	using string_storage_t = storage_t<std::string>;
	using composite_storage_t = storage_t<Composite>;

	Descriptor<int> m_sutInt;
	Descriptor<std::string> m_sutString;
	Descriptor<Composite> m_sutComposite;
    Descriptor<uint8_t> m_sutByte;
};

TEST_F(TestStaticDescriptor, size)
{
	EXPECT_EQ(m_sutInt.size(), sizeof(int));
	EXPECT_EQ(m_sutString.size(), sizeof(std::string));
	EXPECT_EQ(m_sutComposite.size(), sizeof(Composite));
}

TEST_F(TestStaticDescriptor, alignment)
{
	EXPECT_EQ(m_sutInt.alignment(), alignof(int));
	EXPECT_EQ(m_sutString.alignment(), alignof(std::string));
	EXPECT_EQ(m_sutComposite.alignment(), alignof(Composite));
}

TEST_F(TestStaticDescriptor, name)
{
	EXPECT_EQ(m_sutInt.name(), "int32");
	EXPECT_EQ(m_sutString.name(), "string");
	EXPECT_EQ(m_sutComposite.name(), "Composite");
}

TEST_F(TestStaticDescriptor, construct_default)
{
	int_storage_t i;
	string_storage_t s;
	composite_storage_t c;

	m_sutInt.construct(Typeless::From(i));
	m_sutString.construct(Typeless::From(s));
	m_sutComposite.construct(Typeless::From(c));
	
	EXPECT_EQ(reinterpret_cast<int&>(i), int{});
	EXPECT_EQ(reinterpret_cast<std::string&>(s), std::string{});
	EXPECT_EQ(reinterpret_cast<Composite&>(c), Composite{});
}

TEST_F(TestStaticDescriptor, construct_copy)
{
	int_storage_t i;
	string_storage_t s;
	composite_storage_t c;

	int iOther = 42;
	std::string sOther = "foo";
	Composite cOther{ 21, "bar" };

	m_sutInt.construct(Typeless::From(i), Typeless::From(iOther));
	m_sutString.construct(Typeless::From(s), Typeless::From(sOther));
	m_sutComposite.construct(Typeless::From(c), Typeless::From(cOther));
	
	EXPECT_EQ(reinterpret_cast<int&>(i), 42);
	EXPECT_EQ(reinterpret_cast<std::string&>(s), "foo");
	EXPECT_EQ(reinterpret_cast<Composite&>(c), Composite(21, "bar"));
}

TEST_F(TestStaticDescriptor, construct_move)
{
	int_storage_t i;
	string_storage_t s;
	composite_storage_t c;

	int iOther = 42;
	std::string sOther = "foo";
	Composite cOther{ 21, "bar" };

	m_sutInt.construct(Typeless::From(i), Typeless::From(iOther));
	m_sutString.construct(Typeless::From(s), Typeless::From(sOther));
	m_sutComposite.construct(Typeless::From(c), Typeless::From(cOther));
	
	EXPECT_EQ(reinterpret_cast<int&>(i), 42);
	EXPECT_EQ(reinterpret_cast<std::string&>(s), "foo");
	EXPECT_EQ(reinterpret_cast<Composite&>(c), Composite(21, "bar"));
}

TEST_F(TestStaticDescriptor, assign_copy)
{
	int i = 42;
	std::string s = "foo";
	Composite c{ 21, "bar" };

	int iRhs = 73;
	std::string sRhs = "baz";
	Composite cRhs{ 23, "qux" };

	m_sutInt.assign(Typeless::From(i), Typeless::From(iRhs));
	m_sutString.construct(Typeless::From(s), Typeless::From(sRhs));
	m_sutComposite.construct(Typeless::From(c), Typeless::From(cRhs));
	
	EXPECT_EQ(i, 73);
	EXPECT_EQ(s, "baz");
	EXPECT_EQ(c, Composite(23, "qux"));
}

TEST_F(TestStaticDescriptor, assign_move)
{
	int i = 42;
	std::string s = "foo";
	Composite c{ 21, "bar" };

	int iRhs = 73;
	std::string sRhs = "baz";
	Composite cRhs{ 23, "qux" };

	m_sutInt.assign(Typeless::From(i), Typeless::From(iRhs));
	m_sutString.construct(Typeless::From(s), Typeless::From(sRhs));
	m_sutComposite.construct(Typeless::From(c), Typeless::From(cRhs));
	
	EXPECT_EQ(i, 73);
	EXPECT_EQ(s, "baz");
	EXPECT_EQ(c, Composite(23, "qux"));
}

TEST_F(TestStaticDescriptor, swap)
{
	int iLhs = 42;
	std::string sLhs = "foo";
	Composite cLhs{ 21, "bar" };

	int iRhs = 73;
	std::string sRhs = "baz";
	Composite cRhs{ 23, "qux" };

	m_sutInt.swap(Typeless::From(iLhs), Typeless::From(iRhs));
	m_sutString.swap(Typeless::From(sLhs), Typeless::From(sRhs));
	m_sutComposite.swap(Typeless::From(cLhs), Typeless::From(cRhs));
	
	EXPECT_EQ(iLhs, 73);
	EXPECT_EQ(sLhs, "baz");
	EXPECT_EQ(cLhs, Composite(23, "qux"));

	EXPECT_EQ(iRhs, 42);
	EXPECT_EQ(sRhs, "foo");
	EXPECT_EQ(cRhs, Composite(21, "bar"));
}

TEST_F(TestStaticDescriptor, equal)
{
	int i1 = 42;
	int i2 = 42;
	int i3 = 21;
	
	std::string s1 = "foo";
	std::string s2 = "foo";
	std::string s3 = "bar";

	Composite c1{ 21, "bar" };
	Composite c2{ 21, "bar" };
	Composite c3{ 23, "qux" };
	
	EXPECT_TRUE(m_sutInt.equal(Typeless::From(i1), Typeless::From(i2)));
	EXPECT_TRUE(m_sutString.equal(Typeless::From(s1), Typeless::From(s2)));
	EXPECT_TRUE(m_sutComposite.equal(Typeless::From(c1), Typeless::From(c2)));
	
	EXPECT_FALSE(m_sutInt.equal(Typeless::From(i1), Typeless::From(i3)));
	EXPECT_FALSE(m_sutString.equal(Typeless::From(s1), Typeless::From(s3)));
	EXPECT_FALSE(m_sutComposite.equal(Typeless::From(c1), Typeless::From(c3)));
}

TEST_F(TestStaticDescriptor, less)
{
	EXPECT_TRUE(m_sutInt.less(21, 42));
	EXPECT_TRUE(m_sutString.less("bar", "foo"));
	EXPECT_TRUE(m_sutComposite.less(Composite{ 21, "bar" }, Composite{ 21, "foo" }));
	
	EXPECT_FALSE(m_sutInt.less(21, 21));
	EXPECT_FALSE(m_sutString.less("bar", "bar"));
	EXPECT_FALSE(m_sutComposite.less(Composite{ 21, "bar" }, Composite{ 21, "bar" }));

	EXPECT_FALSE(m_sutInt.less(42, 21));
	EXPECT_FALSE(m_sutString.less("foo", "bar"));
	EXPECT_FALSE(m_sutComposite.less(Composite{ 21, "foo" }, Composite{ 21, "bar" }));
}

TEST_F(TestStaticDescriptor, lessEqual)
{
	EXPECT_TRUE(m_sutInt.lessEqual(21, 42));
	EXPECT_TRUE(m_sutString.lessEqual("bar", "foo"));
	EXPECT_TRUE(m_sutComposite.lessEqual(Composite{ 21, "bar" }, Composite{ 21, "foo" }));
	
	EXPECT_TRUE(m_sutInt.lessEqual(21, 21));
	EXPECT_TRUE(m_sutString.lessEqual("bar", "bar"));
	EXPECT_TRUE(m_sutComposite.lessEqual(Composite{ 21, "bar" }, Composite{ 21, "bar" }));

	EXPECT_FALSE(m_sutInt.lessEqual(42, 21));
	EXPECT_FALSE(m_sutString.lessEqual("foo", "bar"));
	EXPECT_FALSE(m_sutComposite.lessEqual(Composite{ 21, "foo" }, Composite{ 21, "bar" }));
}

TEST_F(TestStaticDescriptor, greater)
{
	EXPECT_FALSE(m_sutInt.greater(21, 42));
	EXPECT_FALSE(m_sutString.greater("bar", "foo"));
	EXPECT_FALSE(m_sutComposite.greater(Composite{ 21, "bar" }, Composite{ 21, "foo" }));
	
	EXPECT_FALSE(m_sutInt.greater(21, 21));
	EXPECT_FALSE(m_sutString.greater("bar", "bar"));
	EXPECT_FALSE(m_sutComposite.greater(Composite{ 21, "bar" }, Composite{ 21, "bar" }));

	EXPECT_TRUE(m_sutInt.greater(42, 21));
	EXPECT_TRUE(m_sutString.greater("foo", "bar"));
	EXPECT_TRUE(m_sutComposite.greater(Composite{ 21, "foo" }, Composite{ 21, "bar" }));
}

TEST_F(TestStaticDescriptor, greaterEqual)
{
	EXPECT_FALSE(m_sutInt.greaterEqual(21, 42));
	EXPECT_FALSE(m_sutString.greaterEqual("bar", "foo"));
	EXPECT_FALSE(m_sutComposite.greaterEqual(Composite{ 21, "bar" }, Composite{ 21, "foo" }));
	
	EXPECT_TRUE(m_sutInt.greaterEqual(21, 21));
	EXPECT_TRUE(m_sutString.greaterEqual("bar", "bar"));
	EXPECT_TRUE(m_sutComposite.greaterEqual(Composite{ 21, "bar" }, Composite{ 21, "bar" }));

	EXPECT_TRUE(m_sutInt.greaterEqual(42, 21));
	EXPECT_TRUE(m_sutString.greaterEqual("foo", "bar"));
	EXPECT_TRUE(m_sutComposite.greaterEqual(Composite{ 21, "foo" }, Composite{ 21, "bar" }));
}

TEST_F(TestStaticDescriptor, toString)
{
	EXPECT_EQ(m_sutInt.toString(42), "42");
	EXPECT_EQ(m_sutString.toString("foo"), "foo");
	EXPECT_EQ(m_sutComposite.toString(Composite{ 21, "bar" }), "21,bar");
}

TEST_F(TestStaticDescriptor, fromString)
{
	int i;
	std::string s;
	Composite c;
	uint8_t b;

	m_sutInt.fromString(i, "73");
	m_sutString.fromString(s, "meow");
	m_sutComposite.fromString(c, "7,moo");
	m_sutByte.fromString(b, "1");

	EXPECT_EQ(i, 73);
	EXPECT_EQ(s, "meow");
	EXPECT_EQ(c, Composite(7, "moo"));
	EXPECT_EQ(b, 1);

	EXPECT_THROW(m_sutInt.fromString(i, "foo"), std::runtime_error);
	m_sutString.fromString(s, "");
	EXPECT_THROW(m_sutComposite.fromString(c, ",moo"), std::runtime_error);
}