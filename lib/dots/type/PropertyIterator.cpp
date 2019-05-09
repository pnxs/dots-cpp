#include "PropertyIterator.h"

namespace dots::type
{
	template <bool IsReverse, bool IsConst>
	PropertyIterator<IsReverse, IsConst>::PropertyIterator(struct_t& instance, inner_iterator_t innerIterator, const property_set& set) :
		_instance(&instance),
		_innerIterator(std::move(innerIterator)),
		_set(set)
	{
		if (!hasReachedEnd() && !emplaceProxy().isPartOf(_set))
		{
			++(*this);
		}
	}

	template <bool IsReverse, bool IsConst>
	void PropertyIterator<IsReverse, IsConst>::swap(PropertyIterator& other) noexcept
	{
		std::swap(_instance, other._instance);
		std::swap(_innerIterator, other._innerIterator);
	}

	template <bool IsReverse, bool IsConst>
	void PropertyIterator<IsReverse, IsConst>::swap(PropertyIterator&& other)
	{
		_instance = other._instance;
		other._instance = nullptr;
		_innerIterator = std::move(other._innerIterator);
	}

	template <bool IsReverse, bool IsConst>
	auto PropertyIterator<IsReverse, IsConst>::operator ++ () -> PropertyIterator&
	{
		while (!hasReachedEnd())
		{
			++_innerIterator;

			if (hasReachedEnd() || emplaceProxy().isPartOf(_set))
			{
				break;
			}
		}

		return *this;
	}

	template <bool IsReverse, bool IsConst>
	auto PropertyIterator<IsReverse, IsConst>::operator -- () -> PropertyIterator&
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

	template <bool IsReverse, bool IsConst>
	auto PropertyIterator<IsReverse, IsConst>::operator ++ (int) -> PropertyIterator
	{
		PropertyIterator copy = *this;
		++(*this);

		return copy;
	}

	template <bool IsReverse, bool IsConst>
	auto PropertyIterator<IsReverse, IsConst>::operator -- (int) -> PropertyIterator
	{
		PropertyIterator copy = *this;
		--(*this);

		return copy;
	}

	template <bool IsReverse, bool IsConst>
	auto PropertyIterator<IsReverse, IsConst>::operator * ()-> reference
	{
		if (hasReachedEnd())
		{
			throw std::logic_error{ "attempt to access past-end property" };
		}

		return *_proxy;
	}

	template <bool IsReverse, bool IsConst>
	auto PropertyIterator<IsReverse, IsConst>::operator * () const -> const reference
	{
		return *const_cast<PropertyIterator&>(*this);
	}

	template <bool IsReverse, bool IsConst>
	auto PropertyIterator<IsReverse, IsConst>::operator -> () -> pointer
	{
		return &*(*this);
	}

	template <bool IsReverse, bool IsConst>
	auto PropertyIterator<IsReverse, IsConst>::operator -> () const -> const pointer
	{
		return &*(*this);
	}

	template <bool IsReverse, bool IsConst>
	bool PropertyIterator<IsReverse, IsConst>::operator == (const PropertyIterator& other) const
	{
		return _innerIterator == other._innerIterator;
	}

	template <bool IsReverse, bool IsConst>
	bool PropertyIterator<IsReverse, IsConst>::operator != (const PropertyIterator& other) const
	{
		return !(*this == other);
	}

	template <bool IsReverse, bool IsConst>
	bool PropertyIterator<IsReverse, IsConst>::hasReachedEnd() const
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

	template <bool IsReverse, bool IsConst>
	typename PropertyIterator<IsReverse, IsConst>::reference PropertyIterator<IsReverse, IsConst>::emplaceProxy()
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

	template struct PropertyIterator<false, false>;
	template struct PropertyIterator<false, true>;
	template struct PropertyIterator<true, false>;
	template struct PropertyIterator<true, true>;

	template <typename Iterator>
	PropertyRange<Iterator>::PropertyRange(Iterator begin, Iterator end):
		_begin(std::move(begin)),
		_end(std::move(end))
	{
		/* do nothing */
	}

	template <typename Iterator>
	Iterator PropertyRange<Iterator>::begin() const
	{
		return _begin;
	}

	template <typename Iterator>
	Iterator PropertyRange<Iterator>::end() const
	{
		return _end;
	}

	template struct PropertyRange<property_iterator>;
	template struct PropertyRange<const_property_iterator>;
	template struct PropertyRange<reverse_property_iterator>;
	template struct PropertyRange<const_reverse_property_iterator>;
}