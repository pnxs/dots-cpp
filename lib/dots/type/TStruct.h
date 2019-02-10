#pragma once
#include <string_view>
#include <type_traits>
#include "dots/io/Subscription.h"
#include "StructDescriptor.h"
#include "Struct.h"
#include "Registry.h"

namespace dots
{
	template<class T>
	struct Cbd;
}

namespace dots::type
{
	template <typename, typename, typename, typename>
	struct TProperty;

    template <typename Derived>
    struct TStruct : Struct
    {
		using Cbd = dots::Cbd<Derived>;

		template <typename... PropertyInitializers>
        TStruct(PropertyInitializers&&... propertyInitializers) : Struct(_Descriptor())
        {
			(strip_t<decltype(propertyInitializers)>::property_t::Get(*this)(std::forward<decltype(propertyInitializers)>(propertyInitializers)), ...);
        }
        TStruct(const TStruct& other) = default;
        TStruct(TStruct&& other) = default;
        ~TStruct() = default;

        TStruct& operator = (const TStruct& rhs) = default;
        TStruct& operator = (TStruct&& rhs) = default;

		bool operator == (const Derived& rhs) const
		{
			return _applyKeyPropertyPairs(rhs, [](const auto&... propertyPairs) { return ((propertyPairs.first == propertyPairs.second) && ...); });
		}

		bool operator != (const Derived& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const Derived& rhs) const
		{
			return _applyKeyPropertyPairs(rhs, [](const auto&... propertyPairs) { return ((propertyPairs.first < propertyPairs.second) && ...); });
		}

		auto _properties()
		{
			return std::apply([this](auto&&... args)
			{
				return std::forward_as_tuple(strip_t<decltype(args)>::Get(*this)...);
			}, typename Derived::_properties_t{});
		}

		auto _properties() const
		{
			return std::apply([this](auto&&... args)
			{
				return std::forward_as_tuple(strip_t<decltype(args)>::Get(*this)...);
			}, typename Derived::_properties_t{});
		}

		auto _keyProperties()
		{
			return std::apply([this](auto&&... args)
			{
				return std::forward_as_tuple(strip_t<decltype(args)>::Get(*this)...);
			}, typename Derived::_key_properties_t{});
		}

		auto _keyProperties() const
		{
			return std::apply([this](auto&&... args)
			{
				return std::forward_as_tuple(strip_t<decltype(args)>::Get(*this)...);
			}, typename Derived::_key_properties_t{});
		}

		auto _propertyPairs(const Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
			}, typename Derived::_properties_t{});
		}

		auto _propertyPairs(const Derived& other) const
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
			}, typename Derived::_properties_t{});
		}

		auto _keyPropertyPairs(const Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
			}, typename Derived::_key_properties_t{});
		}

		auto _keyPropertyPairs(const Derived& other) const
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
			}, typename Derived::_key_properties_t{});
		}

		template <typename Callable>
		auto _applyProperties(Callable&& callable)
		{
			return std::apply(std::forward<Callable>(callable), _properties());
		}

		template <typename Callable>
		auto _applyProperties(Callable&& callable) const
		{
			return std::apply(std::forward<Callable>(callable), _properties());
		}

		template <typename Callable>
		auto _applyKeyProperties(Callable&& callable)
		{
			return std::apply(std::forward<Callable>(callable), _keyProperties());
		}

		template <typename Callable>
		auto _applyKeyProperties(Callable&& callable) const
		{
			return std::apply(std::forward<Callable>(callable), _keyProperties());
		}

		template <typename Callable>
		auto _applyPropertyPairs(const Derived& other, Callable&& callable)
		{
			return std::apply(std::forward<Callable>(callable), _propertyPairs(other));
		}

		template <typename Callable>
		auto _applyPropertyPairs(const Derived& other, Callable&& callable) const
		{
			return std::apply(std::forward<Callable>(callable), _propertyPairs(other));
		}

		template <typename Callable>
		auto _applyKeyPropertyPairs(const Derived& other, Callable&& callable)
		{
			return std::apply(std::forward<Callable>(callable), _keyPropertyPairs(other));
		}

		template <typename Callable>
		auto _applyKeyPropertyPairs(const Derived& other, Callable&& callable) const
		{
			return std::apply(std::forward<Callable>(callable), _keyPropertyPairs(other));
		}

		void _publish(const property_set& what = PROPERTY_SET_ALL, bool remove = false) const
		{
			static_assert(!_IsSubstructOnly(), "a substruct-only type cannot be published");

			registerTypeUsage<Derived, PublishedType>();
			Struct::_publish();
		}

		void _remove() const
		{
			static_assert(!_IsSubstructOnly(), "a substruct-only type cannot be removed");

			registerTypeUsage<Derived, PublishedType>();
			Struct::_remove();
		}

        static const StructDescriptor& _Descriptor()
        {
			static const StructDescriptor* structDescriptor = Descriptor::registry().findStructDescriptor(Derived::Description.name.data());

			if (structDescriptor == nullptr)
			{
				std::vector<PropertyDescription> propertyDescriptions = std::apply([](auto&&... args)
				{
					(type::getDescriptor<typename strip_t<decltype(args)>::value_t>(nullptr), ...);

					return std::vector<PropertyDescription>{
						PropertyDescription{
							strip_t<decltype(args)>::Description
						}... };
				}, typename Derived::_properties_t{});
				
				structDescriptor = MakeStructDescriptor(Derived::Description);
			}

			return *structDescriptor;
        }

		static constexpr property_set _KeyPropertySet()
		{
			constexpr property_set KeyPropertySet = std::apply([](auto&&... args)
			{
				return (strip_t<decltype(args)>::PropertySet() | ... );
			}, typename Derived::_key_properties_t{});

			return KeyPropertySet;
		}

		static constexpr bool _IsCached()
		{
			return Derived::Description.flags & Cached;
		}

		static constexpr bool _IsInternal()
		{
			return Derived::Description.flags & Internal;
		}

		static constexpr bool _IsPersistent()
		{
			return Derived::Description.flags & Persistent;
		}

		static constexpr bool _IsCleanup()
		{
			return Derived::Description.flags & Cleanup;
		}

		static constexpr bool _IsLocal()
		{
			return Derived::Description.flags & Local;
		}

		static constexpr bool _IsSubstructOnly()
		{
			return Derived::Description.flags & SubstructOnly;
		}

    protected:
		
		template <typename... PropertyDescriptions>
		static constexpr StructDescription MakeStructDescription(const std::string_view& name, uint8_t flags, PropertyDescriptions&&... propertyDescriptions)
		{
			return StructDescription{ name, flags, { std::forward<PropertyDescriptions>(propertyDescriptions)... }, sizeof...(PropertyDescriptions) };
		}

    private:

		using Struct::_publish;
		using Struct::_remove;

		template <typename, typename, typename, typename>
		friend struct TProperty;

		template <typename T>
		using strip_t = std::remove_pointer_t<std::decay_t<T>>;
    };
}