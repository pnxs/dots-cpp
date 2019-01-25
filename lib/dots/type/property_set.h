#pragma once

#include <cstdint>
#include <bitset>

namespace dots
{

class property_set
{
    uint32_t m_bitset = 0;

public:
    property_set() = default;
    constexpr property_set(uint32_t b) : m_bitset(b) {}

    constexpr bool empty() const { return m_bitset == 0; }
    constexpr operator bool() const { return not empty(); }

    std::size_t count() const
    {
        std::bitset<32> bitset(m_bitset);
        return bitset.count();
    }

    uint32_t value() const
    {
        return m_bitset;
    }

    constexpr bool test(uint32_t pos) const
    {
        return ((m_bitset & (1 << pos)) != 0);
    }

    void clear() { m_bitset = 0; }

    void reset(uint32_t pos)
    {
        m_bitset &= ~ (1 << pos);
    }

    void set(uint32_t pos, bool v = true)
    {
        if (v) {
            m_bitset |= 1 << pos;
        } else {
            reset(pos);
        }
    }

    constexpr bool operator==(const property_set& rhs) const
    {
        return m_bitset == rhs.m_bitset;
    }
    
    constexpr bool operator!=(const property_set& rhs) const
    {
        return not (*this == rhs);
    }

    constexpr bool operator<(const property_set& rhs) const
    {
        return m_bitset < rhs.m_bitset;
    }

    constexpr property_set operator[](uint32_t pos) const
    {
        return test(pos);
    }

    constexpr property_set operator|(const property_set& rhs) const
    {
        return m_bitset | rhs.m_bitset;
    }

    property_set& operator|=(const property_set& rhs)
    {
        m_bitset |= rhs.m_bitset;
        return *this;
    }

    constexpr property_set operator&(const property_set& rhs) const
    {
        return m_bitset & rhs.m_bitset;
    }

    property_set& operator&=(const property_set& rhs)
    {
        m_bitset &= rhs.m_bitset;
        return *this;
    }

    constexpr property_set operator~() const
    {
        return ~m_bitset;
    }

    std::string to_string() const
    {
        return std::bitset<32>(m_bitset).to_string();
    }

    static std::string convert_to_string(const property_set& tp, bool& ok)
    {
        ok = true;
        return tp.to_string();
    }

};

struct AllProperties: public property_set
{
    constexpr AllProperties(): property_set(~property_set()) {}
};

}

#define PROPERTY_SET_NONE dots::property_set()
#define PROPERTY_SET_ALL dots::AllProperties()