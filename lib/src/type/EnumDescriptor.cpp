#include <dots/type/EnumDescriptor.h>

namespace dots::type
{
    EnumeratorDescriptor<Typeless>::EnumeratorDescriptor(uint32_t tag, std::string name) :
        m_tag(tag),
        m_name(std::move(name))
    {
        /* do nothing */
    }

    uint32_t EnumeratorDescriptor<Typeless>::tag() const
    {
        return m_tag;
    }

    const std::string& EnumeratorDescriptor<Typeless>::name() const
    {
        return m_name;
    }

    const Typeless& EnumeratorDescriptor<Typeless>::value() const
    {
        return valueTypeless();
    }

    EnumDescriptor<Typeless, false, void>::EnumDescriptor(key_t key, std::string name, size_t underlyingTypeSize, size_t underlyingTypeAlignment) :
        Descriptor<Typeless>(key, Type::Enum, std::move(name), underlyingTypeSize, underlyingTypeAlignment)
    {
        /* do nothing */
    }
}
