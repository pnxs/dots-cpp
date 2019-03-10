#pragma once
#include "Property.h"

namespace dots::type
{
	template <typename T>
	struct PropertyProxy : Property<T, PropertyProxy<T>>
	{
		PropertyProxy(T& value, const StructProperty& descriptor) :
			_value(&value),
			_descriptor(&descriptor)
		{
			/* do nothing */
		}

		template <typename Derived, std::enable_if_t<std::negation_v<std::is_same<Derived, PropertyProxy<T>>>, int> = 0>
		PropertyProxy(Property<T, Derived>& property) :
			PropertyProxy(*reinterpret_cast<T*>(property.address()), property.structProperty())
		{
			/* do nothing */
		}
		PropertyProxy(const PropertyProxy& other) = default;
		PropertyProxy(PropertyProxy&& other) = default;
		~PropertyProxy() = default;

		PropertyProxy& operator = (const PropertyProxy& rhs) = default;
		PropertyProxy& operator = (PropertyProxy&& rhs) = default;

	private:

		friend struct Property<T, PropertyProxy<T>>;

		T& derivedValue()
		{
			return *_value;
		}

		const T& derivedValue() const
		{
			return const_cast<PropertyProxy&>(*this).derivedValue();
		}

		const StructProperty& derivedDescriptor() const
		{
			return *_descriptor;
		}

		T* _value;
		const StructProperty* _descriptor;
	};

	template <>
	struct PropertyProxy<void> : Property<void, PropertyProxy<void>>
	{
		PropertyProxy(void* value, const StructProperty& descriptor) :
			_value(reinterpret_cast<std::byte*>(value)),
			_descriptor(&descriptor)
		{
			/* do nothing */
		}
		template <typename T, typename Derived, std::enable_if_t<std::negation_v<std::is_same<Derived, PropertyProxy<void>>>, int> = 0>
		PropertyProxy(Property<T, Derived>& property) :
			PropertyProxy(property.address(), property.structProperty())
		{
			/* do nothing */
		}
		PropertyProxy(const PropertyProxy& other) = default;
		PropertyProxy(PropertyProxy&& other) = default;
		~PropertyProxy() = default;

		PropertyProxy& operator = (const PropertyProxy& rhs) = default;
		PropertyProxy& operator = (PropertyProxy&& rhs) = default;

		template <typename T>
		PropertyProxy<T> typed()
		{
			return PropertyProxy<T>{ reinterpret_cast<T&>(*_value), &_descriptor };
		}

	private:

		friend struct Property<void, PropertyProxy<void>>;

		std::byte& derivedValue()
		{
			return *_value;
		}

		const std::byte& derivedValue() const
		{
			return const_cast<PropertyProxy&>(*this).derivedValue();
		}

		const StructProperty& derivedDescriptor() const
		{
			return *_descriptor;
		}

		std::byte* _value;
		const StructProperty* _descriptor;
	};
}