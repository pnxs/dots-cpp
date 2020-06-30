#pragma once
#include <dots/type/Property.h>

namespace dots::type
{
	template <typename T = Typeless>
	struct ProxyProperty : Property<T, ProxyProperty<T>>
	{
		ProxyProperty(T& value, const PropertyDescriptor& descriptor) :
			m_value(&value),
			m_descriptor(&descriptor)
		{
			/* do nothing */
		}
		ProxyProperty(PropertyArea& area, const PropertyDescriptor& descriptor) :
			ProxyProperty(area.getProperty<T>(descriptor.offset()), descriptor)
		{
			/* do nothing */
		}
		template <typename Derived, std::enable_if_t<!Property<T, ProxyProperty<T>>::IsTypeless && !std::is_same_v<Derived, ProxyProperty<T>>, int> = 0>
		ProxyProperty(Property<T, Derived>& property) :
			ProxyProperty(property.storage(), property.descriptor())
		{
			/* do nothing */
		}
		template <typename U, typename Derived, std::enable_if_t<Property<T, ProxyProperty<T>>::IsTypeless && !std::is_same_v<Derived, ProxyProperty<T>>, int> = 0>
		ProxyProperty(Property<U, Derived>& property) :
			ProxyProperty(Typeless::From(property.storage()), property.descriptor())
		{
			/* do nothing */
		}
		ProxyProperty(const ProxyProperty& other) = default;
		ProxyProperty(ProxyProperty&& other) = default;
		~ProxyProperty() = default;

		ProxyProperty& operator = (const ProxyProperty& rhs) = default;
		ProxyProperty& operator = (ProxyProperty&& rhs) = default;

	private:

		friend struct Property<T, ProxyProperty<T>>;

		T& derivedStorage()
		{
			return *m_value;
		}

		const T& derivedStorage() const
		{
			return const_cast<ProxyProperty&>(*this).derivedValue();
		}

		const PropertyDescriptor& derivedDescriptor() const
		{
			return *m_descriptor;
		}

		T* m_value;
		const PropertyDescriptor* m_descriptor;
	};
}