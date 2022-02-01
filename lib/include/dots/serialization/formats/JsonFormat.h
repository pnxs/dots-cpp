#pragma once
#include <dots/serialization/formats/TextFormat.h>

namespace dots::serialization
{
    struct JsonFormat : TextFormat
    {
        static constexpr std::string_view ObjectBegin = "{";
        static constexpr std::string_view ObjectEnd = "}";

        static constexpr std::string_view ObjectMemberNameBegin = "\"";
        static constexpr std::string_view ObjectMemberNameEnd = "\":";
        static constexpr std::string_view ObjectMemberValueBegin = "";
        static constexpr std::string_view ObjectMemberValueEnd = ",";

        static constexpr std::string_view ArrayBegin = "[";
        static constexpr std::string_view ArrayElementSeparator = ",";
        static constexpr std::string_view ArrayEnd = "]";

        static constexpr std::string_view NullValue = "null";

        static constexpr ObjectFormat ObjectFormat = ObjectFormat::WithoutTypeName;
        static constexpr IntegerFormat IntegerFormat = IntegerFormat::WithoutSignSuffix;
        static constexpr FloatFormat FloatFormat = FloatFormat::WithoutSizeSuffix;
        static constexpr BooleanFormat BooleanFormat = BooleanFormat::Literal;
    };
}
