// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/io/auth/Digest.h>
#include <random>
#include <iomanip>
#include <sstream>

namespace dots::io
{
    Nonce::Nonce() :
        Nonce(std::mt19937_64{ std::random_device{}() }())
    {
        /* do nothing */
    }

    Nonce::Nonce(value_t value) :
        m_value(value)
    {
        /* do nothing */
    }

    Nonce::Nonce(std::string_view stringValue) :
        Nonce([&]
        {
            value_t value;
            std::istringstream iss{ stringValue.data() };
            iss >> std::setw(16) >> std::hex >> value;

            return value;
        }())
    {
        /* do nothing */
    }

    auto Nonce::value() const -> const value_t&
    {
        return m_value;
    }

    std::string Nonce::toString() const
    {
        std::ostringstream oss;
        oss << std::setw(16) << std::setfill('0') << std::hex << m_value;

        return oss.str();
    }
}
