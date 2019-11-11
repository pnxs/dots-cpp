#pragma once
#include <dots/type/NewProperty.h>

namespace dots::type
{
	template <typename T = NewTypeless>
	struct NewProxyProperty : NewProperty<T, NewProxyProperty<T>>
	{
		NewProxyProperty(T& value, const NewPropertyDescriptor<T>& descriptor) :
			m_value(&value),
			m_descriptor(&descriptor)
		{
			/* do nothing */
		}
		NewProxyProperty(NewPropertyArea& area, const NewPropertyDescriptor<T>& descriptor) :
			NewProxyProperty(area.getProperty<T>(descriptor.offset()), descriptor)
		{
			/* do nothing */
		}
		template <typename Derived, std::enable_if_t<!NewProperty<T, NewProxyProperty<T>>::IsTypeless && !std::is_same_v<Derived, NewProxyProperty<T>>, int> = 0>
		NewProxyProperty(NewProperty<T, Derived>& property) :
			NewProxyProperty(property.storage(), property.descriptor())
		{
			/* do nothing */
		}
		template <typename U, typename Derived, std::enable_if_t<NewProperty<T, NewProxyProperty<T>>::IsTypeless && !std::is_same_v<Derived, NewProxyProperty<T>>, int> = 0>
		NewProxyProperty(NewProperty<U, Derived>& property) :
			NewProxyProperty(NewTypeless::From(property.storage()), property.descriptor())
		{
			/* do nothing */
		}
		NewProxyProperty(const NewProxyProperty& other) = default;
		NewProxyProperty(NewProxyProperty&& other) = default;
		~NewProxyProperty() = default;

		NewProxyProperty& operator = (const NewProxyProperty& rhs) = default;
		NewProxyProperty& operator = (NewProxyProperty&& rhs) = default;

	private:

		friend struct NewProperty<T, NewProxyProperty<T>>;

		T& derivedStorage()
		{
			return *m_value;
		}

		const T& derivedStorage() const
		{
			return const_cast<NewProxyProperty&>(*this).derivedValue();
		}

		const NewPropertyDescriptor<T>& derivedDescriptor() const
		{
			return *m_descriptor;
		}

		T* m_value;
		const NewPropertyDescriptor<T>* m_descriptor;
	};
}