#pragma once
#include <dots/serialization/TextSerializer.h>
#include <dots/serialization/formats/JsonReader.h>
#include <dots/serialization/formats/JsonWriter.h>

namespace dots::serialization
{
    struct JsonSerializerFormat : TextSerializerFormat
    {
        using reader_t = JsonReader;
        using writer_t = JsonWriter;

        static constexpr PropertySetFormat PropertySetFormat = PropertySetFormat::DecimalValue;
        static constexpr TimepointFormat TimepointFormat = TimepointFormat::FractionalSeconds;
        static constexpr EnumFormat EnumFormat = EnumFormat::Tag;
    };

    struct JsonSerializer : TextSerializer<JsonSerializerFormat, JsonSerializer>
    {
        using base_t = TextSerializer<JsonSerializerFormat, JsonSerializer>;
        using data_t = std::string;

        using base_t::base_t;
    };
}

namespace dots
{
    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    std::string to_json(const T& instance, const property_set_t& includedProperties, serialization::TextOptions options = {})
    {
        return serialization::JsonSerializer::Serialize(instance, includedProperties, options);
    }

    template <typename T>
    std::string to_json(const T& value, serialization::TextOptions options = {})
    {
        return serialization::JsonSerializer::Serialize(value, options);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_json(const std::string& data, T& value)
    {
        return serialization::JsonSerializer::Deserialize(data, value);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    T from_json(const std::string& data)
    {
        return serialization::JsonSerializer::Deserialize<T>(data);
    }
}
