#pragma once
#include <type_traits>
#include "Registry.h"
#include "Property.h"
#include "TPropertyInitializer.h"

namespace dots::type
{
	class Descriptor;

	template <typename>
	struct TStruct;

	template <typename T, typename Derived, typename Previous, typename DerivedStruct>
	struct TProperty : Property<T, Derived>
	{
		using struct_t = DerivedStruct;
		using init_t = TPropertyInitializer<TProperty>;

		using Property<T, Derived>::operator=;
		using Property<T, Derived>::operator();
		using Property<T, Derived>::construct;

		T& operator () (init_t&& init)
		{
			return (*this)(std::move(init.value));
		}

		T& construct(init_t&& init)
		{
			return Property<T, Derived>::construct(std::move(init.value));
		}

		template <typename... Args>
		static void Publish(Args&&... args)
		{
			DerivedStruct derivedStruct{ init_t{ std::forward<Args>(args)... } };
			Get(derivedStruct).publish();
		}

		static const type::Descriptor& Descriptor()
		{
			static const type::Descriptor* descriptor = []()
			{
				type::getDescriptor<T>(nullptr);
				return Registry::fromWireName(Type().data());
			}();
			return *descriptor;
		}

		static constexpr property_set Set()
		{
			property_set propertySet;
			propertySet.set(Tag());

			return propertySet;
		}

		static constexpr bool IsPartOf(const property_set& propertySet)
		{
			return propertySet.test(Tag());
		}

		static constexpr const std::string_view& Name()
		{
			return Derived::Description.name();
		}

		static constexpr const std::string_view& Type()
		{
			return Derived::Description.type();
		}

		static constexpr size_t Offset()
		{
			return Derived::Description.offset();
		}

		static constexpr uint32_t Tag()
		{
			return Derived::Description.tag();
		}

		static constexpr bool IsKey()
		{
			return Derived::Description.isKey();
		}

	protected:

		TProperty()
		{
			/* do nothing */
		}

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
			Property<T, Derived>::destroy();
		}

		TProperty& operator = (const TProperty& rhs)
		{
			if (rhs.isValid())
			{
				Property<T, Derived>::constructOrAssign(static_cast<const Derived&>(rhs));
			}
			else
			{
				Property<T, Derived>::destroy();
			}

			return *this;
		}

		TProperty& operator = (TProperty&& rhs)
		{
			if (rhs.isValid())
			{
				Property<T, Derived>::constructOrAssign(static_cast<Derived&&>(rhs));
			}
			else
			{
				Property<T, Derived>::destroy();
			}

			return *this;
		}

		static constexpr StructProperty MakePropertyDescription(uint32_t tag, const std::string_view& type, const std::string_view& name, bool isKey)
		{
			return StructProperty{ CalculateOffset(), tag, isKey, name, type };
		}

	private:

		template <typename>
		friend struct TStruct;

		friend struct Property<T, Derived>;

		T& derivedValue()
		{
			return _value;
		}

		const T& derivedValue() const
		{
			return const_cast<TProperty&>(*this).derivedValue();
		}

		static constexpr const StructProperty& derivedDescriptor()
		{
			return Derived::Description;
		}

		static Derived& Get(Struct& instance)
		{
			return *reinterpret_cast<Derived*>(reinterpret_cast<char*>(&instance) + Offset());
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

			constexpr size_t alignment = alignof(T);
			constexpr size_t alignedOffset = currentOffset + (alignment - currentOffset % alignment) % alignment;

			return alignedOffset;
		}

		union
		{
			std::aligned_storage_t<sizeof(T), alignof(T)> _storage;
			T _value;
		};
	};
}