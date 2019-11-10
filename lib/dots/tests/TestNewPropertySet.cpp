#include <gtest/gtest.h>
#include <dots/type/NewPropertySet.h>

using dots::type::NewPropertySet;

TEST(TestNewPropertySet, ctor)
{
	EXPECT_TRUE(NewPropertySet{}.empty());
	EXPECT_TRUE(NewPropertySet{ 0x00000000 }.empty());
	EXPECT_TRUE(NewPropertySet{ NewPropertySet::None }.empty());

	EXPECT_FALSE(NewPropertySet{ 0xFFFFFFFF }.empty());
	EXPECT_FALSE(NewPropertySet{ NewPropertySet::All }.empty());
}

TEST(TestNewPropertySet, count)
{
	EXPECT_EQ(NewPropertySet{ 0x00000000 }.count(), 0);	
	EXPECT_EQ(NewPropertySet{ 0xFFFFFFFF }.count(), 32);
	EXPECT_EQ(NewPropertySet{ 0x11FFF387 }.count(), 20);
	EXPECT_EQ(NewPropertySet{ 0x00408231 }.count(), 6);
}

TEST(TestNewPropertySet, operator_equal)
{
	EXPECT_EQ(NewPropertySet{ NewPropertySet::None }, NewPropertySet{ 0x00000000 });
	EXPECT_EQ(NewPropertySet{ NewPropertySet::All }, NewPropertySet{ 0xFFFFFFFF });

	EXPECT_EQ(NewPropertySet{ 0x00000000 }, NewPropertySet{ 0x00000000 });
	EXPECT_EQ(NewPropertySet{ 0xFFFFFFFF }, NewPropertySet{ 0xFFFFFFFF });
	EXPECT_EQ(NewPropertySet{ 0x12345678 }, NewPropertySet{ 0x12345678 });
	
	EXPECT_NE(NewPropertySet{ 0x00000000 }, NewPropertySet{ 0xFFFFFFFF });
	EXPECT_NE(NewPropertySet{ 0x12345678 }, NewPropertySet{ 0x87654321 });
}

TEST(TestNewPropertySet, operator_union)
{
	EXPECT_EQ(NewPropertySet{ 0x00000000 } + NewPropertySet{ 0x00000000 }, NewPropertySet{ 0x00000000 });
	EXPECT_EQ(NewPropertySet{ 0x00000000 } + NewPropertySet{ 0xFFFFFFFF }, NewPropertySet{ 0xFFFFFFFF });
	EXPECT_EQ(NewPropertySet{ 0xFFFFFFFF } + NewPropertySet{ 0x00000000 }, NewPropertySet{ 0xFFFFFFFF });
	EXPECT_EQ(NewPropertySet{ 0xFFFFFFFF } + NewPropertySet{ 0xFFFFFFFF }, NewPropertySet{ 0xFFFFFFFF });
	
	EXPECT_EQ(NewPropertySet{ 0x00FF00FF } + NewPropertySet{ 0x0000FF00 }, NewPropertySet{ 0x00FFFFFF });
	EXPECT_EQ(NewPropertySet{ 0x00FF00FF } + NewPropertySet{ 0x00FF0100 }, NewPropertySet{ 0x00FF01FF });
}

TEST(TestNewPropertySet, operator_difference)
{
	EXPECT_EQ(NewPropertySet{ 0x00000000 } - NewPropertySet{ 0x00000000 }, NewPropertySet{ 0x00000000 });
	EXPECT_EQ(NewPropertySet{ 0x00000000 } - NewPropertySet{ 0xFFFFFFFF }, NewPropertySet{ 0x00000000 });
	EXPECT_EQ(NewPropertySet{ 0xFFFFFFFF } - NewPropertySet{ 0xFFFFFFFF }, NewPropertySet{ 0x00000000 });
	EXPECT_EQ(NewPropertySet{ 0xFFFFFFFF } - NewPropertySet{ 0x00000000 }, NewPropertySet{ 0xFFFFFFFF });
	
	EXPECT_EQ(NewPropertySet{ 0x00FF00FF } - NewPropertySet{ 0x000000FF }, NewPropertySet{ 0x00FF0000 });
	EXPECT_EQ(NewPropertySet{ 0x00FF00FF } - NewPropertySet{ 0xFFFF0000 }, NewPropertySet{ 0x000000FF });
}

