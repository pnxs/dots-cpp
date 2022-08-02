// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/type/Registry.h>

namespace dots::type
{
    template<typename T>
    static void ensureDescriptor()
    {
        Descriptor<T>::Instance();
        Descriptor<vector_t<T>>::Instance();
    }


    Registry::Registry( std::optional<new_type_handler_t> newTypeHandler/* = std::nullopt*/, StaticTypePolicy staticTypePolicy /* = StaticTypePolicy::All*/) :
        m_newTypeHandler(std::move(newTypeHandler))
    {
        // ensure fundamental types are instantiated and added to static descriptor map
        // ensure fundamental vector types are instantiated and added to static descriptor map
        ensureDescriptor<bool_t>();

        ensureDescriptor<int8_t>();
        ensureDescriptor<uint8_t>();
        ensureDescriptor<int16_t>();
        ensureDescriptor<uint16_t>();
        ensureDescriptor<int32_t>();
        ensureDescriptor<uint32_t>();
        ensureDescriptor<int64_t>();
        ensureDescriptor<uint64_t>();

        ensureDescriptor<float32_t>();
        ensureDescriptor<float64_t>();

        ensureDescriptor<property_set_t>();

        ensureDescriptor<timepoint_t>();
        ensureDescriptor<steady_timepoint_t>();
        ensureDescriptor<duration_t>();

        ensureDescriptor<uuid_t>();
        ensureDescriptor<string_t>();

        switch (staticTypePolicy)
        {
            case StaticTypePolicy::FundamentalOnly:
                for (auto&[name, descriptor]: static_descriptors())
                {
                    if (descriptor->isFundamentalType())
                    {
                        m_types.emplace(descriptor);
                    }
                    else if (const auto *vectorDescriptor = descriptor->as<VectorDescriptor>(); vectorDescriptor != nullptr)
                    {
                        if (vectorDescriptor->valueDescriptor().isFundamentalType())
                        {
                            m_types.emplace(descriptor);
                        }
                    }
                }
                break;
            case StaticTypePolicy::InternalOnly:
                for (auto&[name, descriptor]: static_descriptors())
                {
                    if (not IsUserType(*descriptor))
                    {
                        m_types.emplace(descriptor);
                    }
                }
                break;
            case StaticTypePolicy::All:
                for (auto&[name, descriptor]: static_descriptors())
                {
                    m_types.emplace(descriptor);
                }
                break;
        }

    }

    Registry::const_iterator_t Registry::begin() const
    {
        return m_types.begin();
    }

    Registry::const_iterator_t Registry::end() const
    {
        return m_types.end();
    }

    Registry::const_iterator_t Registry::cbegin() const
    {
        return m_types.cbegin();
    }

    Registry::const_iterator_t Registry::cend() const
    {
        return m_types.cend();
    }

