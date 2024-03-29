// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once

namespace dots::serialization
{
    struct TextFormat
    {
        struct string_escape_mapping
        {
            std::string_view from;
            std::string_view to;
        };
        static constexpr std::array<string_escape_mapping, 8> StringEscapeMapping{
            string_escape_mapping{ "\"", "\\\"" },
            string_escape_mapping{ "\\", "\\\\" },
            string_escape_mapping{ "/", "\\/" },
            string_escape_mapping{ "\b", "\\b" },
            string_escape_mapping{ "\f", "\\f" },
            string_escape_mapping{ "\n", "\\n" },
            string_escape_mapping{ "\r", "\\r" },
            string_escape_mapping{ "\t", "\\t" }
        };

        static constexpr std::string_view StringDelimiter = "\"";
        static constexpr std::string_view StringEscape = "\\";

        enum struct ObjectFormat : uint8_t
        {
            WithoutTypeName,
            WithTypeName,
        };

        enum struct IntegerFormat : uint8_t
        {
            WithoutSignSuffix,
            WithSignSuffix,
        };

        enum struct FloatFormat : uint8_t
        {
            WithoutSizeSuffix,
            WithSizeSuffix,
        };

        enum struct BooleanFormat : uint8_t
        {
            Integer,
            Literal,
            String
        };
    };

    struct TextOptions
    {
        enum Policy
        {
            Relaxed,
            Strict
        };

        enum Style
        {
            Minified,
            SingleLine,
            MultiLine
        };

        Style style = SingleLine;
        Policy policy = Relaxed;
        size_t indentSize = 4;
    };
}
