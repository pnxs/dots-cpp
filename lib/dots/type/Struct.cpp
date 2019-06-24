#include "Struct.h"
#include "StructDescriptor.h"
#include "Registry.h"
#include "PropertyIterator.h"
#include "PropertyPairIterator.h"
#include "dots/io/Transceiver.h"
#include <StructDescriptorData.dots.h>

namespace dots::type
{
    Struct::Struct(const StructDescriptor& descriptor) :
        _desc(&descriptor)
    {
        /* do nothing */
    }

    Struct::Struct(const Struct& other) :
        _desc(other._desc)
    {
        /* do nothing */
    }

    Struct::Struct(Struct&& other) :
        _desc(other._desc)
    {
        /* do nothing */
    }

    Struct& Struct::operator = (const Struct& rhs)
    {
        _desc = rhs._desc;
        return *this;
    }

    Struct& Struct::operator = (Struct&& rhs) noexcept
    {
        _desc = rhs._desc;
        return *this;
    }

    const StructDescriptor& Struct::_descriptor() const
    {
        return *_desc;
    }

    bool Struct::_usesDynamicMemory() const
    {
        return _desc->usesDynamicMemory();
    }

    size_t Struct::_dynamicMemoryUsage() const
    {
        return _desc->dynamicMemoryUsage(this);
    }

    size_t Struct::_staticMemoryUsage() const
    {
        return _desc->sizeOf();
    }

    size_t Struct::_totalMemoryUsage() const
    {
        return _staticMemoryUsage() + _dynamicMemoryUsage();
    }

    property_set& Struct::_validProperties()
    {
        return _validPropSet;
    }
    const property_set& Struct::_validProperties() const
    {
        return _validPropSet;
    }

    const property_set& Struct::_keyProperties() const
    {
        return _desc->keys();
    }

    property_iterator Struct::_begin(const property_set& includedProperties)
    {
        return property_iterator{ *this, _descriptor().properties().begin(), includedProperties };
    }

    const_property_iterator Struct::_begin(const property_set& includedProperties) const
    {
        return const_property_iterator{ *this, _descriptor().properties().begin(), includedProperties };
    }

    property_iterator Struct::_end(const property_set& includedProperties)
    {
        return property_iterator{ *this, _descriptor().properties().end(), includedProperties };
    }

    const_property_iterator Struct::_end(const property_set& includedProperties) const
    {
        return const_property_iterator{ *this, _descriptor().properties().end(), includedProperties };
    }

    reverse_property_iterator Struct::_rbegin(const property_set& includedProperties)
    {
        return reverse_property_iterator{ *this, _descriptor().properties().rbegin(), includedProperties };
    }

    const_reverse_property_iterator Struct::_rbegin(const property_set& includedProperties) const
    {
        return const_reverse_property_iterator{ *this, _descriptor().properties().rbegin(), includedProperties };
    }

    reverse_property_iterator Struct::_rend(const property_set& includedProperties)
    {
        return reverse_property_iterator{ *this, _descriptor().properties().rend(), includedProperties };
    }

    const_reverse_property_iterator Struct::_rend(const property_set& includedProperties) const
    {
        return const_reverse_property_iterator{ *this, _descriptor().properties().rend(), includedProperties };
    }

    property_range Struct::_propertyRange(const property_set& includedProperties)
    {
        return property_range{ _begin(includedProperties), _end(includedProperties) };
    }

    const_property_range Struct::_propertyRange(const property_set& includedProperties) const
    {
        return const_property_range{ _begin(includedProperties), _end(includedProperties) };
    }

    reverse_property_range Struct::_propertyRangeReversed(const property_set& includedProperties)
    {
        return reverse_property_range{ _rbegin(includedProperties), _rend(includedProperties) };
    }

    const_reverse_property_range Struct::_propertyRangeReversed(const property_set& includedProperties) const
    {
        return const_reverse_property_range{ _rbegin(includedProperties), _rend(includedProperties) };
    }

    property_pair_range Struct::_propertyRange(Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return property_pair_range{ property_pair_iterator{ _begin(includedProperties), rhs._begin(includedProperties) }, property_pair_iterator{ _end(includedProperties), rhs._end(includedProperties) } };
    }

    property_pair_range_const Struct::_propertyRange(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return property_pair_range_const{ property_pair_iterator_const{ _begin(includedProperties), rhs._begin(includedProperties) }, property_pair_iterator_const{ _end(includedProperties), rhs._end(includedProperties) } };
    }

    const_property_pair_range_const Struct::_propertyRange(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/) const
    {
        return const_property_pair_range_const{ const_property_pair_iterator_const{ _begin(includedProperties), rhs._begin(includedProperties) }, const_property_pair_iterator_const{ _end(includedProperties), rhs._end(includedProperties) } };
    }

