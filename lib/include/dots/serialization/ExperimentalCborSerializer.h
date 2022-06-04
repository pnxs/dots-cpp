#pragma once
#include <vector>
#include <stdexcept>
#include <string>
#include <dots/serialization/Serializer.h>
#include <dots/serialization/formats/CborReader.h>
#include <dots/serialization/formats/CborWriter.h>

namespace dots::serialization
{
    struct ExperimentalCborSerializerFormat
    {
        using reader_t = CborReader;
        using writer_t = CborWriter;
    };

    struct ExperimentalCborSerializer : Serializer<ExperimentalCborSerializerFormat, ExperimentalCborSerializer>
    {
    protected:

        friend TypeVisitor<ExperimentalCborSerializer>;

        template <typename T>
        bool visitStructBeginDerived(const T& instance, property_set_t& includedProperties)
        {
            includedProperties ^= instance._validProperties();
            writer().writeArraySize(includedProperties.count());
            writer().write(includedProperties.toValue());

            return true;
        }

        template <typename T>
        bool visitPropertyBeginDerived(const T&/* property*/, bool/* first*/)
        {
            return true;
        }

        template <typename T>
        bool visitVectorBeginDerived(const vector_t<T>& vector, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            writer().writeArraySize(vector.typelessSize());
            return true;
        }

        template <typename T>
        void visitEnumDerived(const T& value, const type::EnumDescriptor<>& descriptor)
        {
            writer().write(descriptor.enumeratorFromValue(value).tag());
        }

        template <typename T>
        void visitFundamentalTypeDerived(const T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_floating_point_v<T>)
            {
                writer().write<T, CborWriter::FloatFormat::UseFloat16Zeroes>(value);
            }
            else if constexpr(std::is_integral_v<T>)
            {
                writer().write(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                writer().write(value.toValue());
            }
            else if constexpr (std::is_same_v<T, timepoint_t> || std::is_same_v<T, steady_timepoint_t>)
            {
                writer().write<double, CborWriter::FloatFormat::UseFloat16Zeroes>(value.duration().toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, duration_t>)
            {
                writer().write<double, CborWriter::FloatFormat::UseFloat16Zeroes>(value.toFractionalSeconds());
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
        bool visitStructBeginDerived(T&/* instance*/, property_set_t& includedProperties)
        {
            size_t numProperties = reader().readArraySize();
            includedProperties = property_set_t{ reader().read<uint32_t>() };

            if (size_t numValidProperties = includedProperties.count(); numValidProperties != numProperties)
            {
                throw std::runtime_error{ "encountered unexpected valid property set. expected '" + std::to_string(numProperties) + "' number of properties but got '" + std::to_string(numValidProperties) + "'" };
            }

            return true;
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
        void visitEnumDerived(T& value, const type::EnumDescriptor<>& descriptor)
        {
            descriptor.construct(value, descriptor.enumeratorFromTag(reader().read<uint32_t>()).value<T>());
        }

        template <typename T>
        void visitFundamentalTypeDerived(T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                value = reader().read<T>();
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
    std::vector<uint8_t> to_cbor_experimental(const T& instance, const property_set_t& includedProperties)
    {
        return serialization::ExperimentalCborSerializer::Serialize(instance, includedProperties);
    }

    template <typename T>
    std::vector<uint8_t> to_cbor_experimental(const T& value)
    {
        return serialization::ExperimentalCborSerializer::Serialize(value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_cbor_experimental(const uint8_t* data, size_t size, T& value)
    {
        return serialization::ExperimentalCborSerializer::Deserialize(data, size, value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_cbor_experimental(const std::vector<uint8_t>& data, T& value)
    {
        return serialization::ExperimentalCborSerializer::Deserialize(data, value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    T from_cbor_experimental(const uint8_t* data, size_t size)
    {
        return serialization::ExperimentalCborSerializer::Deserialize<T>(data, size);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    T from_cbor_experimental(const std::vector<uint8_t>& data)
    {
        return serialization::ExperimentalCborSerializer::Deserialize<T>(data);
    }
}
