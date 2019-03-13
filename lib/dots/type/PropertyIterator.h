#pragma once
#include <optional>
#include "PropertyProxy.h"

namespace dots::type
{
	template <bool IsReverse, bool IsConst>
	struct PropertyIterator
	{
		// custom iterator traits
		using base_value_type   = PropertyProxy<void>;
		using struct_t          = std::conditional_t<IsConst, const Struct, Struct>;
		using inner_container_t = std::remove_reference_t<decltype(std::declval<struct_t>()._descriptor().properties())>;
		using inner_iterator_t  = std::conditional_t<IsReverse, typename inner_container_t::const_reverse_iterator, typename inner_container_t::const_iterator>;

		// STL iterator traits
		using iterator_category = typename  inner_iterator_t::iterator_category;
		using value_type        = std::conditional_t<IsConst, const base_value_type, base_value_type>;
		using reference         = std::conditional_t<IsConst, const value_type&, value_type&>;
		using pointer           = std::conditional_t<IsConst, const value_type*, value_type*>;

		PropertyIterator(struct_t& instance, inner_iterator_t innerIterator, const property_set& set = PROPERTY_SET_ALL) :
			_instance(&instance),
			_innerIterator(std::move(innerIterator)),
			_set(set)
		{
			if (!emplaceProxy().isPartOf(_set))
			{
				++(*this);
			}
		}
		PropertyIterator(const PropertyIterator& other) = default;
		PropertyIterator(PropertyIterator&& other) = default;
		~PropertyIterator() = default;

		PropertyIterator& operator = (const PropertyIterator& rhs) = default;
		PropertyIterator& operator = (PropertyIterator&& rhs) = default;

		void swap(PropertyIterator& other) noexcept
		{
			std::swap(_instance, other._instance);
			std::swap(_innerIterator, other._innerIterator);
		}

		void swap(PropertyIterator&& other)
		{
			_instance = other._instance;
			other._instance = nullptr;
			_innerIterator = std::move(other._innerIterator);
		}

		PropertyIterator& operator ++ ()
		{
			while (!hasReachedEnd())
			{
				++_innerIterator;

				if (emplaceProxy().isPartOf(_set))
				{
					break;
				}
			}
			
			return *this;
		}

		PropertyIterator& operator -- ()
		{
			while (!hasReachedEnd())
			{
				--_innerIterator;

				if (emplaceProxy().isPartOf(_set))
				{
					break;
				}
			}

			return *this;
		}

		PropertyIterator operator ++ (int)
		{
			PropertyIterator copy = *this;
			++(*this);

			return copy;
		}

		PropertyIterator operator -- (int)
		{
			PropertyIterator copy = *this;
			--(*this);

			return copy;
		}

		reference operator * ()
		{
			if (hasReachedEnd())
			{
				throw std::logic_error{ "attempt to access past-end property" };
			}

			return *_proxy;
		}

		const reference operator * () const
		{
			return *const_cast<PropertyIterator&>(*this);
		}

		pointer operator -> ()
		{
			return &*(*this);
		}

		const pointer operator -> () const
		{
			return &*(*this);
		}

		bool operator == (const PropertyIterator& other) const
		{
			return _innerIterator == other._innerIterator;
		}

		bool operator != (const PropertyIterator& other) const
		{
			return !(*this == other);
		}

	private:

		bool hasReachedEnd() const
		{
			if constexpr (IsReverse)
			{
				return _innerIterator == _instance->_descriptor().properties().rend();
			}
			else
			{
				return _innerIterator == _instance->_descriptor().properties().end();
			}
		}

		reference emplaceProxy()
		{
			// TODO: implement better solution
			if constexpr (IsConst)
			{
				return _proxy.emplace(const_cast<char*>(_innerIterator->address(_instance)), *_innerIterator);
			}
			else
			{
				return _proxy.emplace(_innerIterator->address(_instance), *_innerIterator);
			}
		}

		struct_t* _instance;
		inner_iterator_t _innerIterator;
		property_set _set;
		std::optional<base_value_type> _proxy;
	};

	struct property_iterator : PropertyIterator<false, false>
	{
		using PropertyIterator::PropertyIterator;
	};

	struct const_property_iterator : PropertyIterator<false, true>
	{
		using PropertyIterator::PropertyIterator;
	};

	struct reverse_property_iterator : PropertyIterator<true, false>
	{
		using PropertyIterator::PropertyIterator;
	};

	struct const_reverse_property_iterator : PropertyIterator<true, true>
	{
		using PropertyIterator::PropertyIterator;
	};

	template <typename Iterator>
	struct PropertyRange
	{
		PropertyRange(Iterator begin, Iterator end) :
			_begin(std::move(begin)),
			_end(std::move(end))
		{
			/* do nothing */
		}
		PropertyRange(const PropertyRange& other) = default;
		PropertyRange(PropertyRange&& other) = default;
		~PropertyRange() = default;

		PropertyRange& operator = (const PropertyRange& rhs) = default;
		PropertyRange& operator = (PropertyRange&& rhs) = default;

		Iterator begin() const
		{
			return _begin;
		}

		Iterator end() const
		{
			return _end;
		}

	private:

		Iterator _begin;
		Iterator _end;
	};

	struct property_range : PropertyRange<property_iterator>
	{
		using PropertyRange::PropertyRange;
	};

	struct const_property_range : PropertyRange<const_property_iterator>
	{
		using PropertyRange::PropertyRange;
	};

	struct reverse_property_range : PropertyRange<reverse_property_iterator>
	{
		using PropertyRange::PropertyRange;
	};

	struct const_reverse_property_range : PropertyRange<const_reverse_property_iterator>
	{
		using PropertyRange::PropertyRange;
	};
}