    reverse_property_pair_range Struct::_propertyRangeReversed(Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return reverse_property_pair_range{ reverse_property_pair_iterator{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, reverse_property_pair_iterator{ _rend(includedProperties), rhs._rend(includedProperties) } };
    }

    reverse_property_pair_range_const Struct::_propertyRangeReversed(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return reverse_property_pair_range_const{ reverse_property_pair_iterator_const{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, reverse_property_pair_iterator_const{ _rend(includedProperties), rhs._rend(includedProperties) } };
    }

    const_reverse_property_pair_range_const Struct::_propertyRangeReversed(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/) const
    {
        return const_reverse_property_pair_range_const{ const_reverse_property_pair_iterator_const{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, const_reverse_property_pair_iterator_const{ _rend(includedProperties), rhs._rend(includedProperties) } };
    }

    property_range Struct::_validPropertyRange(const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return _propertyRange(_validProperties() & includedProperties);
    }

    const_property_range Struct::_validPropertyRange(const property_set& includedProperties/* = PROPERTY_SET_ALL*/) const
    {
        return _propertyRange(_validProperties() & includedProperties);
    }

    reverse_property_range Struct::_validPropertyRangeReversed(const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return _propertyRangeReversed(_validProperties() & includedProperties);
    }

    const_reverse_property_range Struct::_validPropertyRangeReversed(const property_set& includedProperties/* = PROPERTY_SET_ALL*/) const
    {
        return _propertyRangeReversed(_validProperties() & includedProperties);
    }

    property_pair_range Struct::_validPropertyRange(Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return _propertyRange(rhs, _validProperties() & rhs._validProperties() & includedProperties);
    }

    property_pair_range_const Struct::_validPropertyRange(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return _propertyRange(rhs, _validProperties() & rhs._validProperties() & includedProperties);
    }

    const_property_pair_range_const Struct::_validPropertyRange(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/) const
    {
        return _propertyRange(rhs, _validProperties() & rhs._validProperties() & includedProperties);
    }

    reverse_property_pair_range Struct::_validPropertyRangeReversed(Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return _propertyRangeReversed(rhs, _validProperties() & rhs._validProperties() & includedProperties);
    }

    reverse_property_pair_range_const Struct::_validPropertyRangeReversed(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        return _propertyRangeReversed(rhs, _validProperties() & rhs._validProperties() & includedProperties);
    }

    const_reverse_property_pair_range_const Struct::_validPropertyRangeReversed(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/) const
    {
        return _propertyRangeReversed(rhs, _validProperties() & rhs._validProperties() & includedProperties);
    }

    Struct& Struct::_assign(const Struct& other, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        property_set assignPropertySet = other._validProperties() & includedProperties;

        for (auto&[propertyThis, propertyOther] : _propertyRange(other))
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

    Struct& Struct::_copy(const Struct& other, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        property_set copyPropertySet = (_validProperties() | other._validProperties()) & includedProperties;

        for (auto&[propertyThis, propertyOther] : _propertyRange(other, copyPropertySet))
        {
            if (propertyOther.isValid())
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

    Struct& Struct::_merge(const Struct& other, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        property_set mergePropertySet = other._validProperties() & includedProperties;
        return _copy(other, mergePropertySet);
    }

    void Struct::_swap(Struct& other, const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        for (auto&[propertyThis, propertyOther] : _propertyRange(other, includedProperties))
        {
            propertyThis.swap(propertyOther);
        }
    }

    void Struct::_clear(const property_set& includedProperties/* = PROPERTY_SET_ALL*/)
    {
        for (auto& property : _propertyRange(includedProperties))
        {
            property.destroy();
        }
    }

    bool Struct::_equal(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/) const
    {
        for (const auto&[propertyThis, propertyOther] : _propertyRange(rhs, includedProperties))
        {
            if (propertyThis != propertyOther)
            {
                return false;
            }
        }

        return true;
    }

    bool Struct::_less(const Struct& rhs, const property_set& includedProperties/* = PROPERTY_SET_ALL*/) const
    {
        if (includedProperties.empty())
        {
            return false;
        }
        else
        {
            for (const auto&[propertyThis, propertyOther] : _propertyRange(rhs, includedProperties))
            {
                if (!(propertyThis < propertyOther))
                {
                    return false;
                }
            }

            return true;
        }        
    }

    property_set Struct::_diffProperties(const Struct& other) const
    {
        property_set symmetricDiff = _validProperties().value() ^ other._validProperties().value();
        property_set intersection = _validProperties() & other._validProperties();

        if (!intersection.empty())
        {
            for (const auto&[propertyThis, propertyOther] : _propertyRange(other, intersection))
            {
                if (propertyThis.td().equal(&*propertyThis, &*propertyOther))
                {
                    symmetricDiff |= propertyThis.set();
                }
            }
        }

        return symmetricDiff;
    }

    bool Struct::_hasProperties(const property_set properties) const
    {
        return (properties & _validPropSet) == properties;
    }

    void Struct::_publish(const property_set& includedProperties/* = PROPERTY_SET_ALL*/, bool remove/* = false*/) const
    {
        onPublishObject->publish(&_descriptor(), *this, includedProperties == PROPERTY_SET_ALL ? _validPropSet : includedProperties, remove);
    }

    void Struct::_remove(const property_set& includedProperties/* = PROPERTY_SET_ALL*/) const
    {
        _publish(includedProperties, true);
    }

    const StructDescriptor* Struct::MakeStructDescriptor(StructDescriptor* newstruct, const StructDescriptorData& structDescriptorData)
    {
        // Check if type is already registred
        {
            auto structDescriptor = Descriptor::registry().findStructDescriptor(structDescriptorData.name);
            if (structDescriptor) return structDescriptor;
        }

        ::new (static_cast<void *>(newstruct)) StructDescriptor(structDescriptorData);
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
            const PropertyDescription& propertyDescription = structDescription.propertyDescriptions[i];
            StructPropertyData structPropertyData;
            structPropertyData.name(propertyDescription.name.data());
            structPropertyData.tag(propertyDescription.tag);
            structPropertyData.isKey(propertyDescription.isKey);
            structPropertyData.type(propertyDescription.type.data());
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