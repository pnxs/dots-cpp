#pragma once

#include <cstdint>
#include <array>

namespace dots {

class uuid
{
    std::array<uint8_t, 16> m_data;

public:
    uuid(const uint8_t data[16]);
    uuid(const std::string& strdata);
    uuid();

    bool operator==(const uuid&) const;
    bool operator<(const uuid& rhs) const;
    bool operator!=(const uuid&) const;

    const auto& data() const { return m_data; }

    std::string toString() const;
    bool fromString(const std::string& str);

    static uuid generateRandom();
};

}

