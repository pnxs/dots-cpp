// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <dots/type/PropertyOffset.h>

namespace dots::type
{
    namespace details
    {
        struct ProtoBase
        {
            // intentionally empty
        };

        struct ProtoStruct : ProtoBase
        {
            void* _descriptor = nullptr;
        };

        template <typename T>
        struct ProtoStaticProperty : ProtoBase
        {
            std::aligned_storage_t<sizeof(T), alignof(T)> t;
        };

        struct ProtoStaticStruct : ProtoStruct
        {
            ProtoStaticProperty<PropertyArea> _propertyArea;
        };
    }

    struct StaticPropertyOffset : PropertyOffset
    {
        static constexpr StaticPropertyOffset MakeOffset(const details::ProtoBase* propertyArea, const details::ProtoBase* property)
        {
            static_assert(sizeof(details::ProtoBase) == 1);
            return StaticPropertyOffset{ static_cast<size_t>(property - propertyArea) };
        }

    private:

        constexpr StaticPropertyOffset(size_t offset) :
            PropertyOffset(std::in_place, offset)
        {
            /* do nothing */
        }
    };
}
