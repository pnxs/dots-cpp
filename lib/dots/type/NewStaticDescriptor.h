#pragma once
#include <dots/type/NewDescriptor.h>

namespace dots::type
{
	template <typename T, typename Base = NewDescriptor<NewTypeless>>
	struct NewStaticDescriptor : Base
	{
		template <typename Base_ = Base, std::enable_if_t<std::is_same_v<Base_, NewDescriptor<NewTypeless>>, int> = 0>
		constexpr NewStaticDescriptor(NewType type, std::string name) : Base(type, std::move(name), sizeof(T), alignof(T))
		{
			/* do nothing */
		}
		
		template <typename Base_ = Base, typename... Args, std::enable_if_t<!std::is_same_v<Base_, NewDescriptor<NewTypeless>>, int> = 0>
		constexpr NewStaticDescriptor(Args&&... args) : Base(std::forward<Args>(args)...)
		{
			/* do nothing */
		}
		
		NewStaticDescriptor(const NewStaticDescriptor& other) = default;
		NewStaticDescriptor(NewStaticDescriptor&& other) = default;
		~NewStaticDescriptor() = default;

		NewStaticDescriptor& operator = (const NewStaticDescriptor& rhs) = default;
		NewStaticDescriptor& operator = (NewStaticDescriptor&& rhs) = default;

		NewTypeless& construct(NewTypeless& value) const override
		{
			return reinterpret_cast<NewTypeless&>(construct(reinterpret_cast<T&>(value)));
		}

		NewTypeless& construct(NewTypeless& value, const NewTypeless& other) const override
		{
			return reinterpret_cast<NewTypeless&>(construct(reinterpret_cast<T&>(value), reinterpret_cast<const T&>(other)));
		}

		NewTypeless& construct(NewTypeless& value, NewTypeless&& other) const override
		{
			return reinterpret_cast<NewTypeless&>(construct(reinterpret_cast<T&>(value), reinterpret_cast<T&&>(other)));
		}

		template <typename... Args>
		constexpr T& construct(T& value, Args&&... args) const
		{
			static_assert(std::is_constructible_v<T, Args...>, "type is not constructible from passed arguments");
			if constexpr (std::is_constructible_v<T, Args...>)
			{
				::new(static_cast<void*>(::std::addressof(value))) T{ std::forward<Args>(args)... };
			}

			return value;
		}

		void destruct(NewTypeless& value) const override
		{
			destruct(reinterpret_cast<T&>(value));
		}

		constexpr void destruct(T& value) const
		{
			value.~T();
		}

		NewTypeless& assign(NewTypeless& lhs, const NewTypeless& rhs) const override
		{
			return reinterpret_cast<NewTypeless&>(assign(reinterpret_cast<T&>(lhs), reinterpret_cast<const T&>(rhs)));
		}

		NewTypeless& assign(NewTypeless& lhs, NewTypeless&& rhs) const override
		{
			return reinterpret_cast<NewTypeless&>(assign(reinterpret_cast<T&>(lhs), reinterpret_cast<T&&>(rhs)));
		}

		template <typename... Args>
		constexpr T& assign(T& value, Args&&... args) const
		{
			static_assert(std::is_constructible_v<T, Args...>, "type is not constructible from passed arguments");
			if constexpr (std::is_constructible_v<T, Args...>)
			{
				value = T{ std::forward<Args>(args)... };
			}

			return value;
		}

		void swap(NewTypeless& value, NewTypeless& other) const override
		{
			swap(reinterpret_cast<T&>(value), reinterpret_cast<T&>(other));
		}

		constexpr void swap(T& lhs, T& rhs) const
		{
			std::swap(lhs, rhs);
		}

		bool equal(const NewTypeless& lhs, const NewTypeless& rhs) const override
		{
			return equal(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
		}

		constexpr bool equal(const T& lhs, const T& rhs) const
		{
			return std::equal_to<T>{}(lhs, rhs);
		}

		bool less(const NewTypeless& lhs, const NewTypeless& rhs) const override
		{
			return less(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
		}

		constexpr bool less(const T& lhs, const T& rhs) const
		{
			return std::less<T>{}(lhs, rhs);
		}

		bool usesDynamicMemory() const override
		{
			return false;;
		}

		size_t dynamicMemoryUsage(const NewTypeless& value) const override
		{
			return dynamicMemoryUsage(reinterpret_cast<const T&>(value));
		}

		constexpr size_t dynamicMemoryUsage(const T&/* value*/) const
		{
			return 0;
		}

		static const NewDescriptor<T>& Instance()
		{
			static NewDescriptor<T> Instance_;
			return Instance_;
		}
	};
}