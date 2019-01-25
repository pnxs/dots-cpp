#pragma once
#include "dots/cpp_config.h"
#include <cstddef>
#include "Descriptor.h"

namespace dots
{
namespace type
{

class StructProperty
{
public:
    StructProperty(const std::string &name, std::size_t offset, int tag, bool key, const Descriptor *td);

    std::string name() const { return m_name; }
    std::size_t offset() const;
    Tag tag() const;
    const Descriptor* td() const;
    bool isKey() const { return m_isKey; }

    bool equal(const void* lhs, const void* rhs) const;
    bool lessThen(const void* lhs, const void* rhs) const;
    void copy(void *lhs, const void* rhs) const;
    void swap(void *lhs, void* rhs) const;
    void clear(void *lhs) const;

    char* address(void* p) const;
    const char* address(const void* p) const;

private:
    std::string m_name;
    std::size_t m_offset = 0;
    Tag m_tag = 0;
    bool m_isKey = false;
    const Descriptor* m_typeDescriptor = nullptr;
};

}
}
