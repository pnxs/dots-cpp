#pragma once
#include <dots/serialization/TextSerializer.h>

namespace dots::serialization
{
    struct StringSerializer : TextSerializer<StringSerializer, StringSerializerTraits>
    {
        using base_t = TextSerializer<StringSerializer, StringSerializerTraits>;
        using data_t = std::string;

        using base_t::base_t;
        StringSerializer(const StringSerializer& other) = default;
        StringSerializer(StringSerializer&& other) = default;
        ~StringSerializer() = default;

        StringSerializer& operator = (const StringSerializer& rhs) = default;
        StringSerializer& operator = (StringSerializer&& rhs) = default;
    };
}

namespace dots
{
    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    std::string to_string(const T& instance, const property_set_t& includedProperties, serialization::StringSerializerOptions options = {})
    {
        return serialization::StringSerializer::Serialize(instance, includedProperties, options);
    }

    template <typename T>
    std::string to_string(const T& value, const type::Descriptor<T>& descriptor, serialization::StringSerializerOptions options = {})
    {
        return serialization::StringSerializer::Serialize(value, descriptor, options);
    }

    template <typename T>
    std::string to_string(const T& value, serialization::StringSerializerOptions options = {})
    {
        return serialization::StringSerializer::Serialize(value, options);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_string(const std::string& data, T& value, const type::Descriptor<T>& descriptor, serialization::StringSerializerOptions options = {})
    {
        return serialization::StringSerializer::Deserialize(data, value, descriptor, options);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_string(const std::string& data, T& value, serialization::StringSerializerOptions options = {})
    {
        return serialization::StringSerializer::Deserialize(data, value, options);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    T from_string(const std::string& data, serialization::StringSerializerOptions options = {})
    {
        return serialization::StringSerializer::Deserialize<T>(data, options);
    }
}
