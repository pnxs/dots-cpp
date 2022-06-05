#include <dots/type/PropertyDescriptor.h>
#include <dots/type/StructDescriptor.h>

namespace dots::type
{
    PropertyDescriptor::PropertyDescriptor(Descriptor<>& descriptor, std::string name, uint32_t tag, bool isKey, PropertyOffset offset):
        m_descriptor{ descriptor.shared_from_this() },
        m_name{ std::move(name) },
        m_tag(tag),
        m_isKey(isKey),
        m_set{ PropertySet::FromIndex(m_tag) },
        m_offset{ offset }
    {
        if (const auto* structDescriptor = m_descriptor->as<StructDescriptor>(); structDescriptor != nullptr)
        {
            m_subAreaOffset.emplace(std::in_place, structDescriptor->areaOffset());
        }
    }
}
