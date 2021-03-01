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
}