#include "dots/type/EnumDescriptor.h"
#include "dots/type/Registry.h"
#include "DotsTestEnum.dots.h"
#include <gtest/gtest.h>


//using namespace rttr;
using namespace dots::type;

TEST(TestEnumDescriptor, construct)
{
#if 0
    auto t = type::get<DotsTestEnum>();

    ASSERT_TRUE(t.is_valid());
    ASSERT_TRUE(t.is_enumeration());

    {
        auto e = t.get_enumeration();
        EXPECT_EQ(e.get_names().size(), 5);
        //std::cout << "Enum size: " << t.get_sizeof() << "\n";
        //std::cout << "Enum align: " << t.get_alignof() << "\n";
    }
#endif

#if 0

    auto ed = Registry::toEnumDescriptorData(t);

    EXPECT_TRUE(ed.hasName());
    EXPECT_TRUE(ed.hasElements());

    ASSERT_FALSE(type::get_by_name("DotsTestEnumA").is_valid());
    ed.setName("DotsTestEnumA");

    auto newEnum = EnumDescriptor::createFromEnumDescriptorData(ed);
    EXPECT_TRUE(newEnum != nullptr);

    EXPECT_TRUE(newEnum->dotsType() == dots::type::DotsType::Enum);

    EXPECT_EQ(newEnum->sizeOf(), 4); //TODO: 4 is not valid on all platforms
    EXPECT_EQ(newEnum->alignOf(), 4);
#endif

    // TODO: rework without rttr
#if 0
    {
        auto e = nt.get_enumeration();
        EXPECT_EQ(e.get_names().size(), 5);

        EXPECT_EQ(e.value_to_name(0), "value1");
        EXPECT_EQ(e.value_to_name(1), "value2");
        EXPECT_EQ(e.value_to_name(2), "value3");
        EXPECT_EQ(e.value_to_name(3), "value4");
        EXPECT_EQ(e.value_to_name(4), "value5");

        EXPECT_EQ(e.name_to_value("value1"), 0);
        EXPECT_EQ(e.name_to_value("value2"), 1);
        EXPECT_EQ(e.name_to_value("value3"), 2);
        EXPECT_EQ(e.name_to_value("value4"), 3);
        EXPECT_EQ(e.name_to_value("value5"), 4);
    }


    auto enumInstance = newEnum->New();

    newEnum->from_string(enumInstance, "value4");

    EXPECT_EQ(newEnum->to_int(enumInstance), 3);

    newEnum->Delete(enumInstance);
    //Descriptor::registry().clear();

    //
    // Test if toEnumDescriptorData can read the previously dynamically
    // registered type with metadata
    //
    auto edA = Registry::toEnumDescriptorData(nt);

    EXPECT_EQ(edA.name(), "DotsTestEnumA");
    ASSERT_TRUE(edA.hasElements());
    ASSERT_EQ(edA.elements().size(), 5);

    EXPECT_EQ(edA.elements().at(0).name(), "value1");
    EXPECT_EQ(edA.elements().at(0).enum_value(), 0);
    EXPECT_EQ(edA.elements().at(0).tag(), 1);

    EXPECT_EQ(edA.elements().at(1).name(), "value2");
    EXPECT_EQ(edA.elements().at(1).enum_value(), 1);
    EXPECT_EQ(edA.elements().at(1).tag(), 2);

    EXPECT_EQ(edA.elements().at(2).name(), "value3");
    EXPECT_EQ(edA.elements().at(2).enum_value(), 2);
    EXPECT_EQ(edA.elements().at(2).tag(), 3);

    EXPECT_EQ(edA.elements().at(3).name(), "value4");
    EXPECT_EQ(edA.elements().at(3).enum_value(), 3);
    EXPECT_EQ(edA.elements().at(3).tag(), 5);

    EXPECT_EQ(edA.elements().at(4).name(), "value5");
    EXPECT_EQ(edA.elements().at(4).enum_value(), 4);
    EXPECT_EQ(edA.elements().at(4).tag(), 19);
#endif
}

#if 0
TEST(TestEnumDescriptor, testRttrEnum)
{
    auto t = type::get_by_name("DotsStructScope");
    ASSERT_TRUE(t.is_valid());
    ASSERT_TRUE(t.is_enumeration());

    auto enumeration = t.get_enumeration();

    EXPECT_EQ(enumeration.get_values().size(), 4);

}
#endif