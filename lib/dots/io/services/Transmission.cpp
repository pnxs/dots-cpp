#include "Transmission.h"

namespace dots
{
	Transmission::Transmission(type::AnyStruct&& instance) :
        m_id(++M_LastId),
        m_instance(std::move(instance))
    {
        /* do nothing */
    }

	auto Transmission::id() const -> id_t
    {
        return m_id;
    }

    const type::AnyStruct& Transmission::instance() const
    {
        return m_instance;
    }
}