#pragma once
#include <string>
#include <vector>
#include <functional>
#include <dots/type/Descriptor.h>
#include <dots/type/PropertyMetadata.h>

namespace dots::type
{
	struct PropertyDescriptor
	{
		PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, PropertyMetadata<> metadata);
		PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, size_t offset, uint32_t tag, bool isKey);
		PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, uint32_t tag, bool isKey);
		PropertyDescriptor(const std::shared_ptr<Descriptor<>>& descriptor, std::string name, const PropertyDescriptor& previous, uint32_t tag, bool isKey);		
		PropertyDescriptor(const PropertyDescriptor& other);
		PropertyDescriptor(PropertyDescriptor&& other) = default;
		~PropertyDescriptor() = default;

		PropertyDescriptor& operator = (const PropertyDescriptor& rhs);
		PropertyDescriptor& operator = (PropertyDescriptor&& rhs) = default;

		const std::shared_ptr<Descriptor<>>& valueDescriptorPtr() const;
		const Descriptor<>& valueDescriptor() const;
		
		const PropertyMetadata<>& metadata() const;
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

		std::shared_ptr<Descriptor<>> m_descriptor;
		std::string m_name;
		PropertyMetadata<> m_metadata;
	};

	using property_descriptor_container_t = std::vector<PropertyDescriptor>;
	using partial_property_descriptor_container_t = std::vector<std::reference_wrapper<const PropertyDescriptor>>;
}