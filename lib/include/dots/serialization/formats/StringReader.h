// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <dots/serialization/formats/TextReader.h>
#include <dots/serialization/formats/StringFormat.h>

namespace dots::serialization
{
    using StringReader = TextReader<StringFormat>;
}
