#pragma once
#include <type_traits>
#include <dots/type/NewProperty.h>

namespace dots::type
{
	template <typename T, typename Derived>
	struct StaticProperty : Property<T, Derived>
	{
		using Property<T, Derived>::operator=;
		
		static constexpr const Descriptor<T>& ValueDescriptor()
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

		static constexpr PropertySet Set()
		{
			return Derived::Descriptor.set();
		}

		static constexpr bool IsPartOf(const PropertySet& propertySet)
		{
			return Set() <= propertySet;
		}

		template <typename... Args>
		static void Publish(Args&&... args)
		{
			Derived::struct_t::_Publish(std::forward<Args>(args)...);
		}

	protected:

		StaticProperty()
		{
			/* do nothing */
		}

		StaticProperty(const StaticProperty& other) : Property<T, Derived>()
		{
			*this = other;
		}

		StaticProperty(StaticProperty&& other)
		{
			*this = std::move(other);
		}

		~StaticProperty()
		{
			Property<T, Derived>::destroy();
		}

		StaticProperty& operator = (const StaticProperty& rhs)
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

		StaticProperty& operator = (StaticProperty&& rhs)
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

	private:

		friend struct Property<T, Derived>;

		T& derivedStorage()
		{
			return m_storage;
		}

		const T& derivedStorage() const
		{
			return const_cast<StaticProperty&>(*this).derivedStorage();
		}

		static const PropertyDescriptor<T>& derivedDescriptor()
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