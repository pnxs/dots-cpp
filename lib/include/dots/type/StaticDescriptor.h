#pragma once
#include <type_traits>
#include <sstream>
#include <iomanip>
#include <charconv>
#include <dots/type/Descriptor.h>
#include <dots/type/DescriptorMap.h>

namespace dots::type
{
    inline DescriptorMap& static_descriptors()
    {
        static DescriptorMap StaticDescriptors;
        return StaticDescriptors;
    }

    struct StaticDescriptor : Descriptor<>
    {
        StaticDescriptor(key_t key, Type type, std::string name, size_t size, size_t alignment) : Descriptor(key, type, std::move(name), size, alignment)
        {
            /* do nothing */
        }

        StaticDescriptor(const StaticDescriptor& other) = delete;
        StaticDescriptor(StaticDescriptor&& other) = delete;
        ~StaticDescriptor() = default;

        StaticDescriptor& operator = (const StaticDescriptor& rhs) = delete;
        StaticDescriptor& operator = (StaticDescriptor&& rhs) = delete;

        using Descriptor<>::construct;
        using Descriptor<>::constructInPlace;
        using Descriptor<>::destruct;
        using Descriptor<>::assign;
        using Descriptor<>::swap;
        using Descriptor<>::equal;
        using Descriptor<>::less;
        using Descriptor<>::lessEqual;
        using Descriptor<>::greater;
        using Descriptor<>::greaterEqual;
        using Descriptor<>::dynamicMemoryUsage;

        Typeless& construct(Typeless& value) const override;
        Typeless& construct(Typeless& value, const Typeless& other) const override;
        Typeless& construct(Typeless& value, Typeless&& other) const override;

        Typeless& constructInPlace(Typeless& value) const override;
        Typeless& constructInPlace(Typeless& value, const Typeless& other) const override;
        Typeless& constructInPlace(Typeless& value, Typeless&& other) const override;

        void destruct(Typeless& value) const override;

        Typeless& assign(Typeless& lhs) const override;
        Typeless& assign(Typeless& lhs, const Typeless& rhs) const override;
        Typeless& assign(Typeless& lhs, Typeless&& rhs) const override;

        void swap(Typeless& value, Typeless& other) const override;
        bool equal(const Typeless& lhs, const Typeless& rhs) const override;
        bool less(const Typeless& lhs, const Typeless& rhs) const override;

        template <typename T>
        static Descriptor<T>& InitInstance()
        {
            if constexpr (is_dynamic_descriptor_v<Descriptor<T>>)
            {
                throw std::logic_error{ "global descriptor not available because Descriptor<T> dynamic" };
            }
            else
            {
                static auto& Instance = static_descriptors().emplace<Descriptor<T>>();
                return Instance;
            }
        }
    };
}
