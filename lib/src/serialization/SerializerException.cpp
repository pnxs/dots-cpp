// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/serialization/SerializerException.h>
#include <fmt/core.h>
#include <dots/fmt/hexdump.h>

namespace dots::serialization
{
    SerializerException::SerializerException(const std::string& msg, const std::string& detail, std::size_t offset, std::span<const uint8_t> inputBuffer)
        : runtime_error(fmt::format("{} at offset {}. {}", msg, offset, detail))
    {
        m_exceptionMessage = fmt::format("{}\n{}\n", std::runtime_error::what(), fmt::extension::format_hexdump(inputBuffer));
    };


    [[nodiscard]] const char* SerializerException::what() const noexcept
    {
        return m_exceptionMessage.c_str();
    }
}