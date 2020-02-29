#pragma once
#include <string_view>
#include <array>
#include <functional>
#include <algorithm>
#include <dots/type/PropertyArea.h>
#include <dots/type/ProxyPropertyPairIterator.h>

namespace dots::type
{
	template <typename Derived>
    struct PropertyContainer
    {
		PropertyContainer() = default;
        PropertyContainer(const PropertyContainer& other) = default;
        PropertyContainer(PropertyContainer&& other) = default;
        ~PropertyContainer() = default;

        PropertyContainer& operator = (const PropertyContainer& rhs) = default;
        PropertyContainer& operator = (PropertyContainer&& rhs) noexcept = default;

		const_proxy_property_iterator operator [] (PropertySet::index_t tag) const
		{
			return _find(tag);
		}

		proxy_property_iterator operator [] (PropertySet::index_t tag)
		{
		    return _find(tag);
		}

		const_proxy_property_iterator operator [] (const std::string_view& name) const
		{
			return _find(name);
		}

		proxy_property_iterator operator [] (const std::string_view& name)
		{
		    return _find(name);
		}

		constexpr const PropertyArea& _propertyArea() const
		{
			return static_cast<const Derived&>(*this).derivedPropertyArea();
		}

		constexpr PropertyArea& _propertyArea()
		{
			return static_cast<Derived&>(*this).derivedPropertyArea();
		}

		constexpr const PropertySet& _validProperties() const
		{
			return _propertyArea().validProperties();
		}
		
    	constexpr const property_descriptor_container_t& _propertyDescriptors() const
		{
			return static_cast<const Derived&>(*this).derivedPropertyDescriptors();
		}
		
	    proxy_property_iterator _begin(const PropertySet& includedProperties = PropertySet::All)
	    {
	        return proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().begin(), includedProperties };
	    }

