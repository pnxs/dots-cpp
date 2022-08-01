// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/type/VectorDescriptor.h>

namespace dots::type
{
    Descriptor<Vector<>>::Descriptor(key_t key, std::string name, Descriptor<>& valueDescriptor, size_t size, size_t alignment):
        StaticDescriptor(key, Type::Vector, std::move(name), size, alignment),
        m_valueDescriptor(valueDescriptor.shared_from_this())
    {
        /* do nothing */
    }

    const Descriptor<Typeless>& Descriptor<Vector<Typeless>>::valueDescriptor() const
    {
        return *m_valueDescriptor;
    }

    Descriptor<Typeless>& Descriptor<Vector<Typeless>>::valueDescriptor()
    {
        return *m_valueDescriptor;
    }
}
