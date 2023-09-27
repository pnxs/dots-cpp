// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once

#include <vector>
#include <span>
#include "fmt/core.h"
#include "fmt/format.h"

namespace fmt::extension
{

    template<typename T>
    struct hexdump_vector_t
    {
        const std::span <T> &data;
    };

    template<typename T>
    auto format_hexdump(const std::span <T> &d)
    {
        return hexdump_vector_t<T>(d);
    }
}

template<typename T>
struct fmt::formatter<fmt::extension::hexdump_vector_t<T>>
{
    // Preparation for switching output formats: (note: use code from 'format-switch' comments)
    // Presentation format: 'b' - bytes
    //format-switch: char presentation = 'b';
    uint8_t lineLength = 16;

    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context &ctx) -> format_parse_context::iterator
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();
        //format-switch: if (it != end && (*it == 'b')) presentation = *it++;

        // Check if reached the end of the range:
        if (it != end && *it != '}') throw_format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    auto format(const fmt::extension::hexdump_vector_t<T> &p, format_context &ctx) const -> format_context::iterator
    {
        std::span data{p.data};
        std::size_t width = lineLength;
        const std::size_t hexLength = lineLength * 3;

        for (std::size_t offset{};; offset += width)
        {
            auto block = data.first(width < data.size() ? width : data.size());

            fmt::format_to(ctx.out(), "{:04x}: {:{}} ",
                           offset,
                           fmt::format("{:02x}", fmt::join(block, " ")),
                           hexLength
            );

            for (auto &c: block)
            {
                fmt::format_to(ctx.out(), "{:c}", (c >= 32 && c <= 127) ? c : '.');
            }

            if (data.size() < width) break;
            fmt::format_to(ctx.out(), "\n");

            data = data.subspan(width);
        }

        return ctx.out();
    }
};