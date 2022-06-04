#include <dots/type/StaticDescriptor.h>
#include <cassert>
#include <dots/type/FundamentalTypes.h>

namespace dots::type
{
    template <typename Callable, typename... Typelesses>
    decltype(auto) apply(Type type, Callable&& callable, Typelesses&&... typelesses)
    {
        assert(type <= Type::string);

        switch (type)
        {
            case Type::boolean:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::bool_t>()...);
            case Type::int8:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::int8_t>()...);
            case Type::uint8:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::uint8_t>()...);
            case Type::int16:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::int16_t>()...);
            case Type::uint16:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::uint16_t>()...);
            case Type::int32:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::int32_t>()...);
            case Type::uint32:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::uint32_t>()...);
            case Type::int64:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::int64_t>()...);
            case Type::uint64:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::uint64_t>()...);
            case Type::float32:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::float32_t>()...);
            case Type::float64:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::float64_t>()...);
            case Type::property_set:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::property_set_t>()...);
            case Type::timepoint:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::timepoint_t>()...);
            case Type::steady_timepoint:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::steady_timepoint_t>()...);
            case Type::duration:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::duration_t>()...);
            case Type::uuid:
                return callable(std::forward<Typelesses>(typelesses).template to<dots::uuid_t>()...);
            case Type::string:
            case Type::Vector:
            case Type::Struct:
            case Type::Enum:
            default: 
                return callable(std::forward<Typelesses>(typelesses).template to<dots::string_t>()...);
        }
    }

    Typeless& StaticDescriptor::construct(Typeless& value) const
    {
        return apply(type(), [](auto& value) -> Typeless&
        {
            return Typeless::From(construct(value));
        }, value);
    }

    Typeless& StaticDescriptor::construct(Typeless& value, const Typeless& other) const
    {
        return apply(type(), [](auto& value, const auto& other) -> Typeless&
        {
            return Typeless::From(construct(value, other));
        }, value, other);
    }

    Typeless& StaticDescriptor::construct(Typeless& value, Typeless&& other) const
    {
        return apply(type(), [](auto& value, auto&& other) -> Typeless&
        {
            return Typeless::From(construct(value, std::forward<decltype(other)>(other)));
        }, value, std::move(other));
    }

    Typeless& StaticDescriptor::constructInPlace(Typeless& value) const
    {
        return apply(type(), [](auto& value) -> Typeless&
        {
            return Typeless::From(constructInPlace(value));
        }, value);
    }

    Typeless& StaticDescriptor::constructInPlace(Typeless& value, const Typeless& other) const
    {
        return apply(type(), [](auto& value, const auto& other) -> Typeless&
        {
            return Typeless::From(constructInPlace(value, other));
        }, value, other);
    }

    Typeless& StaticDescriptor::constructInPlace(Typeless& value, Typeless&& other) const
    {
        return apply(type(), [](auto& value, auto&& other) -> Typeless&
        {
            return Typeless::From(constructInPlace(value, std::forward<decltype(other)>(other)));
        }, value, std::move(other));
    }

    void StaticDescriptor::destruct(Typeless& value) const
    {
        apply(type(), [](auto& value) -> void
        {
            destruct(value);
        }, value);
    }

    Typeless& StaticDescriptor::assign(Typeless& lhs, const Typeless& rhs) const
    {
        return apply(type(), [](auto& lhs, const auto& rhs) -> Typeless&
        {
            return Typeless::From(assign(lhs, rhs));
        }, lhs, rhs);
    }

    Typeless& StaticDescriptor::assign(Typeless& lhs, Typeless&& rhs) const
    {
        return apply(type(), [](auto& lhs, auto&& rhs) -> Typeless&
        {
            return Typeless::From(assign(lhs, std::forward<decltype(rhs)>(rhs)));
        }, lhs, std::move(rhs));
    }

    void StaticDescriptor::swap(Typeless& value, Typeless& other) const
    {
        return apply(type(), [](auto& value, auto& other) -> void
        {
            swap(value, other);
        }, value, other);
    }

    bool StaticDescriptor::equal(const Typeless& lhs, const Typeless& rhs) const
    {
        return apply(type(), [](const auto& lhs, const auto& rhs) -> bool
        {
            return equal(lhs, rhs);
        }, lhs, rhs);
    }

    bool StaticDescriptor::less(const Typeless& lhs, const Typeless& rhs) const
    {
        return apply(type(), [](const auto& lhs, const auto& rhs) -> bool
        {
            return less(lhs, rhs);
        }, lhs, rhs);
    }
}
