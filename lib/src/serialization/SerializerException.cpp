// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/serialization/SerializerException.h>
#include <fmt/core.h>

namespace dots::serialization
{
    SerializerException::SerializerException(const std::string& msg, const std::string& detail, std::size_t offset, std::vector<uint8_t> inputBuffer)
        : runtime_error(fmt::format("{} at offset {}. {}", msg, offset, detail))
    {
        parseInputBuffer(inputBuffer);
    };


    [[nodiscard]] const char* SerializerException::what() const noexcept
    {
        return m_exceptionMessage.c_str();
    }

    /*!
     * @brief convert binary buffer for easier readability
     *
     * @param buffer input stream buffer of malicious object
     */
    void SerializerException::parseInputBuffer(std::vector<uint8_t> buffer)
    {
        m_exceptionMessage = fmt::format("{}\n", std::runtime_error::what());

        for (auto itStartOfLine = buffer.begin(); itStartOfLine < buffer.end();
             std::ranges::advance(itStartOfLine, printLength, buffer.end()))
        {
            auto itEndOfLine = itStartOfLine;
            std::ranges::advance(itEndOfLine, printLength, buffer.end());
            std::string lineHex;
            std::string lineASCII;

            int spacer = 0;
            for (auto it = itStartOfLine; it < itEndOfLine; it++)
            {
                lineHex += fmt::format("{:02x} ", *it);
                lineASCII += (*it < 32) || (*it > 126) ? "." : fmt::format("{:c}", *it);

                spacer++;
                if (spacer == printLength / 2)
                {
                    lineHex += " ";
                    lineASCII += " ";
                }
            }

            m_exceptionMessage += fmt::format("{0:{1}}\t{2}\n", lineHex, printLength * 3 + 1, lineASCII);
        }
    }

}