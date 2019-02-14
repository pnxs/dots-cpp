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
		explicit TStruct(PropertyInitializers&&... propertyInitializers) : Struct(_Descriptor())
		{
			(strip_t<decltype(propertyInitializers)>::property_t::Get(*this)(std::forward<decltype(propertyInitializers)>(propertyInitializers)), ...);
		}

		bool operator == (const Derived& rhs) const
		{
			return _applyKeyPropertyPairs(rhs, [](const auto&... propertyPairs)
			{
				if constexpr (sizeof...(propertyPairs) == 0)
				{
					return true;
				}
				else
				{
					return ((propertyPairs.first == propertyPairs.second) && ...);
				}
			});
		}

		bool operator != (const Derived& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const Derived& rhs) const
		{
			return _applyKeyPropertyPairs(rhs, [](const auto&... propertyPairs)
			{
				if constexpr (sizeof...(propertyPairs) == 0)
				{
					return false;
				}
				else
				{
					return ((propertyPairs.first < propertyPairs.second) && ...);
				}
			});
		}

		void swap(Derived& other, const property_set& what = PROPERTY_SET_ALL)
		{
			_applyPropertyPairs(other, [&](const auto&... propertyPairs)
			{
				auto swap = [&](auto& propertyThis, auto& propertyOther)
				{
					if (strip_t<decltype(propertyThis)>::IsPartOf(what))
					{
						propertyThis.swap(propertyOther);
					}
				};

				(swap(propertyPairs.first, propertyPairs.second), ...);
			});
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

		auto _propertyPairs(Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<strip_t<decltype(args)>&, strip_t<decltype(args)>&>(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
			}, typename Derived::_properties_t{});
		}

		auto _propertyPairs(const Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<strip_t<decltype(args)>&, const strip_t<decltype(args)>&>(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
			}, typename Derived::_properties_t{});
		}

		auto _propertyPairs(const Derived& other) const
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<const strip_t<decltype(args)>&, const strip_t<decltype(args)>&>(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
			}, typename Derived::_properties_t{});
		}

		auto _keyPropertyPairs(Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<strip_t<decltype(args)>&, strip_t<decltype(args)>&>(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
			}, typename Derived::_key_properties_t{});
		}

		auto _keyPropertyPairs(const Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<strip_t<decltype(args)>&, const strip_t<decltype(args)>&>(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
			}, typename Derived::_key_properties_t{});
		}

		auto _keyPropertyPairs(const Derived& other) const
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<const strip_t<decltype(args)>&, const strip_t<decltype(args)>&>(strip_t<decltype(args)>::Get(*this), strip_t<decltype(args)>::Get(other))...);
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
		auto _applyPropertyPairs(Derived& other, Callable&& callable)
		{
			return std::apply(std::forward<Callable>(callable), _propertyPairs(other));
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
		auto _applyKeyPropertyPairs(Derived& other, Callable&& callable)
		{
			return std::apply(std::forward<Callable>(callable), _keyPropertyPairs(other));
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
			Struct::_publish(what, remove);
		}

		void _remove(const property_set& what = PROPERTY_SET_ALL) const
		{
			static_assert(!_IsSubstructOnly(), "a substruct-only type cannot be removed");

			registerTypeUsage<Derived, PublishedType>();
			Struct::_remove(what);
		}

		property_set _diffPropertySet(const Derived& other) const
		{
			property_set symmetricDiff = _validPropertySet().value() ^ other._validPropertySet().value();
			property_set intersection = _validPropertySet() & other._validPropertySet();
			
			if (!intersection.empty())
			{
				_applyPropertyPairs(other, [&](const auto&... propertyPairs)
				{
					auto check_inequality = [&](auto& propertyThis, auto& propertyOther)
					{
						using property_t = strip_t<decltype(propertyThis)>;

						if (property_t::IsPartOf(intersection) && property_t::Descriptor().equal(&*propertyThis, &*propertyOther))
						{
							symmetricDiff |= property_t::PropertySet();
						}
					};

					return (check_inequality(propertyPairs.first, propertyPairs.second), ...);
				});
			}

			return symmetricDiff;
		}

        static const StructDescriptor& _Descriptor()
        {
			static const StructDescriptor* structDescriptor = Descriptor::registry().findStructDescriptor(Derived::Description.name.data());

			if (structDescriptor == nullptr)
			{
				// allocate space for descriptor to avoid recursive descriptor construction issues
				StructDescriptor* structDescriptorAddr = reinterpret_cast<StructDescriptor*>(std::malloc(sizeof(StructDescriptor)));
				structDescriptor = structDescriptorAddr;

				// create descriptors of property types if they not already exist
				std::apply([](auto&&... args)
				{
					(strip_t<decltype(args)>::Descriptor(), ...);
				}, typename Derived::_properties_t{});

				// create descriptor of derived type				
				MakeStructDescriptor(structDescriptorAddr, Derived::Description);
			}

			return *structDescriptor;
        }

		static constexpr property_set _KeyPropertySet()
		{
			constexpr property_set KeyPropertySet = std::apply([](auto&&... args)
			{
				if constexpr (sizeof...(args) == 0)
				{
					return property_set{};
				}
				else
				{
					return (strip_t<decltype(args)>::PropertySet() | ...);
				}
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

		TStruct(const TStruct& other) = default;
		TStruct(TStruct&& other) = default;
		~TStruct() = default;

		TStruct& operator = (const TStruct& rhs) = default;
		TStruct& operator = (TStruct&& rhs) = default;
		
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

	template <typename Derived>
	std::ostream& operator << (std::ostream& os, const TStruct<Derived>& instance)
	{
		os << instance._Descriptor().to_string(instance);
		return os;
	}
}