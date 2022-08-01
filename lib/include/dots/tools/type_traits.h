// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <type_traits>

namespace dots::tools
{
    template <typename T>
    using always_false = std::false_type;

    template <typename T>
    using always_false_t = always_false<T>;

    template <typename T>
    constexpr bool always_false_v = always_false<T>::value;
}
