#pragma once
#include <array>
#include <vector>
#include <type_traits>
#include <string_view>
#include <dots/io/auth/Nonce.h>

struct SHA256state_st;
using SHA256_CTX = SHA256state_st;

namespace dots::io
{
    struct Digest
    {
        using value_t = std::array<uint8_t, 32>;

        Digest(const value_t& value);
        Digest(std::string_view stringValue);
        Digest(Nonce nonce, std::string_view cnonce, std::string_view userName, std::string_view secret);
        Digest(Nonce nonce, Nonce cnonce, std::string_view userName, std::string_view secret);
        Digest(const Digest& other) = default;
        Digest(Digest&& other) = default;
        ~Digest() = default;

        Digest& operator = (const Digest& rhs) = default;
        Digest& operator = (Digest&& rhs) = default;

        const value_t& value() const;
        std::string toString() const;

    private:

        template<typename T> struct is_contiguous_container: public std::false_type {};
        template<typename T, std::size_t N> struct is_contiguous_container<std::array<T, N>>: public std::true_type {};
        template<typename T> struct is_contiguous_container<std::vector<T>>: public std::true_type {};
        template<typename T> struct is_contiguous_container<std::basic_string<T>>: public std::true_type {};

        void init(SHA256_CTX& ctx);

        void update(SHA256_CTX& ctx, std::string_view data);
        void update(SHA256_CTX& ctx, const void* data, size_t len);

        template<typename T, std::enable_if_t<is_contiguous_container<T>::value, int> = 0>
        void update(SHA256_CTX& ctx, const T& range)
        {
            update(ctx, range.data(), range.size());
        }

        auto final(SHA256_CTX& ctx) -> value_t;

        value_t m_value;
    };

}