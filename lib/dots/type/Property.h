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
			return isValid() && valueUnchecked() == rhs;
		}

		bool operator != (const T& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const T& rhs) const
		{
			return isValid() && valueUnchecked() < rhs;
		}

		bool operator == (const Property& rhs) const
		{
			return isValid() && rhs.isValid() && valueUnchecked() == rhs.valueUnchecked();
		}

		bool operator != (const Property& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const Property& rhs) const
		{
			return !rhs.isValid() || isValid() && valueUnchecked() < rhs.valueUnchecked();
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
			return *derived().descriptorAddress();
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

			return valueUnchecked();
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

			::new (static_cast<void *>(::std::addressof(valueUnchecked()))) T(std::forward<Args>(args)...);
			validPropertySet().set(tag(), true);

			return valueUnchecked();
		}

		template <typename... Args>
		T& assign(Args&&... args)
		{
			valueUnchecked() = T(std::forward<Args>(args)...);
			validPropertySet().set(tag(), true);

			return valueUnchecked();
		}

		template <typename... Args>
		T& constructOrValue(Args&&... args)
		{
			if (isValid())
			{
				return valueUnchecked();
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
					std::swap(valueUnchecked(), other.valueUnchecked());
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
				valueUnchecked().~T();
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

        bool equal(const void* rhs) const
        {
            return td().equal(&valueUnchecked(), rhs);
        }

        bool lessThan(const void* rhs) const
        {
            return td().lessThan(&valueUnchecked(), rhs);
        }

        void copy(const void* rhs)
        {
            return td().copy(&valueUnchecked(), rhs);
        }

        void swap(void* rhs)
        {
            return td().swap(&valueUnchecked(), rhs);
        }

        void clear()
        {
            return td().clear(&valueUnchecked());
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
			return std::move(valueUnchecked());
		}

    private:

		property_set& validPropertySet()
		{
			return const_cast<property_set&>(instance()._validPropertySet());
		}

		const property_set& validPropertySet() const
		{
			return const_cast<Property&>(*this).validPropertySet();
		}

		Struct& instance()
		{
			return *reinterpret_cast<Struct*>(reinterpret_cast<char*>(&valueUnchecked()) - offset());
		}

		const Struct& instance() const
		{
			return const_cast<Property&>(*this).instance();
		}

		T& valueUnchecked()
		{
			return *derived().valueAddress();
		}

		const T& valueUnchecked() const
		{
			return *derived().valueAddress();
		}

		Derived& derived()
		{
			return static_cast<Derived&>(*this);
		}

		const Derived& derived() const
		{
			return const_cast<Property&>(*this).derived();
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