// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <dots/type/PropertyDescriptor.h>

namespace dots::type
{
    struct PropertyPath
    {
        using elements_t = std::vector<std::reference_wrapper<const PropertyDescriptor>>;

        PropertyPath(elements_t elements) :
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

        PropertyPath(const PropertyDescriptor& descriptor):
            PropertyPath(elements_t{ std::ref(descriptor) })
        {
            /* do nothing */
        }

        const elements_t& elements() const
        {
            return m_elements;
        }

        size_t offset() const
        {
            return m_offset;
        }

        const PropertyDescriptor& destination() const
        {
            return m_elements.back();
        }

    private:

        elements_t m_elements;
        size_t m_offset;
    };
}
