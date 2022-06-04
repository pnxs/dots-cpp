#include <dots/type/StructDescriptor.h>
#include <dots/type/Struct.h>
#include <dots/io/DescriptorConverter.h>
#include <dots/type/DynamicStruct.h>

namespace dots::type
{
    StructDescriptor<>::StructDescriptor(key_t key, std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptors, size_t areaOffset, size_t size, size_t alignment) :
        StaticDescriptor(key, Type::Struct, std::move(name), size, alignment),
        m_flags(flags),
        m_propertyDescriptors(propertyDescriptors),
        m_areaOffset(areaOffset),
        m_numSubStructs(0)
    {
        for (const PropertyDescriptor& propertyDescriptor : m_propertyDescriptors)
        {
            m_properties += propertyDescriptor.set();

            if (propertyDescriptor.isKey())
            {
                m_keyProperties += propertyDescriptor.set();
            }

            if (propertyDescriptor.valueDescriptor().type() == Type::Struct)
            {
                ++m_numSubStructs;
            }

            if (propertyDescriptor.valueDescriptor().usesDynamicMemory())
            {
                m_dynamicMemoryProperties += propertyDescriptor.set();
            }
        }
    }

    Typeless& StructDescriptor<>::construct(Typeless& value) const
    {
        return Typeless::From(construct(value.to<Struct>()));
    }

    Struct& StructDescriptor<>::construct(Struct& instance) const
    {
        ::new(static_cast<void*>(::std::addressof(instance))) Struct{ *this };
        ::new(static_cast<void*>(::std::addressof(propertyArea(instance)))) PropertyArea{};

        return instance;
    }

    Typeless& StructDescriptor<>::construct(Typeless& value, const Typeless& other) const
    {
        return Typeless::From(construct(value.to<Struct>(), other.to<Struct>()));
    }

    Struct& StructDescriptor<>::construct(Struct& instance, const Struct& other) const
    {
        ::new(static_cast<void*>(::std::addressof(instance))) Struct{ other };
        ::new(static_cast<void*>(::std::addressof(propertyArea(instance)))) PropertyArea{};

        for (auto&[propertyInstance, propertyOther] : instance._propertyRange(other, other._validProperties()))
        {
            propertyInstance.construct(propertyOther);
        }

        return instance;
    }

    Typeless& StructDescriptor<>::construct(Typeless& value, Typeless&& other) const
    {
        return Typeless::From(construct(value.to<Struct>(), other.to<Struct>()));
    }

    Struct& StructDescriptor<>::construct(Struct& instance, Struct&& other) const
    {
        ::new(static_cast<void*>(::std::addressof(instance))) Struct{ other };
        ::new(static_cast<void*>(::std::addressof(propertyArea(instance)))) PropertyArea{};

        for (auto&[propertyInstance, propertyOther] : instance._propertyRange(other, other._validProperties()))
        {
            propertyInstance.construct(std::move(propertyOther));
        }

        return instance;
    }

    Typeless& StructDescriptor<>::constructInPlace(Typeless& value) const
    {
        return construct(value);
    }

    Struct& StructDescriptor<>::constructInPlace(Struct& instance) const
    {
        return construct(instance);
    }

    Typeless& StructDescriptor<>::constructInPlace(Typeless& value, const Typeless& other) const
    {
        return construct(value, other);
    }

    Struct& StructDescriptor<>::constructInPlace(Struct& instance, const Struct& other) const
    {
        return construct(instance, other);
    }

    Typeless& StructDescriptor<>::constructInPlace(Typeless& value, Typeless&& other) const
    {
        return construct(value, std::move(other));
    }

    Struct& StructDescriptor<>::constructInPlace(Struct& instance, Struct&& other) const
    {
        return construct(instance, std::move(other));
    }

    void StructDescriptor<>::destruct(Typeless& value) const
    {
        Typeless::From(destruct(value.to<Struct>()));
    }

    Struct& StructDescriptor<>::destruct(Struct& instance) const
    {
        PropertyArea& propertyArea = instance._propertyArea();
        PropertySet& validProperties = instance._propertyArea().validProperties();
        PropertySet validDynamicProperties = validProperties ^ instance._descriptor().dynamicMemoryProperties();

        for (const PropertyDescriptor& propertyDescriptor : instance._propertyDescriptors())
        {
            if (propertyDescriptor.set() <= validDynamicProperties)
            {
                Typeless& value = propertyArea.getProperty<Typeless>(propertyDescriptor.offset());
                propertyDescriptor.valueDescriptor().destruct(value);
            }
        }

        validProperties = {};

        return instance;
    }

