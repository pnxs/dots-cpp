#pragma once
#include <string_view>
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
		
        ~TProperty() = default;
		TProperty(const TProperty& other) = default;
		TProperty(TProperty&& other) = default;

		TProperty& operator = (const TProperty& rhs) = default;
        TProperty& operator = (TProperty&& rhs) = default;		

		template <typename U>
		Derived& operator = (U&& rhs) &
		{
			_value = std::forward<U>(rhs);
			validPropertySet().set(Tag(), true);

			return static_cast<Derived&>(*this);
		}

		template <typename... Args>
		Derived& operator () (Args&&... args) &
		{
			if constexpr (sizeof...(Args) == 0)
			{
				return (*this) = T{};
			}
			else
			{
				return (*this) = T(std::forward<Args>(args)...);
			}
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
			return value();
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
			return isValid() && _value == rhs;
		}

		bool operator != (const T& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const T& rhs) const
		{
			return isValid() && _value < rhs;
		}

		bool operator == (const Derived& rhs) const
		{
			return isValid() && rhs.isValid() && _value == rhs._value;
		}

		bool operator != (const Derived& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator < (const Derived& rhs) const
		{
			return !rhs.isValid() || isValid() && _value < rhs._value;
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

		template <typename, typename, typename, typename>
		friend std::ostream& operator<< (std::ostream& stream, const TProperty& property);

		TProperty() = default;

		T& value()
		{
			
			return _value;
		}

		const T& value() const
		{
			return const_cast<TProperty&>(*this).value();
		}

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

        value_t _value;
    };
}