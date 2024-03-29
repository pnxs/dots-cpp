// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/type/Descriptor.h>

namespace dots::type
{
    Descriptor<Typeless>::Descriptor(key_t key, Type type, std::string name, size_t size, size_t alignment) :
        shared_ptr_only(key),
        m_type(type),
        m_name(std::move(name)),
        m_size(size),
        m_alignment(alignment)
    {
        /* do nothing */
    }

    bool Descriptor<Typeless>::isFundamentalType() const
    {
        return IsFundamentalType(type());
    }

    bool Descriptor<Typeless>::lessEqual(const Typeless& lhs, const Typeless& rhs) const
    {
        return !greater(lhs, rhs);
    }

    bool Descriptor<Typeless>::greater(const Typeless& lhs, const Typeless& rhs) const
    {
        return less(rhs, lhs);
    }

    bool Descriptor<Typeless>::greaterEqual(const Typeless& lhs, const Typeless& rhs) const
    {
        return !less(lhs, rhs);
    }

    bool Descriptor<Typeless>::usesDynamicMemory() const
    {
        return false;
    }

    size_t Descriptor<Typeless>::dynamicMemoryUsage(const Typeless&/* value*/) const
    {
        return 0;
    }

    bool Descriptor<Typeless>::IsFundamentalType(const Descriptor& descriptor)
    {
        return descriptor.isFundamentalType();
    }

    bool Descriptor<Typeless>::IsFundamentalType(Type type)
    {
        switch (type)
        {
            case Type::boolean:
            case Type::int8:
            case Type::int16:
            case Type::int32:
            case Type::int64:
            case Type::uint8:
            case Type::uint16:
            case Type::uint32:
            case Type::uint64:
            case Type::float32:
            case Type::float64:
            case Type::property_set:
            case Type::timepoint:
            case Type::steady_timepoint:
            case Type::duration:
            case Type::string:
            case Type::uuid:
                return true;

            case Type::Vector:
            case Type::Enum:
            case Type::Struct:
                return false;
        }

        return false;
    }
}
