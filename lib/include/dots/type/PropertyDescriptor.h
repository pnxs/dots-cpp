#pragma once
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <dots/type/PropertySet.h>
#include <dots/type/Descriptor.h>
#include <dots/type/PropertyOffset.h>

namespace dots::type
{
    struct PropertyDescriptor
    {
        PropertyDescriptor(Descriptor<>& descriptor, std::string name, uint32_t tag, bool isKey, PropertyOffset offset);
        PropertyDescriptor(const PropertyDescriptor& other) = default;
        PropertyDescriptor(PropertyDescriptor&& other) = default;
        ~PropertyDescriptor() = default;

        PropertyDescriptor& operator = (const PropertyDescriptor& rhs) = default;
        PropertyDescriptor& operator = (PropertyDescriptor&& rhs) = default;

        const Descriptor<>& valueDescriptor() const
        {
            return *m_descriptor;
        }

        Descriptor<>& valueDescriptor()
        {
            return *m_descriptor;
        }

        const std::string& name() const
        {
            return m_name;
        }

        uint32_t tag() const
        {
            return m_tag;
        }

        bool isKey() const
        {
            return m_isKey;
        }

        PropertySet set() const
        {
            return m_set;
        }

        PropertyOffset offset() const
        {
            return m_offset;
        }

        std::optional<PropertyOffset> subAreaOffset() const
        {
            return m_subAreaOffset;
        }

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
        PropertyOffset m_offset;
        std::optional<PropertyOffset> m_subAreaOffset;
    };

    using property_descriptor_container_t = std::vector<PropertyDescriptor>;
    using partial_property_descriptor_container_t = std::vector<std::reference_wrapper<const PropertyDescriptor>>;
    using property_descriptor_path_t = std::vector<property_descriptor_container_t::const_iterator>;
}