#pragma once
#include <dots/cpp_config.h>
#include "Descriptor.h"

namespace dots
{
namespace type
{


class VectorDescriptor: public Descriptor
{
public:
    static const VectorDescriptor* createDescriptor(const string& vectorTypeName);


    void construct(void*) const final override;
    void destruct(void*) const final override;

    virtual std::string to_string(const void* lhs) const final override;
    virtual bool from_string(void* lhs, const std::string& str) const final override;

    bool equal(const void* lhs, const void* rhs) const final override;
    bool lessThan(const void* lhs, const void* rhs) const final override;
    void copy(void* lhs, const void* rhs) const final override;
    void swap(void* lhs, void* rhs) const final override;
    void clear(void* obj) const override;

    void resize(const void* obj, size_t size) const;
    size_t get_size(const void *obj) const;
    char * get_data(const void *obj, size_t idx = 0) const;

    const Descriptor* vtd() const;

private:
    VectorDescriptor(const Descriptor* vtd);

    const Descriptor* m_valueTypeDescriptor = nullptr;
};

static
inline
const VectorDescriptor* toVectorDescriptor(const Descriptor* d)
{
    return dynamic_cast<const VectorDescriptor*>(d);
}


}
}
