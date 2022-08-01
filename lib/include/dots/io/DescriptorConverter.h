// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <dots/type/Registry.h>
#include <EnumDescriptorData.dots.h>
#include <StructDescriptorData.dots.h>

namespace dots::io
{
    struct DescriptorConverter
    {
        DescriptorConverter(type::Registry& registry);

        type::EnumDescriptor& operator () (const types::EnumDescriptorData& enumData) const;
        type::StructDescriptor& operator () (const types::StructDescriptorData& structData) const;

        types::EnumDescriptorData operator () (const type::EnumDescriptor& enumDescriptor);
        types::StructDescriptorData operator () (const type::StructDescriptor& structDescriptor);

    private:

        std::reference_wrapper<type::Registry> m_registry;
    };
}
