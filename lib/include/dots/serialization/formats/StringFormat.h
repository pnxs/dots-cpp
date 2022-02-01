#pragma once
#include <dots/serialization/formats/TextFormat.h>

namespace dots::serialization
{
    struct StringFormat : TextFormat
    {
        static constexpr std::string_view ObjectBegin = "{";
        static constexpr std::string_view ObjectEnd = "}";

        static constexpr std::string_view ObjectMemberNameBegin = ".";
        static constexpr std::string_view ObjectMemberNameEnd = "";
        static constexpr std::string_view ObjectMemberValueBegin = "=";
        static constexpr std::string_view ObjectMemberValueEnd = ",";

        static constexpr std::string_view ArrayBegin = "{";
        static constexpr std::string_view ArrayElementSeparator = ",";
        static constexpr std::string_view ArrayEnd = "}";

        static constexpr std::string_view NullValue = "<invalid>";

        static constexpr ObjectFormat ObjectFormat = ObjectFormat::WithTypeName;
        static constexpr IntegerFormat IntegerFormat = IntegerFormat::WithSignSuffix;
        static constexpr FloatFormat FloatFormat = FloatFormat::WithSizeSuffix;
        static constexpr BooleanFormat BooleanFormat = BooleanFormat::Literal;
    };
}
