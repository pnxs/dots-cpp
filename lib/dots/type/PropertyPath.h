#pragma once
#include <variant>
#include <dots/type/PropertyDescriptor.h>

namespace dots::type
{
    struct PropertyPath
    {
        using elements_t = std::vector<std::reference_wrapper<const PropertyDescriptor>>;

        PropertyPath(elements_t elements);
        explicit PropertyPath(const PropertyDescriptor& descriptor);

        PropertyPath(const PropertyPath& other) = default;
        PropertyPath(PropertyPath&& other) = default;
        ~PropertyPath() = default;

        PropertyPath& operator = (const PropertyPath& rhs) = default;
        PropertyPath& operator = (PropertyPath&& rhs) = default;

        const elements_t& elements() const;
        size_t offset() const;
        const PropertyDescriptor& destination() const;

    private:

        elements_t m_elements;
        size_t m_offset;
    };
}