// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <stdexcept>
#include <span>

namespace dots::serialization
{
    struct SerializerException : public std::runtime_error
    {
        SerializerException(const std::string& msg, const std::string& detail, std::size_t offset, std::span<const uint8_t> inputBuffer);

        [[nodiscard]] const char* what() const noexcept override;

    private:
        std::string m_exceptionMessage;
    };
}