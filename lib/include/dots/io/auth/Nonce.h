// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <string_view>

namespace dots::io
{
    struct Nonce
    {
        using value_t = uint64_t;

        Nonce();
        Nonce(value_t value);
        Nonce(std::string_view stringValue);

        const value_t& value() const;
        std::string toString() const;

    private:

        value_t m_value;
    };
}
