#include <dots/type/property_set.h>
#include "StructDescriptor.h"
#include "Registry.h"

#include "StructDescriptorData.dots.h"
#include "DotsStructFlags.dots.h"

namespace dots
{
namespace type
{


StructDescriptor::StructDescriptor(const DescriptorData& sd, std::size_t sizeOf, std::size_t alignOf)
:Descriptor(sd.name, DotsType::Struct, sizeOf, alignOf)
{
    m_descriptorData = std::make_unique<DescriptorData>(sd);
    auto& flags = sd.flags.isValid() ? *m_descriptorData->flags : m_descriptorData->flags();

    if (not flags.cleanup.isValid())    flags.cleanup(false);
    if (not flags.persistent.isValid()) flags.persistent(false);
    if (not flags.local.isValid())      flags.local(false);
    if (not flags.cached.isValid())     flags.cached(false);
    if (not flags.internal.isValid())   flags.internal(false);
    if (not flags.substructOnly.isValid()) flags.substructOnly(false);
}

void StructDescriptor::construct(void *obj) const
{
	::new (obj) Struct(*this);
}

void StructDescriptor::destruct(void *obj) const
{
    for(auto& pd : m_properties) // Iterate over PropertyDescriptors
    {
		if (validProperties(obj).test(pd.tag()))
		{
			pd.td()->destruct(pd.address(obj));
		}
    }
}

std::string StructDescriptor::to_string(const void* lhs) const
{
    return {};
}

bool StructDescriptor::from_string(void* lhs, const std::string& str) const
{
    return false;
}

bool StructDescriptor::equal(const void *lhs, const void *rhs) const
{
    for (auto& pd : m_properties)
    {
        if (validProperties(lhs)[pd.tag()] && validProperties(rhs)[pd.tag()])
        {
            // on equal property-sets
            if (not pd.equal(lhs, rhs))
            {
                return false;
            }
        }
        else
        {
            // check if this property is valid
            if (validProperties(lhs)[pd.tag()] || validProperties(rhs)[pd.tag()])
            {
                return false;
            }
        }
    }
    return true;
}

bool StructDescriptor::lessThan(const void *lhs, const void *rhs) const
{
    for (auto& pd : m_properties)
    {
        if (pd.isKey())
        {
            if (pd.lessThen(lhs, rhs)) // lhs < rhs
                return true;
            if (pd.lessThen(rhs, lhs)) // lhs > rhs
                return false;
        }
    }
    return false;
}

void StructDescriptor::copy(void *lhs, const void *rhs) const
{
    copy(lhs, rhs, PROPERTY_SET_ALL);

}

void StructDescriptor::swap(void *lhs, void *rhs) const
{
    swap(lhs, rhs, PROPERTY_SET_ALL);
}

void StructDescriptor::copy(void *lhs, const void *rhs, property_set properties) const
{
    properties &= validProperties(rhs);
	auto& p = validProperties(lhs);

    for (auto& pd : m_properties)
    {
        if (properties[pd.tag()])
		{
			if (!validProperties(lhs)[pd.tag()])
			{
				pd.td()->construct(pd.address(lhs));
			}

            pd.copy(lhs, rhs);
			validProperties(lhs).set(pd.tag());
        }
    }
}

void StructDescriptor::swap(void *lhs, void *rhs, property_set properties) const
{
    for (auto& pd : m_properties)
    {
        const auto tag = pd.tag();

        if (properties[tag])
        {
            bool vl = validProperties(lhs)[tag];
            bool vr = validProperties(rhs)[tag];

            if (vl || vr)
            {
                if (vl != vr)
                {
                    validProperties(lhs).set(tag, vr);
                    validProperties(rhs).set(tag, vl);
                }

                pd.swap(lhs, rhs);
            }
        }
    }
}

property_set &StructDescriptor::validProperties(const void *obj) const
{
    return const_cast<property_set&>(reinterpret_cast<const Struct*>(obj)->_validPropertySet());
}

const property_set &StructDescriptor::propertySet() const
{
    return m_propertySet;
}

void StructDescriptor::clear(void *lhs) const
{
    clear(lhs, PROPERTY_SET_ALL);
}

void StructDescriptor::clear(void *lhs, property_set properties) const
{
    validProperties(lhs) &= ~properties;

    for (auto& pd : m_properties)
    {
        const auto tag = pd.tag();
        if (properties[tag])
        {
            pd.clear(lhs);
        }
    }

}

property_set StructDescriptor::diff(const void *lhs, const void *rhs, property_set properties) const
{
    property_set delta;

    for (auto& pd : m_properties)
    {
        const auto tag = pd.tag();
        if (properties[tag])
        {
            if (validProperties(lhs)[tag] && validProperties(rhs)[tag])
            {
                if (not pd.equal(lhs, rhs))
                {
                    delta.set(tag);
                }
            } else
            {
                if (validProperties(lhs)[tag] || validProperties(rhs)[tag])
                {
                    delta.set(tag);
                }

            }
        }
    }

    return delta;
}

void StructDescriptor::merge(void *lhs, const void *rhs, const void *properties) const
{
    for (auto& pd : m_properties)
    {
        const auto tag = pd.tag();
        if (validProperties(properties)[tag])
        {
            auto isValid = validProperties(rhs)[tag];

            if (isValid)
            {
                auto sd = dynamic_cast<const StructDescriptor*>(pd.td());

                if (sd)
                {
                    sd->merge(pd.address(lhs), pd.address(rhs), pd.address(properties));
                } else
                {
                    pd.copy(lhs, rhs);
                }
            }
        }
    }
}

const property_set &StructDescriptor::keys() const
{
    return m_keyProperties;
}

const StructDescriptor::DescriptorData &StructDescriptor::descriptorData() const
{
    return *(m_descriptorData.get());
}

bool StructDescriptor::cached() const
{
    return m_descriptorData->flags->cached;
}

bool StructDescriptor::cleanup() const
{
    return m_descriptorData->flags->cleanup;
}

bool StructDescriptor::local() const
{
    return m_descriptorData->flags->local;
}

bool StructDescriptor::persistent() const
{
    return m_descriptorData->flags->persistent;
}

bool StructDescriptor::internal() const
{
    return m_descriptorData->flags->internal;
}

bool StructDescriptor::substructOnly() const
{
    return m_descriptorData->flags->substructOnly;
}


void StructDescriptorSet::merge(const StructDescriptorSet &rhs)
{
    insert(rhs.begin(), rhs.end());
}

}
}
