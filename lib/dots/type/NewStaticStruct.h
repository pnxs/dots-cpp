#pragma once
#include <type_traits>
#include <utility>
#include <dots/type/NewStruct.h>
#include <dots/type/TPropertyInitializer.h>
#include <dots/io/Subscription.h>

namespace dots
{
	template<typename T>
	struct Event;
}

namespace dots::type
{
    template <typename Derived>
    struct NewStaticStruct : NewStruct
    {
		using Cbd = dots::Event<Derived>;

		template <typename... PropertyInitializers>
		explicit NewStaticStruct(PropertyInitializers&&... propertyInitializers) : NewStruct(_Descriptor())
		{
			static_assert(std::conjunction_v<is_t_property_initializer_t<strip_t<PropertyInitializers>>...>, "a struct can only be constructed by its property initializers");
			(getProperty<strip_t<typename strip_t<PropertyInitializers>::property_t>>().construct(std::forward<decltype(propertyInitializers.value)>(propertyInitializers.value)), ...);
		}

		bool operator == (const Derived& rhs) const
		{
			return _equal(rhs);
		}

		bool operator != (const Derived& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const Derived& rhs) const
		{
			return _less(rhs);
		}

    	bool operator <= (const Derived& rhs) const
		{
			return _lessEqual(rhs);
		}

    	bool operator > (const Derived& rhs) const
		{
			return _greater(rhs);
		}

    	bool operator >= (const Derived& rhs) const
		{
			return _greaterEqual(rhs);
		}

		auto _properties()
		{
			return std::apply([this](auto&&... args)
			{
				return std::forward_as_tuple(getProperty<strip_t<decltype(args)>>()...);
			}, typename Derived::_properties_t{});
		}

		auto _properties() const
		{
			return std::apply([this](auto&&... args)
			{
				return std::forward_as_tuple(getProperty<strip_t<decltype(args)>>()...);
			}, typename Derived::_properties_t{});
		}

		auto _keyProperties()
		{
			return std::apply([this](auto&&... args)
			{
				return std::forward_as_tuple(getProperty<strip_t<decltype(args)>>()...);
			}, typename Derived::_key_properties_t{});
		}

		auto _keyProperties() const
		{
			return std::apply([this](auto&&... args)
			{
				return std::forward_as_tuple(getProperty<strip_t<decltype(args)>>()...);
			}, typename Derived::_key_properties_t{});
		}

