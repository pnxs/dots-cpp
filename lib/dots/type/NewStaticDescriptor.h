#pragma once
#include <type_traits>
#include <string_view>
#include <sstream>
#include <iomanip>
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
				::new(static_cast<void*>(::std::addressof(value))) T(std::forward<Args>(args)...);
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
				value = T(std::forward<Args>(args)...);
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

		bool lessEqual(const NewTypeless& lhs, const NewTypeless& rhs) const override
		{
			return lessEqual(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
		}

		constexpr bool lessEqual(const T& lhs, const T& rhs) const
		{
			return !greater(lhs, rhs);
		}

		bool greater(const NewTypeless& lhs, const NewTypeless& rhs) const override
		{
			return greater(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
		}

		constexpr bool greater(const T& lhs, const T& rhs) const
		{
			return less(rhs, lhs);
		}

		bool greaterEqual(const NewTypeless& lhs, const NewTypeless& rhs) const override
		{
			return greaterEqual(reinterpret_cast<const T&>(lhs), reinterpret_cast<const T&>(rhs));
		}

		constexpr bool greaterEqual(const T& lhs, const T& rhs) const
		{
			return !less(lhs, rhs);
		}

		bool usesDynamicMemory() const override
		{
			return false;
		}

		size_t dynamicMemoryUsage(const NewTypeless& value) const override
		{
			return dynamicMemoryUsage(reinterpret_cast<const T&>(value));
		}

		constexpr size_t dynamicMemoryUsage(const T&/* value*/) const
		{
			return 0;
		}

		void fromString(NewTypeless& storage, const std::string_view& value) const override
		{
			fromString(storage.to<T>(), value);
		}

		constexpr void fromString(T& storage, const std::string_view& value) const
		{
			// TODO: use std::from_chars where applicable
			
			if constexpr (std::is_same_v<T, bool>)
			{
				if (value == "1" || value == "true")
				{
					construct(storage, true);
				}
				else if (value == "0" || value == "false")
				{
					construct(storage, false);
				}
				else
				{
					throw std::runtime_error{ "cannot construct boolean from string: " + std::string{ value } };
				}
				
			}
			else if constexpr (std::is_constructible_v<T, std::string_view>)
			{
				construct(storage, value);
			}
			else if constexpr (std::disjunction_v<std::is_integral<T>, std::is_floating_point<T>, is_istreamable<T>>)
			{
				T t;
				std::istringstream iss{ value.data() };
				iss.exceptions(std::istringstream::failbit | std::istringstream::badbit);
				iss >> t;
				construct(storage, std::move(t));
			}
			else
			{
				return NewDescriptor<>::fromString(NewTypeless::From(storage), value);
			}
		}
		
		std::string toString(const NewTypeless& value) const override
		{
			return toString(value.to<T>());
		}

		std::string toString(const T& value) const
		{
			// TODO: use std::to_chars where applicable
			
			if constexpr (std::is_same_v<T, bool>)
			{
				return value ? "true" : "false";
			}
			else if constexpr (std::is_integral_v<T>)
			{
				return std::to_string(value);
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				std::ostringstream oss;
				oss.exceptions(std::istringstream::failbit | std::istringstream::badbit);
				oss << std::setprecision(std::numeric_limits<T>::digits10 + 1) << value;

				return oss.str();
			}
			else if constexpr (std::is_constructible_v<std::string, T>)
			{
				return value;
			}
			else if constexpr (is_ostreamable_v<T>)
			{
				std::ostringstream oss;
				oss.exceptions(std::ostringstream::failbit | std::ostringstream::badbit);
				oss << value;

				return oss.str();
			}
			else
			{
				return NewDescriptor<>::toString(NewTypeless::From(value));
			}
		}

		static const NewDescriptor<T>& Instance()
		{
			static NewDescriptor<T> Instance_;
			return Instance_;
		}

	private:

		template<typename U, typename = void>
		struct is_ostreamable: std::false_type {};
		template<typename U>
		struct is_ostreamable<U, std::void_t<decltype(std::declval<std::ostream&>()<<std::declval<const U&>())>> : std::true_type {};
		template <typename U>
		using is_ostreamable_t = typename is_ostreamable<U>::type;
		template <typename U>
		static constexpr bool is_ostreamable_v = is_ostreamable_t<U>::value;

		template<typename U, typename = void>
		struct is_istreamable: std::false_type {};
		template<typename U>
		struct is_istreamable<U, std::void_t<decltype(std::declval<std::istream&>()>>std::declval<U&>())>> : std::true_type {};
		template <typename U>
		using is_istreamable_t = typename is_istreamable<U>::type;
		template <typename U>
		static constexpr bool is_istreamable_v = is_istreamable_t<U>::value;
	};
}