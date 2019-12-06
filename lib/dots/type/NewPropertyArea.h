#pragma once
#include <dots/type/NewPropertySet.h>

namespace dots::type
{
	struct NewPropertyArea
	{
		constexpr NewPropertyArea() = default;
		
		constexpr NewPropertyArea(const NewPropertyArea& /*other*/)
		{
			/* do nothing */
		}
		
		constexpr NewPropertyArea(NewPropertyArea&& /*other*/) noexcept
		{
			/* do nothing */
		}
		
		~NewPropertyArea() = default;

		constexpr NewPropertyArea& operator = (const NewPropertyArea& /*rhs*/)
		{
			return *this;
		}
		
		constexpr NewPropertyArea& operator = (NewPropertyArea&& /*rhs*/) noexcept
		{
			return *this;
		}

		constexpr const NewPropertySet& validProperties() const
		{
			return m_validProperties;
		}

		constexpr NewPropertySet& validProperties()
		{
			return m_validProperties;
		}

		template <typename P>
    	const P& getProperty(size_t offset) const
		{
			return *reinterpret_cast<const P*>(reinterpret_cast<const char*>(this) + offset);
		}

    	template <typename P>
		P& getProperty(size_t offset)
		{
			return const_cast<P&>(std::as_const(*this).getProperty<P>(offset));
		}

		template <typename P>
    	const P& getProperty() const
		{
			return getProperty<P>(P::Offset());
		}

    	template <typename P>
		P& getProperty()
		{
			return const_cast<P&>(std::as_const(*this).getProperty<P>());
		}

		template <typename P>
    	static const NewPropertyArea& GetArea(const P& property, size_t offset)
		{
			return *reinterpret_cast<const NewPropertyArea*>(reinterpret_cast<const char*>(&property) - offset);
		}

    	template <typename P>
		static NewPropertyArea& GetArea(P& property, size_t offset)
		{
			return const_cast<NewPropertyArea&>(GetProperty(std::as_const(property), offset));
		}

		template <typename P>
    	static const NewPropertyArea& GetArea(const P& /*property*/)
		{
			return GetArea(P::Offset());
		}

    	template <typename P>
		static NewPropertyArea& GetArea(P& property)
		{
			return const_cast<NewPropertyArea&>(GetProperty(std::as_const(property)));
		}

	private:

		NewPropertySet m_validProperties;
	};
}