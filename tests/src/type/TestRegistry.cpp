#include <dots/testing/gtest/gtest.h>
#include <dots/type/Registry.h>
#include <DotsHeader.dots.h>
#include <DotsTestStruct.dots.h>

struct TestRegistry : ::testing::Test
{
};

#define EXPECT_TYPE_IN_REGISTRY(typeName_) [&] \
{ \
    EXPECT_TRUE(sut.hasType(typeName_)); \
    EXPECT_NE(sut.findType(typeName_), nullptr); \
    EXPECT_NO_THROW(sut.getType(typeName_)); \
}();

#define EXPECT_TYPE_NOT_IN_REGISTRY(typeName_) [&] \
{ \
    EXPECT_FALSE(sut.hasType(typeName_)); \
    EXPECT_EQ(sut.findType(typeName_), nullptr); \
    EXPECT_THROW(sut.getType(typeName_), std::logic_error); \
}();

#define EXPECT_STRUCT_TYPE_IN_REGISTRY(typeName_) [&] \
{ \
    EXPECT_TYPE_IN_REGISTRY(typeName_); \
    EXPECT_NE(sut.findStructType(typeName_), nullptr); \
    EXPECT_NO_THROW(sut.getStructType(typeName_)); \
    EXPECT_EQ(sut.findEnumType(typeName_), nullptr); \
    EXPECT_THROW(sut.getEnumType(typeName_), std::logic_error); \
}();

#define EXPECT_STRUCT_TYPE_NOT_IN_REGISTRY(typeName_) [&] \
{ \
    EXPECT_TYPE_NOT_IN_REGISTRY(typeName_); \
    EXPECT_EQ(sut.findStructType(typeName_), nullptr); \
    EXPECT_THROW(sut.getStructType(typeName_), std::logic_error); \
}();

TEST_F(TestRegistry, ctor_UserTypesWhenStaticTypesAreEnabled)
{
    dots::type::Registry sut;
    EXPECT_STRUCT_TYPE_IN_REGISTRY(DotsHeader::_Descriptor().name());
    EXPECT_STRUCT_TYPE_IN_REGISTRY(DotsTestStruct::_Descriptor().name());
    EXPECT_STRUCT_TYPE_NOT_IN_REGISTRY("Foobar");
}

TEST_F(TestRegistry, ctor_NoUserTypesWhenStaticTypesAreDisabled)
{
    dots::type::Registry sut{ nullptr, dots::type::Registry::StaticTypePolicy::None };
    EXPECT_STRUCT_TYPE_NOT_IN_REGISTRY(DotsHeader::_Descriptor().name());
    EXPECT_STRUCT_TYPE_NOT_IN_REGISTRY(DotsTestStruct::_Descriptor().name());
    EXPECT_STRUCT_TYPE_NOT_IN_REGISTRY("Foobar");

    EXPECT_EQ(sut.size(), 34); // 17 fundamental types (*2 due to vector<T>)
}

