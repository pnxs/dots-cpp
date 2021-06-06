#include <dots/type/StructDescriptor.h>
#include <dots/type/Struct.h>
#include <dots/io/DescriptorConverter.h>
#include <dots/type/DynamicStruct.h>

namespace dots::type
{
    StructDescriptor<Typeless, false, void>::StructDescriptor(key_t key, std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptors, size_t areaOffset, size_t size, size_t alignment) :
        Descriptor<Typeless>(key, Type::Struct, std::move(name), size, alignment),
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

    Typeless& StructDescriptor<Typeless, false, void>::construct(Typeless& value) const
    {
        return Typeless::From(construct(value.to<Struct>()));
    }

    Struct& StructDescriptor<Typeless, false, void>::construct(Struct& instance) const
    {
        ::new(static_cast<void*>(::std::addressof(instance))) Struct{ *this };
        ::new(static_cast<void*>(::std::addressof(propertyArea(instance)))) PropertyArea{};

        return instance;
    }

    Typeless& StructDescriptor<Typeless, false, void>::construct(Typeless& value, const Typeless& other) const
    {
        return Typeless::From(construct(value.to<Struct>(), other.to<Struct>()));
    }

    Struct& StructDescriptor<Typeless, false, void>::construct(Struct& instance, const Struct& other) const
    {
        ::new(static_cast<void*>(::std::addressof(instance))) Struct{ other };
        ::new(static_cast<void*>(::std::addressof(propertyArea(instance)))) PropertyArea{};

        for (auto&[propertyInstance, propertyOther] : instance._propertyRange(other, other._validProperties()))
        {
            propertyInstance.construct(propertyOther);
        }

        return instance;
    }

    Typeless& StructDescriptor<Typeless, false, void>::construct(Typeless& value, Typeless&& other) const
    {
        return Typeless::From(construct(value.to<Struct>(), other.to<Struct>()));
    }

    Struct& StructDescriptor<Typeless, false, void>::construct(Struct& instance, Struct&& other) const
    {
        ::new(static_cast<void*>(::std::addressof(instance))) Struct{ std::move(other) };
        ::new(static_cast<void*>(::std::addressof(propertyArea(instance)))) PropertyArea{};

        for (auto&[propertyInstance, propertyOther] : instance._propertyRange(other, other._validProperties()))
        {
            propertyInstance.construct(std::move(propertyOther));
        }

        return instance;
    }

    Typeless& StructDescriptor<Typeless, false, void>::constructInPlace(Typeless& value) const
    {
        return construct(value);
    }

    Struct& StructDescriptor<Typeless, false, void>::constructInPlace(Struct& instance) const
    {
        return construct(instance);
    }

    Typeless& StructDescriptor<Typeless, false, void>::constructInPlace(Typeless& value, const Typeless& other) const
    {
        return construct(value, other);
    }

    Struct& StructDescriptor<Typeless, false, void>::constructInPlace(Struct& instance, const Struct& other) const
    {
        return construct(instance, other);
    }

    Typeless& StructDescriptor<Typeless, false, void>::constructInPlace(Typeless& value, Typeless&& other) const
    {
        return construct(value, std::move(other));
    }

    Struct& StructDescriptor<Typeless, false, void>::constructInPlace(Struct& instance, Struct&& other) const
    {
        return construct(instance, std::move(other));
    }

    void StructDescriptor<Typeless, false, void>::destruct(Typeless& value) const
    {
        Typeless::From(destruct(value.to<Struct>()));
    }

    Struct& StructDescriptor<Typeless, false, void>::destruct(Struct& instance) const
    {
        for (auto& property : instance._propertyRange())
        {
            property.destroy();
        }

        return instance;
    }

    Typeless& StructDescriptor<Typeless, false, void>::assign(Typeless& lhs, const Typeless& rhs) const
    {
        return Typeless::From(assign(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All));
    }

    Typeless& StructDescriptor<Typeless, false, void>::assign(Typeless& lhs, Typeless&& rhs) const
    {
        return Typeless::From(assign(lhs.to<Struct>(), std::move(rhs).to<Struct>(), PropertySet::All));
    }

    void StructDescriptor<Typeless, false, void>::swap(Typeless& value, Typeless& other) const
    {
        return swap(value.to<Struct>(), other.to<Struct>(), PropertySet::All);
    }

