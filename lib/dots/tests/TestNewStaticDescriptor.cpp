#include <string>
#include <type_traits>
#include <gtest/gtest.h>
#include <dots/type/NewDescriptor.h>
#include <dots/type/NewStaticDescriptor.h>

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

	int i;
	std::string s;
};

namespace dots::type
{
	template <>
	struct NewDescriptor<int> : NewStaticDescriptor<int>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::int32, "int32") {}
	};
	
	template <>
	struct NewDescriptor<std::string> : NewStaticDescriptor<std::string>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::string, "string") {}
	};

	template <>
	struct NewDescriptor<Composite> : NewStaticDescriptor<Composite>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::Struct, "Composite") {}
	};
}

using namespace dots::type;

struct TestNewStaticDescriptor : ::testing::Test
{
protected:

	template <typename T>
	using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
	using int_storage_t = storage_t<int>;
	using string_storage_t = storage_t<std::string>;
	using composite_storage_t = storage_t<Composite>;

	NewDescriptor<int> m_sutInt;
	NewDescriptor<std::string> m_sutString;
	NewDescriptor<Composite> m_sutComposite;
};

TEST_F(TestNewStaticDescriptor, size)
{
	EXPECT_EQ(m_sutInt.size(), sizeof(int));
	EXPECT_EQ(m_sutString.size(), sizeof(std::string));
	EXPECT_EQ(m_sutComposite.size(), sizeof(Composite));
}

TEST_F(TestNewStaticDescriptor, alignment)
{
	EXPECT_EQ(m_sutInt.alignment(), alignof(int));
	EXPECT_EQ(m_sutString.alignment(), alignof(std::string));
	EXPECT_EQ(m_sutComposite.alignment(), alignof(Composite));
}

TEST_F(TestNewStaticDescriptor, name)
{
	EXPECT_EQ(m_sutInt.name(), "int32");
	EXPECT_EQ(m_sutString.name(), "string");
	EXPECT_EQ(m_sutComposite.name(), "Composite");
}

TEST_F(TestNewStaticDescriptor, construct_default)
{
	int_storage_t i;
	string_storage_t s;
	composite_storage_t c;

	m_sutInt.construct(NewTypeless::From(i));
	m_sutString.construct(NewTypeless::From(s));
	m_sutComposite.construct(NewTypeless::From(c));
	
	EXPECT_EQ(reinterpret_cast<int&>(i), int{});
	EXPECT_EQ(reinterpret_cast<std::string&>(s), std::string{});
	EXPECT_EQ(reinterpret_cast<Composite&>(c), Composite{});
}

TEST_F(TestNewStaticDescriptor, construct_copy)
{
	int_storage_t i;
	string_storage_t s;
	composite_storage_t c;

	int iOther = 42;
	std::string sOther = "foo";
	Composite cOther{ 21, "bar" };

	m_sutInt.construct(NewTypeless::From(i), NewTypeless::From(iOther));
	m_sutString.construct(NewTypeless::From(s), NewTypeless::From(sOther));
	m_sutComposite.construct(NewTypeless::From(c), NewTypeless::From(cOther));
	
	EXPECT_EQ(reinterpret_cast<int&>(i), 42);
	EXPECT_EQ(reinterpret_cast<std::string&>(s), "foo");
	EXPECT_EQ(reinterpret_cast<Composite&>(c), Composite(21, "bar"));
}

TEST_F(TestNewStaticDescriptor, construct_move)
{
	int_storage_t i;
	string_storage_t s;
	composite_storage_t c;

	int iOther = 42;
	std::string sOther = "foo";
	Composite cOther{ 21, "bar" };

	m_sutInt.construct(NewTypeless::From(i), NewTypeless::From(iOther));
	m_sutString.construct(NewTypeless::From(s), NewTypeless::From(sOther));
	m_sutComposite.construct(NewTypeless::From(c), NewTypeless::From(cOther));
	
	EXPECT_EQ(reinterpret_cast<int&>(i), 42);
	EXPECT_EQ(reinterpret_cast<std::string&>(s), "foo");
	EXPECT_EQ(reinterpret_cast<Composite&>(c), Composite(21, "bar"));
}

