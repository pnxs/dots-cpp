// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <functional>
#include <dots/type/StaticDescriptor.h>

namespace dots::type
{
    /*!
     * @brief Storage for an arbitrary DOTS object held inside a single property.
     *
     * AnyObject is the runtime representation of the IDL `any` (and `variant`)
     * field type. It is intentionally serialization-agnostic and decode-free:
     * it stores only the contained type's identity (its name) and the object's
     * canonical-CBOR payload as opaque bytes. This is what lets brokers route
     * such fields without ever needing the contained type's descriptor.
     *
     * Conversion to and from a live dots::type::Struct is performed by the
     * free helpers dots::to_any()/dots::from_any() in the serialization layer
     * (where both AnyObject and CBOR are visible), not here, to avoid a
     * layering inversion.
     */
    struct AnyObject
    {
        AnyObject() = default;

        AnyObject(std::string typeName, std::vector<uint8_t> payload) :
            m_typeName{ std::move(typeName) },
            m_payload{ std::move(payload) }
        {
            /* do nothing */
        }

        AnyObject(const AnyObject& other) = default;
        AnyObject(AnyObject&& other) = default;
        ~AnyObject() = default;

        AnyObject& operator = (const AnyObject& rhs) = default;
        AnyObject& operator = (AnyObject&& rhs) = default;

        std::string_view typeName() const
        {
            return m_typeName;
        }

        const std::vector<uint8_t>& payload() const
        {
            return m_payload;
        }

        bool empty() const
        {
            return m_typeName.empty();
        }

        bool operator == (const AnyObject& rhs) const
        {
            return m_typeName == rhs.m_typeName && m_payload == rhs.m_payload;
        }

        bool operator != (const AnyObject& rhs) const
        {
            return !(*this == rhs);
        }

        bool operator < (const AnyObject& rhs) const
        {
            return m_typeName != rhs.m_typeName ? m_typeName < rhs.m_typeName : m_payload < rhs.m_payload;
        }

    private:

        std::string m_typeName;
        std::vector<uint8_t> m_payload;
    };
}

namespace dots::types
{
    using any_t = type::AnyObject;
}

namespace std
{
    template <>
    struct hash<dots::type::AnyObject>
    {
        size_t operator () (const dots::type::AnyObject& value) const noexcept
        {
            size_t h = std::hash<std::string_view>{}(value.typeName());
            for (uint8_t byte : value.payload())
            {
                h ^= std::hash<uint8_t>{}(byte) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            }
            return h;
        }
    };
}

namespace dots::type
{
    template <>
    struct Descriptor<types::any_t> : StaticDescriptor
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::Any, "any", sizeof(types::any_t), alignof(types::any_t)) {}
        static auto& Instance() { return InitInstance<types::any_t>(); }

        // StaticDescriptor's generic apply() only dispatches Type values <= string,
        // so Type::Any must override the typeless operations to act on AnyObject
        // directly (same pattern as VectorDescriptor).

        Typeless& construct(Typeless& value) const override
        {
            return Typeless::From(StaticDescriptor::construct(value.to<types::any_t>()));
        }

        Typeless& construct(Typeless& value, const Typeless& other) const override
        {
            return Typeless::From(StaticDescriptor::construct(value.to<types::any_t>(), other.to<types::any_t>()));
        }

        Typeless& construct(Typeless& value, Typeless&& other) const override
        {
            return Typeless::From(StaticDescriptor::construct(value.to<types::any_t>(), std::move(other).to<types::any_t>()));
        }

        Typeless& constructInPlace(Typeless& value) const override
        {
            return construct(value);
        }

        Typeless& constructInPlace(Typeless& value, const Typeless& other) const override
        {
            return construct(value, other);
        }

        Typeless& constructInPlace(Typeless& value, Typeless&& other) const override
        {
            return construct(value, std::move(other));
        }

        void destruct(Typeless& value) const override
        {
            StaticDescriptor::destruct(value.to<types::any_t>());
        }

        Typeless& assign(Typeless& lhs) const override
        {
            return Typeless::From(StaticDescriptor::assign(lhs.to<types::any_t>()));
        }

        Typeless& assign(Typeless& lhs, const Typeless& rhs) const override
        {
            return Typeless::From(StaticDescriptor::assign(lhs.to<types::any_t>(), rhs.to<types::any_t>()));
        }

        Typeless& assign(Typeless& lhs, Typeless&& rhs) const override
        {
            return Typeless::From(StaticDescriptor::assign(lhs.to<types::any_t>(), std::move(rhs).to<types::any_t>()));
        }

        void swap(Typeless& value, Typeless& other) const override
        {
            StaticDescriptor::swap(value.to<types::any_t>(), other.to<types::any_t>());
        }

        bool equal(const Typeless& lhs, const Typeless& rhs) const override
        {
            return StaticDescriptor::equal(lhs.to<types::any_t>(), rhs.to<types::any_t>());
        }

        bool less(const Typeless& lhs, const Typeless& rhs) const override
        {
            return StaticDescriptor::less(lhs.to<types::any_t>(), rhs.to<types::any_t>());
        }

        size_t hash(const Typeless& value) const override
        {
            return StaticDescriptor::hash(value.to<types::any_t>());
        }

        // re-expose the base template overload (dynamicMemoryUsage(const T&)),
        // which the Typeless override below would otherwise hide -- VectorDescriptor
        // calls valueDescriptor().dynamicMemoryUsage(element) with a typed element.
        using StaticDescriptor::dynamicMemoryUsage;

        bool usesDynamicMemory() const override
        {
            return true;
        }

        size_t dynamicMemoryUsage(const Typeless& value) const override
        {
            const auto& any = value.to<types::any_t>();
            return any.typeName().size() + any.payload().size();
        }
    };
}

namespace dots
{
    using types::any_t;
}
