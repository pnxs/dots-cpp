// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <array>
#include <string_view>

namespace dots::type
{
    struct Uuid
    {
        using value_t = std::array<uint8_t, 16>;

        Uuid() = default;
        Uuid(const uint8_t data[16]);
        Uuid(const value_t& data);

        const value_t& data() const;

        bool operator == (const Uuid&) const;
        bool operator < (const Uuid& rhs) const;
        bool operator != (const Uuid&) const;

        std::string toString() const;

        static Uuid FromString(std::string_view value);
        static Uuid FromData(std::string_view data);
        static Uuid Random();

    private:

        value_t m_data;
    };
}