    Typeless& StructDescriptor<>::assign(Typeless& lhs, const Typeless& rhs) const
    {
        return Typeless::From(assign(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet{ PropertySet::All }));
    }

    Typeless& StructDescriptor<>::assign(Typeless& lhs, Typeless&& rhs) const
    {
        return Typeless::From(assign(lhs.to<Struct>(), std::move(rhs).to<Struct>(), PropertySet{ PropertySet::All }));
    }

    void StructDescriptor<>::swap(Typeless& value, Typeless& other) const
    {
        return swap(value.to<Struct>(), other.to<Struct>(), PropertySet{ PropertySet::All });
    }

    bool StructDescriptor<>::equal(const Typeless& lhs, const Typeless& rhs) const
    {
        return equal(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet{ PropertySet::All });
    }

    bool StructDescriptor<>::less(const Typeless& lhs, const Typeless& rhs) const
    {
        return less(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet{ PropertySet::All });
    }

    size_t StructDescriptor<>::areaOffset() const
    {
        return m_areaOffset;
    }

    size_t StructDescriptor<>::numSubStructs() const
    {
        return m_numSubStructs;
    }

    bool StructDescriptor<>::usesDynamicMemory() const
    {
        return !m_dynamicMemoryProperties.empty();
    }

    size_t StructDescriptor<>::dynamicMemoryUsage(const Typeless& instance) const
    {
        return dynamicMemoryUsage(instance.to<Struct>());
    }

    size_t StructDescriptor<>::dynamicMemoryUsage(const Struct& instance) const
    {
        if (usesDynamicMemory())
        {
            size_t dynMemUsage = 0;

            for (const ProxyProperty<>& property : instance._propertyRange(m_dynamicMemoryProperties ^ instance._validProperties()))
            {
                dynMemUsage += property.descriptor().valueDescriptor().dynamicMemoryUsage(property);
            }

            return dynMemUsage;
        }
        else
        {
            return 0;
        }
    }

    Struct& StructDescriptor<>::assign(Struct& instance, const Struct& other, PropertySet includedProperties) const
    {
        PropertySet assignProperties = other._validProperties() ^ includedProperties;

        for (auto&[propertyThis, propertyOther] : instance._propertyRange(other))
        {
            if (propertyThis.isPartOf(assignProperties))
            {
                propertyThis.constructOrAssign(propertyOther);
            }
            else
            {
                propertyThis.destroy();
            }
        }

        return instance;
    }

    Struct& StructDescriptor<>::assign(Struct& instance, Struct&& other, PropertySet includedProperties) const
    {
        PropertySet assignProperties = other._validProperties() ^ includedProperties;

        for (auto&[propertyThis, propertyOther] : instance._propertyRange(other))
        {
            if (propertyThis.isPartOf(assignProperties))
            {
                propertyThis.constructOrAssign(std::move(propertyOther));
            }
            else
            {
                propertyThis.destroy();
            }
        }

        return instance;
    }

    Struct& StructDescriptor<>::copy(Struct& instance, const Struct& other, PropertySet includedProperties) const
    {
        PropertySet copyProperties = (instance._validProperties() + other._validProperties()) ^ includedProperties;

        for (auto&[propertyThis, propertyOther] : instance._propertyRange(other, copyProperties))
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

        return instance;
    }

    Struct& StructDescriptor<>::merge(Struct& instance, const Struct& other, PropertySet includedProperties) const
    {
        PropertySet mergeProperties = other._validProperties() ^ includedProperties;

        for (auto& [propertyThis, propertyOther] : instance._propertyRange(other, mergeProperties))
        {
            if (propertyThis.descriptor().valueDescriptor().type() == Type::Struct)
            {
                propertyThis.constructOrValue().to<Struct>()._merge(propertyOther->to<Struct>());
            }
            else
            {
                propertyThis.constructOrAssign(propertyOther);
            }
        }

        return instance;
    }

    void StructDescriptor<>::swap(Struct& instance, Struct& other, PropertySet includedProperties) const
    {
        for (auto&[propertyThis, propertyOther] : instance._propertyRange(other, includedProperties))
        {
            propertyThis.swap(propertyOther);
        }
    }

    void StructDescriptor<>::clear(Struct& instance, PropertySet includedProperties) const
    {
        for (auto& property : instance._propertyRange(includedProperties))
        {
            property.destroy();
        }
    }

