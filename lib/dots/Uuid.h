#pragma once

#include <cstdint>
#include <array>

namespace dots::type
{
    struct Uuid
    {
        Uuid(const uint8_t data[16]);
        Uuid(const std::string& strdata);
        Uuid();

        bool operator == (const Uuid&) const;
        bool operator < (const Uuid& rhs) const;
        bool operator != (const Uuid&) const;

        const auto& data() const { return m_data; }

        std::string toString() const;
        bool fromString(const std::string& str);

        static Uuid generateRandom();

    private:

        std::array<uint8_t, 16> m_data;
    };
}
