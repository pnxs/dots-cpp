// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <string_view>

namespace dots::tools
{
    /**
     * File line function.
     * Stores the information of a specific sourcecode point.
     */
    struct Flf
    {
        constexpr Flf(std::string_view filePath, int l, const std::string_view fun) :
            file(filePath.substr(filePath.find_last_of('/') + 1)),
            line(l),
            func(fun)
        {}

        std::string_view file;
        const int line;
        std::string_view func;
    };
}

#define FLF dots::tools::Flf(__FILE__, __LINE__, __FUNCTION__)
