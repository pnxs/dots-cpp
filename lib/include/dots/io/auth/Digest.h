// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <array>
#include <vector>
#include <type_traits>
#include <string_view>
#include <dots/io/auth/Nonce.h>

namespace picosha2
{
    class hash256_one_by_one;
}

namespace dots::io
{
    struct Digest
    {
        using value_t = std::array<uint8_t, 32>;

        Digest(const value_t& value);
        Digest(std::string_view stringValue);
        Digest(Nonce nonce, std::string_view cnonce, std::string_view userName, std::string_view secret);
        Digest(Nonce nonce, Nonce cnonce, std::string_view userName, std::string_view secret);

        const value_t& value() const;
        std::string toString() const;

    private:

        template<typename T> struct is_contiguous_container: std::false_type {};
        template<typename T, std::size_t N> struct is_contiguous_container<std::array<T, N>>: std::true_type {};
        template<typename T> struct is_contiguous_container<std::vector<T>>: std::true_type {};
        template<typename T> struct is_contiguous_container<std::basic_string<T>>: std::true_type {};

        void update(picosha2::hash256_one_by_one& hasher, std::string_view data);
        void update(picosha2::hash256_one_by_one& hasher, const uint8_t* data, size_t len);

        template<typename T, std::enable_if_t<is_contiguous_container<T>::value, int> = 0>
        void update(picosha2::hash256_one_by_one& hasher, const T& range)
        {
            update(hasher, range.data(), range.size());
        }

        auto final(picosha2::hash256_one_by_one& hasher) -> value_t;

        value_t m_value;
    };

}
