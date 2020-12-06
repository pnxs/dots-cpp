#pragma once
#include <memory>
#include <dots/type/AnyStruct.h>
#include <DotsHeader.dots.h>

namespace dots::io
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

        const DotsHeader& header() const&;
        DotsHeader& header() &;
        DotsHeader header() &&;

        const type::AnyStruct& instance() const&;
        type::AnyStruct instance() &&;

        template <size_t I>
        const auto& get() const &
        {
            if constexpr (I == 0)
            {
                return header();
            }
            else if constexpr (I == 1)
            {
                return instance();
            }
        }

        template <size_t I>
        auto& get() &
        {
            if constexpr (I == 0)
            {
                return header();
            }
            else if constexpr (I == 1)
            {
                return instance();
            }
        }

        template <size_t I>
        auto get() &&
        {
            if constexpr (I == 0)
            {
                return std::move(*this).header();
            }
            else if constexpr (I == 1)
            {
                return std::move(*this).instance();
            }
        }

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

namespace std
{
    template <> struct tuple_size<dots::io::Transmission> : std::integral_constant<size_t, 2> {};
    template <> struct tuple_element<0, dots::io::Transmission> { using type = DotsHeader; };
    template <> struct tuple_element<1, dots::io::Transmission> { using type = dots::type::AnyStruct; };
}