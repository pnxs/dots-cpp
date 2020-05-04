#pragma once
#include <memory>
#include <dots/type/AnyStruct.h>
#include <DotsHeader.dots.h>

namespace dots
{
	struct Transmission
	{
        using id_t = uint64_t;

		Transmission(DotsHeader header, type::AnyStruct instance);
		Transmission(const Transmission& other) = delete;
		Transmission(Transmission&& other) = default;
		~Transmission() = default;

		Transmission& operator = (const Transmission& rhs) = delete;
		Transmission& operator = (Transmission&& rhs) = default;

		id_t id() const;

		const DotsHeader& header() const;
		DotsHeader& header();

		const type::AnyStruct& instance() const;

    private:

        inline static id_t M_LastId = 0;

        struct TransmissionData
        {
            id_t id;
			DotsHeader header;
		    type::AnyStruct instance;
        };

		std::unique_ptr<TransmissionData> m_data;
	};
}