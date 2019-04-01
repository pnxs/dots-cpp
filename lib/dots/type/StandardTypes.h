#pragma once

#include "property_set.h"
#include "Descriptor.h"
#include <dots/eventloop/string_convert.h>

namespace dots
{
namespace type
{

template<class T>
inline
void clear_value(T& obj)
{
    obj = T();
}

template<>
inline
void clear_value<std::string>(std::string& obj)
{
    obj.clear();
}

class StandardTypeDescriptorBase: public Descriptor
{
public:
    StandardTypeDescriptorBase(const std::string& dotsName, DotsType dotsType, std::size_t sizeOf, std::size_t alignOf)
        :Descriptor(dotsName, dotsType, sizeOf, alignOf)
    {}
};


template<class T>
class StandardTypeDescriptor: public StandardTypeDescriptorBase
{
public:
    StandardTypeDescriptor(const std::string& dotsName, DotsType dotsType)
    :StandardTypeDescriptorBase(dotsName, dotsType, sizeof(T), alignof(T) )
    {}

    void construct(void* obj) const final override
    {
        new(obj) T;
    }

    void destruct(void* obj) const final override
    {
        ((T*)obj)->~T();
    }

    std::string to_string(const void* lhs) const override
    {
        return ::to_string(*(const T*)lhs);
    }

    bool from_string(void* lhs, const std::string& str) const final override
    {
        return ::from_string(str, *(T*)lhs);

    }

    virtual bool equal(const void* lhs, const void* rhs) const final override
    {
        return std::equal_to<T>()(*(const T*)lhs, *(const T*)rhs);
    }

    virtual bool lessThan(const void* lhs, const void* rhs) const final override
    {
        return std::less<T>()(*(const T*)lhs, *(const T*)rhs);
    }

    virtual void copy(void* lhs, const void* rhs) const final override
    {
        *(T*)lhs = *(const T*) rhs;
    }

    virtual void swap(void* lhs, void* rhs) const final override
    {
        std::swap(*(T*)lhs, *(T*)rhs);
    }

    void clear(void* lhs) const final override
    {
        clear_value(*(T*)lhs);
    }
};

extern template
class StandardTypeDescriptor<bool>;

extern template
class StandardTypeDescriptor<int8_t>;

extern template
class StandardTypeDescriptor<int16_t>;

extern template
class StandardTypeDescriptor<int32_t>;

extern template
class StandardTypeDescriptor<int64_t>;

extern template
class StandardTypeDescriptor<uint8_t>;

extern template
class StandardTypeDescriptor<uint16_t>;

extern template
class StandardTypeDescriptor<uint32_t>;

extern template
class StandardTypeDescriptor<uint64_t>;

extern template
class StandardTypeDescriptor<float>;

extern template
class StandardTypeDescriptor<double>;

extern template
class StandardTypeDescriptor<long double>;

extern template
class StandardTypeDescriptor<std::string>;

extern template
class StandardTypeDescriptor<dots::property_set>;

extern template
class StandardTypeDescriptor<void*>;

static
inline
const StandardTypeDescriptorBase* toStandardTypeDescriptor(const Descriptor* d)
{
    return dynamic_cast<const StandardTypeDescriptorBase*>(d);
}


}
}

template<>
inline
std::string to_string<dots::property_set>(const dots::property_set &p)
{
    return p.to_string();
}

template<>
inline
bool from_string<dots::property_set>(const std::string &/*str*/, dots::property_set &/*p*/)
{
    return false;
}
