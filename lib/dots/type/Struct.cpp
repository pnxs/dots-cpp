#include "Struct.h"
#include "StructDescriptor.h"
#include "Registry.h"
#include "PropertyIterator.h"
#include "PropertyPairIterator.h"
#include "dots/io/Transceiver.h"
#include <StructDescriptorData.dots.h>

struct StructProperties
{
	std::size_t size;
	std::size_t alignment;
};

static size_t evalPropertyOffset(const dots::type::Descriptor* td, size_t start)
{
	size_t align = td->alignOf();
	return start + (align - (start % align)) % align;
}

static size_t evalMaxPropertyAlignment(const StructDescriptorData &sd)
{
	size_t maxAlign = alignof(dots::type::Struct);

	for (auto &p : *sd.properties)
	{
		auto td = dots::type::Descriptor::registry().findDescriptor(p.type);
		size_t align = td->alignOf();
		if (align > maxAlign)
			maxAlign = align;
	}
	return maxAlign;
}

static StructProperties getStructProperties(const StructDescriptorData &sd)
{
	size_t sizeOf = sizeof(dots::type::Struct);
	size_t alignOf = alignof(dots::type::Struct);

	size_t lastPropertyOffset = sizeof(dots::type::Struct);

	for (auto &p : *sd.properties)
	{
		std::string dots_type_name = p.type;
		auto td = dots::type::Registry::fromWireName(dots_type_name);
		if (not td) {
			throw std::runtime_error("getStructProperties: missing type: " + dots_type_name);
		}

		size_t offset = evalPropertyOffset(td, lastPropertyOffset);
		lastPropertyOffset = offset + td->sizeOf();
	}

	{
		auto pointerType = dots::type::Descriptor::registry().findDescriptor("pointer");
		sizeOf = evalPropertyOffset(pointerType, lastPropertyOffset);
		alignOf = evalMaxPropertyAlignment(sd);
	}

	return { sizeOf, alignOf };
}

static uint32_t calculateMaxTagValue(const StructDescriptorData &sd)
{
	uint32_t maxValue = 0;

	for (auto& t : *sd.properties)
	{
		maxValue = std::max(*t.tag, maxValue);
	}

	return maxValue;
}

namespace dots::type
{
    Struct::Struct(const StructDescriptor& descriptor) :
        _desc(&descriptor)
    {
        /* do nothing */
    }

    Struct::Struct(const Struct& other) :
		_validPropSet{},
		_desc(other._desc)
    {
	    /* do nothing */
    }

    Struct::Struct(Struct&& other) :
		_validPropSet{},
		_desc(other._desc)
    {
		/* do nothing */
    }

    Struct& Struct::operator = (const Struct& rhs)
    {
		_validPropSet = {};
		_desc = rhs._desc;

		return *this;
    }

    Struct& Struct::operator = (Struct&& rhs)
    {
		_validPropSet = {};
		_desc = rhs._desc;

		return *this;
    }

    const StructDescriptor& Struct::_descriptor() const
    {
        return *_desc;
    }

	property_set& Struct::_validPropertySet()
	{
		return _validPropSet;
	}
	const property_set& Struct::_validPropertySet() const
	{
		return _validPropSet;
	}

	const property_set& Struct::_keyPropertySet() const
	{
		return _desc->keys();
	}

	property_set Struct::_diffPropertySet(const Struct& other) const
	{
		property_set symmetricDiff = _validPropertySet().value() ^ other._validPropertySet().value();
		property_set intersection = _validPropertySet() & other._validPropertySet();

		if (!intersection.empty())
		{
			for (const auto&[propertyThis, propertyOther] : _propertyPairRange(other, intersection))
			{
				if (propertyThis.td().equal(&*propertyThis, &*propertyOther))
				{
					symmetricDiff |= propertyThis.set();
				}
			}
		}
		
		return symmetricDiff;
	}

	property_iterator Struct::_begin(const property_set& propertySet)
	{
		return property_iterator{ *this, _descriptor().properties().begin(), propertySet };
	}

	const_property_iterator Struct::_begin(const property_set& propertySet) const
	{
		return const_property_iterator{ *this, _descriptor().properties().begin(), propertySet };
	}

    property_iterator Struct::_end(const property_set& propertySet)
	{
		return property_iterator{ *this, _descriptor().properties().end(), propertySet };
	}

	const_property_iterator Struct::_end(const property_set& propertySet) const
	{
		return const_property_iterator{ *this, _descriptor().properties().end(), propertySet };
	}

	reverse_property_iterator Struct::_rbegin(const property_set& propertySet)
	{
		return reverse_property_iterator{ *this, _descriptor().properties().rbegin(), propertySet };
	}

	const_reverse_property_iterator Struct::_rbegin(const property_set& propertySet) const
	{
		return const_reverse_property_iterator{ *this, _descriptor().properties().rbegin(), propertySet };
	}

