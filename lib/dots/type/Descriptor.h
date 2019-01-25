#pragma once

#include <cstddef>
#include <string>
#include <memory>

namespace dots
{
namespace type
{

enum class DotsType {
    int8, int16, int32, int64,
    uint8, uint16, uint32, uint64,
    boolean, pointer,
    float16, float32, float64, string, property_set,
    timepoint, steady_timepoint, duration, uuid,
    Vector, Struct, Enum
};

bool isDotsBaseType(type::DotsType dotsType);

class Registry;

class Descriptor
{
public:
    Descriptor(const std::string& dotsTypeName, DotsType dotsType, std::size_t sizeOf, std::size_t alignOf);
    virtual ~Descriptor();

    std::size_t sizeOf() const { return m_sizeOf; }
    std::size_t alignOf() const { return m_alignOf; }

    const std::string& name() const { return m_dotsTypeName; }

    DotsType dotsType() const { return m_dotsType; }

    virtual void construct(void*) const = 0;
    virtual void destruct(void*) const = 0;

    void* New() const;
    void Delete(void*) const;
    std::shared_ptr<void> make_shared() const;

    virtual std::string to_string(const void* lhs) const = 0;
    virtual bool from_string(void* lhs, const std::string& str) const = 0;

    virtual bool equal(const void* lhs, const void* rhs) const = 0;
    virtual bool lessThan(const void* lhs, const void* rhs) const = 0;
    virtual void copy(void* lhs, const void* rhs) const = 0;
    virtual void swap(void* lhs, void* rhs) const = 0;

    virtual void clear(void* lhs) const = 0;

    static Registry& registry();

protected:
    std::string m_dotsTypeName;
    DotsType m_dotsType;
    std::size_t m_sizeOf;
    std::size_t m_alignOf;
};


}
}