TEST_F(TestNewStaticDescriptor, assign_copy)
{
	int i = 42;
	std::string s = "foo";
	Composite c{ 21, "bar" };

	int iRhs = 73;
	std::string sRhs = "baz";
	Composite cRhs{ 23, "qux" };

	m_sutInt.assign(NewTypeless::From(i), NewTypeless::From(iRhs));
	m_sutString.construct(NewTypeless::From(s), NewTypeless::From(sRhs));
	m_sutComposite.construct(NewTypeless::From(c), NewTypeless::From(cRhs));
	
	EXPECT_EQ(i, 73);
	EXPECT_EQ(s, "baz");
	EXPECT_EQ(c, Composite(23, "qux"));
}

TEST_F(TestNewStaticDescriptor, assign_move)
{
	int i = 42;
	std::string s = "foo";
	Composite c{ 21, "bar" };

	int iRhs = 73;
	std::string sRhs = "baz";
	Composite cRhs{ 23, "qux" };

	m_sutInt.assign(NewTypeless::From(i), NewTypeless::From(iRhs));
	m_sutString.construct(NewTypeless::From(s), NewTypeless::From(sRhs));
	m_sutComposite.construct(NewTypeless::From(c), NewTypeless::From(cRhs));
	
	EXPECT_EQ(i, 73);
	EXPECT_EQ(s, "baz");
	EXPECT_EQ(c, Composite(23, "qux"));
}

TEST_F(TestNewStaticDescriptor, swap)
{
	int iLhs = 42;
	std::string sLhs = "foo";
	Composite cLhs{ 21, "bar" };

	int iRhs = 73;
	std::string sRhs = "baz";
	Composite cRhs{ 23, "qux" };

	m_sutInt.swap(NewTypeless::From(iLhs), NewTypeless::From(iRhs));
	m_sutString.swap(NewTypeless::From(sLhs), NewTypeless::From(sRhs));
	m_sutComposite.swap(NewTypeless::From(cLhs), NewTypeless::From(cRhs));
	
	EXPECT_EQ(iLhs, 73);
	EXPECT_EQ(sLhs, "baz");
	EXPECT_EQ(cLhs, Composite(23, "qux"));

	EXPECT_EQ(iRhs, 42);
	EXPECT_EQ(sRhs, "foo");
	EXPECT_EQ(cRhs, Composite(21, "bar"));
}

TEST_F(TestNewStaticDescriptor, equal)
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
	
	EXPECT_TRUE(m_sutInt.equal(NewTypeless::From(i1), NewTypeless::From(i2)));
	EXPECT_TRUE(m_sutString.equal(NewTypeless::From(s1), NewTypeless::From(s2)));
	EXPECT_TRUE(m_sutComposite.equal(NewTypeless::From(c1), NewTypeless::From(c2)));
	
	EXPECT_FALSE(m_sutInt.equal(NewTypeless::From(i1), NewTypeless::From(i3)));
	EXPECT_FALSE(m_sutString.equal(NewTypeless::From(s1), NewTypeless::From(s3)));
	EXPECT_FALSE(m_sutComposite.equal(NewTypeless::From(c1), NewTypeless::From(c3)));
}

TEST_F(TestNewStaticDescriptor, less)
{
	int i1 = 21;
	int i2 = 42;
	int i3 = 21;
	
	std::string s1 = "bar";
	std::string s2 = "foo";
	std::string s3 = "bar";

	Composite c1{ 21, "bar" };
	Composite c2{ 21, "qux" };
	Composite c3{ 21, "bar" };
	
	EXPECT_TRUE(m_sutInt.less(NewTypeless::From(i1), NewTypeless::From(i2)));
	EXPECT_TRUE(m_sutString.less(NewTypeless::From(s1), NewTypeless::From(s2)));
	EXPECT_TRUE(m_sutComposite.less(NewTypeless::From(c1), NewTypeless::From(c2)));
	
	EXPECT_FALSE(m_sutInt.less(NewTypeless::From(i1), NewTypeless::From(i3)));
	EXPECT_FALSE(m_sutString.less(NewTypeless::From(s1), NewTypeless::From(s3)));
	EXPECT_FALSE(m_sutComposite.less(NewTypeless::From(c1), NewTypeless::From(c3)));
}