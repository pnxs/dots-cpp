#pragma once
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <dots/type/Descriptor.h>
#include <dots/type/PropertyOffset.h>

namespace dots::type
{
    struct PropertyDescriptor
    {
        PropertyDescriptor(std::shared_ptr<Descriptor<>> descriptor, std::string name, uint32_t tag, bool isKey, std::optional<PropertyOffset<>> offset = std::nullopt);
        PropertyDescriptor(const PropertyDescriptor& other) = default;
        PropertyDescriptor(PropertyDescriptor&& other) = default;
        ~PropertyDescriptor() = default;

        PropertyDescriptor& operator = (const PropertyDescriptor& rhs) = default;
        PropertyDescriptor& operator = (PropertyDescriptor&& rhs) = default;

        const std::shared_ptr<Descriptor<>>& valueDescriptorPtr() const;
        const Descriptor<>& valueDescriptor() const;

        const std::string& name() const;
        uint32_t tag() const;
        bool isKey() const;
        PropertySet set() const;

        PropertyOffset<> offset() const;
        std::optional<PropertyOffset<>> subAreaOffset() const;

        [[deprecated("only available for backwards compatibility and should be replaced by property iteration")]]
        char* address(void* p) const;

        [[deprecated("only available for backwards compatibility and should be replaced by property iteration")]]
        const char* address(const void* p) const;

    private:

        std::shared_ptr<Descriptor<>> m_descriptor;
        std::string m_name;
        uint32_t m_tag;
        bool m_isKey;
        PropertySet m_set;
        PropertyOffset<> m_offset;
        std::optional<PropertyOffset<>> m_subAreaOffset;
    };

    using property_descriptor_container_t = std::vector<PropertyDescriptor>;
    using partial_property_descriptor_container_t = std::vector<std::reference_wrapper<const PropertyDescriptor>>;
    using property_descriptor_path_t = std::vector<property_descriptor_container_t::const_iterator>;
    using property_path_t = partial_property_descriptor_container_t;
}