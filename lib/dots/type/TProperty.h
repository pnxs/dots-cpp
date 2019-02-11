#pragma once
#include <string_view>
#include <type_traits>
#include <iostream>
#include "Registry.h"
#include "Struct.h"
#include "StructDescriptor.h"

namespace dots::type
{
	class Descriptor;
	class StructProperty;

	template <typename>
	struct TStruct;

    template <typename T, typename Derived, typename Previous, typename DerivedStruct>
    struct TProperty
    {
		static_assert(std::conjunction_v<std::negation<std::is_pointer<T>>, std::negation<std::is_reference<T>>>);

		using value_t = T;
		using struct_t = DerivedStruct;

		struct init_t
		{
			using property_t = TProperty;
			template <typename... Args>
			init_t(Args&&... args) : value(std::forward<Args>(args)...) {}
			T value;
		};		

		template <typename U, std::enable_if_t<!std::disjunction_v<std::is_same<std::remove_reference_t<U>, TProperty>, std::is_same<std::remove_reference_t<U>, Derived>>, int> = 0>
		Derived& operator = (U&& rhs)
		{
			constructOrAssign(std::forward<U>(rhs));
			return static_cast<Derived&>(*this);
		}

		template <typename... Args>
		T& operator () (Args&&... args)
		{
			if (isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to construct already valid property: " } + DerivedStruct::Description.name.data() + "." + Name().data() };
			}

			if constexpr (sizeof...(Args) == 0)
			{
				static_assert(std::is_default_constructible_v<T>);
				construct(T{});
			}
			else
			{
				construct(std::forward<Args>(args)...);
			}

			return rawValue();
		}

		T& operator () (init_t&& init) 
		{
			return (*this)(std::move(init.value));
		}

		T& operator * ()
		{
			return validValue();
		}

		const T& operator * () const
		{
			return validValue();
		}

		T* operator -> ()
		{
			return &validValue();
		}

		const T* operator -> () const
		{
			return &validValue();
		}

		operator T& ()
		{
			return validValue();
		}

		operator const T& () const
		{
			return validValue();
		}

		bool operator == (const T& rhs) const
		{
			return isValid() && rawValue() == rhs;
		}

		bool operator != (const T& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const T& rhs) const
		{
			return isValid() && rawValue() < rhs;
		}

		bool operator == (const Derived& rhs) const
		{
			return isValid() && rhs.isValid() && rawValue() == rhs.rawValue();
		}

		bool operator != (const Derived& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const Derived& rhs) const
		{
			return !rhs.isValid() || isValid() && rawValue() < rhs.rawValue();
		}

		template <bool AssertValidity = true>
		T&& extract()
        {
	        if constexpr (AssertValidity)
			{
				throw std::runtime_error{ std::string{ "attempt to extract invalid property: " } +DerivedStruct::Description.name.data() + "." + Name().data() };
			}

			validPropertySet().set(Tag(), false);
			return std::move(rawValue());
        }

		void swap(Derived& other)
        {
	        if (isValid())
	        {
		        if (other.isValid())
		        {
					std::swap(rawValue(), other.rawValue());
		        }
				else
				{
					other(extract<false>());
				}
	        }
			else if (other.isValid())
			{
				(*this)(other.template extract<false>());
			}
        }

		bool isValid() const
		{
			return validPropertySet().test(Tag());
		}

		void publish() const
        {
	        if (!isValid())
	        {
				throw std::runtime_error{ std::string{ "attempt to publish invalid property: " } + DerivedStruct::Description.name.data() + "." + Name().data() };
	        }

			instance()._publish(PropertySet());
        }

		template <typename... Args>
		static void Publish(Args&&... args)
        {
			DerivedStruct derivedStruct{ init_t{ std::forward<Args>(args)... } };
			Get(derivedStruct).publish();
        }

		static const type::Descriptor& Descriptor()
		{
			static const type::Descriptor* descriptor = type::getDescriptor<T>(nullptr);
			return *descriptor;
		}

		static constexpr property_set PropertySet()
        {
			property_set propertySet;
			propertySet.set(Tag());

			return propertySet;
        }

