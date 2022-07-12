// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/type/AnyStruct.h>

namespace dots::type
{
    AnyStruct::AnyStruct(const StructDescriptor& descriptor):
        _instance{ static_cast<Struct*>(::operator new(descriptor.size())) }
    {
        descriptor.constructInPlace(Typeless::From(*_instance));
    }
}
