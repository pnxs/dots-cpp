// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <vector>
#include <dots/serialization/Serializer.h>
#include <dots/serialization/formats/CborReader.h>
#include <dots/serialization/formats/CborWriter.h>
#include <dots/type/AnyObject.h>
#include <dots/type/AnyStruct.h>
#include <dots/type/Registry.h>

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
        void visitEnumDerived(const T& value, const type::EnumDescriptor& descriptor)
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
            else if constexpr (std::is_same_v<T, type::AnyObject>)
            {
                // opaque envelope: [ typeName, payload-bytes ]
                writer().writeArraySize(2);
                writer().write(value.typeName());
                writer().writeByteString(value.payload().data(), value.payload().size());
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }

        template <typename T>
        bool visitStructBeginDerived(T& instance, property_set_t& includedProperties)
        {
            const type::StructDescriptor& descriptor = instance._descriptor();

            size_t numProperties = reader().readMapSize();

            for (size_t i = 0; i < numProperties; ++i)
            {
                uint32_t tag = reader().read<uint32_t>();

                if (visitingLevel<false>() > 0)
                {
                    includedProperties = property_set_t::All;
                }

                if (const type::PropertyDescriptor* pd = descriptor.findPropertyByTag(tag);
                    pd != nullptr && pd->set() <= includedProperties) [[likely]]
                {
                    type::ProxyProperty<> property{ instance, *pd };
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
            property.valueOrEmplace();
            return true;
        }

        template <typename T>
        bool visitVectorBeginDerived(vector_t<T>& vector, const type::Descriptor<vector_t<T>>& descriptor)
        {
            descriptor.fill(vector, reader().readArraySize());
            return true;
        }

        template <typename T>
        void visitEnumDerived(T& value, const type::EnumDescriptor& descriptor)
        {
            descriptor.construct(value, descriptor.enumeratorFromTag(reader().read<uint32_t>()).value<T>());
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
            else if constexpr (std::is_same_v<T, type::AnyObject>)
            {
                // opaque envelope: [ typeName, payload-bytes ]
                if (size_t arraySize = reader().readArraySize(); arraySize != 2)
                {
                    throw std::runtime_error{ "invalid any envelope: expected array of 2, got " + std::to_string(arraySize) };
                }

                string_t typeName;
                reader().read(typeName);
                std::vector<uint8_t> payload;
                reader().readByteString(payload);
                value = type::AnyObject{ std::move(typeName), std::move(payload) };
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

    /*!
     * @brief Wrap a live DOTS struct into an AnyObject by serializing it to
     * canonical CBOR. The contained type's name is taken from its descriptor.
     */
    inline type::AnyObject to_any(const type::Struct& instance)
    {
        return type::AnyObject{ instance._descriptor().name(), to_cbor(instance, instance._validProperties()) };
    }

    /*!
     * @brief Recover the DOTS struct stored in an AnyObject. The contained
     * type is resolved by name against the given registry (throws if unknown,
     * which by the DOTS descriptor contract indicates the contained type's
     * descriptor was not published before the object).
     */
    inline type::AnyStruct from_any(const type::AnyObject& any, const type::Registry& registry)
    {
        const type::StructDescriptor& descriptor = registry.getStructType(any.typeName());
        type::AnyStruct instance{ descriptor };
        from_cbor(any.payload(), *instance);
        return instance;
    }
}
