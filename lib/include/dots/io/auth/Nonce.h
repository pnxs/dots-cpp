#pragma once
#include <string_view>

namespace dots::io
{
    struct Nonce
    {
        using value_t = uint64_t;

        Nonce();
        Nonce(value_t value);
        Nonce(std::string_view stringValue);

        const value_t& value() const;
        std::string toString() const;

    private:

        value_t m_value;
    };
}
