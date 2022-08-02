// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once

namespace dots::serialization
{
    template <typename Data>
    struct Reader
    {
        using data_t = Data;
        using value_t = typename data_t::value_type;

        void setInput(const value_t* inputData, size_t inputDataSize)
        {
            m_inputData = inputData;
            m_inputDataEnd = m_inputData + inputDataSize;
            m_inputDataBegin = m_inputData;
        }

        void setInput(const data_t& input)
        {
            setInput(input.data(), input.size());
        }

        void setInput(data_t&& input) = delete;

        const value_t* inputData() const
        {
            return m_inputData;
        }

        const value_t*& inputData()
        {
            return m_inputData;
        }

        const value_t* inputDataBegin() const
        {
            return m_inputDataBegin;
        }

        const value_t*& inputDataBegin()
        {
            return m_inputDataBegin;
        }

        const value_t* inputDataEnd() const
        {
            return m_inputDataEnd;
        }

        const value_t*& inputDataEnd()
        {
            return m_inputDataEnd;
        }

        size_t inputOffset() const
        {
            return m_inputData - m_inputDataBegin;
        }

        size_t inputAvailable() const
        {
            return m_inputDataEnd - m_inputData;
        }

    private:

        const value_t* m_inputData = nullptr;
        const value_t* m_inputDataBegin = nullptr;
        const value_t* m_inputDataEnd = nullptr;
    };
}
