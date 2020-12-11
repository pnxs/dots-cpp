#include <dots/io/Transmission.h>

namespace dots::io
{
    Transmission::Transmission(DotsHeader header, type::AnyStruct instance) :
        m_data{ std::make_unique<TransmissionData>(TransmissionData{ ++M_LastId, std::move(header), std::move(instance) }) }
    {
        /* do nothing */
    }

    auto Transmission::id() const -> id_t
    {
        return m_data->id;
    }

    const DotsHeader& Transmission::header() const&
    {
        return m_data->header;
    }

    DotsHeader& Transmission::header() &
    {
        return m_data->header;
    }

    DotsHeader Transmission::header() &&
    {
        return DotsHeader{ std::move(m_data->header) };
    }

    const type::AnyStruct& Transmission::instance() const&
    {
        return m_data->instance;
    }

    type::AnyStruct Transmission::instance() &&
    {
        return type::AnyStruct{ std::move(m_data->instance) };
    }
}