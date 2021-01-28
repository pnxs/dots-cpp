#pragma once
#include <string_view>
#include <dots/type/PropertyOffset.h>

namespace dots::type
{
    struct StaticPropertyMetadata
    {
        constexpr StaticPropertyMetadata(std::string_view name, uint32_t tag, bool isKey, PropertyOffset offset) :
            m_name(name),
            m_tag(tag),
            m_isKey(isKey),
            m_set{PropertySet::FromIndex(m_tag)},
            m_offset(offset)
        {
            /* do nothing */
        }
        
        constexpr StaticPropertyMetadata(const StaticPropertyMetadata& other) = default;
        constexpr StaticPropertyMetadata(StaticPropertyMetadata&& other) = default;
        ~StaticPropertyMetadata() = default;

        constexpr StaticPropertyMetadata& operator = (const StaticPropertyMetadata& rhs) = default;
        constexpr StaticPropertyMetadata& operator = (StaticPropertyMetadata&& rhs) = default;

        constexpr std::string_view name() const
        {
            return m_name;
        }

        constexpr uint32_t tag() const
        {
            return m_tag;
        }

        constexpr bool isKey() const
        {
            return m_isKey;
        }

        constexpr PropertySet set() const
        {
            return m_set;
        }

        constexpr PropertyOffset offset() const
        {
            return m_offset;
        }

    private:

        std::string_view m_name;
        uint32_t m_tag;
        bool m_isKey;
        PropertySet m_set;
        PropertyOffset m_offset;
    };
}