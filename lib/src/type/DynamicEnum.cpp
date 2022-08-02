// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/type/DynamicEnum.h>

namespace dots::type
{
    Descriptor<DynamicEnum>::Descriptor(key_t key, std::string name, std::vector<EnumeratorDescriptor> enumeratorDescriptors):
        EnumDescriptor(key, std::move(name), std::move(enumeratorDescriptors))
    {
        /* do nothing */
    }
}
