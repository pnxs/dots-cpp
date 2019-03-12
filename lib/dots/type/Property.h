#pragma once
#include <string_view>
#include <functional>
#include <type_traits>
#include <iostream>
#include <cstddef>
#include "StructProperty.h"
#include "Struct.h"
#include "property_set.h"

namespace dots::type
{
	template <typename T, typename Derived>
	struct Property
	{
		static_assert(std::conjunction_v<std::negation<std::is_pointer<T>>, std::negation<std::is_reference<T>>>);
		static constexpr bool IsTypeless = std::is_same_v<T, void>;
		using value_t = std::conditional_t<IsTypeless, std::byte, T>;

		template <typename U, std::enable_if_t<!std::disjunction_v<std::is_same<std::remove_reference_t<U>, Property>, std::is_same<std::remove_reference_t<U>, Derived>>, int> = 0>
		Derived& operator = (U&& rhs)
		{
			Property<T, Derived>::constructOrAssign(std::forward<U>(rhs));
			return static_cast<Derived&>(*this);
		}

		template <typename... Args>
		value_t& operator () (Args&&... args)
		{
			return construct(std::forward<Args>(args)...);
		}

		value_t& operator * ()
		{
			return value();
		}

		const value_t& operator * () const
		{
			return value();
		}

		value_t* operator -> ()
		{
			return &value();
		}

		const value_t* operator -> () const
		{
			return &value();
		}

		operator value_t& ()
		{
			return value();
		}

		operator const value_t& () const
		{
			return value();
		}

		bool operator == (const value_t& rhs) const
		{
			return equal(rhs);
		}

		bool operator == (const Property& rhs) const
		{
			return equal(rhs);
		}

		bool operator != (const value_t& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator != (const Property& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const value_t& rhs) const
		{
			return less(rhs);
		}

		bool operator < (const Property& rhs) const
		{
			return less(rhs);
		}

		bool isValid() const
		{
			return validPropertySet().test(tag());
		}

		value_t& construct(const Derived& rhs)
		{
			if (!rhs.isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to construct from invalid property: " } + name().data() + "." + name().data() };
			}
			
			construct(rhs.valueReference());

			return *this;
		}

		value_t& construct(Derived&& rhs)
		{
			if (!rhs.isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to construct from invalid property: " } + name().data() + "." + name().data() };
			}

			construct(rhs.extractUnchecked());

			return *this;
		}

		template <typename... Args>
		value_t& construct(Args&&... args)
		{
			if constexpr (sizeof...(Args) == 0)
			{
				static_assert(std::is_default_constructible_v<T> || IsTypeless);
			}

			if (isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to construct already valid property: " } + name().data() + "." + name().data() };
			}

			valueConstruct(std::forward<Args>(args)...);
			validPropertySet().set(tag(), true);

			return valueReference();
		}

		void destroy()
		{
			if (isValid())
			{
				valueDestroy();
				validPropertySet().set(tag(), false);
			}
		}

		value_t& value()
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to access invalid property: " } + name().data() + "." + name().data() };
			}

			return valueReference();
		}

		const value_t& value() const
		{
			return const_cast<Property&>(*this).value();
		}

		template <typename... Args>
		value_t& constructOrValue(Args&&... args)
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

		value_t& assign(const Derived& rhs)
		{
			if (rhs.isValid())
			{
				assign(rhs.valueReference());
			}
			else
			{
				destroy();
			}

			return *this;
		}

		value_t& assign(Derived&& rhs)
		{
			if (rhs.isValid())
			{
				assign(rhs.extractUnchecked());
			}
			else
			{
				destroy();
			}

			return *this;
		}