	reverse_property_iterator Struct::_rend(const property_set& propertySet)
	{
		return reverse_property_iterator{ *this, _descriptor().properties().rend(), propertySet };
	}

	const_reverse_property_iterator Struct::_rend(const property_set& propertySet) const
	{
		return const_reverse_property_iterator{ *this, _descriptor().properties().rend(), propertySet };
	}

    property_range Struct::_propertyRange(const property_set& propertySet)
    {
		return property_range{ _begin(propertySet), _end(propertySet) };
    }

    const_property_range Struct::_propertyRange(const property_set& propertySet) const
    {
		return const_property_range{ _begin(propertySet), _end(propertySet) };
    }

    reverse_property_range Struct::_propertyRangeReversed(const property_set& propertySet)
    {
		return reverse_property_range{ _rbegin(propertySet), _rend(propertySet) };
    }

    const_reverse_property_range Struct::_propertyRangeReversed(const property_set& propertySet) const
    {
		return const_reverse_property_range{ _rbegin(propertySet), _rend(propertySet) };
    }

    property_pair_range Struct::_propertyPairRange(Struct& rhs, const property_set& propertySet/* = PROPERTY_SET_ALL*/)
    {
		return property_pair_range{ property_pair_iterator{ _begin(propertySet), rhs._begin(propertySet) }, property_pair_iterator{ _end(propertySet), rhs._end(propertySet) } };
    }

    property_pair_range_const Struct::_propertyPairRange(const Struct& rhs, const property_set& propertySet/* = PROPERTY_SET_ALL*/)
    {
		return property_pair_range_const{ property_pair_iterator_const{ _begin(propertySet), rhs._begin(propertySet) }, property_pair_iterator_const{ _end(propertySet), rhs._end(propertySet) } };
    }

    const_property_pair_range_const Struct::_propertyPairRange(const Struct& rhs, const property_set& propertySet/* = PROPERTY_SET_ALL*/) const
    {
		return const_property_pair_range_const{ const_property_pair_iterator_const{ _begin(propertySet), rhs._begin(propertySet) }, const_property_pair_iterator_const{ _end(propertySet), rhs._end(propertySet) } };
    }

    reverse_property_pair_range Struct::_propertyPairRangeReversed(Struct& rhs, const property_set& propertySet/* = PROPERTY_SET_ALL*/)
    {
		return reverse_property_pair_range{ reverse_property_pair_iterator{ _rbegin(propertySet), rhs._rbegin(propertySet) }, reverse_property_pair_iterator{ _rend(propertySet), rhs._rend(propertySet) } };
    }

    reverse_property_pair_range_const Struct::_propertyPairRangeReversed(const Struct& rhs, const property_set& propertySet/* = PROPERTY_SET_ALL*/)
    {
		return reverse_property_pair_range_const{ reverse_property_pair_iterator_const{ _rbegin(propertySet), rhs._rbegin(propertySet) }, reverse_property_pair_iterator_const{ _rend(propertySet), rhs._rend(propertySet) } };
    }

    const_reverse_property_pair_range_const Struct::_propertyPairRangeReversed(const Struct& rhs, const property_set& propertySet/* = PROPERTY_SET_ALL*/) const
    {
		return const_reverse_property_pair_range_const{ const_reverse_property_pair_iterator_const{ _rbegin(propertySet), rhs._rbegin(propertySet) }, const_reverse_property_pair_iterator_const{ _rend(propertySet), rhs._rend(propertySet) } };
    }

    Struct& Struct::_assign(const Struct& other, const property_set& propertySet/* = PROPERTY_SET_ALL*/)
    {
		property_set assignPropertySet = other._validPropertySet() & propertySet;

		for (auto&[propertyThis, propertyOther] : _propertyPairRange(other))
		{
			if (propertyThis.isPartOf(assignPropertySet))
			{
				propertyThis.constructOrAssign(propertyOther);
			}
			else
			{
				propertyThis.destroy();
			}
		}

		return *this;
    }

    Struct& Struct::_copy(const Struct& other, const property_set& propertySet/* = PROPERTY_SET_ALL*/)
    {
		property_set copyPropertySet = (_validPropertySet() | other._validPropertySet()) & propertySet;

		for (auto&[propertyThis, propertyOther] : _propertyPairRange(other, copyPropertySet))
		{
			propertyThis.constructOrAssign(propertyOther);
		}

		return *this;
    }

    Struct& Struct::_merge(const Struct& other, const property_set& propertySet/* = PROPERTY_SET_ALL*/)
    {
		property_set mergePropertySet = other._validPropertySet() & propertySet;
		return _copy(other, mergePropertySet);
    }

    void Struct::_swap(Struct& other, const property_set& propertySet/* = PROPERTY_SET_ALL*/)
    {
		for (auto&[propertyThis, propertyOther] : _propertyPairRange(other, propertySet))
		{
			propertyThis.swap(propertyOther);
		}
    }

