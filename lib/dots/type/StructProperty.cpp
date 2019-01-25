#include "StructProperty.h"

namespace dots
{
namespace type
{

StructProperty::StructProperty(const std::string &name, std::size_t offset, int tag, bool key, const Descriptor *td)
:m_name(name), m_offset(offset), m_tag(tag), m_isKey(key), m_typeDescriptor(td)
{

}

bool StructProperty::equal(const void *lhs, const void *rhs) const
{
    return m_typeDescriptor->equal(address(lhs), address(rhs));
}

bool StructProperty::lessThen(const void *lhs, const void *rhs) const
{
    return m_typeDescriptor->lessThan(address(lhs), address(rhs));
}

void StructProperty::copy(void *lhs, const void *rhs) const
{
    m_typeDescriptor->copy(address(lhs), address(rhs));
}

void StructProperty::swap(void *lhs, void *rhs) const
{
    m_typeDescriptor->swap(address(lhs), address(rhs));
}

char *StructProperty::address(void *p) const
{
    return reinterpret_cast<char*>((char*)p + offset());
}

const char *StructProperty::address(const void *p) const
{
    return reinterpret_cast<const char*>((const char*)p + offset());
}

std::size_t StructProperty::offset() const
{
    return m_offset;
}

const Descriptor *StructProperty::td() const
{
    return m_typeDescriptor;
}

Tag StructProperty::tag() const
{
    return m_tag;
}

void StructProperty::clear(void *lhs) const
{
    m_typeDescriptor->clear(address(lhs));
}


}
}
