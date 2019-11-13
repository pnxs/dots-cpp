#pragma once
#include <string_view>
#include <array>
#include <functional>
#include <dots/type/NewPropertyArea.h>
#include <dots/type/NewProxyPropertyPairIterator.h>

namespace dots::type
{
	template <typename Derived>
    struct NewPropertyContainer
    {
		NewPropertyContainer() = default;
        NewPropertyContainer(const NewPropertyContainer& other) = default;
        NewPropertyContainer(NewPropertyContainer&& other) = default;
        ~NewPropertyContainer() = default;

        NewPropertyContainer& operator = (const NewPropertyContainer& rhs) = default;
        NewPropertyContainer& operator = (NewPropertyContainer&& rhs) noexcept = default;

		constexpr const NewPropertyArea& _propertyArea() const
		{
			return static_cast<const Derived&>(*this).derivedPropertyArea();
		}

		constexpr NewPropertyArea& _propertyArea()
		{
			return static_cast<Derived&>(*this).derivedPropertyArea();
		}

		constexpr const NewPropertySet& _validProperties() const
		{
			return _propertyArea().validProperties();
		}
		
    	constexpr const new_property_descriptor_container_t& _propertyDescriptors() const
		{
			return static_cast<const Derived&>(*this).derivedPropertyDescriptors();
		}
		
	    new_proxy_property_iterator _begin(const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().begin(), includedProperties };
	    }

	    new_const_proxy_property_iterator _begin(const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return new_const_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().begin(), includedProperties };
	    }

	    new_proxy_property_iterator _end(const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().end(), includedProperties };
	    }

	    new_const_proxy_property_iterator _end(const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return new_const_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().end(), includedProperties };
	    }

	    new_reverse_proxy_property_iterator _rbegin(const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_reverse_proxy_property_iterator{ _propertyArea(),_propertyDescriptors(),  _propertyDescriptors().rbegin(), includedProperties };
	    }

	    new_const_reverse_proxy_property_iterator _rbegin(const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return new_const_reverse_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().rbegin(), includedProperties };
	    }

	    new_reverse_proxy_property_iterator _rend(const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_reverse_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().rend(), includedProperties };
	    }

	    new_const_reverse_proxy_property_iterator _rend(const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return new_const_reverse_proxy_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().rend(), includedProperties };
	    }

	    new_proxy_property_range _propertyRange(const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_proxy_property_range{ _begin(includedProperties), _end(includedProperties) };
	    }

	    new_const_proxy_property_range _propertyRange(const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return new_const_proxy_property_range{ _begin(includedProperties), _end(includedProperties) };
	    }

	    new_reverse_proxy_property_range _propertyRangeReversed(const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_reverse_proxy_property_range{ _rbegin(includedProperties), _rend(includedProperties) };
	    }

	    new_const_reverse_proxy_property_range _propertyRangeReversed(const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return new_const_reverse_proxy_property_range{ _rbegin(includedProperties), _rend(includedProperties) };
	    }

	    new_proxy_property_pair_range _propertyRange(Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_proxy_property_pair_range{ new_proxy_property_pair_iterator{ _begin(includedProperties), rhs._begin(includedProperties) }, new_proxy_property_pair_iterator{ _end(includedProperties), rhs._end(includedProperties) } };
	    }

	    new_proxy_property_pair_range_const _propertyRange(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_proxy_property_pair_range_const{ new_proxy_property_pair_iterator_const{ _begin(includedProperties), rhs._begin(includedProperties) }, new_proxy_property_pair_iterator_const{ _end(includedProperties), rhs._end(includedProperties) } };
	    }

	    new_const_proxy_property_pair_range_const _propertyRange(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return new_const_proxy_property_pair_range_const{ new_const_proxy_property_pair_iterator_const{ _begin(includedProperties), rhs._begin(includedProperties) }, new_const_proxy_property_pair_iterator_const{ _end(includedProperties), rhs._end(includedProperties) } };
	    }

	    new_reverse_proxy_property_pair_range _propertyRangeReversed(Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_reverse_proxy_property_pair_range{ new_reverse_proxy_property_pair_iterator{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, new_reverse_proxy_property_pair_iterator{ _rend(includedProperties), rhs._rend(includedProperties) } };
	    }

	    new_reverse_proxy_property_pair_range_const _propertyRangeReversed(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return new_reverse_proxy_property_pair_range_const{ new_reverse_proxy_property_pair_iterator_const{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, new_reverse_proxy_property_pair_iterator_const{ _rend(includedProperties), rhs._rend(includedProperties) } };
	    }

	    new_const_reverse_proxy_property_pair_range_const _propertyRangeReversed(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return new_const_reverse_proxy_property_pair_range_const{ new_const_reverse_proxy_property_pair_iterator_const{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, new_const_reverse_proxy_property_pair_iterator_const{ _rend(includedProperties), rhs._rend(includedProperties) } };
	    }

	    new_proxy_property_range _validPropertyRange(const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return _propertyRange(_validProperties() ^ includedProperties);
	    }

	    new_const_proxy_property_range _validPropertyRange(const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return _propertyRange(_validProperties() ^ includedProperties);
	    }

	    new_reverse_proxy_property_range _validPropertyRangeReversed(const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return _propertyRangeReversed(_validProperties() ^ includedProperties);
	    }

	    new_const_reverse_proxy_property_range _validPropertyRangeReversed(const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return _propertyRangeReversed(_validProperties() ^ includedProperties);
	    }

	    new_proxy_property_pair_range _validPropertyRange(Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return _propertyRange(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    new_proxy_property_pair_range_const _validPropertyRange(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return _propertyRange(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    new_const_proxy_property_pair_range_const _validPropertyRange(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return _propertyRange(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    new_reverse_proxy_property_pair_range _validPropertyRangeReversed(Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return _propertyRangeReversed(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    new_reverse_proxy_property_pair_range_const _validPropertyRangeReversed(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All)
	    {
	        return _propertyRangeReversed(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }

	    new_const_reverse_proxy_property_pair_range_const _validPropertyRangeReversed(const Derived& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const
	    {
	        return _propertyRangeReversed(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
	    }
    };

	template <typename Derived>
	new_proxy_property_iterator begin(NewPropertyContainer<Derived>& container)
    {
        return container._begin();
    }

	template <typename Derived>
    new_const_proxy_property_iterator begin(const NewPropertyContainer<Derived>& container)
    {
        return container._begin();
    }

	template <typename Derived>
    new_proxy_property_iterator end(NewPropertyContainer<Derived>& container)
    {
        return container._end();
    }

	template <typename Derived>
    new_const_proxy_property_iterator end(const NewPropertyContainer<Derived>& container)
    {
        return container._end();
    }

	template <typename Derived>
    new_reverse_proxy_property_iterator rbegin(NewPropertyContainer<Derived>& container)
    {
        return container._rbegin();
    }

	template <typename Derived>
    new_const_reverse_proxy_property_iterator rbegin(const NewPropertyContainer<Derived>& container)
    {
        return container._rbegin();
    }

	template <typename Derived>
    new_reverse_proxy_property_iterator rend(NewPropertyContainer<Derived>& container)
    {
        return container._rend();
    }

	template <typename Derived>
    new_const_reverse_proxy_property_iterator rend(const NewPropertyContainer<Derived>& container)
    {
        return container._rend();
    }
}