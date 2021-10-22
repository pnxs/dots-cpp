#include <dots/type/TypeVisitor.h>

namespace dots::type
{
    void TypeVisitor<void, void>::visitConstBegin()
    {
        TypeVisitor<TypeVisitor<void>>::visitBeginDerived<true>();
    }

    void TypeVisitor<void, void>::visitConstEnd()
    {
        TypeVisitor<TypeVisitor<void>>::visitEndDerived<true>();
    }

    void TypeVisitor<void, void>::visitMutableBegin()
    {
        TypeVisitor<TypeVisitor<void>>::visitBeginDerived<false>();
    }

    void TypeVisitor<void, void>::visitMutableEnd()
    {
        TypeVisitor<TypeVisitor<void>>::visitEndDerived<false>();
    }

    bool TypeVisitor<void, void>::visitStructBegin(const Struct& instance, PropertySet& includedProperties)
    {
        return TypeVisitor<TypeVisitor<void>>::visitStructBeginDerived(instance, includedProperties);
    }

    bool TypeVisitor<void, void>::visitStructBegin(Struct& instance, PropertySet& includedProperties)
    {
        return TypeVisitor<TypeVisitor<void>>::visitStructBeginDerived(instance, includedProperties);
    }

    void TypeVisitor<void, void>::visitStructEnd(const Struct& instance, const PropertySet& includedProperties)
    {
        TypeVisitor<TypeVisitor<void>>::visitStructEndDerived(instance, includedProperties);
    }

    void TypeVisitor<void, void>::visitStructEnd(Struct& instance, const PropertySet& includedProperties)
    {
        TypeVisitor<TypeVisitor<void>>::visitStructEndDerived(instance, includedProperties);
    }

    bool TypeVisitor<void, void>::visitPropertyBegin(const ProxyProperty<>& property, bool first)
    {
        return TypeVisitor<TypeVisitor<void>>::visitPropertyBeginDerived(property, first);
    }

    bool TypeVisitor<void, void>::visitPropertyBegin(ProxyProperty<>& property, bool first)
    {
        return TypeVisitor<TypeVisitor<void>>::visitPropertyBeginDerived(property, first);
    }

    void TypeVisitor<void, void>::visitPropertyEnd(const ProxyProperty<>& property, bool first)
    {
        TypeVisitor<TypeVisitor<void>>::visitPropertyEndDerived(property, first);
    }

    void TypeVisitor<void, void>::visitPropertyEnd(ProxyProperty<>& property, bool first)
    {
        TypeVisitor<TypeVisitor<void>>::visitPropertyEndDerived(property, first);
    }

    bool TypeVisitor<void, void>::visitVectorBegin(const Vector<>& vector, const VectorDescriptor& descriptor)
    {
        return TypeVisitor<TypeVisitor<void>>::visitVectorBeginDerived(vector, descriptor);
    }

    bool TypeVisitor<void, void>::visitVectorBegin(Vector<>& vector, const VectorDescriptor& descriptor)
    {
        return TypeVisitor<TypeVisitor<void>>::visitVectorBeginDerived(vector, descriptor);
    }

    bool TypeVisitor<void, void>::visitVectorValueBegin(const Vector<>& vector, const VectorDescriptor& descriptor, size_t index)
    {
        return TypeVisitor<TypeVisitor<void>>::visitVectorValueBeginDerived(vector, descriptor, index);
    }

    bool TypeVisitor<void, void>::visitVectorValueBegin(Vector<>& vector, const VectorDescriptor& descriptor, size_t index)
    {
        return TypeVisitor<TypeVisitor<void>>::visitVectorValueBeginDerived(vector, descriptor, index);
    }

    void TypeVisitor<void, void>::visitVectorValueEnd(const Vector<>& vector, const VectorDescriptor& descriptor, size_t index)
    {
        TypeVisitor<TypeVisitor<void>>::visitVectorValueEndDerived(vector, descriptor, index);
    }

    void TypeVisitor<void, void>::visitVectorValueEnd(Vector<>& vector, const VectorDescriptor& descriptor, size_t index)
    {
        TypeVisitor<TypeVisitor<void>>::visitVectorValueEndDerived(vector, descriptor, index);
    }

    void TypeVisitor<void, void>::visitVectorEnd(const Vector<>& vector, const VectorDescriptor& descriptor)
    {
        TypeVisitor<TypeVisitor<void>>::visitVectorEndDerived(vector, descriptor);
    }

    void TypeVisitor<void, void>::visitVectorEnd(Vector<>& vector, const VectorDescriptor& descriptor)
    {
        TypeVisitor<TypeVisitor<void>>::visitVectorEndDerived(vector, descriptor);
    }

    void TypeVisitor<void, void>::visitEnum(const Typeless& value, const EnumDescriptor<>& descriptor)
    {
        TypeVisitor<TypeVisitor<void>>::visitEnumDerived(value, descriptor);
    }

    void TypeVisitor<void, void>::visitEnum(Typeless& value, const EnumDescriptor<>& descriptor)
    {
        TypeVisitor<TypeVisitor<void>>::visitEnumDerived(value, descriptor);
    }

    bool TypeVisitor<void, void>::visitFundamentalTypeBegin(const Typeless&/* value*/, const Descriptor<>&/* descriptor*/)
    {
        return true;
    }

    bool TypeVisitor<void, void>::visitFundamentalTypeBegin(Typeless&/* value*/, const Descriptor<>&/* descriptor*/)
    {
        return true;
    }

    void TypeVisitor<void, void>::visitFundamentalType(const bool_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<bool_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(bool_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<bool_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const int8_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<int8_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(int8_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<int8_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const uint8_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uint8_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(uint8_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uint8_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const int16_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<int16_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(int16_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<int16_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const uint16_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uint16_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(uint16_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uint16_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const int32_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<int32_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(int32_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<int32_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const uint32_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uint32_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(uint32_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uint32_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const int64_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<int64_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(int64_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<int64_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const uint64_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uint64_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(uint64_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uint64_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const float32_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<float32_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(float32_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<float32_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const float64_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<float64_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(float64_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<float64_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const property_set_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<property_set_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(property_set_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<property_set_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const timepoint_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<timepoint_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(timepoint_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<timepoint_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const steady_timepoint_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<steady_timepoint_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(steady_timepoint_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<steady_timepoint_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const duration_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<duration_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(duration_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<duration_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const uuid_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uuid_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(uuid_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<uuid_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(const string_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<string_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalType(string_t& value)
    {
        TypeVisitor<TypeVisitor<void>>::visitFundamentalTypeDerived(value, Descriptor<string_t>::Instance());
    }

    void TypeVisitor<void, void>::visitFundamentalTypeEnd(const Typeless&/* value*/, const Descriptor<>&/* descriptor*/)
    {
        /* do nothing */
    }

    void TypeVisitor<void, void>::visitFundamentalTypeEnd(Typeless&/* value*/, const Descriptor<>&/* descriptor*/)
    {
        /* do nothing */
    }
}
