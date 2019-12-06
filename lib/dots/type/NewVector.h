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

		virtual NewVector& operator = (const NewVector& /*rhs*/)
		{
			// note that this is only a necessary dummy implementation that always gets overriden by sub-class
			return *this;
		}
		
		virtual NewVector& operator = (NewVector&& /*rhs*/) noexcept
		{
			// note that this is only a necessary dummy implementation that always gets overriden by sub-class
			return *this;
		}

		virtual size_t typelessSize() const noexcept = 0;
		virtual NewTypeless& typelessAt(size_t pos) = 0;
		virtual const NewTypeless& typelessAt(size_t pos) const = 0;
		virtual const NewTypeless* typelessData() const = 0;
		virtual NewTypeless* typelessData() = 0;
		virtual void typelessResize(size_t n) = 0;

	protected:

		NewVector() = default;
		NewVector(const NewVector& other) = default;
		NewVector(NewVector&& other) = default;
	};

	template <typename T>
	struct NewVector<T> : NewVector<NewTypeless>, std::vector<std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>>
	{
		using vector_t = std::vector<std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>>;
		
		NewVector() = default;
		
		NewVector(const NewVector& other) = default;
		NewVector(NewVector&& other) = default;

		using vector_t::vector;

		NewVector(const vector_t& other) noexcept :
			vector_t(other)
		{
			/* do nothing */
		}

		NewVector(vector_t&& other) noexcept :
			vector_t(std::move(other))
		{
			/* do nothing */
		}
		
		// ReSharper disable CppHidingFunction
		~NewVector() = default; // note: hiding a non-virtual destructor is harmless for stateless sub-classes
		// ReSharper restore CppHidingFunction

		NewVector& operator = (const NewVector& rhs) = default;
		NewVector& operator = (NewVector&& rhs) = default;

		using vector_t::operator=;

		NewVector& operator = (const vector_t& rhs) noexcept
		{
			return vector_t::operator=(rhs);
		}
		
		NewVector& operator = (vector_t&& rhs) noexcept
		{
			return vector_t::operator=(std::move(rhs));
		}

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
			return vector_t::size();
		}

		const NewTypeless& typelessAt(size_t pos) const override
		{
			return NewTypeless::From(vector_t::operator[](pos));
		}
		
		NewTypeless& typelessAt(size_t pos) override
		{
			return NewTypeless::From(vector_t::operator[](pos));
		}		

		NewTypeless* typelessData() override
		{
			return NewTypeless::From(vector_t::data());
		}
		
		const NewTypeless* typelessData() const override
		{
			return NewTypeless::From(vector_t::data());
		}

		void typelessResize(size_t n) override 
		{
			vector_t::resize(n);
		}
	};
}