    void Struct::_clear(const property_set& propertySet/* = PROPERTY_SET_ALL*/)
    {
		for (auto& property : _propertyRange(propertySet))
		{
			property.destroy();
		}
    }

    bool Struct::_equal(const Struct& rhs) const
    {
		for (const auto&[propertyThis, propertyOther] : _propertyPairRange(rhs))
		{
			if (propertyThis != propertyOther)
			{
				return false;
			}
		}

		return true;
    }

    bool Struct::_less(const Struct& rhs) const
    {
		for (const auto&[propertyThis, propertyOther] : _propertyPairRange(rhs))
		{
			if (!(propertyThis < propertyOther))
			{
				return false;
			}
		}

		return true;
    }

    void Struct::_publish(const property_set& what/* = PROPERTY_SET_ALL*/, bool remove/* = false*/) const
    {
		onPublishObject->publish(&_descriptor(), this, what == PROPERTY_SET_ALL ? _validPropSet : what, remove);
    }

    void Struct::_remove(const property_set& what/* = PROPERTY_SET_ALL*/) const
    {
		_publish(what, true);
    }

    const StructDescriptor* Struct::MakeStructDescriptor(StructDescriptor* newstruct, const StructDescriptorData& structDescriptorData)
	{
		// Check if type is already registred
		{
			auto structDescriptor = Descriptor::registry().findStructDescriptor(structDescriptorData.name);
			if (structDescriptor) return structDescriptor;
		}

		auto structProperties = getStructProperties(structDescriptorData);
		::new (static_cast<void *>(newstruct)) StructDescriptor(structDescriptorData, structProperties.size, structProperties.alignment);

		std::size_t lastOffset = sizeof(Struct);


		for (const StructPropertyData &p : *newstruct->descriptorData().properties)
		{
			std::string dots_type_name = p.type; // DOTS typename
			auto td = Registry::fromWireName(dots_type_name);

			std::size_t offset = evalPropertyOffset(td, lastOffset);
			// Create Properties
			const Descriptor* propertyTypeDescriptor = td;
			if (propertyTypeDescriptor)
			{
				newstruct->m_properties.push_back(StructProperty(p.name, offset, p.tag, p.isKey, propertyTypeDescriptor));
				newstruct->m_propertySet.set(p.tag);
				if (p.isKey) {
					newstruct->m_keyProperties.set(p.tag);
				}
			}
			else
			{
				// Error, because the needed type is not found
				throw std::runtime_error("missing type '" + dots_type_name + "' for property '" + *p.name + "'");
			}
			lastOffset = offset + propertyTypeDescriptor->sizeOf();
		}

		if (structDescriptorData.publisherId.isValid())
		{
			newstruct->m_publisherId = structDescriptorData.publisherId;
		}


		Descriptor::registry().onNewStruct(newstruct);

		return newstruct;
	}

	const StructDescriptor* Struct::MakeStructDescriptor(StructDescriptor* structDescriptorAddr, const StructDescription& structDescription)
    {
		StructDescriptorData structDescriptorData;
		structDescriptorData.name(structDescription.name.data());

		auto& flags = structDescriptorData.flags();
		flags.cached(structDescription.flags & Cached);
		flags.internal(structDescription.flags & Internal);
		flags.persistent(structDescription.flags & Persistent);
		flags.cleanup(structDescription.flags & Cleanup);
		flags.local(structDescription.flags & Local);
		flags.substructOnly(structDescription.flags & SubstructOnly);

		auto& properties = structDescriptorData.properties();

		for (size_t i = 0; i < structDescription.numProperties; ++i)
		{
			const StructProperty& propertyDescription = structDescription.propertyDescriptions[i];
			StructPropertyData structPropertyData;
			structPropertyData.name(propertyDescription.name().data());
			structPropertyData.tag(propertyDescription.tag());
			structPropertyData.isKey(propertyDescription.isKey());
			structPropertyData.type(propertyDescription.type().data());
			properties.emplace_back(structPropertyData);
		}

		return MakeStructDescriptor(structDescriptorAddr, structDescriptorData);
    }

	property_iterator begin(Struct& instance)
    {
		return instance._begin();
    }

	const_property_iterator begin(const Struct& instance)
    {
		return instance._begin();
    }

	property_iterator end(Struct& instance)
    {
		return instance._end();
    }

	const_property_iterator end(const Struct& instance)
    {
		return instance._end();
    }

	reverse_property_iterator rbegin(Struct& instance)
    {
		return instance._rbegin();
    }

	const_reverse_property_iterator rbegin(const Struct& instance)
    {
		return instance._rbegin();
    }

	reverse_property_iterator rend(Struct& instance)
    {
		return instance._rend();
    }

	const_reverse_property_iterator rend(const Struct& instance)
    {
		return instance._rend();
    }
}