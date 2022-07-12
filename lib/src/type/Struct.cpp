// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/type/Struct.h>
#include <dots/type/StructDescriptor.h>

namespace dots::type
{
    size_t Struct::_totalMemoryUsage() const
    {
        return _staticMemoryUsage() + _dynamicMemoryUsage();
    }
}