	    const_proxy_property_iterator _begin(const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return const_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().begin(), includedProperties };
	    }

	    proxy_property_iterator _end(const PropertySet& includedProperties = PropertySet::All)
	    {
	        return proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().end(), includedProperties };
	    }

	    const_proxy_property_iterator _end(const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return const_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().end(), includedProperties };
	    }

	    reverse_proxy_property_iterator _rbegin(const PropertySet& includedProperties = PropertySet::All)
	    {
	        return reverse_proxy_property_iterator{ _propertyArea(),_propertyDescriptors(),  _propertyDescriptors().rbegin(), includedProperties };
	    }

	    const_reverse_proxy_property_iterator _rbegin(const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return const_reverse_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().rbegin(), includedProperties };
	    }

	    reverse_proxy_property_iterator _rend(const PropertySet& includedProperties = PropertySet::All)
	    {
	        return reverse_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().rend(), includedProperties };
	    }

	    const_reverse_proxy_property_iterator _rend(const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return const_reverse_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().rend(), includedProperties };
	    }

	    proxy_property_range _propertyRange(const PropertySet& includedProperties = PropertySet::All)
	    {
	        return proxy_property_range{ _begin(includedProperties), _end(includedProperties) };
	    }

	    const_proxy_property_range _propertyRange(const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return const_proxy_property_range{ _begin(includedProperties), _end(includedProperties) };
	    }

	    reverse_proxy_property_range _propertyRangeReversed(const PropertySet& includedProperties = PropertySet::All)
	    {
	        return reverse_proxy_property_range{ _rbegin(includedProperties), _rend(includedProperties) };
	    }

	    const_reverse_proxy_property_range _propertyRangeReversed(const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return const_reverse_proxy_property_range{ _rbegin(includedProperties), _rend(includedProperties) };
	    }

	    proxy_property_pair_range _propertyRange(Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
	    {
	        return proxy_property_pair_range{ proxy_property_pair_iterator{ _begin(includedProperties), rhs._begin(includedProperties) }, proxy_property_pair_iterator{ _end(includedProperties), rhs._end(includedProperties) } };
	    }

	    proxy_property_pair_range_const _propertyRange(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
	    {
	        return proxy_property_pair_range_const{ proxy_property_pair_iterator_const{ _begin(includedProperties), rhs._begin(includedProperties) }, proxy_property_pair_iterator_const{ _end(includedProperties), rhs._end(includedProperties) } };
	    }

	    const_proxy_property_pair_range_const _propertyRange(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return const_proxy_property_pair_range_const{ const_proxy_property_pair_iterator_const{ _begin(includedProperties), rhs._begin(includedProperties) }, const_proxy_property_pair_iterator_const{ _end(includedProperties), rhs._end(includedProperties) } };
	    }

	    reverse_proxy_property_pair_range _propertyRangeReversed(Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
	    {
	        return reverse_proxy_property_pair_range{ reverse_proxy_property_pair_iterator{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, reverse_proxy_property_pair_iterator{ _rend(includedProperties), rhs._rend(includedProperties) } };
	    }

	    reverse_proxy_property_pair_range_const _propertyRangeReversed(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
	    {
	        return reverse_proxy_property_pair_range_const{ reverse_proxy_property_pair_iterator_const{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, reverse_proxy_property_pair_iterator_const{ _rend(includedProperties), rhs._rend(includedProperties) } };
	    }

	    const_reverse_proxy_property_pair_range_const _propertyRangeReversed(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return const_reverse_proxy_property_pair_range_const{ const_reverse_proxy_property_pair_iterator_const{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, const_reverse_proxy_property_pair_iterator_const{ _rend(includedProperties), rhs._rend(includedProperties) } };
	    }

	    proxy_property_range _validPropertyRange(const PropertySet& includedProperties = PropertySet::All)
	    {
	        return _propertyRange(_validProperties() ^ includedProperties);
	    }

	    const_proxy_property_range _validPropertyRange(const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return _propertyRange(_validProperties() ^ includedProperties);
	    }

	    reverse_proxy_property_range _validPropertyRangeReversed(const PropertySet& includedProperties = PropertySet::All)
	    {
	        return _propertyRangeReversed(_validProperties() ^ includedProperties);
	    }

	    const_reverse_proxy_property_range _validPropertyRangeReversed(const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return _propertyRangeReversed(_validProperties() ^ includedProperties);
	    }

	    proxy_property_pair_range _validPropertyRange(Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
	    {
	        return _propertyRange(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    proxy_property_pair_range_const _validPropertyRange(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
	    {
	        return _propertyRange(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    const_proxy_property_pair_range_const _validPropertyRange(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return _propertyRange(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    reverse_proxy_property_pair_range _validPropertyRangeReversed(Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
	    {
	        return _propertyRangeReversed(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    reverse_proxy_property_pair_range_const _validPropertyRangeReversed(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
	    {
	        return _propertyRangeReversed(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    const_reverse_proxy_property_pair_range_const _validPropertyRangeReversed(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
	    {
	        return _propertyRangeReversed(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

		template <typename Callable>
		const_proxy_property_iterator _find(Callable&& callable) const
		{
			return std::find_if(_begin(), _end(), std::forward<Callable>(callable));
		}

		template <typename Callable>
		proxy_property_iterator _find(Callable&& callable)
		{
		    return std::find_if(_begin(), _end(), std::forward<Callable>(callable));
		}

		const_proxy_property_iterator _find(PropertySet::index_t tag) const
		{
			return _find([&](const ProxyProperty<>& property)
			{
				return property.descriptor().tag() == tag;
			});
		}

		proxy_property_iterator _find(PropertySet::index_t tag)
		{
		    return _find([&](const ProxyProperty<>& property)
			{
			    return property.descriptor().tag() == tag;
			});
		}

		const_proxy_property_iterator _find(const std::string_view& name) const
		{
			return _find([&](const ProxyProperty<>& property)
			{
			    return property.descriptor().name() == name;
			});
		}

		proxy_property_iterator _find(const std::string_view& name)
		{
		    return _find([&](const ProxyProperty<>& property)
			{
			    return property.descriptor().name() == name;
			});
		}
    };

	template <typename Derived>
	proxy_property_iterator begin(PropertyContainer<Derived>& container)
    {
        return container._begin();
    }

	template <typename Derived>
    const_proxy_property_iterator begin(const PropertyContainer<Derived>& container)
    {
        return container._begin();
    }

	template <typename Derived>
    proxy_property_iterator end(PropertyContainer<Derived>& container)
    {
        return container._end();
    }

	template <typename Derived>
    const_proxy_property_iterator end(const PropertyContainer<Derived>& container)
    {
        return container._end();
    }

	template <typename Derived>
    reverse_proxy_property_iterator rbegin(PropertyContainer<Derived>& container)
    {
        return container._rbegin();
    }

	template <typename Derived>
    const_reverse_proxy_property_iterator rbegin(const PropertyContainer<Derived>& container)
    {
        return container._rbegin();
    }

	template <typename Derived>
    reverse_proxy_property_iterator rend(PropertyContainer<Derived>& container)
    {
        return container._rend();
    }

	template <typename Derived>
    const_reverse_proxy_property_iterator rend(const PropertyContainer<Derived>& container)
    {
        return container._rend();
    }
}