    bool StructDescriptor<>::equal(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const
    {
        for (const auto&[propertyThis, propertyOther] : lhs._propertyRange(rhs, includedProperties))
        {
            if (propertyThis != propertyOther)
            {
                return false;
            }
        }

        return true;
    }

    bool StructDescriptor<>::same(const Struct& lhs, const Struct& rhs) const
    {
        return lhs._equal(rhs, lhs._keyProperties());
    }

    bool StructDescriptor<>::less(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const
    {
        if (includedProperties.empty())
        {
            return false;
        }
        else
        {
            for (const auto&[propertyThis, propertyOther] : lhs._propertyRange(rhs, includedProperties))
            {
                if (propertyThis < propertyOther)
                {
                    return true;
                }
                else if (propertyThis > propertyOther)
                {
                    return false;
                }
            }

            return false;
        }
    }

    bool StructDescriptor<>::lessEqual(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const
    {
        return !lhs._greater(rhs, includedProperties);
    }

    bool StructDescriptor<>::greater(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const
    {
        return rhs._less(lhs, includedProperties);
    }

    bool StructDescriptor<>::greaterEqual(const Struct& lhs, const Struct& rhs, PropertySet includedProperties) const
    {
        return !lhs._less(rhs, includedProperties);
    }

    PropertySet StructDescriptor<>::diffProperties(const Struct& instance, const Struct& other, PropertySet includedProperties) const
    {
        PropertySet symmetricDiff = instance._validProperties().symmetricDifference(other._validProperties()) ^ includedProperties;
        PropertySet intersection = instance._validProperties() ^ other._validProperties() ^ includedProperties;

        if (!intersection.empty())
        {
            for (const auto&[propertyThis, propertyOther] : instance._propertyRange(other, intersection))
            {
                if (propertyThis != propertyOther)
                {
                    symmetricDiff += propertyThis.descriptor().set();
                }
            }
        }

        return symmetricDiff;
    }

    uint8_t StructDescriptor<>::flags() const
    {
        return m_flags;
    }

    bool StructDescriptor<>::cached() const
    {
        return static_cast<bool>(m_flags & Cached);
    }

    bool StructDescriptor<>::cleanup() const
    {
        return static_cast<bool>(m_flags & Cleanup);
    }

    bool StructDescriptor<>::local() const
    {
        return static_cast<bool>(m_flags & Local);
    }

    bool StructDescriptor<>::persistent() const
    {
        return static_cast<bool>(m_flags & Persistent);
    }

    bool StructDescriptor<>::internal() const
    {
        return static_cast<bool>(m_flags & Internal);
    }

    bool StructDescriptor<>::substructOnly() const
    {
        return static_cast<bool>(m_flags & SubstructOnly);
    }

    const property_descriptor_container_t& StructDescriptor<>::propertyDescriptors() const
    {
        return m_propertyDescriptors;
    }

    partial_property_descriptor_container_t StructDescriptor<>::propertyDescriptors(PropertySet properties) const
    {
        partial_property_descriptor_container_t partialPropertyDescriptors;

        for (const PropertyDescriptor& propertyDescriptor : m_propertyDescriptors)
        {
            if (propertyDescriptor.set() <= properties)
            {
                partialPropertyDescriptors.emplace_back(propertyDescriptor);
            }
        }

        return partialPropertyDescriptors;
    }

    property_descriptor_container_t& StructDescriptor<>::propertyDescriptors()
    {
        return const_cast<property_descriptor_container_t&>(std::as_const(*this).propertyDescriptors());
    }

    const std::vector<PropertyPath>& StructDescriptor<>::propertyPaths() const
    {
        if (m_propertyPaths.empty())
        {
            for (const PropertyDescriptor& propertyDescriptor : m_propertyDescriptors)
            {
                m_propertyPaths.emplace_back(propertyDescriptor);

                if (const auto* structDescriptor = propertyDescriptor.valueDescriptor().as<StructDescriptor<>>(); structDescriptor != nullptr)
                {
                    for (const PropertyPath& subPropertyPath : structDescriptor->propertyPaths())
                    {
                        PropertyPath::elements_t elements;
                        elements.emplace_back(propertyDescriptor);
                        elements.insert(elements.end(), subPropertyPath.elements().begin(), subPropertyPath.elements().end());
                        m_propertyPaths.emplace_back(std::move(elements));
                    }
                }
            }
        }

        return m_propertyPaths;
    }

    PropertySet StructDescriptor<>::properties() const
    {
        return m_properties;
    }

    PropertySet StructDescriptor<>::keyProperties() const
    {
        return m_keyProperties;
    }

    PropertySet StructDescriptor<>::dynamicMemoryProperties() const
    {
        return m_dynamicMemoryProperties;
    }
}
