#pragma once
#include <string_view>
#include <type_traits>
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
		using value_t = T;
		using struct_t = DerivedStruct;
		
        ~TProperty()
        {
	        if (isValid())
	        {
				rawValue().~T();
	        }
        }
		TProperty(const TProperty& other) = default;
		TProperty(TProperty&& other) = default;

		TProperty& operator = (const TProperty& rhs) = default;
        TProperty& operator = (TProperty&& rhs) = default;		

		template <typename U>
		Derived& operator = (U&& rhs) &
		{
			constructOrAssign(std::forward<U>(rhs));
			return static_cast<Derived&>(*this);
		}

		template <typename... Args>
		Derived& operator () (Args&&... args) &
		{
			if (isValid())
			{
				throw std::runtime_error{ std::string{ "attempt to construct already valid property: " } + DerivedStruct::Description.name.data() + "." + Name().data() };
			}

			if constexpr (sizeof...(Args) == 0)
			{
				construct(T{});
			}
			else
			{
				construct(std::forward<Args>(args)...);
			}

			return static_cast<Derived&>(*this);
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
			return isValid() && validValue() == rhs;
		}

		bool operator != (const T& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const T& rhs) const
		{
			return isValid() && validValue() < rhs;
		}

		bool operator == (const Derived& rhs) const
		{
			return isValid() && rhs.isValid() && validValue() == rhs._value;
		}

		bool operator != (const Derived& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const Derived& rhs) const
		{
			return !rhs.isValid() || isValid() && validValue() < rhs._value;
		}

		bool isValid() const
		{
			return validPropertySet().test(Tag());
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

		static constexpr Struct::PropertyDescription MakePropertyDescription(uint32_t tag, const std::string_view& type, const std::string_view& name, bool isKey)
		{
			return Struct::PropertyDescription{ CalculateOffset(), tag, isKey, name, type };
		}

    private:

		template <typename>
		friend struct TStruct;
		
    	friend DerivedStruct;

		TProperty() = default;

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
}