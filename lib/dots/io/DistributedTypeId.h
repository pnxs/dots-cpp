#pragma once

#include <cstdint>
#include <dots/type/StructDescriptor.h>
#include <map>
#include <dots/functional/signal.h>

#include "DotsTypes.dots.h"

namespace dots {

class DistributedTypeId
{
public:
    typedef uint32_t TypeId;

    explicit DistributedTypeId(bool master = false);
    ~DistributedTypeId() = default;

    TypeId createTypeId(const type::Descriptor<>* td);
    void registerTypeId(const TypeId, const type::Descriptor<>* td);
    void removeTypeId(const TypeId id);

    TypeId findTypeId(const std::string& name) const;

    const type::Descriptor<>* findDescriptor(const TypeId id) const;
    const type::Descriptor<>* findDescriptor(const std::string& name) const;
    const type::StructDescriptor<>* findStructDescriptor(const TypeId id) const;

private:
    void handleDotsType(const DotsTypes::Cbd& cbd);

    bool m_master = false;
    TypeId m_nextTypeId = 1;
    std::map<TypeId, const type::Descriptor<>*> m_typeMap;
    std::map<std::string, TypeId> m_nameIdMap;
};

}