TEST(TestNewPropertySet, operator_intersection)
{
	EXPECT_EQ(NewPropertySet{ 0x00000000 } ^ NewPropertySet{ 0x00000000 }, NewPropertySet{ 0x00000000 });
	EXPECT_EQ(NewPropertySet{ 0x00000000 } ^ NewPropertySet{ 0xFFFFFFFF }, NewPropertySet{ 0x00000000 });
	EXPECT_EQ(NewPropertySet{ 0xFFFFFFFF } ^ NewPropertySet{ 0xFFFFFFFF }, NewPropertySet{ 0xFFFFFFFF });
	EXPECT_EQ(NewPropertySet{ 0xFFFFFFFF } ^ NewPropertySet{ 0x00000000 }, NewPropertySet{ 0x00000000 });
	
	EXPECT_EQ(NewPropertySet{ 0x00FF00FF } ^ NewPropertySet{ 0x000000FF }, NewPropertySet{ 0x000000FF });
	EXPECT_EQ(NewPropertySet{ 0x00FF00FF } ^ NewPropertySet{ 0xFFFF0000 }, NewPropertySet{ 0x00FF0000 });
}

TEST(TestNewPropertySet, operator_subset)
{
	EXPECT_TRUE(NewPropertySet{ 0x00000000 } <= NewPropertySet{ 0x00000000 });
	EXPECT_TRUE(NewPropertySet{ 0x00000000 } <= NewPropertySet{ 0xFFFFFFFF });
	EXPECT_TRUE(NewPropertySet{ 0xFFFFFFFF } <= NewPropertySet{ 0xFFFFFFFF });
	EXPECT_FALSE(NewPropertySet{ 0xFFFFFFFF } <= NewPropertySet{ 0x00000000 });
	
	EXPECT_TRUE(NewPropertySet{ 0x000000FF } <= NewPropertySet{ 0x00FF00FF });
	EXPECT_FALSE(NewPropertySet{ 0x00FF00FF } <= NewPropertySet{ 0x00FF00F0 });
}

TEST(TestNewPropertySet, operator_strict_subset)
{
	EXPECT_FALSE(NewPropertySet{ 0x00000000 } < NewPropertySet{ 0x00000000 });
	EXPECT_TRUE(NewPropertySet{ 0x00000000 } < NewPropertySet{ 0xFFFFFFFF });
	EXPECT_FALSE(NewPropertySet{ 0xFFFFFFFF } < NewPropertySet{ 0xFFFFFFFF });
	EXPECT_FALSE(NewPropertySet{ 0xFFFFFFFF } < NewPropertySet{ 0x00000000 });
	
	EXPECT_TRUE(NewPropertySet{ 0x000000FF } < NewPropertySet{ 0x00FF00FF });
	EXPECT_FALSE(NewPropertySet{ 0x00FF00FF } < NewPropertySet{ 0x00FF00F0 });
}

TEST(TestNewPropertySet, operator_superset)
{
	EXPECT_TRUE(NewPropertySet{ 0x00000000 } >= NewPropertySet{ 0x00000000 });
	EXPECT_FALSE(NewPropertySet{ 0x00000000 } >= NewPropertySet{ 0xFFFFFFFF });
	EXPECT_TRUE(NewPropertySet{ 0xFFFFFFFF } >= NewPropertySet{ 0xFFFFFFFF });
	EXPECT_TRUE(NewPropertySet{ 0xFFFFFFFF } >= NewPropertySet{ 0x00000000 });
	
	EXPECT_FALSE(NewPropertySet{ 0x000000FF } >= NewPropertySet{ 0x00FF00FF });
	EXPECT_TRUE(NewPropertySet{ 0x00FF00FF } >= NewPropertySet{ 0x00FF00F0 });
}

TEST(TestNewPropertySet, operator_strict_superset)
{
	EXPECT_FALSE(NewPropertySet{ 0x00000000 } > NewPropertySet{ 0x00000000 });
	EXPECT_FALSE(NewPropertySet{ 0x00000000 } > NewPropertySet{ 0xFFFFFFFF });
	EXPECT_FALSE(NewPropertySet{ 0xFFFFFFFF } > NewPropertySet{ 0xFFFFFFFF });
	EXPECT_TRUE(NewPropertySet{ 0xFFFFFFFF } > NewPropertySet{ 0x00000000 });
	
	EXPECT_FALSE(NewPropertySet{ 0x000000FF } > NewPropertySet{ 0x00FF00FF });
	EXPECT_TRUE(NewPropertySet{ 0x00FF00FF } > NewPropertySet{ 0x00FF00F0 });
}

TEST(TestNewPropertySet, toString)
{
	EXPECT_EQ(NewPropertySet{ 0b00000000000000000000000000000000 }.toString(), "00000000000000000000000000000000");
	EXPECT_EQ(NewPropertySet{ 0b11111111111111111111111111111111 }.toString(), "11111111111111111111111111111111");
	EXPECT_EQ(NewPropertySet{ 0b10101010101010101010101010101010 }.toString(), "10101010101010101010101010101010");
	EXPECT_EQ(NewPropertySet{ 0b10101010101010101010101010101010 }.toString('B', 'A'), "ABABABABABABABABABABABABABABABAB");
}