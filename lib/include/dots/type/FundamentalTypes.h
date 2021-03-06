#pragma once
#include <dots/type/StaticDescriptor.h>
#include <dots/type/PropertySet.h>
#include <dots/type/Chrono.h>
#include <dots/type/Uuid.h>
#include <dots/type/Vector.h>
#include <dots/type/VectorDescriptor.h>

namespace dots::types
{
    using bool_t             = bool;

    using int8_t             = std::int8_t;
    using uint8_t            = std::uint8_t;
    using int16_t            = std::int16_t;
    using uint16_t           = std::uint16_t;
    using int32_t            = std::int32_t;
    using uint32_t           = std::uint32_t;
    using int64_t            = std::int64_t;
    using uint64_t           = std::uint64_t;

    using float32_t          = float;
    using float64_t          = double;

    using property_set_t     = type::PropertySet;

    using timepoint_t        = type::TimePoint;
    using steady_timepoint_t = type::SteadyTimePoint;
    using duration_t         = type::Duration;

    using uuid_t             = type::Uuid;
    using string_t           = std::string;

    template <typename T>
    using vector_t           = type::Vector<T>;

    template <typename T = void>
    struct float128_t
    {
        static_assert(std::is_base_of_v<T, T>, "float128 type is currently not supported");
    };

    namespace literals
    {
        using namespace type::literals;
    }
}

namespace dots::type
{
    template <>
    struct Descriptor<types::bool_t> : StaticDescriptor<types::bool_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::boolean, "bool") {}
    };

    template <>
    struct Descriptor<types::int8_t> : StaticDescriptor<types::int8_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::int8, "int8") {}
    };

    template <>
    struct Descriptor<types::uint8_t> : StaticDescriptor<types::uint8_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::uint8, "uint8") {}
    };

    template <>
    struct Descriptor<types::int16_t> : StaticDescriptor<types::int16_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::int16, "int16") {}
    };

    template <>
    struct Descriptor<types::uint16_t> : StaticDescriptor<types::uint16_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::uint16, "uint16") {}
    };

    template <>
    struct Descriptor<types::int32_t> : StaticDescriptor<types::int32_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::int32, "int32") {}
    };

    template <>
    struct Descriptor<types::uint32_t> : StaticDescriptor<types::uint32_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::uint32, "uint32") {}
    };

    template <>
    struct Descriptor<types::int64_t> : StaticDescriptor<types::int64_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::int64, "int64") {}
    };

    template <>
    struct Descriptor<types::uint64_t> : StaticDescriptor<types::uint64_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::uint64, "uint64") {}
    };

    template <>
    struct Descriptor<types::float32_t> : StaticDescriptor<types::float32_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::float32, "float32") {}
    };

    template <>
    struct Descriptor<types::float64_t> : StaticDescriptor<types::float64_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::float64, "float64") {}
    };

    template <>
    struct Descriptor<types::property_set_t> : StaticDescriptor<types::property_set_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::property_set, "property_set") {}
    };

    template <>
    struct Descriptor<types::timepoint_t> : StaticDescriptor<types::timepoint_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::timepoint, "timepoint") {}
    };

    template <>
    struct Descriptor<types::steady_timepoint_t> : StaticDescriptor<types::steady_timepoint_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::steady_timepoint, "steady_timepoint") {}
    };

    template <>
    struct Descriptor<types::duration_t> : StaticDescriptor<types::duration_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::duration, "duration") {}
    };

    template <>
    struct Descriptor<types::uuid_t> : StaticDescriptor<types::uuid_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::uuid, "uuid") {}
    };

    template <>
    struct Descriptor<types::string_t> : StaticDescriptor<types::string_t>
    {
        Descriptor(key_t key) : StaticDescriptor(key, Type::string, "string") {}

        bool usesDynamicMemory() const override
        {
            return true;
        }

        size_t dynamicMemoryUsage(const Typeless& value) const override
        {
            return dynamicMemoryUsage(value.to<types::string_t>());
        }

        size_t dynamicMemoryUsage(const types::string_t& value) const
        {
            return value.empty() ? 0 : value.size() + 1;
        }
    };
}

#ifndef DOTS_NOT_USE_FUNDAMENTAL_TYPES
namespace dots
{
    using types::bool_t;

    using types::int8_t;
    using types::uint8_t;
    using types::int16_t;
    using types::uint16_t;
    using types::int32_t;
    using types::uint32_t;
    using types::int64_t;
    using types::uint64_t;

    using types::float32_t;
    using types::float64_t;

    using types::property_set_t;

    using types::timepoint_t;
    using types::steady_timepoint_t;
    using types::duration_t;

    using types::uuid_t;
    using types::string_t;

    using types::vector_t;
}
#endif