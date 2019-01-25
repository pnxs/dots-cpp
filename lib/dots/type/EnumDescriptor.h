#pragma once

#include "Descriptor.h"
#include <map>

namespace dots {

namespace types {
class EnumDescriptorData;
class EnumElementDescriptor;
}

namespace type {

class EnumDescriptor : public Descriptor
{
public:
    typedef int32_t enum_type;
    typedef dots::types::EnumDescriptorData DescriptorData;

    static const EnumDescriptor * createFromEnumDescriptorData(const DescriptorData &sd);

    void construct(void *) const final override;
    void destruct(void *) const final override;

    std::string to_string(const void *lhs) const final;
    bool from_string(void *lhs, const std::string &str) const final override;

    bool equal(const void *lhs, const void *rhs) const final override;
    bool lessThan(const void *lhs, const void *rhs) const final override;
    void copy(void *lhs, const void *rhs) const final override;
    void swap(void *lhs, void *rhs) const final override;

    void clear(void *lhs) const final override;

    const DescriptorData &descriptorData() const;

    enum_type to_int(const void *p) const;
    enum_type &to_int(void *p) const;

    void from_int(void *p, int intValue) const;
    void from_key(void *p, int32_t key) const;

    int32_t validValue() const;
    int32_t value2key(enum_type value) const;


    const std::map<int32_t, dots::types::EnumElementDescriptor> &elements() const
    { return m_elements; }

private:
    explicit EnumDescriptor(const DescriptorData &ed);

    std::map<int32_t, dots::types::EnumElementDescriptor> m_elements;
    std::shared_ptr<DescriptorData> m_descriptorData;
};

static
inline
const EnumDescriptor *toEnumDescriptor(const Descriptor *d)
{
    return dynamic_cast<const EnumDescriptor *>(d);
}

template<class T>
class EnumDescriptorInit final : public EnumDescriptor
{
public:
    static const EnumDescriptor *_td();
};


template<class T>
class Enum
{
public:
    Enum():m_data((T)td()->validValue()) {}
    Enum(T value):m_data(value) {}

    static const EnumDescriptor* td()
    {
        return EnumDescriptorInit<T>::_td();
    }

    std::string toString() const
    {
        return td()->to_string(&m_data);
    }


private:
    T m_data;

};

template<class T>
static inline std::string enumToString(T e)
{
    return Enum<T>(e).toString();
}

}
}

/*

template<class T>
inline
std::string to_string(const T& val)
{
    //pnxs::ostringstream os;
    std::ostringstream os;
    os << val;
    return os.str();
}
 */