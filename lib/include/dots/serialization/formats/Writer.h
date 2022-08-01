// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once

namespace dots::serialization
{
    template <typename Data>
    struct Writer
    {
        using data_t = Data;
        using value_t = typename data_t::value_type;

        const data_t& output() const
        {
            return m_output;
        }

        data_t& output()
        {
            return m_output;
        }

    private:

        data_t m_output;
    };
}
