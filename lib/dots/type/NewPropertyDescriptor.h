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
		NewPropertyDescriptor(const std::shared_ptr<NewDescriptor<>>& descriptor, std::string name, size_t offset, uint32_t tag, bool isKey);
		NewPropertyDescriptor(const std::shared_ptr<NewDescriptor<>>& descriptor, std::string name, uint32_t tag, bool isKey);
		NewPropertyDescriptor(const std::shared_ptr<NewDescriptor<>>& descriptor, std::string name, const NewPropertyDescriptor<>& previous, uint32_t tag, bool isKey);
		NewPropertyDescriptor(const NewPropertyDescriptor& other) = default;
		NewPropertyDescriptor(NewPropertyDescriptor&& other) = default;
		~NewPropertyDescriptor() = default;

		NewPropertyDescriptor& operator = (const NewPropertyDescriptor& rhs) = default;
		NewPropertyDescriptor& operator = (NewPropertyDescriptor&& rhs) = default;

		const std::shared_ptr<NewDescriptor<>>& valueDescriptorPtr() const;
		const NewDescriptor<>& valueDescriptor() const;
		const std::string& name() const;
		size_t offset() const;
		uint32_t tag() const;
		bool isKey() const;
		NewPropertySet set() const;

	private:

		static size_t CalculateOffset(const NewDescriptor<>& descriptor, const NewPropertyDescriptor<>& previous);
		static size_t CalculateOffset(const NewDescriptor<>& descriptor, size_t previousOffset, size_t previousSize);

		std::shared_ptr<NewDescriptor<>> m_descriptor;
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
			NewPropertyDescriptor<>(valueDescriptorPtr(), std::move(name), offset, tag, isKey)
		{
			/* do nothing */
		}
		
		NewPropertyDescriptor(std::string name, uint32_t tag, bool isKey) :
			NewPropertyDescriptor<>(valueDescriptorPtr(), std::move(name), tag, isKey)
		{
			/* do nothing */
		}
		
		NewPropertyDescriptor(std::string name, const NewPropertyDescriptor<>& previous, uint32_t tag, bool isKey) :
			NewPropertyDescriptor<>(valueDescriptorPtr(), std::move(name), previous, tag, isKey)
		{
			/* do nothing */
		}
		
		NewPropertyDescriptor(const NewPropertyDescriptor& other) = default;
		NewPropertyDescriptor(NewPropertyDescriptor&& other) = default;
		~NewPropertyDescriptor() = default;

		NewPropertyDescriptor& operator = (const NewPropertyDescriptor& rhs) = default;
		NewPropertyDescriptor& operator = (NewPropertyDescriptor&& rhs) = default;

		static const std::shared_ptr<NewDescriptor<T>>& valueDescriptorPtr()
		{
			return NewDescriptor<T>::InstancePtr();
		}

		static const NewDescriptor<T>& valueDescriptor()
		{
			return NewDescriptor<T>::Instance();
		}

	private:

		using NewPropertyDescriptor<>::valueDescriptorPtr;
		using NewPropertyDescriptor<>::valueDescriptor;
	};

	using new_property_descriptor_container_t = std::vector<const NewPropertyDescriptor<>*>;
}