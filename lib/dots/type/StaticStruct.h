#pragma once
#include <type_traits>
#include <utility>
#include <dots/type/Struct.h>
#include <dots/type/PropertyInitializer.h>
#include <dots/io/Subscription.h>

namespace dots
{
	template<typename T>
	struct Event;
}

namespace dots::type
{
    template <typename Derived>
    struct StaticStruct : Struct
    {
		using Cbd = dots::Event<Derived>;

    	StaticStruct() : Struct(_Descriptor())
    	{
    		/* do nothing */
    	}

		template <typename... PropertyInitializers, std::enable_if_t<sizeof...(PropertyInitializers) >= 1 && std::conjunction_v<is_property_initializer_t<std::remove_pointer_t<std::decay_t<PropertyInitializers>>>...>, int> = 0>
		explicit StaticStruct(PropertyInitializers&&... propertyInitializers) : Struct(_Descriptor())
		{
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

		template <typename Callable>
		auto _applyProperties(Callable&& callable)
		{
			return _applyProperties(std::forward<Callable>(callable), typename Derived::_properties_t{});
		}

		template <typename Callable>
		auto _applyProperties(Callable&& callable) const
		{
			return _applyProperties(std::forward<Callable>(callable), typename Derived::_properties_t{});
		}

    	template <typename Callable>
		auto _applyKeyProperties(Callable&& callable)
		{
			return _applyProperties(std::forward<Callable>(callable), typename Derived::_key_properties_t{});
		}

		template <typename Callable>
		auto _applyKeyProperties(Callable&& callable) const
		{
			return _applyProperties(std::forward<Callable>(callable), typename Derived::_key_properties_t{});
		}

		template <typename Callable>
		auto _applyPropertyPairs(Derived& other, Callable&& callable)
		{
			return _applyPropertyPairs(other, std::forward<Callable>(callable), typename Derived::_properties_t{});
		}

		template <typename Callable>
		auto _applyPropertyPairs(const Derived& other, Callable&& callable)
		{
			return _applyPropertyPairs(other, std::forward<Callable>(callable), typename Derived::_properties_t{});
		}

		template <typename Callable>
		auto _applyPropertyPairs(const Derived& other, Callable&& callable) const
		{
			return _applyPropertyPairs(other, std::forward<Callable>(callable), typename Derived::_properties_t{});
		}

    	template <typename Callable>
		auto _applyKeyPropertyPairs(Derived& other, Callable&& callable)
		{
			return _applyPropertyPairs(other, std::forward<Callable>(callable), typename Derived::_key_properties_t{});
		}

		template <typename Callable>
		auto _applyKeyPropertyPairs(const Derived& other, Callable&& callable)
		{
			return _applyPropertyPairs(other, std::forward<Callable>(callable), typename Derived::_key_properties_t{});
		}

		template <typename Callable>
		auto _applyKeyPropertyPairs(const Derived& other, Callable&& callable) const
		{
			return _applyPropertyPairs(other, std::forward<Callable>(callable), typename Derived::_key_properties_t{});
		}

		Derived& _assign(const Derived& other, const PropertySet& includedProperties = PropertySet::All)
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

		Derived& _copy(const Derived& other, const PropertySet& includedProperties = PropertySet::All)
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

		Derived& _merge(const Derived& other, const PropertySet& includedProperties = PropertySet::All)
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

		void _swap(Derived& other, const PropertySet& includedProperties = PropertySet::All)
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

		void _clear(const PropertySet& includedProperties = PropertySet::All)
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
			}, typename Derived::_properties_t{});
		}

		bool _equal(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
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

		bool _less(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
		{
			return _applyPropertyPairs(rhs, [&](const auto&... propertyPairs)
			{
				return _less(includedProperties, propertyPairs...);
			});
		}

    	bool _lessEqual(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
		{
			return !_greater(rhs, includedProperties);
		}

    	bool _greater(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
		{
			return rhs._less(*this, includedProperties);
		}

    	bool _greaterEqual(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
		{
			return !_less(rhs, includedProperties);
		}

		PropertySet _diffProperties(const Derived& other, const PropertySet& includedProperties = PropertySet::All) const
		{
			PropertySet symmetricDiff = _validProperties().symmetricDifference(other._validProperties()) ^ includedProperties;
			PropertySet intersection = _validProperties() ^ other._validProperties() ^ includedProperties;

			if (!intersection.empty())
			{
				_applyPropertyPairs(other, [&](const auto&... propertyPairs)
				{
					auto check_inequality = [&](auto& propertyThis, auto& propertyOther)
					{
						using property_t = strip_t<decltype(propertyThis)>;

						if (property_t::IsPartOf(intersection) && propertyThis != propertyOther)
						{
							symmetricDiff += property_t::Set();
						}
					};

					return (check_inequality(propertyPairs.first, propertyPairs.second), ...);
				});
			}

			return symmetricDiff;
		}

    	void _publish(const PropertySet& includedProperties = PropertySet::All, bool remove = false) const
		{
    		static_assert(!Derived::_SubstructOnly, "a substruct-only type cannot be published");
    		
			registerTypeUsage<Derived, PublishedType>();
			Struct::_publish(includedProperties, remove);
		}

		void _remove(const PropertySet& includedProperties = PropertySet::All) const
		{
    		static_assert(!Derived::_SubstructOnly, "a substruct-only type cannot be removed");
    		
			registerTypeUsage<Derived, PublishedType>();
			Struct::_remove(includedProperties);
		}

    	const PropertyArea& _propertyArea() const
		{
			return m_propertyArea;
		}

    	PropertyArea& _propertyArea()
		{
			return m_propertyArea;
		}

    	const PropertySet& _validProperties() const
		{
			return m_propertyArea.validProperties();
		}

    	template <typename P>
    	const P& getProperty() const
		{
			return m_propertyArea.getProperty<P>();
		}

    	template <typename P>
		P& getProperty()
		{
			return const_cast<P&>(std::as_const(*this).template getProperty<P>());
		}

    	static const std::shared_ptr<Descriptor<>>& _DescriptorPtr()
        {
			return Descriptor<Derived>::InstancePtr();
        }

    	static const Descriptor<Derived>& _Descriptor()
        {
			return Descriptor<Derived>::Instance();
        }

		static PropertySet _KeyProperties()
		{
			static PropertySet KeyProperties = std::apply([](auto&&... args)
			{
				if constexpr (sizeof...(args) == 0)
				{
					return PropertySet{};
				}
				else
				{
					return (strip_t<decltype(args)>::Set() | ...);
				}
			}, typename Derived::_key_properties_t{});

			return KeyProperties;
		}

    protected:

		StaticStruct(const StaticStruct& other) = default;
		StaticStruct(StaticStruct&& other) = default;
		~StaticStruct() = default;

		StaticStruct& operator = (const StaticStruct& rhs) = default;
		StaticStruct& operator = (StaticStruct&& rhs) = default;

    private:

    	using Struct::_assign;
		using Struct::_copy;
		using Struct::_merge;
		using Struct::_swap;
		using Struct::_clear;
    	
    	using Struct::_equal;
		using Struct::_same;
    	
        using Struct::_less;
    	using Struct::_lessEqual;
    	using Struct::_greater;
    	using Struct::_greaterEqual;

    	using Struct::_diffProperties;

		using Struct::_publish;
		using Struct::_remove;

    	using Struct::_propertyArea;
    	using Struct::_validProperties;

    	template <typename Callable, typename... Properties>
    	auto _applyProperties(Callable&& callable, std::tuple<Properties...>) const
    	{
    		return callable(getProperty<strip_t<Properties>>()...);
    	}

    	template <typename Callable, typename... Properties>
    	auto _applyProperties(Callable&& callable, std::tuple<Properties...>)
    	{
    		return callable(getProperty<strip_t<Properties>>()...);
    	}

    	template <typename Callable, typename... Properties>
		auto _applyPropertyPairs(const Derived& other, Callable&& callable, std::tuple<Properties...>) const
		{
			return callable(std::pair<const strip_t<Properties>&, const strip_t<Properties>&>{ getProperty<strip_t<Properties>>(), other.template getProperty<strip_t<Properties>>() }...);
		}

    	template <typename Callable, typename... Properties>
		auto _applyPropertyPairs(const Derived& other, Callable&& callable, std::tuple<Properties...>)
		{
			return callable(std::pair<strip_t<Properties>&, const strip_t<Properties>&>{ getProperty<strip_t<Properties>>(), other.template getProperty<strip_t<Properties>>() }...);
		}

    	template <typename Callable, typename... Properties>
		auto _applyPropertyPairs(Derived& other, Callable&& callable, std::tuple<Properties...>)
		{
			return callable(std::pair<strip_t<Properties>&, strip_t<Properties>&>{ getProperty<strip_t<Properties>>(), other.template getProperty<strip_t<Properties>>() }...);
		}

    	template <typename PropertyPair, typename... PropertyPairs>
    	bool _less(const PropertySet& includedProperties, const PropertyPair& firstPair, const PropertyPairs&... remainingPairs) const
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

    	PropertyArea m_propertyArea;
    };
}