		static constexpr bool IsPartOf(const property_set& propertySet)
		{
			return propertySet.test(Tag());
		}

        static constexpr std::string_view Name()
        {
            return Derived::Description.name;
        }

        static constexpr size_t Offset()
        {
            return Derived::Description.offset;
        }

        static constexpr uint32_t Tag()
        {
            return Derived::Description.tag;
        }

        static constexpr bool IsKey()
        {
            return Derived::Description.isKey;
        }

    protected:

		TProperty() = default;

		TProperty(const TProperty& other)
		{
			*this = other;
		}

		TProperty(TProperty&& other)
		{
			*this = std::move(other);
		}

		~TProperty()
		{
			destroy();
		}

		TProperty& operator = (const TProperty& rhs)
		{
			if (isValid())
			{
				if (rhs.isValid())
				{
					rawValue() = rhs.rawValue();
				}
				else
				{
					destroy();
				}
			}
			else if (rhs.isValid())
			{
				(*this)(rawValue());
			}

			return *this;
		}

		TProperty& operator = (TProperty&& rhs)
		{
			if (isValid())
			{
				if (rhs.isValid())
				{
					rawValue() = rhs.template extract<false>();
				}
				else
				{
					destroy();
				}
			}
			else if (rhs.isValid())
			{
				(*this)(rhs.template extract<false>());
			}

			return *this;
		}

		static constexpr Struct::PropertyDescription MakePropertyDescription(uint32_t tag, const std::string_view& type, const std::string_view& name, bool isKey)
		{
			return Struct::PropertyDescription{ CalculateOffset(), tag, isKey, name, type };
		}

    private:

		template <typename>
		friend struct TStruct;	

		property_set& validPropertySet()
		{
			return reinterpret_cast<property_set&>(instance());
		}

		const property_set& validPropertySet() const
		{
			return const_cast<TProperty&>(*this).validPropertySet();
		}

		Struct& instance()
		{
			return *reinterpret_cast<Struct*>(this - Offset());
		}

		const Struct& instance() const
		{
			return const_cast<TProperty&>(*this).instance();
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

		template <typename... Args>
		T& assign(Args&&... args)
		{
			rawValue() = T(std::forward<Args>(args)...);
			validPropertySet().set(Tag(), true);

			return rawValue();
		}

		template <typename... Args>
		T& construct(Args&&... args)
		{
			::new (static_cast<void *>(::std::addressof(_value))) T(std::forward<Args>(args)...);
			validPropertySet().set(Tag(), true);

			return rawValue();
		}

		void destroy()
		{
			if (isValid())
			{
				rawValue().~T();
				validPropertySet().set(Tag(), false);
			}
		}

		T& validValue()
		{
			if (!isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to access invalid property: " } + DerivedStruct::Description.name.data() + "." + Name().data() };
			}

			return rawValue();
		}

		const T& validValue() const
		{
			return const_cast<TProperty&>(*this).validValue();
		}

		T& rawValue()
		{
			return reinterpret_cast<T&>(*&_value);
		}

		const T& rawValue() const
		{
			return const_cast<TProperty&>(*this).rawValue();
		}

		static Derived& Get(Struct& instance)
		{
			return *reinterpret_cast<Derived*>(&instance + Offset());
		}

		static const Derived& Get(const Struct& instance)
		{
			return Get(const_cast<Struct&>(instance));
		}

		static constexpr size_t CalculateOffset()
		{
			constexpr size_t currentOffset = []()
			{
				if constexpr (std::is_same_v<Previous, void>)
				{
					return sizeof(Struct);
				}
				else
				{
					return Previous::Offset() + sizeof(typename Previous::value_t);
				}
			}();

			constexpr size_t alignment = alignof(value_t);
			constexpr size_t alignedOffset = currentOffset + (alignment - currentOffset % alignment) % alignment;

			return alignedOffset;
		}

        std::aligned_storage<sizeof(T)> _value;
    };

	template <typename T, typename Derived, typename Previous, typename DerivedStruct>
	std::ostream& operator << (std::ostream& os, const TProperty<T, Derived, Previous, DerivedStruct>& property)
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