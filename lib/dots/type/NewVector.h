#pragma once
#include <vector>
#include <dots/type/NewTypeless.h>

namespace dots::type
{
	template <typename T = NewTypeless, typename = void>
	struct NewVector;

	template <>
	struct NewVector<NewTypeless>
	{		
		virtual ~NewVector() = default;

		virtual NewVector& operator = (const NewVector& rhs)
		{
			// note that this is only a necessary dummy implementation that always gets overriden by sub-class
			return *this;
		}
		
		virtual NewVector& operator = (NewVector&& rhs) noexcept
		{
			// note that this is only a necessary dummy implementation that always gets overriden by sub-class
			return *this;
		}

		virtual size_t typelessSize() const noexcept = 0;
		virtual NewTypeless& typelessAt(size_t pos) = 0;
		virtual const NewTypeless& typelessAt(size_t pos) const = 0;
		virtual const NewTypeless* typelessData() const = 0;
		virtual NewTypeless* typelessData() = 0;

	protected:

		NewVector() = default;
		NewVector(const NewVector& other) = default;
		NewVector(NewVector&& other) = default;
	};

	template <typename T>
	struct NewVector<T> : NewVector<NewTypeless>, std::vector<T>
	{
		NewVector() = default;
		NewVector(const NewVector& other) = default;
		NewVector(NewVector&& other) = default;
		// ReSharper disable CppHidingFunction
		~NewVector() = default; // note: hiding a non-virtual destructor is harmless for stateless sub-classes
		// ReSharper restore CppHidingFunction

		NewVector& operator = (const NewVector& rhs) = default;
		NewVector& operator = (NewVector&& rhs) = default;

		NewVector& operator = (const NewVector<NewTypeless>& rhs) override
		{
			// TODO: ensure compatible type
			*this = static_cast<const NewVector&>(rhs);
			return *this;
		}
		
		NewVector& operator = (NewVector<NewTypeless>&& rhs) noexcept override
		{
			// TODO: ensure compatible type
			*this = static_cast<NewVector&&>(rhs);
			return *this;
		}

		size_t typelessSize() const noexcept override
		{
			return std::vector<T>::size();
		}

		const NewTypeless& typelessAt(size_t pos) const override
		{
			return NewTypeless::From(std::vector<T>::operator[](pos));
		}
		
		NewTypeless& typelessAt(size_t pos) override
		{
			return NewTypeless::From(std::vector<T>::operator[](pos));
		}		

		NewTypeless* typelessData() override
		{
			return NewTypeless::From(std::vector<T>::data());
		}
		
		const NewTypeless* typelessData() const override
		{
			return NewTypeless::From(NewTypeless::From(std::vector<T>::data()));
		}

		using std::vector<T>::vector;
		using std::vector<T>::operator=;
	};
}