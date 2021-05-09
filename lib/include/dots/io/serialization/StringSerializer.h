#pragma once
#include <dots/io/serialization/StringSerializerBase.h>

namespace dots::io
{
    template <typename Derived = void>
    struct StringSerializer : StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, StringSerializer<void>, Derived>, StringSerializerTraits>
    {
        using base_t = StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, StringSerializer<void>, Derived>, StringSerializerTraits>;
        using data_t = std::string;

        using base_t::base_t;
        StringSerializer(const StringSerializer& other) = default;
        StringSerializer(StringSerializer&& other) = default;
        ~StringSerializer() = default;

        StringSerializer& operator = (const StringSerializer& rhs) = default;
        StringSerializer& operator = (StringSerializer&& rhs) = default;
    };

    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    std::string to_string(const T& instance, const property_set_t& includedProperties, StringSerializerOptions options = {})
    {
        return StringSerializer<>::Serialize(instance, includedProperties, options);
    }

    template <typename T>
    std::string to_string(const T& value, const type::Descriptor<T>& descriptor, StringSerializerOptions options = {})
    {
        return StringSerializer<>::Serialize(value, descriptor, options);
    }

    template <typename T>
    std::string to_string(const T& value, StringSerializerOptions options = {})
    {
        return StringSerializer<>::Serialize(value, options);
    }

    template <typename T>
    size_t from_string(const std::string& data, T& value, const type::Descriptor<T>& descriptor)
    {
        return StringSerializer<>::Deserialize(data, value, descriptor);
    }

    template <typename T>
    size_t from_string(const std::string& data, T& value)
    {
        return StringSerializer<>::Deserialize(data, value);
    }

    template <typename T>
    T from_string(const std::string& data)
    {
        return StringSerializer<>::Deserialize<T>(data);
    }
}