    const Descriptor<>* Registry::findType(std::string_view name, bool assertNotNull/* = false*/) const
    {
        if (const Descriptor<>* descriptor = m_types.find(name); descriptor == nullptr)
        {
            if (assertNotNull)
            {
                throw std::logic_error{std::string{"no type registered with name1: "} + name.data()};
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return descriptor;
        }
    }

    const EnumDescriptor* Registry::findEnumType(std::string_view name, bool assertNotNull/* = false*/) const
    {
        auto descriptor = static_cast<const EnumDescriptor*>(findType(name, assertNotNull));

        if (descriptor != nullptr && descriptor->type() != Type::Enum)
        {
            descriptor = nullptr;
        }

        if (assertNotNull && descriptor == nullptr)
        {
            throw std::logic_error{ std::string{ "registered type with name '" } + name.data() + "' is not an enum type" };
        }

        return descriptor;
    }

    const StructDescriptor* Registry::findStructType(std::string_view name, bool assertNotNull/* = false*/) const
    {
        auto descriptor = static_cast<const StructDescriptor*>(findType(name, assertNotNull));

        if (descriptor != nullptr && descriptor->type() != Type::Struct)
        {
            descriptor = nullptr;
        }

        if (assertNotNull && descriptor == nullptr)
        {
            throw std::logic_error{ std::string{ "registered type with name '" } + name.data() + "' is not a struct type" };
        }

        return descriptor;
    }

    Descriptor<>* Registry::findType(std::string_view name, bool assertNotNull)
    {
        return const_cast<Descriptor<>*>(std::as_const(*this).findType(name, assertNotNull));
    }

    EnumDescriptor* Registry::findEnumType(std::string_view name, bool assertNotNull)
    {
        return const_cast<EnumDescriptor*>(std::as_const(*this).findEnumType(name, assertNotNull));
    }

    StructDescriptor* Registry::findStructType(std::string_view name, bool assertNotNull)
    {
        return const_cast<StructDescriptor*>(std::as_const(*this).findStructType(name, assertNotNull));
    }

    const Descriptor<>& Registry::getType(std::string_view name) const
    {
        return *findType(name, true);
    }

    const EnumDescriptor& Registry::getEnumType(std::string_view name) const
    {
        return *findEnumType(name, true);
    }

    const  StructDescriptor& Registry::getStructType(std::string_view name) const
    {
        return *findStructType(name, true);
    }

    Descriptor<>& Registry::getType(std::string_view name)
    {
        return const_cast<Descriptor<>&>(std::as_const(*this).getType(name));
    }

    EnumDescriptor& Registry::getEnumType(std::string_view name)
    {
        return const_cast<EnumDescriptor&>(std::as_const(*this).getEnumType(name));
    }

    StructDescriptor& Registry::getStructType(std::string_view name)
    {
        return const_cast<StructDescriptor&>(std::as_const(*this).getStructType(name));
    }

    bool Registry::hasType(std::string_view name) const
    {
        return findType(name) != nullptr;
    }

    size_t Registry::size() const
    {
        return m_types.size();
    }

    Descriptor<>& Registry::registerType(Descriptor<>& descriptor, bool assertNewType/* = true*/)
    {
        if (auto descriptor_ = findType(descriptor.name()); descriptor_ != nullptr)
        {
            if (assertNewType)
            {
                throw std::logic_error{ "there already is a type with name: " + descriptor.name() };
            }
            else
            {
                return *descriptor_;
            }
        }

        m_types.emplace(descriptor);

        if (auto vectorDescriptor = descriptor.as<VectorDescriptor>(); vectorDescriptor != nullptr)
        {
            registerType(vectorDescriptor->valueDescriptor(), false);
        }
        else if (auto enumDescriptor = descriptor.as<EnumDescriptor>(); enumDescriptor != nullptr)
        {
            registerType(enumDescriptor->underlyingDescriptor(), false);
        }
        else if (auto structDescriptor = descriptor.as<StructDescriptor>(); structDescriptor != nullptr)
        {
            for (PropertyDescriptor& propertyDescriptor : structDescriptor->propertyDescriptors())
            {
                registerType(propertyDescriptor.valueDescriptor(), false);
            }
        }

        if (m_newTypeHandler != std::nullopt)
        {
            (*m_newTypeHandler)(descriptor);
        }

        return descriptor;
    }

    Descriptor<>& Registry::registerType(std::shared_ptr<Descriptor<>> descriptor, bool assertNewType)
    {
        return registerType(*descriptor, assertNewType);
    }

    void Registry::deregisterType(const Descriptor<>& descriptor, bool assertRegisteredType/* = true*/)
    {
        m_types.erase(descriptor.name(), assertRegisteredType);
    }

    void Registry::deregisterType(std::string_view name, bool assertRegisteredType/* = true*/)
    {
        m_types.erase(name, assertRegisteredType);
    }

    bool Registry::IsUserType(const Descriptor<>& descriptor)
    {
        if (const auto* structDescriptor = descriptor.as<StructDescriptor>(); structDescriptor != nullptr)
        {
            return !structDescriptor->internal();
        }
        else if (const auto* vectorDescriptor = descriptor.as<VectorDescriptor>(); vectorDescriptor != nullptr)
        {
            return IsUserType(vectorDescriptor->valueDescriptor());
        }
        else
        {
            return false;
        }
    }
}
