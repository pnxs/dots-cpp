#pragma once
#include <string_view>
#include <dots/io/serialization/StringSerializerBase.h>

namespace dots::io
{
    struct JsonSerializerTraits : StringSerializerTraits
    {
        static constexpr std::string_view StructBegin = "{";
        static constexpr std::string_view StructEnd = "}";

        static constexpr std::string_view PropertyNameBegin = "\"";
        static constexpr std::string_view PropertyNameEnd = "\":";
        static constexpr std::string_view PropertyValueBegin = "";
        static constexpr std::string_view PropertyValueEnd = ",";
        static constexpr std::string_view PropertyInvalidValue = "null";

        static constexpr std::string_view VectorBegin = "[";
        static constexpr std::string_view VectorValueSeparator = ",";
        static constexpr std::string_view VectorEnd = "]";

        static constexpr bool UserTypeNames = false;

        static constexpr bool NumericPropertySets = true;
        static constexpr bool NumericTimePoints = true;
        static constexpr bool NumericEnums = true;

        static constexpr bool IntegerBasePrefixes = true;
        static constexpr bool IntegerSignSuffixes = false;

        static constexpr bool FloatSizeSuffix = false;
    };

    template <typename Derived = void>
    struct JsonSerializer : StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, JsonSerializer<void>, Derived>, JsonSerializerTraits>
    {
        using base_t = StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, JsonSerializer<void>, Derived>, JsonSerializerTraits>;
        using data_t = std::string;

        using base_t::base_t;
        JsonSerializer(const JsonSerializer& other) = default;
        JsonSerializer(JsonSerializer&& other) = default;
        ~JsonSerializer() = default;

        JsonSerializer& operator = (const JsonSerializer& rhs) = default;
        JsonSerializer& operator = (JsonSerializer&& rhs) = default;
    };

    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    std::string to_json(const T& instance, const property_set_t& includedProperties = property_set_t::All)
    {
        return JsonSerializer<>::Serialize(instance, includedProperties);
    }

    template <typename T, std::enable_if_t<!std::is_base_of_v<type::Struct, T>, int> = 0>
    std::string to_json(const T& value)
    {
        return JsonSerializer<>::Serialize(value);
    }

    template <typename T>
    size_t from_json(const std::string& data, T& value)
    {
        return JsonSerializer<>::Deserialize(data, value);
    }

    template <typename T>
    T from_json(const std::string& data)
    {
        return JsonSerializer<>::Deserialize<T>(data);
    }
}