#pragma once
#include <string>
#include <vector>
#include <dots/type/NewDescriptor.h>
#include <dots/type/NewPropertySet.h>

namespace dots::type
{
	template <typename T = NewTypeless, typename = void>
	struct NewPropertyDescriptor;

	template <>
	struct NewPropertyDescriptor<>
	{
		NewPropertyDescriptor(const NewDescriptor<>& descriptor, std::string name, size_t offset, uint32_t tag, bool isKey);
		NewPropertyDescriptor(const NewDescriptor<>& descriptor, std::string name, uint32_t tag, bool isKey);
		NewPropertyDescriptor(const NewDescriptor<>& descriptor, std::string name, const NewPropertyDescriptor<>& previous, uint32_t tag, bool isKey);
		NewPropertyDescriptor(const NewPropertyDescriptor& other) = default;
		NewPropertyDescriptor(NewPropertyDescriptor&& other) = default;
		~NewPropertyDescriptor() = default;

		NewPropertyDescriptor& operator = (const NewPropertyDescriptor& rhs) = default;
		NewPropertyDescriptor& operator = (NewPropertyDescriptor&& rhs) = default;

		const NewDescriptor<>& valueDescriptor() const;
		const std::string& name() const;
		size_t offset() const;
		uint32_t tag() const;
		bool isKey() const;
		NewPropertySet set() const;

	private:

		static size_t CalculateOffset(const NewDescriptor<>& descriptor, const NewPropertyDescriptor<>& previous);
		static size_t CalculateOffset(const NewDescriptor<>& descriptor, size_t previousOffset, size_t previousSize);

		const NewDescriptor<>* m_descriptor;
		std::string m_name;
		size_t m_offset;
		uint32_t m_tag;
		bool m_isKey;
		NewPropertySet m_set;
	};

	template <typename T>
	struct NewPropertyDescriptor<T> : NewPropertyDescriptor<>
	{
		NewPropertyDescriptor(std::string name, size_t offset, uint32_t tag, bool isKey) :
			NewPropertyDescriptor<>(valueDescriptor(), std::move(name), offset, tag, isKey)
		{
			/* do nothing */
		}
		
		NewPropertyDescriptor(std::string name, uint32_t tag, bool isKey) :
			NewPropertyDescriptor<>(valueDescriptor(), std::move(name), tag, isKey)
		{
			/* do nothing */
		}
		
		NewPropertyDescriptor(std::string name, const NewPropertyDescriptor<>& previous, uint32_t tag, bool isKey) :
			NewPropertyDescriptor<>(valueDescriptor(), std::move(name), previous, tag, isKey)
		{
			/* do nothing */
		}
		
		NewPropertyDescriptor(const NewPropertyDescriptor& other) = default;
		NewPropertyDescriptor(NewPropertyDescriptor&& other) = default;
		~NewPropertyDescriptor() = default;

		NewPropertyDescriptor& operator = (const NewPropertyDescriptor& rhs) = default;
		NewPropertyDescriptor& operator = (NewPropertyDescriptor&& rhs) = default;

		static const NewDescriptor<T>& valueDescriptor()
		{
			return NewDescriptor<T>::Instance();
		}

	private:

		using NewPropertyDescriptor<>::valueDescriptor;
	};

	using new_property_descriptor_container_t = std::vector<const NewPropertyDescriptor<>*>;
}