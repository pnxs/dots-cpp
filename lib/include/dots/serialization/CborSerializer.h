#pragma once
#include <vector>
#include <dots/serialization/CborSerializerBase.h>
#include <dots/type/TypeVisitor.h>

namespace dots::serialization
{
    struct CborSerializer : CborSerializerBase<CborSerializer, false>
    {
        using data_t = std::vector<uint8_t>;

        CborSerializer() = default;
        CborSerializer(const CborSerializer& other) = default;
        CborSerializer(CborSerializer&& other) = default;
        ~CborSerializer() = default;

        CborSerializer& operator = (const CborSerializer& rhs) = default;
        CborSerializer& operator = (CborSerializer&& rhs) = default;

    protected:

        friend TypeVisitor<CborSerializer>;
        friend serializer_base_t;

        template <typename T>
        bool visitStructBeginDerived(const T& instance, property_set_t& includedProperties)
        {
            includedProperties ^= instance._validProperties();
            writeHead(Cbor::MajorType::Map, includedProperties.count());

            return true;
        }

        template <typename T>
        bool visitPropertyBeginDerived(const T& property, bool/* first*/)
        {
            if (serializeLevel() > 0)
            {
                write(property.descriptor().tag());
            }

            return true;
        }

        template <typename T>
        bool visitVectorBeginDerived(const vector_t<T>& vector, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            writeHead(Cbor::MajorType::Array, vector.typelessSize());
            return true;
        }

        template <typename T>
        void visitEnumDerived(const T& value, const type::EnumDescriptor<T>& descriptor)
        {
            write(descriptor.enumeratorFromValue(value).tag());
        }

        template <typename T>
        void visitFundamentalTypeDerived(const T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                write(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                write(value.toValue());
            }
            else if constexpr (std::is_same_v<T, timepoint_t> || std::is_same_v<T, steady_timepoint_t>)
            {
                write(value.duration().toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, duration_t>)
            {
                write(value.toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                write(value.data());
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                write(value);
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }

        void serializeTupleBeginDerived()
        {
            writeHead(Cbor::MajorType::IndefiniteArray);
        }

        void serializeTupleEndDerived()
        {
            writeHead(Cbor::MajorType::IndefiniteArrayBreak);
        }

        template <typename T>
        bool visitStructBeginDerived(T& instance, property_set_t&/* includedProperties*/)
        {
            const type::StructDescriptor<>& descriptor = instance._descriptor();
            const type::property_descriptor_container_t& propertyDescriptors = descriptor.propertyDescriptors();

            size_t numProperties = readHead<size_t>(Cbor::MajorType::Map);

            for (size_t i = 0; i < numProperties; ++i)
            {
                uint32_t tag = read<uint32_t>();

                if (auto it = std::find_if(propertyDescriptors.begin(), propertyDescriptors.end(), [tag](const auto& p) { return p.tag() == tag; }); it != propertyDescriptors.end())
                {
                    const type::PropertyDescriptor& propertyDescriptor = *it;
                    type::ProxyProperty<> property{ instance, propertyDescriptor };
                    visit(property);
                }
            }

            return false;
        }

        template <typename T>
        bool visitPropertyBeginDerived(T& property, bool/* first*/)
        {
            property.constructOrValue();
            return true;
        }

        template <typename T>
        bool visitVectorBeginDerived(vector_t<T>& vector, const type::Descriptor<vector_t<T>>& descriptor)
        {
            descriptor.fill(vector, readHead<size_t>(Cbor::MajorType::Array));
            return true;
        }

        template <typename T>
        void visitEnumDerived(T& value, const type::EnumDescriptor<T>& descriptor)
        {
            descriptor.construct(value, descriptor.enumeratorFromTag(read<uint32_t>()).value());
        }

        template <typename T>
        void visitFundamentalTypeDerived(T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                read(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                value = property_set_t{ read<uint32_t>() };
            }
            else if constexpr (std::is_same_v<T, timepoint_t> || std::is_same_v<T, steady_timepoint_t> || std::is_same_v<T, duration_t>)
            {
                value = T{ duration_t{ read<float64_t>() } };
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                value = uuid_t{ read<uuid_t::value_t>() };
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                read(value);
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }

        void deserializeTupleBeginDerived()
        {
            readHead(Cbor::MajorType::IndefiniteArray);
        }

        void deserializeTupleEndDerived()
        {
            readHead(Cbor::MajorType::IndefiniteArrayBreak);
        }
    };
}

namespace dots
{
    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    std::vector<uint8_t> to_cbor(const T& instance, const property_set_t& includedProperties)
    {
        return serialization::CborSerializer::Serialize(instance, includedProperties);
    }

    template <typename T>
    std::vector<uint8_t> to_cbor(const T& value)
    {
        return serialization::CborSerializer::Serialize(value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_cbor(const uint8_t* data, size_t size, T& value)
    {
        return serialization::CborSerializer::Deserialize(data, size, value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_cbor(const std::vector<uint8_t>& data, T& value)
    {
        return serialization::CborSerializer::Deserialize(data, value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    T from_cbor(const uint8_t* data, size_t size)
    {
        return serialization::CborSerializer::Deserialize<T>(data, size);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    T from_cbor(const std::vector<uint8_t>& data)
    {
        return serialization::CborSerializer::Deserialize<T>(data);
    }
}
