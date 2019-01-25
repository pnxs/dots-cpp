#include "Descriptor.h"
#include "Registry.h"

namespace dots {
namespace type {

bool isDotsBaseType(type::DotsType dotsType)
{
    switch (dotsType)
    {
        case type::DotsType::int8:
        case type::DotsType::int16:
        case type::DotsType::int32:
        case type::DotsType::int64:
        case type::DotsType::uint8:
        case type::DotsType::uint16:
        case type::DotsType::uint32:
        case type::DotsType::uint64:
        case type::DotsType::boolean:
        case type::DotsType::float16:
        case type::DotsType::float32:
        case type::DotsType::float64:
        case type::DotsType::string:
        case type::DotsType::property_set:
        case type::DotsType::timepoint:
        case type::DotsType::steady_timepoint:
        case type::DotsType::duration:
        case type::DotsType::uuid:
        case type::DotsType::Enum:
        case type::DotsType::pointer:
            return true;

        case type::DotsType::Vector:
        case type::DotsType::Struct:
            return false;
    }
    return false;
}

void *Descriptor::New() const
{
    void *obj = ::operator new(sizeOf());
    construct(obj);
    return obj;
}

void Descriptor::Delete(void *obj) const
{
    destruct(obj);
    ::operator delete(obj);
}

std::shared_ptr<void> Descriptor::make_shared() const
{
    return {New(), bind(&Descriptor::Delete, this, _1)};
}

Descriptor::Descriptor(const std::string &dotsName, DotsType dotsType, std::size_t sizeOf,
                             std::size_t alignOf)
        : m_dotsTypeName(dotsName), m_dotsType(dotsType), m_sizeOf(sizeOf), m_alignOf(alignOf)
{
    //printf("add dots::Descriptor(%s <-> %s, %d, %d)\n", name().c_str(), name.c_str(), sizeOf(), alignOf());
    registry().insertType(dotsName, this);
}

Descriptor::~Descriptor()
{
}

Registry &type::Descriptor::registry()
{
    static Registry *registry = new Registry;
    registry->checkPopulate();
    return *registry;
}

}
}