#pragma once
#include <type_traits>
#include <dots/type/NewProperty.h>

namespace dots::type
{
	template <typename T, typename Derived>
	struct NewStaticProperty : NewProperty<T, Derived>
	{
		using NewProperty<T, Derived>::operator=;
		
		static constexpr const NewDescriptor<T>& ValueDescriptor()
		{
			return Derived::Descriptor.valueDescriptor();
		}

		static constexpr const std::string& Name()
		{
			return Derived::Descriptor.name();
		}

		static constexpr size_t Offset()
		{
			return Derived::Descriptor.offset();
		}

		static constexpr uint32_t Tag()
		{
			return Derived::Descriptor.tag();
		}

		static constexpr bool IsKey()
		{
			return Derived::Descriptor.isKey();
		}

		static constexpr NewPropertySet Set()
		{
			return Derived::Descriptor.set();
		}

		static constexpr bool IsPartOf(const NewPropertySet& propertySet)
		{
			return Set() <= propertySet;
		}

		template <typename... Args>
		static void Publish(Args&&... args)
		{
			Derived::struct_t::_Publish(std::forward<Args>(args)...);
		}

	protected:

		NewStaticProperty()
		{
			/* do nothing */
		}

		NewStaticProperty(const NewStaticProperty& other) : NewProperty<T, Derived>()
		{
			*this = other;
		}

		NewStaticProperty(NewStaticProperty&& other)
		{
			*this = std::move(other);
		}

		~NewStaticProperty()
		{
			NewProperty<T, Derived>::destroy();
		}

		NewStaticProperty& operator = (const NewStaticProperty& rhs)
		{
			if (rhs.isValid())
			{
				NewProperty<T, Derived>::constructOrAssign(static_cast<const Derived&>(rhs));
			}
			else
			{
				NewProperty<T, Derived>::destroy();
			}

			return *this;
		}

		NewStaticProperty& operator = (NewStaticProperty&& rhs)
		{
			if (rhs.isValid())
			{
				NewProperty<T, Derived>::constructOrAssign(static_cast<Derived&&>(rhs));
			}
			else
			{
				NewProperty<T, Derived>::destroy();
			}

			return *this;
		}

	private:

		friend struct NewProperty<T, Derived>;

		T& derivedStorage()
		{
			return m_storage;
		}

		const T& derivedStorage() const
		{
			return const_cast<NewStaticProperty&>(*this).derivedStorage();
		}

		static const NewPropertyDescriptor<T>& derivedDescriptor()
		{
			return Derived::Descriptor;
		}

		union
		{
			T m_storage;
			std::aligned_storage_t<sizeof(T), alignof(T)> m_data;
		};
	};
}