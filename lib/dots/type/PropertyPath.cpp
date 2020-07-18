#include <dots/type/PropertyPath.h>

namespace dots::type
{
    PropertyPath::PropertyPath(elements_t elements):
        m_elements{ std::move(elements) },
        m_offset(0)
    {
        for (size_t i = 0; i < m_elements.size() - 1; ++i)
        {
            const PropertyDescriptor& descriptor = m_elements[i];
            m_offset += descriptor.offset();
            m_offset += *descriptor.subAreaOffset();
        }

        m_offset += m_elements[m_elements.size() - 1].get().offset();
    }

    PropertyPath::PropertyPath(const PropertyDescriptor& descriptor):
        PropertyPath(elements_t{ std::ref(descriptor) })
    {
        /* do nothing */
    }

    const PropertyPath::elements_t& PropertyPath::elements() const
    {
        return m_elements;
    }

    size_t PropertyPath::offset() const
    {
        return m_offset;
    }

    const PropertyDescriptor& PropertyPath::destination() const
    {
        return m_elements.back();
    }
}
