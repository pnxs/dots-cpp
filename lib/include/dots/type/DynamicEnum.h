#pragma once
#include <limits>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/EnumDescriptor.h>

namespace dots::type
{
    enum class DynamicEnum : int32_t
    {
        Min = std::numeric_limits<int32_t>::min(),
        Max = std::numeric_limits<int32_t>::max()
    };

    template <>
    struct Descriptor<DynamicEnum> : EnumDescriptor<DynamicEnum, false>
    {
        using value_t = DynamicEnum;
        static_assert(std::is_same_v<details::underlying_type_t<DynamicEnum>, int32_t>);

        static constexpr bool IsDynamic = true;

        Descriptor(std::string name, std::vector<EnumeratorDescriptor<DynamicEnum>> enumeratorDescriptors);
        Descriptor(const Descriptor& other) = delete;
        Descriptor(Descriptor&& other) = default;
        ~Descriptor() = default;

        Descriptor& operator = (const Descriptor& rhs) = delete;
        Descriptor& operator = (Descriptor&& rhs) = default;
    };
}