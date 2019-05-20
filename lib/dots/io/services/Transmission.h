#pragma once
#include <cstdint>
#include <dots/type/AnyStruct.h>

namespace dots
{
	struct Transmission
	{
        using id_t = uint64_t;

		explicit Transmission(type::AnyStruct&& instance);
		Transmission(const Transmission& other) = delete;
		Transmission(Transmission&& other) = default;
		~Transmission() = default;

		Transmission& operator = (const Transmission& rhs) = delete;
		Transmission& operator = (Transmission&& rhs) = default;

		id_t id() const;
		const type::AnyStruct& instance() const;

    private:

        inline static id_t M_LastId = 0;

        id_t m_id;
		type::AnyStruct m_instance;
	};
}