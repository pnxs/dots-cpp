#pragma once
#include <utility>

namespace dots::type
{
	struct NewTypeless
	{
		NewTypeless() = delete;
		NewTypeless(const NewTypeless& other) = delete;
		NewTypeless(NewTypeless&& other) = delete;
		~NewTypeless() = delete;

		NewTypeless& operator = (const NewTypeless& rhs) = delete;
		NewTypeless& operator = (NewTypeless&& rhs) = delete;

		template <typename T>
		const T& to() const
		{
			return reinterpret_cast<const T&>(*this);
		}

		template <typename T>
		T& to()
		{
			return const_cast<T&>(std::as_const(*this).to<T>());
		}

		template <typename T>
		static const NewTypeless& From(const T& t)
		{
			return reinterpret_cast<const NewTypeless&>(t);
		}

		template <typename T>
		static NewTypeless& From(T& t)
		{
			return reinterpret_cast<NewTypeless&>(t);
		}

		template <typename T>
		static NewTypeless&& From(T&& t)
		{
			return reinterpret_cast<NewTypeless&&>(t);
		}

		template <typename T>
		static const NewTypeless* From(const T* t)
		{
			return reinterpret_cast<const NewTypeless*>(t);
		}

		template <typename T>
		static NewTypeless* From(T* t)
		{
			return reinterpret_cast<NewTypeless*>(t);
		}
	};
}