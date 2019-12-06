#pragma once
#include <vector>
#include <type_traits>
#include <string_view>
#include <functional>
#include <algorithm>
#include <dots/type/NewDescriptor.h>
#include <dots/type/NewStaticDescriptor.h>

namespace dots::type
{
	namespace details
	{
		template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
		static std::underlying_type_t<T> underlying_type(T&&);
		template <typename T, std::enable_if_t<!std::is_enum_v<T>, int> = 0>
		static T underlying_type(T&&);

		template <typename T>
		using underlying_type_t = std::decay_t<decltype(underlying_type(std::declval<T>()))>;
	}
	
	template <typename E = NewTypeless>
	struct NewEnumeratorDescriptor;
	
	template <>
	struct NewEnumeratorDescriptor<NewTypeless>
	{
		NewEnumeratorDescriptor(uint32_t tag, std::string name);
		NewEnumeratorDescriptor(const NewEnumeratorDescriptor& other) = default;
		NewEnumeratorDescriptor(NewEnumeratorDescriptor&& other) = default;
		virtual ~NewEnumeratorDescriptor() = default;

		NewEnumeratorDescriptor& operator = (const NewEnumeratorDescriptor& rhs) = default;
		NewEnumeratorDescriptor& operator = (NewEnumeratorDescriptor&& rhs) = default;

		uint32_t tag() const;
		const std::string& name() const;

		virtual std::shared_ptr<NewDescriptor<>> underlyingDescriptorPtr() const = 0;
		virtual const NewDescriptor<NewTypeless>& underlyingDescriptor() const = 0;
		virtual const NewTypeless& valueTypeless() const = 0;

	private:

		uint32_t m_tag;		
		std::string m_name;	
	};
	
	template <typename E>
	struct NewEnumeratorDescriptor : NewEnumeratorDescriptor<NewTypeless>
	{
		using enum_t = E;
		using underlying_type_t = details::underlying_type_t<E>;
		
		NewEnumeratorDescriptor(uint32_t tag, std::string name, enum_t value) :
			NewEnumeratorDescriptor<NewTypeless>(tag, std::move(name)),
			m_value(std::move(value))
		{
			/* do nothing */
		}
		NewEnumeratorDescriptor(const NewEnumeratorDescriptor& other) = default;
		NewEnumeratorDescriptor(NewEnumeratorDescriptor&& other) = default;
		~NewEnumeratorDescriptor() = default;

		NewEnumeratorDescriptor& operator = (const NewEnumeratorDescriptor& rhs) = default;
		NewEnumeratorDescriptor& operator = (NewEnumeratorDescriptor&& rhs) = default;

		std::shared_ptr<NewDescriptor<>> underlyingDescriptorPtr() const override
		{
			return NewDescriptor<underlying_type_t>::InstancePtr();			
		}

		const NewDescriptor<underlying_type_t>& underlyingDescriptor() const override
		{
			return NewDescriptor<underlying_type_t>::Instance();			
		}

		const NewTypeless& valueTypeless() const
		{
			return NewTypeless::From(m_value);
		}
		
		enum_t value() const
		{
			return m_value;
		}

	private:
		
		enum_t m_value;
	};

	template <typename E = NewTypeless, typename = void>
	struct NewEnumDescriptor;

	template <>
	struct NewEnumDescriptor<NewTypeless> : NewDescriptor<NewTypeless>
	{
		using enumerator_ref_t = std::reference_wrapper<NewEnumeratorDescriptor<>>;
		
		NewEnumDescriptor(std::string name, const NewDescriptor<NewTypeless>& underlyingDescriptor);
		NewEnumDescriptor(const NewEnumDescriptor& other) = default;
		NewEnumDescriptor(NewEnumDescriptor&& other) = default;
		~NewEnumDescriptor() = default;

		NewEnumDescriptor& operator = (const NewEnumDescriptor& rhs) = default;
		NewEnumDescriptor& operator = (NewEnumDescriptor&& rhs) = default;

		virtual std::shared_ptr<NewDescriptor<>> underlyingDescriptorPtr() const = 0;
		virtual const NewDescriptor<NewTypeless>& underlyingDescriptor() const = 0;

		virtual const std::vector<enumerator_ref_t>& enumeratorsTypeless() const = 0;

		virtual const NewEnumeratorDescriptor<>& enumeratorFromTag(uint32_t tag) const = 0;
		virtual const NewEnumeratorDescriptor<>& enumeratorFromName(const std::string_view& name) const = 0;
		virtual const NewEnumeratorDescriptor<>& enumeratorFromValue(const NewTypeless& value) const = 0;		
	};

	template <typename E>
	struct NewEnumDescriptor<E> : NewStaticDescriptor<E, NewEnumDescriptor<NewTypeless>>
	{
		using enum_t = E;
		using underlying_type_t = details::underlying_type_t<E>;
		
