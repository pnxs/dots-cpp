// SPDX-License-Identifier: LGPL-3.0-only
#pragma once
#include <dots/type/AnyObject.h>

namespace dots::type
{
    struct Struct;

    // Consume one deterministic CBOR key array, rejecting incomplete identities.
    // Returns the byte count, allowing a reference to be read from a larger frame.
    size_t instance_key_size(const uint8_t* data, size_t size, bool allowNull = false);

    class InstanceRef
    {
    public:
        InstanceRef() = default;
        InstanceRef(std::string typeName, std::vector<uint8_t> key);
        std::string_view typeName() const { return m_typeName; }
        const std::vector<uint8_t>& key() const { return m_key; }
        bool empty() const { return m_typeName.empty(); }
        std::string toString() const { return AnyObject{m_typeName, m_key}.toString(); }
        static InstanceRef FromString(std::string_view text);
        bool operator==(const InstanceRef& rhs) const { return m_typeName == rhs.m_typeName && m_key == rhs.m_key; }
        bool operator!=(const InstanceRef& rhs) const { return !(*this == rhs); }
        bool operator<(const InstanceRef& rhs) const { return m_typeName != rhs.m_typeName ? m_typeName < rhs.m_typeName : m_key < rhs.m_key; }
    private:
        std::string m_typeName;
        std::vector<uint8_t> m_key{0x80};
    };

    template <typename T>
    class TypedInstanceRef : public InstanceRef
    {
    public:
        TypedInstanceRef() = default;
        explicit TypedInstanceRef(std::vector<uint8_t> key) : InstanceRef(std::string{T::_Name}, std::move(key))
        {
            static_assert(std::is_base_of_v<Struct, T>, "instance_ref target must be a struct");
        }
        explicit TypedInstanceRef(const InstanceRef& ref) : InstanceRef(ref)
        {
            if (ref.typeName() != T::_Name) throw std::invalid_argument{"instance_ref target type mismatch"};
        }
        static TypedInstanceRef FromString(std::string_view text) { return TypedInstanceRef{InstanceRef::FromString(text)}; }
    };
}

namespace std
{
    template <> struct hash<dots::type::InstanceRef>
    {
        size_t operator()(const dots::type::InstanceRef& value) const noexcept
        {
            size_t h = hash<string_view>{}(value.typeName());
            for (auto byte : value.key()) h = dots::tools::hashCombine(h, hash<uint8_t>{}(byte));
            return h;
        }
    };
    template <typename T> struct hash<dots::type::TypedInstanceRef<T>> : hash<dots::type::InstanceRef> {};
}

namespace dots::type
{
    namespace detail
    {
        template <typename Reference, typename Base>
        struct InstanceRefOperations : Base
        {
            using Base::Base;
            Typeless& construct(Typeless& value) const override
            {
                return Typeless::From(StaticDescriptor::construct(value.to<Reference>()));
            }

            Typeless& construct(Typeless& value, const Typeless& other) const override
            {
                return Typeless::From(StaticDescriptor::construct(value.to<Reference>(), other.to<Reference>()));
            }

            Typeless& construct(Typeless& value, Typeless&& other) const override
            {
                return Typeless::From(StaticDescriptor::construct(value.to<Reference>(), std::move(other).to<Reference>()));
            }

            Typeless& constructInPlace(Typeless& value) const override
            {
                return this->construct(value);
            }

            Typeless& constructInPlace(Typeless& value, const Typeless& other) const override
            {
                return this->construct(value, other);
            }

            Typeless& constructInPlace(Typeless& value, Typeless&& other) const override
            {
                return this->construct(value, std::move(other));
            }

            void destruct(Typeless& value) const override
            {
                StaticDescriptor::destruct(value.to<Reference>());
            }

            Typeless& assign(Typeless& lhs) const override
            {
                return Typeless::From(StaticDescriptor::assign(lhs.to<Reference>()));
            }

            Typeless& assign(Typeless& lhs, const Typeless& rhs) const override
            {
                return Typeless::From(StaticDescriptor::assign(lhs.to<Reference>(), rhs.to<Reference>()));
            }

            Typeless& assign(Typeless& lhs, Typeless&& rhs) const override
            {
                return Typeless::From(StaticDescriptor::assign(lhs.to<Reference>(), std::move(rhs).to<Reference>()));
            }

            void swap(Typeless& value, Typeless& other) const override
            {
                StaticDescriptor::swap(value.to<Reference>(), other.to<Reference>());
            }

            bool equal(const Typeless& lhs, const Typeless& rhs) const override
            {
                return StaticDescriptor::equal(lhs.to<Reference>(), rhs.to<Reference>());
            }

            bool less(const Typeless& lhs, const Typeless& rhs) const override
            {
                return StaticDescriptor::less(lhs.to<Reference>(), rhs.to<Reference>());
            }

            size_t hash(const Typeless& value) const override
            {
                return StaticDescriptor::hash(value.to<Reference>());
            }

            using Base::dynamicMemoryUsage;

            bool usesDynamicMemory() const override
            {
                return true;
            }

            size_t dynamicMemoryUsage(const Typeless& value) const override
            {
                const auto& ref = value.to<Reference>();
                return ref.typeName().size() + ref.key().size();
            }
        };
    }

    template <> struct Descriptor<InstanceRef> : detail::InstanceRefOperations<InstanceRef, StaticDescriptor>
    {
        using base_t = detail::InstanceRefOperations<InstanceRef, StaticDescriptor>;
        Descriptor(key_t key, std::string target = {}) :
            base_t(key, Type::InstanceRef, target.empty() ? "instance_ref" : "instance_ref<" + target + ">", sizeof(InstanceRef), alignof(InstanceRef)),
            m_target(std::move(target)) {}
        static auto& Instance() { return InitInstance<InstanceRef>(); }
        const std::string& targetTypeName() const { return m_target; }
        bool isFundamentalType() const override { return m_target.empty(); }
        virtual InstanceRef& reference(Typeless& value) const { return value.to<InstanceRef>(); }
        virtual const InstanceRef& reference(const Typeless& value) const { return value.to<InstanceRef>(); }
    private:
        std::string m_target;
    };

    template <typename T> struct Descriptor<TypedInstanceRef<T>> : detail::InstanceRefOperations<TypedInstanceRef<T>, Descriptor<InstanceRef>>
    {
        using base_t = detail::InstanceRefOperations<TypedInstanceRef<T>, Descriptor<InstanceRef>>;
        Descriptor(typename base_t::key_t key) : base_t(key, std::string{T::_Name})
        {
            static_assert(std::is_base_of_v<Struct, T>, "instance_ref target must be a struct");
            static_assert(sizeof(TypedInstanceRef<T>) == sizeof(InstanceRef));
            static_assert(alignof(TypedInstanceRef<T>) == alignof(InstanceRef));
        }
        static auto& Instance() { return StaticDescriptor::InitInstance<TypedInstanceRef<T>>(); }
        InstanceRef& reference(Typeless& value) const override { return value.to<TypedInstanceRef<T>>(); }
        const InstanceRef& reference(const Typeless& value) const override { return value.to<TypedInstanceRef<T>>(); }
    };
}

namespace dots::types
{
    using instance_ref_t = type::InstanceRef;
    template <typename T> using typed_instance_ref_t = type::TypedInstanceRef<T>;
}
namespace dots
{
    using types::instance_ref_t;
    using types::typed_instance_ref_t;
}
