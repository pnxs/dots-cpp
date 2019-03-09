#pragma once
#include <string_view>
#include <functional>
#include <type_traits>
#include <iostream>
#include "StructProperty.h"
#include "Struct.h"
#include "property_set.h"

namespace dots::type
{
	template <typename T, typename Derived>
    struct Property
    {
		static_assert(std::conjunction_v<std::negation<std::is_pointer<T>>, std::negation<std::is_reference<T>>>);
		using value_t = T;

		template <typename... Args>
		T& operator () (Args&&... args)
		{
			return construct(std::forward<Args>(args)...);
		}

		T& operator * ()
		{
			return value();
		}

		const T& operator * () const
		{
			return value();
		}

		T* operator -> ()
		{
			return &value();
		}

		const T* operator -> () const
		{
			return &value();
		}

		operator T& ()
		{
			return value();
		}

		operator const T& () const
		{
			return value();
		}

		bool operator == (const T& rhs) const
		{
			return isValid() && valueEqual(rhs);
		}

		bool operator != (const T& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const T& rhs) const
		{
			return isValid() && valueLess(rhs);
		}

		bool operator == (const Property& rhs) const
		{
			return rhs.isValid() && *this == *rhs;
		}

		bool operator != (const Property& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const Property& rhs) const
		{
			return !rhs.isValid() || isValid() && valueLess(rhs.valueReference());
		}

		constexpr size_t offset() const
		{
			return structProperty().offset();
		}

		constexpr uint32_t tag() const
		{
			return structProperty().tag();
		}

		constexpr bool isKey() const
		{
			return structProperty().isKey();
		}
        
		constexpr const std::string_view& name() const
        {
            return structProperty().name();
        }

		constexpr const std::string_view& type() const
		{
			return structProperty().type();
		}

		constexpr property_set set() const
		{
			property_set propertySet;
			propertySet.set(tag());

			return propertySet;
		}

		constexpr bool isPartOf(const property_set& propertySet) const
		{
			return propertySet.test(tag());
		}

		constexpr const StructProperty& structProperty() const
		{
			return static_cast<const Derived&>(*this).derivedDescriptor();
		}

		const Descriptor& td() const
		{
			return *structProperty().td();
		}

		bool isValid() const
		{
			return validPropertySet().test(tag());
		}

		T& value()
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to access invalid property: " } + name().data() + "." + name().data() };
			}

			return valueReference();
		}

		const T& value() const
		{
			return const_cast<Property&>(*this).value();
		}

		template <typename... Args>
		T& construct(Args&&... args)
		{
			if constexpr (sizeof...(Args) == 0)
			{
				static_assert(std::is_default_constructible_v<T>);
			}

			if (isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to construct already valid property: " } + name().data() + "." + name().data() };
			}

			valueConstruct(std::forward<Args>(args)...);
			validPropertySet().set(tag(), true);

			return valueReference();
		}

		template <typename... Args>
		T& assign(Args&&... args)
		{
			valueAssign(T(std::forward<Args>(args)...));
			validPropertySet().set(tag(), true);

			return valueReference();
		}

		template <typename... Args>
		T& constructOrValue(Args&&... args)
		{
			if (isValid())
			{
				return valueReference();
			}
			else
			{
				return construct(std::forward<Args>(args)...);
			}
		}

		template <typename... Args>
		T& constructOrAssign(Args&&... args)
		{
			if (isValid())
			{
				return assign(std::forward<Args>(args)...);
			}
			else
			{
				return construct(std::forward<Args>(args)...);
			}
		}

		void swap(Property& other)
		{
			if (isValid())
			{
				if (other.isValid())
				{
					valueSwap(other.valueReference());
				}
				else
				{
					other.construct(extractUnchecked());
				}
			}
			else if (other.isValid())
			{
				construct(other.extractUnchecked());
			}
		}

		T&& extract()
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to extract invalid property: " } + name().data() + "." + name().data() };
			}

			return extractUnchecked();
		}

		void destroy()
		{
			if (isValid())
			{
				valueDestroy();
				validPropertySet().set(tag(), false);
			}
		}

		void publish() const
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to publish invalid property: " } + name().data() + "." + name().data() };
			}

			instance()._publish(set());
		}

	protected:

		constexpr Property() = default;
		constexpr Property(const Property& other) = default;
		constexpr Property(Property&& other) = default;
		~Property() = default;

		constexpr Property& operator = (const Property& rhs) = default;
		constexpr Property& operator = (Property&& rhs) = default;

		T&& extractUnchecked()
		{
			validPropertySet().set(tag(), false);
			return std::move(valueReference());
		}

    private:

		constexpr T& valueReference()
		{
			return static_cast<Derived&>(*this).derivedValue();
		}

		constexpr const T& valueReference() const
		{
			return const_cast<Property&>(*this).valueReference();
		}

		template <typename... Args>
		constexpr T& valueConstruct(Args&&... args)
		{
			::new (static_cast<void *>(::std::addressof(valueReference()))) T(std::forward<Args>(args)...);
			return valueReference();
		}

		constexpr void valueDestroy()
		{
			valueReference().~T();
		}

		constexpr T& valueAssign(const T& rhs)
		{
			return valueReference() = rhs;
		}

		constexpr T& valueMove(T&& rhs)
		{
			return valueReference() = std::move(rhs);
		}

		void valueSwap(T& rhs)
		{
			std::swap(valueReference(), rhs);
		}

		constexpr bool valueEqual(const T& rhs) const
		{
			return valueReference() == rhs;
		}

		constexpr bool valueLess(const T& rhs) const
		{
			return valueReference() < rhs;
		}

		Struct& instance()
		{
			return *reinterpret_cast<Struct*>(reinterpret_cast<char*>(&valueReference()) - offset());
		}

		const Struct& instance() const
		{
			return const_cast<Property&>(*this).instance();
		}

		property_set& validPropertySet()
		{
			return const_cast<property_set&>(instance()._validPropertySet());
		}

		const property_set& validPropertySet() const
		{
			return const_cast<Property&>(*this).validPropertySet();
		}
    };

	template <typename T, typename Derived>
	std::ostream& operator << (std::ostream& os, const Property<T, Derived>& property)
	{
		if (property.isValid())
		{
			os << *property;
		}
		else
		{
			os << "<invalid-property>";
		}

		return os;
	}
}