		NewEnumDescriptor(std::string name, std::vector<NewEnumeratorDescriptor<E>> enumeratorDescriptors) :
			NewStaticDescriptor<E, NewEnumDescriptor<NewTypeless>>(std::move(name), underlyingDescriptor()),
			m_enumerators{ std::move(enumeratorDescriptors) }
		{
			for (NewEnumeratorDescriptor<>& enumerator : m_enumerators)
			{
				m_enumeratorsTypeless.emplace_back(std::ref(enumerator));
			}
		}
		NewEnumDescriptor(const NewEnumDescriptor& other) = delete;
		NewEnumDescriptor(NewEnumDescriptor&& other) = default;
		~NewEnumDescriptor() = default;

		NewEnumDescriptor& operator = (const NewEnumDescriptor& rhs) = delete;
		NewEnumDescriptor& operator = (NewEnumDescriptor&& rhs) = default;

		std::shared_ptr<NewDescriptor<>> underlyingDescriptorPtr() const override
		{
			return NewDescriptor<underlying_type_t>::InstancePtr();			
		}

		const NewDescriptor<underlying_type_t>& underlyingDescriptor() const override
		{
			return NewDescriptor<underlying_type_t>::Instance();			
		}

		const std::vector<NewEnumDescriptor<>::enumerator_ref_t>& enumeratorsTypeless() const override
		{
			return m_enumeratorsTypeless;
		}

		const std::vector<NewEnumeratorDescriptor<E>>& enumerators() const
		{
			return m_enumerators;
		}

		const NewEnumeratorDescriptor<E>& enumeratorFromTag(uint32_t tag) const override
		{
			auto it = std::find_if(m_enumerators.begin(), m_enumerators.end(), [tag](const NewEnumeratorDescriptor<E>& enumeratorDescriptor)
			{
				return enumeratorDescriptor.tag() == tag;
			});

			if (it == m_enumerators.end())
			{
				throw std::logic_error{ "Enum '" + NewDescriptor<>::name() + "' does not have enumerator with given tag: " + std::to_string(tag) };
			}

			return *it;
		}
		
		const NewEnumeratorDescriptor<E>& enumeratorFromName(const std::string_view& name) const override
		{
			auto it = std::find_if(m_enumerators.begin(), m_enumerators.end(), [&name](const NewEnumeratorDescriptor<E>& enumeratorDescriptor)
			{
				return enumeratorDescriptor.name() == name;
			});

			if (it == m_enumerators.end())
			{
				throw std::logic_error{ "Enum '" + NewDescriptor<>::name() + "' does not have enumerator with given name: " + name.data() };
			}

			return *it;
		}
		
		const NewEnumeratorDescriptor<E>& enumeratorFromValue(const NewTypeless& value) const override
		{
			return enumeratorFromValue(value.to<E>());
		}

		const NewEnumeratorDescriptor<E>& enumeratorFromValue(const E& value) const
		{
			auto it = std::find_if(m_enumerators.begin(), m_enumerators.end(), [&value](const NewEnumeratorDescriptor<E>& enumeratorDescriptor)
			{
				return enumeratorDescriptor.value() == value;
			});

			if (it == m_enumerators.end())
			{
				throw std::logic_error{ "Enum '" + NewDescriptor<>::name() + "' does not have enumerator with given value" };
			}

			return *it;
		}
		
	private:

		std::vector<NewEnumeratorDescriptor<E>> m_enumerators;
		std::vector<NewEnumDescriptor<>::enumerator_ref_t> m_enumeratorsTypeless;
	};
}
namespace dots::type
{

	template<typename T, typename = void>
	constexpr bool is_defined_v = false;
	template<typename T>
	constexpr bool is_defined_v<T, decltype(sizeof(T), void())> = true;
	template<typename T>
	using is_defined_t = std::conditional_t<is_defined_v<T>, std::true_type, std::false_type>;
	template <typename T>
	struct has_enum_type : std::conditional_t<std::conjunction_v<std::is_enum<T>, is_defined_t<NewDescriptor<T>>>, std::true_type, std::false_type> {};
	template <typename T>
	using has_enum_type_t = typename has_enum_type<T>::type;
	template <typename T>
	constexpr bool has_enum_type_v = has_enum_type_t<T>::value;
}

namespace dots::types
{
	template <typename E, std::enable_if_t<dots::type::has_enum_type_v<E>, int> = 0>
	std::ostream& operator << (std::ostream& os, const E& enumerator)
	{
		os << type::NewDescriptor<E>::Instance().enumeratorFromValue(enumerator).name();
		return os;
	}

	template <typename E, std::enable_if_t<dots::type::has_enum_type_v<E>, int> = 0>
	const std::string& to_string(const E& enumerator)
	{
		return type::NewDescriptor<E>::Instance().enumeratorFromValue(enumerator).name();
	}
}