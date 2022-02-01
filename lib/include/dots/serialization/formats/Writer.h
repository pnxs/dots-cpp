#pragma once

namespace dots::serialization
{
    template <typename Data>
    struct Writer
    {
        using data_t = Data;
        using value_t = typename data_t::value_type;

        Writer() = default;
        Writer(const Writer& other) = default;
        Writer(Writer&& other) = default;
        ~Writer() = default;

        Writer& operator = (const Writer& rhs) = default;
        Writer& operator = (Writer&& rhs) = default;

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
