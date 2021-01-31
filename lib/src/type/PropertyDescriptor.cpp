#include <dots/type/PropertyDescriptor.h>
#include <dots/type/Struct.h>
#include <dots/type/PropertyArea.h>

namespace dots::type
{
    PropertyDescriptor::PropertyDescriptor(std::shared_ptr<Descriptor<>> descriptor, std::string name, uint32_t tag, bool isKey, PropertyOffset offset):
        m_descriptor{ descriptor },
        m_name{ std::move(name) },
        m_tag(tag),
        m_isKey(isKey),
        m_set{ PropertySet::FromIndex(m_tag) },
        m_offset{ std::move(offset) }
    {
        if (m_descriptor->type() == Type::Struct)
        {
            m_subAreaOffset.emplace(std::in_place, static_cast<const StructDescriptor<>&>(*m_descriptor).areaOffset());
        }
    }

    char* PropertyDescriptor::address(void* p) const
    {
        return reinterpret_cast<char*>(&reinterpret_cast<Struct*>(p)->_propertyArea()) + offset();
    }

    const char* PropertyDescriptor::address(const void* p) const
    {
        return reinterpret_cast<const char*>(&reinterpret_cast<const Struct*>(p)->_propertyArea()) + offset();
    }
}