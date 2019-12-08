#pragma once
#include <string_view>
#include <functional>
#include <type_traits>
#include <iostream>
#include <cstddef>
#include <dots/type/PropertyArea.h>
#include <dots/type/PropertyDescriptor.h>

namespace dots::type
{
	template <typename T, typename Derived>
	struct Property
	{
		static_assert(std::conjunction_v<std::negation<std::is_pointer<T>>, std::negation<std::is_reference<T>>>);
		using value_t = T;
		static constexpr bool IsTypeless = std::is_same_v<T, Typeless>;

		template <typename U, std::enable_if_t<!std::disjunction_v<std::is_same<std::remove_reference_t<U>, Property>, std::is_same<std::remove_reference_t<U>, Derived>>, int> = 0>
		Derived& operator = (U&& rhs)
		{
			Property<T, Derived>::constructOrAssign(std::forward<U>(rhs));
			return static_cast<Derived&>(*this);
		}

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
			return equal(rhs);
		}

		bool operator == (const Derived& rhs) const
		{
			return equal(rhs);
		}

		bool operator != (const T& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator != (const Derived& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const T& rhs) const
		{
			return less(rhs);
		}

		bool operator < (const Derived& rhs) const
		{
			return less(rhs);
		}

		bool operator <= (const T& rhs) const
		{
			return lessEqual(rhs);
		}

		bool operator <= (const Derived& rhs) const
		{
			return lessEqual(rhs);
		}

		bool operator > (const T& rhs) const
		{
			return greater(rhs);
		}

		bool operator > (const Derived& rhs) const
		{
			return greater(rhs);
		}

		bool operator >= (const T& rhs) const
		{
			return greaterEqual(rhs);
		}

		bool operator >= (const Derived& rhs) const
		{
			return greaterEqual(rhs);
		}

		bool isValid() const
		{
			return metadata().set() <= validProperties();
		}

		T& construct(const Derived& rhs)
		{
			if (!rhs.isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to construct from invalid property: " } + metadata().name().data() };
			}
			
			construct(rhs.storage());

			return *this;
		}

		T& construct(Derived&& rhs)
		{
			if (!rhs.isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to construct from invalid property: " } + metadata().name().data() };
			}

			construct(std::move(rhs.storage()));
			rhs.destroy();

			return *this;
		}

		template <typename... Args>
		T& construct(Args&&... args)
		{
			if (isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to construct already valid property: " } + metadata().name().data() };
			}

			static_assert(!IsTypeless || sizeof...(Args) <= 1, "typeless construction only supports default construction or a single argument");
			if constexpr (!IsTypeless || sizeof...(Args) <= 1)
			{
				valueDescriptor().construct(storage(), std::forward<Args>(args)...);
			}
			
			setValid();

			return storage();
		}

		void destroy()
		{
			if (isValid())
			{
				valueDescriptor().destruct(storage());
				setInvalid();
			}
		}

		T& value()
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to access invalid property: " } + metadata().name().data() };
			}

			return storage();
		}

		const T& value() const
		{
			return const_cast<Property&>(*this).value();
		}

		template <typename... Args>
		T& constructOrValue(Args&&... args)
		{
			if (isValid())
			{
				return storage();
			}
			else
			{
				return construct(std::forward<Args>(args)...);
			}
		}

		T& assign(const Derived& rhs)
		{
			if (!rhs.isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to assign from invalid property: " } + metadata().name().data() };
			}
			
			assign(rhs.storage());

			return *this;
		}

		T& assign(Derived&& rhs)
		{
			if (!rhs.isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to assign from invalid property: " } + metadata().name().data() };
			}

			assign(std::move(rhs.storage()));
			rhs.destroy();

			return *this;
		}

		template <typename... Args>
		T& assign(Args&&... args)
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to assign invalid property: " } + metadata().name().data() };
			}

			static_assert(!IsTypeless || sizeof...(Args) <= 1, "typeless assignment only supports a single argument");
			if constexpr (!IsTypeless || sizeof...(Args) <= 1)
			{
				valueDescriptor().assign(storage(), std::forward<Args>(args)...);
			}
			
			setValid();

			return storage();
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

		void swap(Derived& other)
		{
			if (isValid())
			{
				if (other.isValid())
				{
					valueDescriptor().swap(storage(), other);
				}
				else
				{
					other.construct(std::move(storage()));
					destroy();
				}
			}
			else if (other.isValid())
			{
				construct(std::move(other.storage()));
				other.destroy();
			}
		}

		bool equal(const T& rhs) const
		{
			return isValid() && valueDescriptor().equal(storage(), rhs);
		}

		bool equal(const Derived& rhs) const
		{
			if (rhs.isValid())
			{
				return equal(rhs.storage());
			}
			else
			{
				return !isValid();
			}			
		}

		bool less(const T& rhs) const
		{
			return isValid() && valueDescriptor().less(storage(), rhs);
		}

		bool less(const Derived& rhs) const
		{
			if (rhs.isValid())
			{
				return less(rhs.storage());
			}
			else
			{
				return isValid();
			}
		}

		bool lessEqual(const T& rhs) const
		{
			return !greater(rhs);
		}

		bool lessEqual(const Derived& rhs) const
		{
			return !greater(rhs);
		}

		bool greater(const T& rhs) const
		{
			return !isValid() || valueDescriptor().less(rhs, storage());
		}

		bool greater(const Derived& rhs) const
		{
			return rhs.less(*this);
		}

		bool greaterEqual(const T& rhs) const
		{
			return !less(rhs);
		}

		bool greaterEqual(const Derived& rhs) const
		{
			return !less(rhs);
		}

		constexpr const PropertyMetadata<T>& metadata() const
		{
			return static_cast<const Derived&>(*this).derivedMetadata();
		}

		constexpr const PropertyDescriptor<T>& descriptor() const
		{
			return static_cast<const Derived&>(*this).derivedDescriptor();
		}

		constexpr const Descriptor<T>& valueDescriptorPtr() const
		{
			return descriptor().valueDescriptorPtr();
		}

		constexpr const Descriptor<T>& valueDescriptor() const
		{
			return descriptor().valueDescriptor();
		}

		constexpr bool isPartOf(const PropertySet& propertySet) const
		{
			return metadata().set() <= propertySet;
		}

		constexpr T& storage()
		{
			return static_cast<Derived&>(*this).derivedStorage();
		}

		constexpr const T& storage() const
		{
			return const_cast<Property&>(*this).storage();
		}

	protected:

		constexpr Property() = default;
		constexpr Property(const Property& other) = default;
		constexpr Property(Property&& other) = default;
		~Property() = default;

		constexpr Property& operator = (const Property& rhs) = default;
		constexpr Property& operator = (Property&& rhs) = default;

	private:

		const PropertySet& validProperties() const
		{
			return PropertyArea::GetArea(storage(), metadata().offset()).validProperties();
		}

		PropertySet& validProperties()
		{
			return const_cast<PropertySet&>(std::as_const(*this).validProperties());
		}		

		void setValid()
		{
			validProperties() += metadata().set();
		}

		void setInvalid()
		{
			validProperties() -= metadata().set();
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