		template <typename... Args>
		value_t& assign(Args&&... args)
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to assign invalid property: " } + name().data() + "." + name().data() };
			}

			valueAssign(std::forward<Args>(args)...);
			validPropertySet().set(tag(), true);

			return valueReference();
		}

		template <typename... Args>
		value_t& constructOrAssign(Args&&... args)
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
					if constexpr (IsTypeless)
					{
						other.construct(valueReference());
						destroy();
					}
					else
					{
						other.construct(extractUnchecked());
					}
				}
			}
			else if (other.isValid())
			{
				if constexpr (IsTypeless)
				{
					construct(other.valueReference());
					other.destroy();
				}
				else
				{
					construct(other.extractUnchecked());
				}
			}
		}

		template <bool IsTypeless_ = IsTypeless, std::enable_if_t<!IsTypeless_, int> = 0>
		value_t&& extract()
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to extract invalid property: " } + name().data() + "." + name().data() };
			}

			return extractUnchecked();
		}

		bool equal(const value_t& rhs) const
		{
			return isValid() && valueEqual(rhs);
		}

		bool equal(const Property& rhs) const
		{
			return rhs.isValid() && equal(*rhs);
		}

		bool less(const value_t& rhs) const
		{
			return isValid() && valueLess(rhs);
		}

		bool less(const Property& rhs) const
		{
			return !rhs.isValid() || less(*rhs);
		}

		void publish() const
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to publish invalid property: " } + name().data() + "." + name().data() };
			}

			instance()._publish(set());
		}

		void* address()
		{
			return &valueReference();
		}

		const void* address() const
		{
			return const_cast<Property&>(*this).address();
		}

		constexpr const StructProperty& structProperty() const
		{
			return static_cast<const Derived&>(*this).derivedDescriptor();
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

		const Descriptor& td() const
		{
			return *structProperty().td();
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

	protected:

		constexpr Property() = default;
		constexpr Property(const Property& other) = default;
		constexpr Property(Property&& other) = default;
		~Property() = default;

		constexpr Property& operator = (const Property& rhs) = default;
		constexpr Property& operator = (Property&& rhs) = default;

		template <bool IsTypeless_ = IsTypeless, std::enable_if_t<!IsTypeless_, int> = 0>
		value_t&& extractUnchecked()
		{
			validPropertySet().set(tag(), false);
			return std::move(valueReference());
		}

	private:

		constexpr value_t& valueReference()
		{
			return static_cast<Derived&>(*this).derivedValue();
		}

		constexpr const value_t& valueReference() const
		{
			return const_cast<Property&>(*this).valueReference();
		}

		template <typename... Args>
		constexpr value_t& valueConstruct(Args&&... args)
		{
			if constexpr (IsTypeless)
			{
				// note: the redundant if constexpr statements are intentional to improve readability when the build stops due to a static assertion error
				static_assert(sizeof...(Args) <= 1, "typeless construction only supports default construction or a single argument");

				if constexpr (sizeof...(Args) <= 1)
				{
					td().construct(&valueReference());

					if constexpr (sizeof...(Args) == 1)
					{
						valueAssign(std::forward<Args>(args)...);
					}
				}
			}
			else
			{
				::new (static_cast<void *>(::std::addressof(valueReference()))) T(std::forward<Args>(args)...);
			}

			return valueReference();
		}

		constexpr void valueDestroy()
		{
			if constexpr (IsTypeless)
			{
				td().destruct(&valueReference());
			}
			else
			{
				valueReference().~T();
			}
		}

		template <typename... Args>
		constexpr value_t& valueAssign(Args&&... args)
		{
			if constexpr (IsTypeless)
			{
				// note: the redundant if constexpr statements are intentional to improve readability when the build stops due to a static assertion error
				static_assert(sizeof...(Args) == 1, "typeless assignment only supports a single argument");

				if constexpr (sizeof...(Args) == 1)
				{
					static_assert(std::is_same_v<std::remove_reference_t<std::common_type_t<Args...>>, value_t>, "typeless assignment only supports a typeless argument");

					if constexpr (std::is_same_v<std::remove_reference_t<std::common_type_t<Args...>>, value_t>)
					{
						const value_t& rhs = static_cast<const value_t&>(std::get<0>(std::forward_as_tuple(args...)));
						td().copy(&valueReference(), &rhs);
					}
				}

				return valueReference();
			}
			else
			{
				return valueReference() = T(std::forward<Args>(args)...);
			}
		}

		constexpr value_t& valueMove(value_t&& rhs)
		{
			if constexpr (IsTypeless)
			{
				td().copy(&valueReference(), &rhs);
				return valueReference();
			}
			else
			{
				return valueReference() = std::move(rhs);
			}
		}

		void valueSwap(value_t& rhs)
		{
			if constexpr (IsTypeless)
			{
				td().swap(&valueReference(), &rhs);
			}
			else
			{
				std::swap(valueReference(), rhs);
			}
		}

		constexpr bool valueEqual(const value_t& rhs) const
		{
			if constexpr (IsTypeless)
			{
				return td().equal(&valueReference(), &rhs);
			}
			else
			{
				return valueReference() == rhs;
			}
		}

		constexpr bool valueLess(const value_t& rhs) const
		{
			if constexpr (IsTypeless)
			{
				return td().lessThan(&valueReference(), &rhs);
			}
			else
			{
				return valueReference() < rhs;
			}
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