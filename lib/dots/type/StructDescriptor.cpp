#include <dots/type/property_set.h>
#include "StructDescriptor.h"
#include "Registry.h"

#include "StructDescriptorData.dots.h"
#include "DotsStructFlags.dots.h"

namespace dots::type
{
	StructDescriptor::StructDescriptor(const DescriptorData& sd) :
		Descriptor(sd.name, DotsType::Struct, determineSize(sd), determineAlignment(sd))
	{
		m_descriptorData = std::make_unique<DescriptorData>(sd);
		auto& flags = sd.flags.isValid() ? *m_descriptorData->flags : m_descriptorData->flags();

		if (not flags.cleanup.isValid())    flags.cleanup(false);
		if (not flags.persistent.isValid()) flags.persistent(false);
		if (not flags.local.isValid())      flags.local(false);
		if (not flags.cached.isValid())     flags.cached(false);
		if (not flags.internal.isValid())   flags.internal(false);
		if (not flags.substructOnly.isValid()) flags.substructOnly(false);

		std::size_t lastOffset = sizeof(Struct);


        for (const StructPropertyData &p : *m_descriptorData->properties)
        {
            std::string dots_type_name = p.type; // DOTS typename
            auto td = Registry::fromWireName(dots_type_name);

            std::size_t offset = determineOffset(td, lastOffset);
            // Create Properties
            const Descriptor* propertyTypeDescriptor = td;
            if (propertyTypeDescriptor)
            {
                m_properties.push_back(StructProperty(p.name, offset, p.tag, p.isKey, propertyTypeDescriptor));
                m_propertySet.set(p.tag);
                if (p.isKey) {
                    m_keyProperties.set(p.tag);
                }
            }
            else
            {
                // Error, because the needed type is not found
                throw std::runtime_error("missing type '" + dots_type_name + "' for property '" + *p.name + "'");
            }
            lastOffset = offset + propertyTypeDescriptor->sizeOf();
        }

        if (sd.publisherId.isValid())
        {
            m_publisherId = sd.publisherId;
        }
	}

	const StructDescriptor * StructDescriptor::createFromStructDescriptorData(const StructDescriptorData &sd)
	{
	    // Check if type is already registred
	    {
	        auto structDescriptor = registry().findStructDescriptor(sd.name);
	        if (structDescriptor) return structDescriptor;
	    }

	    return new StructDescriptor(sd);
	}

	void StructDescriptor::construct(void* obj) const
	{
		::new (obj) Struct(*this);

		for (auto& pd : m_properties)
		{
			pd.td()->construct(pd.address(obj));
		}
	}

	void StructDescriptor::destruct(void* obj) const
	{
		for (auto& pd : m_properties) // Iterate over PropertyDescriptors
		{
			if (validProperties(obj).test(pd.tag()))
			{
				pd.td()->destruct(pd.address(obj));
			}
		}
	}

	bool StructDescriptor::usesDynamicMemory() const
	{
		// TODO: improve when overhauling descriptors
		if (m_dynamicMemoryProperties == std::nullopt)
		{
			std::vector<const StructProperty*>& dynamicMemoryProperties = m_dynamicMemoryProperties.emplace();

			for (const StructProperty& sp : m_properties)
			{
				if (sp.td()->usesDynamicMemory())
				{
					dynamicMemoryProperties.emplace_back(&sp);
				}
			}
		}

		return !m_dynamicMemoryProperties->empty();
	}

	size_t StructDescriptor::dynamicMemoryUsage(const void* lhs) const
	{
		// TODO: improve when overhauling descriptors
		if (usesDynamicMemory())
		{
			const property_set& validProperties = reinterpret_cast<const Struct*>(lhs)->_validProperties();
			size_t dynMemUsage = 0;

			for (const StructProperty* sp : *m_dynamicMemoryProperties)
			{
				if (validProperties.test(sp->tag()))
				{
					dynMemUsage += sp->td()->dynamicMemoryUsage(sp->address(lhs));
				}
			}

			return dynMemUsage;
		}
		else
		{
			return 0;
		}
	}

	std::string StructDescriptor::to_string(const void* /*lhs*/) const
	{
		return {};
	}

	bool StructDescriptor::from_string(void* /*lhs*/, const std::string& /*str*/) const
	{
		return false;
	}

	bool StructDescriptor::equal(const void* lhs, const void* rhs) const
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

	bool StructDescriptor::lessThan(const void* lhs, const void* rhs) const
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

	void StructDescriptor::copy(void* lhs, const void* rhs) const
	{
		copy(lhs, rhs, PROPERTY_SET_ALL);

	}

	void StructDescriptor::swap(void* lhs, void* rhs) const
	{
		swap(lhs, rhs, PROPERTY_SET_ALL);
	}

	void StructDescriptor::copy(void* lhs, const void* rhs, property_set properties) const
	{
		properties &= validProperties(rhs);

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

	void StructDescriptor::swap(void* lhs, void* rhs, property_set properties) const
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

	property_set& StructDescriptor::validProperties(const void* obj) const
	{
		return const_cast<property_set&>(reinterpret_cast<const Struct*>(obj)->_validProperties());
	}

	const property_set& StructDescriptor::propertySet() const
	{
		return m_propertySet;
	}

	void StructDescriptor::clear(void* lhs) const
	{
		clear(lhs, PROPERTY_SET_ALL);
	}

	void StructDescriptor::clear(void* lhs, property_set properties) const
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

	property_set StructDescriptor::diff(const void* lhs, const void* rhs, property_set properties) const
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
				}
				else
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

	void StructDescriptor::merge(void* lhs, const void* rhs, const void* properties) const
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
					}
					else
					{
						pd.copy(lhs, rhs);
					}
				}
			}
		}
	}

	const property_set& StructDescriptor::keys() const
	{
		return m_keyProperties;
	}

	const StructDescriptor::DescriptorData& StructDescriptor::descriptorData() const
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


	void StructDescriptorSet::merge(const StructDescriptorSet& rhs)
	{
		insert(rhs.begin(), rhs.end());
	}

	size_t StructDescriptor::determineSize(const StructDescriptorData &sd)
	{
	    size_t lastPropertyOffset = sizeof(Struct);
		size_t sizeOf;

	    for (auto &p : *sd.properties)
	    {
	        std::string dots_type_name = p.type;
	        auto td = Registry::fromWireName(dots_type_name);
	        if (!td)
			{
	            throw std::runtime_error("getStructProperties: missing type: " + dots_type_name);
	        }

	        size_t offset = determineOffset(td, lastPropertyOffset);
	        lastPropertyOffset = offset + td->sizeOf();
	    }

	    {
	        auto pointerType = registry().findDescriptor("pointer");
	        sizeOf = determineOffset(pointerType, lastPropertyOffset);
	    }

	    return sizeOf;
	}

	size_t StructDescriptor::determineAlignment(const types::StructDescriptorData& sd)
	{
		size_t maxAlign = alignof(Struct);

	    for (auto &p : *sd.properties)
	    {
	        auto td = registry().findDescriptor(p.type);
	        size_t align = td->alignOf();
	        if (align > maxAlign)
	            maxAlign = align;
	    }

	    return maxAlign;
	}

	size_t StructDescriptor::determineOffset(const Descriptor* td, size_t start)
	{
		size_t align = td->alignOf();
		return start + (align - (start % align)) % align;
	}
}