TEST_F(TestRegistry, registerType)
{
    auto& descriptor = dots::type::Descriptor<DotsTestStruct>::Instance();
    ::testing::MockFunction<void(const dots::type::Descriptor<>&)> mockNewTypeHandler;
    dots::type::Registry sut{ mockNewTypeHandler.AsStdFunction(), dots::type::Registry::StaticTypePolicy::None };

    EXPECT_STRUCT_TYPE_NOT_IN_REGISTRY(descriptor.name());
    EXPECT_CALL(mockNewTypeHandler, Call(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(mockNewTypeHandler, Call(::testing::Ref(descriptor))).Times(1);

    auto& registeredDescriptor = sut.registerType(descriptor);
    EXPECT_THROW(sut.registerType(dots::type::Descriptor<DotsTestSubStruct>::Instance()), std::logic_error);

    EXPECT_EQ(&registeredDescriptor, &descriptor);
    EXPECT_STRUCT_TYPE_IN_REGISTRY(descriptor.name());
}

TEST_F(TestRegistry, deregisterType)
{
    auto& descriptor = dots::type::Descriptor<DotsTestStruct>::Instance();
    dots::type::Registry sut{ nullptr, dots::type::Registry::StaticTypePolicy::None };

    sut.registerType(descriptor);
    EXPECT_STRUCT_TYPE_IN_REGISTRY(descriptor.name());

    EXPECT_THROW(sut.deregisterType("Foobar"), std::logic_error);
    sut.deregisterType(descriptor);
    EXPECT_STRUCT_TYPE_NOT_IN_REGISTRY(DotsTestStruct::_Descriptor().name());
}

#if 0
TEST_F(TestRegistry, deregisterStaticType)
{
    auto& descriptor = DotsHeader::_Descriptor();
    dots::type::Registry sut;

    EXPECT_STRUCT_TYPE_IN_REGISTRY(descriptor.name());
    EXPECT_TRUE(sut.hasType(descriptor.name()));

    EXPECT_THROW(sut.deregisterType("Foobar"), std::logic_error);
    sut.deregisterType(descriptor);
    EXPECT_STRUCT_TYPE_NOT_IN_REGISTRY(descriptor.name());
    EXPECT_FALSE(sut.hasType(descriptor.name()));
}
#endif

TEST_F(TestRegistry, forEach)
{
    using namespace dots::type;
    
    Registry sut{ nullptr, dots::type::Registry::StaticTypePolicy::None };
    sut.registerType(Descriptor<DotsTestStruct>::Instance());
    sut.registerType(Descriptor<dots::vector_t<DotsTestStruct>>::Instance());

    {
        ::testing::MockFunction<void(const Descriptor<>&)> mockTypeHandler;
        EXPECT_CALL(mockTypeHandler, Call(::testing::_)).Times(::testing::AnyNumber());
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsHeader>::Instance()))).Times(0);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsTestSubStruct>::Instance()))).Times(1);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsTestEnum>::Instance()))).Times(1);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsTestStruct>::Instance()))).Times(1);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<dots::vector_t<DotsTestStruct>>::Instance()))).Times(1);

        sut.forEach(mockTypeHandler.AsStdFunction());
    }

    {
        ::testing::MockFunction<void(const StructDescriptor<>&)> mockTypeHandler;
        EXPECT_CALL(mockTypeHandler, Call(::testing::Property(&Descriptor<>::type, Type::Struct))).Times(::testing::AnyNumber());
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsHeader>::Instance()))).Times(0);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsTestSubStruct>::Instance()))).Times(1);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsTestStruct>::Instance()))).Times(1);

        sut.forEach<StructDescriptor<>>(mockTypeHandler.AsStdFunction());
    }

    {
        ::testing::MockFunction<void(const Descriptor<>&)> mockTypeHandler;
        EXPECT_CALL(mockTypeHandler, Call(::testing::Property(&Descriptor<>::type, Type::Struct))).Times(::testing::AnyNumber());
        EXPECT_CALL(mockTypeHandler, Call(::testing::Property(&Descriptor<>::type, Type::Enum))).Times(::testing::AnyNumber());
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsHeader>::Instance()))).Times(0);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsTestSubStruct>::Instance()))).Times(1);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsTestEnum>::Instance()))).Times(1);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsTestStruct>::Instance()))).Times(1);

        sut.forEach<StructDescriptor<>, EnumDescriptor<>>(mockTypeHandler.AsStdFunction());
    }

    {
        ::testing::MockFunction<void(const Descriptor<>&)> mockTypeHandler;
        EXPECT_CALL(mockTypeHandler, Call(::testing::Property(&Descriptor<>::type, Type::Vector))).Times(::testing::AnyNumber());
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<DotsTestStruct>::Instance()))).Times(1);
        EXPECT_CALL(mockTypeHandler, Call(::testing::Ref(Descriptor<dots::vector_t<DotsTestStruct>>::Instance()))).Times(1);

        sut.forEach<VectorDescriptor, Descriptor<DotsTestStruct>>(mockTypeHandler.AsStdFunction());
    }
}