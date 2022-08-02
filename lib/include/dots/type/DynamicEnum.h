// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
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
    struct Descriptor<DynamicEnum> : EnumDescriptor
    {
        using value_t = DynamicEnum;
        static_assert(std::is_same_v<std::underlying_type_t<DynamicEnum>, int32_t>);

        static constexpr bool IsDynamic = true;

        Descriptor(key_t key, std::string name, std::vector<EnumeratorDescriptor> enumeratorDescriptors);
        Descriptor(const Descriptor& other) = delete;
        Descriptor(Descriptor&& other) = delete;
        ~Descriptor() override = default;

        Descriptor& operator = (const Descriptor& rhs) = delete;
        Descriptor& operator = (Descriptor&& rhs) = delete;

        static auto& Instance()
        {
            return InitInstance<DynamicEnum>();
        }
    };
}
