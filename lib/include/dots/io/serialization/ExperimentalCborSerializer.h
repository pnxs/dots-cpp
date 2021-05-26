#pragma once
#include <vector>
#include <stdexcept>
#include <string>
#include <dots/io/serialization/CborSerializerBase.h>
#include <dots/type/TypeVisitor.h>

namespace dots::io
{
    struct ExperimentalCborSerializer : CborSerializerBase<ExperimentalCborSerializer>
    {
        using data_t = std::vector<uint8_t>;

        ExperimentalCborSerializer() = default;
        ExperimentalCborSerializer(const ExperimentalCborSerializer& other) = default;
        ExperimentalCborSerializer(ExperimentalCborSerializer&& other) = default;
        ~ExperimentalCborSerializer() = default;

        ExperimentalCborSerializer& operator = (const ExperimentalCborSerializer& rhs) = default;
        ExperimentalCborSerializer& operator = (ExperimentalCborSerializer&& rhs) = default;

    protected:

        friend TypeVisitor<ExperimentalCborSerializer>;
        friend serializer_base_t;

        template <typename T>
        bool visitStructBeginDerived(const T& instance, property_set_t& includedProperties)
        {
            includedProperties ^= instance._validProperties();
            writeHead(Cbor::MajorType::Array, includedProperties.count());
            write(includedProperties.toValue());

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
        bool visitStructBeginDerived(T&/* instance*/, property_set_t& includedProperties)
        {
            size_t numProperties = readHead<size_t>(Cbor::MajorType::Array);
            includedProperties = property_set_t{ read<uint32_t>() };

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
                value = read<T>();
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

    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    std::vector<uint8_t> to_cbor_experimental(const T& instance, const property_set_t& includedProperties)
    {
        return ExperimentalCborSerializer::Serialize(instance, includedProperties);
    }

    template <typename T>
    std::vector<uint8_t> to_cbor_experimental(const T& value)
    {
        return ExperimentalCborSerializer::Serialize(value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_cbor_experimental(const uint8_t* data, size_t size, T& value)
    {
        return ExperimentalCborSerializer::Deserialize(data, size, value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_cbor_experimental(const std::vector<uint8_t>& data, T& value)
    {
        return ExperimentalCborSerializer::Deserialize(data, value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    T from_cbor_experimental(const uint8_t* data, size_t size)
    {
        return ExperimentalCborSerializer::Deserialize<T>(data, size);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    T from_cbor_experimental(const std::vector<uint8_t>& data)
    {
        return ExperimentalCborSerializer::Deserialize<T>(data);
    }
}