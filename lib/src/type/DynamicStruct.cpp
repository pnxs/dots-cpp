// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/type/DynamicStruct.h>
#include <dots/type/FundamentalTypes.h>
#include <memory>

namespace dots::type
{
    DynamicStruct::DynamicStruct(const Descriptor<DynamicStruct>& descriptor) :
        Struct(descriptor),
        m_propertyAreaStorage{ std::unique_ptr<PropertyArea>{ static_cast<PropertyArea*>(::operator new(descriptor.size() - sizeof(DynamicStruct))) } },
        m_propertyArea(m_propertyAreaStorage.get())
    {
        ::new(static_cast<void*>(m_propertyArea)) PropertyArea{};
    }

    DynamicStruct::DynamicStruct(const Descriptor<DynamicStruct>& descriptor, PropertyArea* propertyArea) :
        Struct(descriptor),
        m_propertyArea{ propertyArea }
    {
        ::new(static_cast<void*>(m_propertyArea)) PropertyArea{};
    }
}
