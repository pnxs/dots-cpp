#pragma once
#include <string>
#include <vector>
#include <dots/type/NewDescriptor.h>
#include <dots/type/NewPropertySet.h>

namespace dots::type
{
	template <typename T = Typeless, typename = void>
	struct PropertyDescriptor;

	template <>
	struct PropertyDescriptor<>
	{
		PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, size_t offset, uint32_t tag, bool isKey);
		PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, uint32_t tag, bool isKey);
		PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, const PropertyDescriptor<>& previous, uint32_t tag, bool isKey);
		PropertyDescriptor(const PropertyDescriptor& other) = default;
		PropertyDescriptor(PropertyDescriptor&& other) = default;
		~PropertyDescriptor() = default;

		PropertyDescriptor& operator = (const PropertyDescriptor& rhs) = default;
		PropertyDescriptor& operator = (PropertyDescriptor&& rhs) = default;

		const std::shared_ptr<Descriptor<>>& valueDescriptorPtr() const;
		const Descriptor<>& valueDescriptor() const;
		const std::string& name() const;
		size_t offset() const;
		uint32_t tag() const;
		bool isKey() const;
		PropertySet set() const;

		[[deprecated("only available for backwards compatibility and should be replaced by property iteration")]]
		char* address(void* p) const;

		[[deprecated("only available for backwards compatibility and should be replaced by property iteration")]]
		const char* address(const void* p) const;

	private:

		static size_t CalculateOffset(const Descriptor<>& descriptor, const PropertyDescriptor<>& previous);
		static size_t CalculateOffset(const Descriptor<>& descriptor, size_t previousOffset, size_t previousSize);

		std::shared_ptr<Descriptor<>> m_descriptor;
		std::string m_name;
		size_t m_offset;
		uint32_t m_tag;
		bool m_isKey;
		PropertySet m_set;
	};

	template <typename T>
	struct PropertyDescriptor<T> : PropertyDescriptor<>
	{
		PropertyDescriptor(std::string name, size_t offset, uint32_t tag, bool isKey) :
			PropertyDescriptor<>(valueDescriptorPtr(), std::move(name), offset, tag, isKey)
		{
			/* do nothing */
		}
		
		PropertyDescriptor(std::string name, uint32_t tag, bool isKey) :
			PropertyDescriptor<>(valueDescriptorPtr(), std::move(name), tag, isKey)
		{
			/* do nothing */
		}
		
		PropertyDescriptor(std::string name, const PropertyDescriptor<>& previous, uint32_t tag, bool isKey) :
			PropertyDescriptor<>(valueDescriptorPtr(), std::move(name), previous, tag, isKey)
		{
			/* do nothing */
		}
		
		PropertyDescriptor(const PropertyDescriptor& other) = default;
		PropertyDescriptor(PropertyDescriptor&& other) = default;
		~PropertyDescriptor() = default;

		PropertyDescriptor& operator = (const PropertyDescriptor& rhs) = default;
		PropertyDescriptor& operator = (PropertyDescriptor&& rhs) = default;

		static const std::shared_ptr<Descriptor<T>>& valueDescriptorPtr()
		{
			return Descriptor<T>::InstancePtr();
		}

		static const Descriptor<T>& valueDescriptor()
		{
			return Descriptor<T>::Instance();
		}

	private:

		using PropertyDescriptor<>::valueDescriptorPtr;
		using PropertyDescriptor<>::valueDescriptor;
	};

	using property_descriptor_container_t = std::vector<const PropertyDescriptor<>*>;
}