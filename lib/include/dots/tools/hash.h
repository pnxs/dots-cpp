// SPDX-License-Identifier: LGPL-3.0-only
#pragma once
#include <cstddef>

namespace dots::tools
{
    /*!
     * @brief Fold another hash value into a running seed.
     *
     * Boost-style golden-ratio hash combining. This is the single definition
     * used by all element-wise hashes (structs, vectors, any objects).
     */
    constexpr std::size_t hashCombine(std::size_t seed, std::size_t value) noexcept
    {
        return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
    }
}
