#pragma once
#include <dots/io/serialization/StringSerializerBase.h>

namespace dots::io
{
    struct AsciiSerializerTraits : StringSerializerTraits {};

    template <typename Derived = void>
    struct AsciiSerializer : StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, AsciiSerializer<void>, Derived>, AsciiSerializerTraits>
    {
        using base_t = StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, AsciiSerializer<void>, Derived>, AsciiSerializerTraits>;
        using data_t = std::string;

        using base_t::base_t;
        AsciiSerializer(const AsciiSerializer& other) = default;
        AsciiSerializer(AsciiSerializer&& other) = default;
        ~AsciiSerializer() = default;

        AsciiSerializer& operator = (const AsciiSerializer& rhs) = default;
        AsciiSerializer& operator = (AsciiSerializer&& rhs) = default;
    };

    struct ClassicMultiLineAsciiSerializerTraits : StringSerializerTraits
    {
        static constexpr std::string_view StructBegin = "{";
        static constexpr std::string_view StructEnd = "}";

        static constexpr std::string_view PropertyNameBegin = "";
        static constexpr std::string_view PropertyNameEnd = "";
        static constexpr std::string_view PropertyValueBegin = "=";
        static constexpr std::string_view PropertyValueEnd = "";
        static constexpr std::string_view PropertyInvalidValue = "null";

        static constexpr std::string_view VectorBegin = "[";
        static constexpr std::string_view VectorValueSeparator = ",";
        static constexpr std::string_view VectorEnd = "]";

        static constexpr bool UserTypeNames = false;

        static constexpr bool NumericBooleans = true;
        static constexpr bool NumericPropertySets = true;
        static constexpr bool NumericTimePoints = true;
        static constexpr bool NumericEnums = false;

        static constexpr bool IntegerBasePrefixes = true;
        static constexpr bool IntegerSignSuffixes = false;

        static constexpr bool FloatSizeSuffix = true;
    };

    template <typename Derived = void>
    struct ClassicMultiLineAsciiSerializer : StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, ClassicMultiLineAsciiSerializer<void>, Derived>, ClassicMultiLineAsciiSerializerTraits>
    {
        using base_t = StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, ClassicMultiLineAsciiSerializer<void>, Derived>, ClassicMultiLineAsciiSerializerTraits>;
        using data_t = std::string;

        ClassicMultiLineAsciiSerializer(StringSerializerOptions options = {}) :
            base_t(std::move(options))
        {
            /* do nothing */
        }

        ClassicMultiLineAsciiSerializer(size_t indentSize) :
            ClassicMultiLineAsciiSerializer(StringSerializerOptions{ true, true, indentSize })
        {
            /* do nothing */
        }
    };

    struct ClassicSingleLineAsciiSerializerTraits : StringSerializerTraits
    {
        static constexpr std::string_view StructBegin = "<";
        static constexpr std::string_view StructEnd = ">";

        static constexpr std::string_view PropertyNameBegin = "";
        static constexpr std::string_view PropertyNameEnd = "";
        static constexpr std::string_view PropertyValueBegin = ":";
        static constexpr std::string_view PropertyValueEnd = " ";
        static constexpr std::string_view PropertyInvalidValue = "null";

        static constexpr std::string_view VectorBegin = "[";
        static constexpr std::string_view VectorValueSeparator = ",";
        static constexpr std::string_view VectorEnd = "]";

        static constexpr bool UserTypeNames = false;

        static constexpr bool NumericBooleans = true;
        static constexpr bool NumericPropertySets = true;
        static constexpr bool NumericTimePoints = true;
        static constexpr bool NumericEnums = false;

        static constexpr bool IntegerBasePrefixes = true;
        static constexpr bool IntegerSignSuffixes = false;

        static constexpr bool FloatSizeSuffix = true;
    };

    template <typename Derived = void>
    struct ClassicSingleLineAsciiSerializer : StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, ClassicSingleLineAsciiSerializer<void>, Derived>, ClassicSingleLineAsciiSerializerTraits>
    {
        using base_t = StringSerializerBase<std::conditional_t<std::is_same_v<Derived, void>, ClassicSingleLineAsciiSerializer<void>, Derived>, ClassicSingleLineAsciiSerializerTraits>;
        using data_t = std::string;

        ClassicSingleLineAsciiSerializer(StringSerializerOptions options = {}) :
            base_t(std::move(options))
        {
            /* do nothing */
        }

        ClassicSingleLineAsciiSerializer(size_t indentSize) :
            ClassicSingleLineAsciiSerializer(StringSerializerOptions{ true, false, indentSize })
        {
            /* do nothing */
        }
    };
}