#include "DistributedTypeId.h"
#include <dots/io/Transceiver.h>
#include <dots/type/Registry.h>

namespace dots
{

DistributedTypeId::DistributedTypeId(bool master)
:m_master(master)
{
    if (not master)
    {
        dots::subscribe<DotsTypes>(FUN(*this, handleDotsType));
    }
    else
    {
        m_connectionOnNewTypes = type::Descriptor::registry().onNewType.connect(FUN(*this, createTypeId));
    }
}

DistributedTypeId::~DistributedTypeId()
{
    if (m_master)
    {
        m_connectionOnNewTypes.disconnect();
    }
}

DistributedTypeId::TypeId DistributedTypeId::createTypeId(const type::Descriptor *td)
{
    registerTypeId(m_nextTypeId, td);

	DotsTypes dotsType(DotsTypes::id_t_i{ m_nextTypeId });
    dotsType.name(td->name());
    dotsType._publish();

    LOG_DEBUG_S("create DotsTypes(" << m_nextTypeId << ", " << td->name() << ")");

    return m_nextTypeId++;
}

void DistributedTypeId::registerTypeId(const DistributedTypeId::TypeId id, const type::Descriptor *td)
{
    auto ret = m_typeMap.insert({id, td});
    if (ret.second == false) {
        throw std::runtime_error("unable to insert TypeId " + std::to_string(id));
    }
    auto ret2 = m_nameIdMap.insert({td->name(), id});
    if (ret2.second == false) {
        m_typeMap.erase(id);
        throw std::runtime_error("unable to insert '" + td->name() + "'(" + std::to_string(id) + ") into nameIdMap");
    }
}

void DistributedTypeId::removeTypeId(const DistributedTypeId::TypeId id)
{
    m_typeMap.erase(id);
}

const type::Descriptor *DistributedTypeId::findDescriptor(const DistributedTypeId::TypeId id) const
{
    auto iter = m_typeMap.find(id);
    if (iter != m_typeMap.end()) {
        return iter->second;
    }
    return nullptr;
}

const type::StructDescriptor *DistributedTypeId::findStructDescriptor(const DistributedTypeId::TypeId id) const
{
    return dots::type::toStructDescriptor(findDescriptor(id));
}

const type::Descriptor *DistributedTypeId::findDescriptor(const string& name) const
{
    auto typeId = findTypeId(name);
    if (typeId > 0) {
        return findDescriptor(typeId);
    }
    return nullptr;
}

DistributedTypeId::TypeId DistributedTypeId::findTypeId(const string &name) const
{
    auto iter = m_nameIdMap.find(name);
    if (iter != m_nameIdMap.end()) {
        return iter->second;
    }
    return 0;
}

void DistributedTypeId::handleDotsType(const DotsTypes::Cbd &cbd)
{
    if (cbd.isCreate()) {
        auto td = type::Descriptor::registry().findDescriptor(cbd().name);
        if (td) {
            registerTypeId(cbd().id, td);
        }
    }

}

}