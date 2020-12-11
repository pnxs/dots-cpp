#pragma once
#include <dots/type/PropertyOffset.h>

namespace dots::type
{
    namespace details
    {
        struct ProtoStaticStruct : Struct
        {
            ProtoStaticStruct(const StructDescriptor<>& descriptor) : Struct(descriptor) {}
            PropertyArea m_propertyArea;
        };

        template <typename P>
        struct ProtoStruct : ProtoStaticStruct
        {
            P p1;
        };

        template <typename P>
        constexpr bool is_tightly_packed_v = sizeof(ProtoStruct<P>) == sizeof(ProtoStaticStruct);

        template <typename P>
        constexpr size_t initial_padding_v = is_tightly_packed_v<P> ? 0 : sizeof(ProtoStaticStruct) - sizeof(Struct) - sizeof(PropertyArea);

        template <typename P>
        constexpr size_t initial_offset_v = sizeof(PropertyArea) + initial_padding_v<P>;
    }

    template <typename T>
    struct StaticPropertyOffset : PropertyOffset
    {
        static constexpr StaticPropertyOffset<T> First()
        {
            return StaticPropertyOffset<T>{ 0, details::initial_offset_v<T> };
        }

        template <typename U>
        static constexpr StaticPropertyOffset<T> Next(const StaticPropertyOffset<U>& previous)
        {
            return StaticPropertyOffset<T>{ previous, sizeof(U) }; 
        }

        StaticPropertyOffset(const StaticPropertyOffset& other) = default;
        StaticPropertyOffset(StaticPropertyOffset&& other) = default;
        ~StaticPropertyOffset() = default;

        StaticPropertyOffset& operator = (const StaticPropertyOffset& rhs) = default;
        StaticPropertyOffset& operator = (StaticPropertyOffset&& rhs) = default;

    private:

        constexpr StaticPropertyOffset(size_t previousOffset, size_t previousSize) :
            PropertyOffset(alignof(T), previousOffset, previousSize)
        {
            /* do nothing */
        }
    };
}