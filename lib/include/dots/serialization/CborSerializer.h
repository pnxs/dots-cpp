#pragma once
#include <vector>
#include <dots/serialization/Serializer.h>
#include <dots/serialization/formats/CborReader.h>
#include <dots/serialization/formats/CborWriter.h>

namespace dots::serialization
{
    struct CborSerializerFormat
    {
        using reader_t = CborReader;
        using writer_t = CborWriter;
    };

    struct CborSerializer : Serializer<CborSerializerFormat, CborSerializer>
    {
    protected:

        friend TypeVisitor<CborSerializer>;

        template <typename T>
        bool visitStructBeginDerived(const T& instance, property_set_t& includedProperties)
        {
            includedProperties ^= instance._validProperties();
            writer().writeMapSize(includedProperties.count());

            return true;
        }

        template <typename T>
        bool visitPropertyBeginDerived(const T& property, bool/* first*/)
        {
            if (visitingLevel<true>() > 0)
            {
                writer().write(property.descriptor().tag());
            }

            return true;
        }

        template <typename T>
        bool visitVectorBeginDerived(const vector_t<T>& vector, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            writer().writeArraySize(vector.typelessSize());
            return true;
        }

        template <typename T>
        void visitEnumDerived(const T& value, const type::EnumDescriptor<T>& descriptor)
        {
            writer().write(descriptor.enumeratorFromValue(value).tag());
        }

        template <typename T>
        void visitFundamentalTypeDerived(const T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                writer().write(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                writer().write(value.toValue());
            }
            else if constexpr (std::is_same_v<T, timepoint_t> || std::is_same_v<T, steady_timepoint_t>)
            {
                writer().write(value.duration().toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, duration_t>)
            {
                writer().write(value.toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                writer().write(value.data());
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                writer().write(value);
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }

        template <typename T>
        bool visitStructBeginDerived(T& instance, property_set_t&/* includedProperties*/)
        {
            const type::StructDescriptor<>& descriptor = instance._descriptor();
            const type::property_descriptor_container_t& propertyDescriptors = descriptor.propertyDescriptors();

            size_t numProperties = reader().readMapSize();

            for (size_t i = 0; i < numProperties; ++i)
            {
                uint32_t tag = reader().read<uint32_t>();

                if (auto it = std::find_if(propertyDescriptors.begin(), propertyDescriptors.end(), [tag](const auto& p) { return p.tag() == tag; }); it != propertyDescriptors.end())
                {
                    const type::PropertyDescriptor& propertyDescriptor = *it;
                    type::ProxyProperty<> property{ instance, propertyDescriptor };
                    visit(property);
                }
                else
                {
                    reader().skip();
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
            descriptor.fill(vector, reader().readArraySize());
            return true;
        }

        template <typename T>
        void visitEnumDerived(T& value, const type::EnumDescriptor<T>& descriptor)
        {
            descriptor.construct(value, descriptor.enumeratorFromTag(reader().read<uint32_t>()).value());
        }

        template <typename T>
        void visitFundamentalTypeDerived(T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                reader().read(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                value = property_set_t{ reader().read<uint32_t>() };
            }
            else if constexpr (std::is_same_v<T, timepoint_t> || std::is_same_v<T, steady_timepoint_t> || std::is_same_v<T, duration_t>)
            {
                value = T{ duration_t{ reader().read<float64_t>() } };
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                value = uuid_t{ reader().read<uuid_t::value_t>() };
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                reader().read(value);
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
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