    bool StructDescriptor<Typeless, false, void>::equal(const Typeless& lhs, const Typeless& rhs) const
    {
        return equal(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
    }

    bool StructDescriptor<Typeless, false, void>::less(const Typeless& lhs, const Typeless& rhs) const
    {
        return less(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
    }

    bool StructDescriptor<Typeless, false, void>::lessEqual(const Typeless& lhs, const Typeless& rhs) const
    {
        return lessEqual(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
    }

    bool StructDescriptor<Typeless, false, void>::greater(const Typeless& lhs, const Typeless& rhs) const
    {
        return greater(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
    }

    bool StructDescriptor<Typeless, false, void>::greaterEqual(const Typeless& lhs, const Typeless& rhs) const
    {
        return greaterEqual(lhs.to<Struct>(), rhs.to<Struct>(), PropertySet::All);
    }

    size_t StructDescriptor<Typeless, false, void>::areaOffset() const
    {
        return m_areaOffset;
    }

    size_t StructDescriptor<Typeless, false, void>::numSubStructs() const
    {
        return m_numSubStructs;
    }

    bool StructDescriptor<Typeless, false, void>::usesDynamicMemory() const
    {
        return m_dynamicMemoryProperties.empty();
    }

    size_t StructDescriptor<Typeless, false, void>::dynamicMemoryUsage(const Typeless& instance) const
    {
        return dynamicMemoryUsage(instance.to<Struct>());
    }

    size_t StructDescriptor<Typeless, false, void>::dynamicMemoryUsage(const Struct& instance) const
    {
        if (usesDynamicMemory())
        {
            size_t dynMemUsage = 0;

            for (const ProxyProperty<>& property : instance._propertyRange(m_dynamicMemoryProperties))
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

    Struct& StructDescriptor<Typeless, false, void>::assign(Struct& instance, const Struct& other, const PropertySet& includedProperties) const
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

    Struct& StructDescriptor<Typeless, false, void>::assign(Struct& instance, Struct&& other, const PropertySet& includedProperties) const
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

    Struct& StructDescriptor<Typeless, false, void>::copy(Struct& instance, const Struct& other, const PropertySet& includedProperties) const
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

    Struct& StructDescriptor<Typeless, false, void>::merge(Struct& instance, const Struct& other, const PropertySet& includedProperties) const
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

    void StructDescriptor<Typeless, false, void>::swap(Struct& instance, Struct& other, const PropertySet& includedProperties) const
    {
        for (auto&[propertyThis, propertyOther] : instance._propertyRange(other, includedProperties))
        {
            propertyThis.swap(propertyOther);
        }
    }

    void StructDescriptor<Typeless, false, void>::clear(Struct& instance, const PropertySet& includedProperties) const
    {
        for (auto& property : instance._propertyRange(includedProperties))
        {
            property.destroy();
        }
    }

    bool StructDescriptor<Typeless, false, void>::equal(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
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

    bool StructDescriptor<Typeless, false, void>::same(const Struct& lhs, const Struct& rhs) const
    {
        return lhs._equal(rhs, lhs._keyProperties());
    }

    bool StructDescriptor<Typeless, false, void>::less(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
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

    bool StructDescriptor<Typeless, false, void>::lessEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
    {
        return !lhs._greater(rhs, includedProperties);
    }

    bool StructDescriptor<Typeless, false, void>::greater(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
    {
        return rhs._less(lhs, includedProperties);
    }

    bool StructDescriptor<Typeless, false, void>::greaterEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const
    {
        return !lhs._less(rhs, includedProperties);
    }

    PropertySet StructDescriptor<Typeless, false, void>::diffProperties(const Struct& instance, const Struct& other, const PropertySet& includedProperties) const
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

    uint8_t StructDescriptor<Typeless, false, void>::flags() const
    {
        return m_flags;
    }

    bool StructDescriptor<Typeless, false, void>::cached() const
    {
        return static_cast<bool>(m_flags & Cached);
    }

    bool StructDescriptor<Typeless, false, void>::cleanup() const
    {
        return static_cast<bool>(m_flags & Cleanup);
    }

    bool StructDescriptor<Typeless, false, void>::local() const
    {
        return static_cast<bool>(m_flags & Local);
    }

    bool StructDescriptor<Typeless, false, void>::persistent() const
    {
        return static_cast<bool>(m_flags & Persistent);
    }

    bool StructDescriptor<Typeless, false, void>::internal() const
    {
        return static_cast<bool>(m_flags & Internal);
    }

    bool StructDescriptor<Typeless, false, void>::substructOnly() const
    {
        return static_cast<bool>(m_flags & SubstructOnly);
    }

    const property_descriptor_container_t& StructDescriptor<Typeless, false, void>::propertyDescriptors() const
    {
        return m_propertyDescriptors;
    }

    partial_property_descriptor_container_t StructDescriptor<Typeless, false, void>::propertyDescriptors(const PropertySet& properties) const
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

    property_descriptor_container_t& StructDescriptor<Typeless, false, void>::propertyDescriptors()
    {
        return const_cast<property_descriptor_container_t&>(std::as_const(*this).propertyDescriptors());
    }

    const std::vector<PropertyPath>& StructDescriptor<Typeless, false, void>::propertyPaths() const
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

    const PropertySet& StructDescriptor<Typeless, false, void>::properties() const
    {
        return m_properties;
    }

    const PropertySet& StructDescriptor<Typeless, false, void>::keyProperties() const
    {
        return m_keyProperties;
    }
}