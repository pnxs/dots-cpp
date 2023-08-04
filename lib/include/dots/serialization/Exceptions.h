// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <stdexcept>
#include <boost/algorithm/hex.hpp>

namespace dots::serialization
{
    struct serializerException : public std::runtime_error
    {
        serializerException(const std::string& arg, std::vector<uint8_t> inputBuffer) : runtime_error(arg), m_inputBuffer(std::move(inputBuffer))
        {
            parseInputBuffer();
        };

        [[nodiscard]] const char* what() const noexcept override
        {
            return m_final.c_str();
        }

    private:
        std::vector<uint8_t> m_inputBuffer;
        std::string m_final;
        static int const printLength = 16;

        /*!
         * @brief convert binary buffer for easier readability
         */
        void parseInputBuffer()
        {
            m_final += std::runtime_error::what();
            m_final += "\n";

            auto itStart = m_inputBuffer.begin();
            auto itAdvance = itStart;
            std::ranges::advance(itAdvance, printLength, m_inputBuffer.end());
            for (; itStart < m_inputBuffer.end();)
            {
                static int spacer = 0;
                static int proceeded = 0;
                for (auto it = itStart; it < itAdvance; it++)
                {
                    m_final += boost::algorithm::hex_lower(std::string(it, it + 1));
                    m_final += " ";

                    if (spacer == printLength / 2 - 1)
                    {
                        m_final += " ";
                    }
                    spacer++;
                }
                spacer = 0;

                if (proceeded != 0)
                {
                    m_final += std::string(proceeded * 3, ' ');
                }

                m_final += "\t";

                for (auto it = itStart; it < itAdvance; it++)
                {
                    m_final += (*it < 32) ? '.' : *it;
                    if (spacer == printLength / 2 - 1)
                    {
                        m_final += " ";
                    }
                    spacer++;
                }
                spacer = 0;

                m_final += "\n";
                itStart = itAdvance;
                proceeded = std::ranges::advance(itAdvance, printLength, m_inputBuffer.end());
            }
        }
    };
}