		auto _propertyPairs(Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<strip_t<decltype(args)>&, strip_t<decltype(args)>&>(getProperty<strip_t<decltype(args)>>(), other.template getProperty<strip_t<decltype(args)>>())...);
			}, typename Derived::_properties_t{});
		}

		auto _propertyPairs(const Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<strip_t<decltype(args)>&, const strip_t<decltype(args)>&>(getProperty<strip_t<decltype(args)>>(), other.template getProperty<strip_t<decltype(args)>>())...);
			}, typename Derived::_properties_t{});
		}

		auto _propertyPairs(const Derived& other) const
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<const strip_t<decltype(args)>&, const strip_t<decltype(args)>&>(getProperty<strip_t<decltype(args)>>(), other.template getProperty<strip_t<decltype(args)>>())...);
			}, typename Derived::_properties_t{});
		}

		auto _keyPropertyPairs(Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<strip_t<decltype(args)>&, strip_t<decltype(args)>&>(getProperty<strip_t<decltype(args)>>(), other.template getProperty<strip_t<decltype(args)>>())...);
			}, typename Derived::_key_properties_t{});
		}

		auto _keyPropertyPairs(const Derived& other)
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<strip_t<decltype(args)>&, const strip_t<decltype(args)>&>(getProperty<strip_t<decltype(args)>>(), other.template getProperty<strip_t<decltype(args)>>())...);
			}, typename Derived::_key_properties_t{});
		}

		auto _keyPropertyPairs(const Derived& other) const
		{
			return std::apply([this, &other](auto&&... args)
			{
				return std::make_tuple(std::pair<const strip_t<decltype(args)>&, const strip_t<decltype(args)>&>(getProperty<strip_t<decltype(args)>>(), other.template getProperty<strip_t<decltype(args)>>())...);
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

		Derived& _assign(const Derived& other, const NewPropertySet& includedProperties = NewPropertySet::All)
		{
			_applyPropertyPairs(other, [&](const auto&... propertyPairs)
			{
				auto assign = [&](auto& propertyThis, auto& propertyOther)
				{
					if (strip_t<decltype(propertyThis)>::IsPartOf(includedProperties))
					{
						propertyThis = propertyOther;
					}
					else
					{
						propertyThis.destroy();
					}
				};

				(assign(propertyPairs.first, propertyPairs.second), ...);
			});

			return static_cast<Derived&>(*this);
		}

		Derived& _copy(const Derived& other, const NewPropertySet& includedProperties = NewPropertySet::All)
		{
			_applyPropertyPairs(other, [&](const auto&... propertyPairs)
			{
				auto copy = [&](auto& propertyThis, auto& propertyOther)
				{
					if (strip_t<decltype(propertyThis)>::IsPartOf(includedProperties))
					{
						propertyThis = propertyOther;
					}
				};

				(copy(propertyPairs.first, propertyPairs.second), ...);
			});

			return static_cast<Derived&>(*this);
		}

		Derived& _merge(const Derived& other, const NewPropertySet& includedProperties = NewPropertySet::All)
		{
			_applyPropertyPairs(other, [&](const auto&... propertyPairs)
			{
				auto merge = [&](auto& propertyThis, auto& propertyOther)
				{
					if (strip_t<decltype(propertyOther)>::IsPartOf(includedProperties) && propertyOther.isValid())
					{
						propertyThis = propertyOther;
					}
				};

				(merge(propertyPairs.first, propertyPairs.second), ...);
			});

			return static_cast<Derived&>(*this);
		}

		void _swap(Derived& other, const NewPropertySet& includedProperties = NewPropertySet::All)
		{
			_applyPropertyPairs(other, [&](const auto&... propertyPairs)
			{
				auto swap = [&](auto& propertyThis, auto& propertyOther)
				{
					if (strip_t<decltype(propertyThis)>::IsPartOf(includedProperties))
					{
						propertyThis.swap(propertyOther);
					}
				};

				(swap(propertyPairs.first, propertyPairs.second), ...);
			});
		}

		void _clear(const NewPropertySet& includedProperties = NewPropertySet::All)
		{
			_applyProperties([&](auto&... properties)
			{
				auto destroy = [&](auto& property)
				{
					if (strip_t<decltype(property)>::IsPartOf(includedProperties))
					{
						property.destroy();
					}
				};

				(destroy(properties), ...);
			});
		}

		bool _equal(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const
		{
			return _applyPropertyPairs(rhs, [&](const auto&... propertyPairs)
			{
				auto equal = [&](auto& propertyThis, auto& propertyOther)
				{
					if (strip_t<decltype(propertyThis)>::IsPartOf(includedProperties))
					{
						return propertyThis == propertyOther;
					}
					else
					{
						return true;
					}
				};

				return (equal(propertyPairs.first, propertyPairs.second) && ...);
			});
		}

    	bool _same(const Derived& rhs) const
		{
			return _applyKeyPropertyPairs(rhs, [&](const auto&... propertyPairs)
			{
				if constexpr (sizeof...(propertyPairs) == 0)
				{
					return true;
				}
				else
				{
					auto equal = [&](auto& propertyThis, auto& propertyOther)
					{
						return propertyThis == propertyOther;
					};

					return (equal(propertyPairs.first, propertyPairs.second) && ...);
				}
			});
		}

		bool _less(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const
		{
			return _applyPropertyPairs(rhs, [&](const auto&... propertyPairs)
			{
				return _less(includedProperties, propertyPairs...);
			});
		}

    	bool _lessEqual(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const
		{
			return !_greater(rhs, includedProperties);
		}

    	bool _greater(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const
		{
			return rhs._less(*this, includedProperties);
		}

    	bool _greaterEqual(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const
		{
			return !_less(rhs, includedProperties);
		}

		NewPropertySet _diffProperties(const Derived& other, const NewPropertySet& includedProperties = NewPropertySet::All) const
		{
			NewPropertySet symmetricDiff = _validProperties().symmetricDifference(other._validProperties()) ^ includedProperties;
			NewPropertySet intersection = _validProperties() ^ other._validProperties() ^ includedProperties;

			if (!intersection.empty())
			{
				_applyPropertyPairs(other, [&](const auto&... propertyPairs)
				{
					auto check_inequality = [&](auto& propertyThis, auto& propertyOther)
					{
						using property_t = strip_t<decltype(propertyThis)>;

						if (property_t::IsPartOf(intersection) && !property_t::ValueDescriptor().equal(propertyThis, propertyOther))
						{
							symmetricDiff += property_t::Set();
						}
					};

					return (check_inequality(propertyPairs.first, propertyPairs.second), ...);
				});
			}

			return symmetricDiff;
		}

    	template <typename P>
    	const P& getProperty() const
		{
			return _propertyArea().getProperty<P>();
		}

    	template <typename P>
		P& getProperty()
		{
			return const_cast<P&>(std::as_const(*this).template getProperty<P>());
		}

    	static const NewDescriptor<Derived>& _Descriptor()
        {
			return NewDescriptor<Derived>::Instance();
        }

		static NewPropertySet _KeyProperties()
		{
			static NewPropertySet KeyProperties = std::apply([](auto&&... args)
			{
				if constexpr (sizeof...(args) == 0)
				{
					return NewPropertySet{};
				}
				else
				{
					return (strip_t<decltype(args)>::Set() | ...);
				}
			}, typename Derived::_key_properties_t{});

			return KeyProperties;
		}

    protected:

		NewStaticStruct(const NewStaticStruct& other) = default;
		NewStaticStruct(NewStaticStruct&& other) = default;
		~NewStaticStruct() = default;

		NewStaticStruct& operator = (const NewStaticStruct& rhs) = default;
		NewStaticStruct& operator = (NewStaticStruct&& rhs) = default;

    private:

    	using NewStruct::_assign;
		using NewStruct::_copy;
		using NewStruct::_merge;
		using NewStruct::_swap;
		using NewStruct::_clear;
    	
    	using NewStruct::_equal;
		using NewStruct::_same;
    	
        using NewStruct::_less;
    	using NewStruct::_lessEqual;
    	using NewStruct::_greater;
    	using NewStruct::_greaterEqual;

    	using NewStruct::_diffProperties;

		/*using NewStruct::_publish;
		using NewStruct::_remove;*/

    	template <typename PropertyPair, typename... PropertyPairs>
    	bool _less(const NewPropertySet& includedProperties, const PropertyPair& firstPair, const PropertyPairs&... remainingPairs) const
    	{
			if (includedProperties.empty())
    		{
    			return false;
    		}
    		
			// this is a property pair version of the less operation as it is defined for std::tuple: lhsHead < rhsHead || (!(rhsHead < lhsHead) && lhsTail < rhsTail)
    		
    		auto less = [&](auto& propertyThis, auto& propertyOther)
			{
    			return propertyThis.IsPartOf(includedProperties) && propertyThis < propertyOther;
			};

    		if constexpr (sizeof...(remainingPairs) == 0)
			{
				return less(firstPair.first, firstPair.second);
			}
			else
			{
				return less(firstPair.first, firstPair.second) || (!less(firstPair.second, firstPair.first) && _less(includedProperties, remainingPairs...));
			}
    	}

		template <typename T>
		using strip_t = std::remove_pointer_t<std::decay_t<T>>;
    };
}