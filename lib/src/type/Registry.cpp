#include <dots/type/Registry.h>

namespace dots::type
{
    template<typename T>
    static void ensureDescriptor()
    {
        Descriptor<T>::Instance();
        Descriptor<types::vector_t<T>>::Instance();
    }


    Registry::Registry(new_type_handler_t newTypeHandler/* = nullptr*/, StaticTypePolicy staticTypePolicy /* = StaticTypePolicy::All*/) :
        m_newTypeHandler(std::move(newTypeHandler)),
        m_staticTypePolicy(staticTypePolicy)
    {
        // ensure fundamental types are instantiated and added to static descriptor map
        // ensure fundamental vector types are instantiated and added to static descriptor map
        ensureDescriptor<types::bool_t>();

        ensureDescriptor<types::int8_t>();
        ensureDescriptor<types::uint8_t>();
        ensureDescriptor<types::int16_t>();
        ensureDescriptor<types::uint16_t>();
        ensureDescriptor<types::int32_t>();
        ensureDescriptor<types::uint32_t>();
        ensureDescriptor<types::int64_t>();
        ensureDescriptor<types::uint64_t>();

        ensureDescriptor<types::float32_t>();
        ensureDescriptor<types::float64_t>();

        ensureDescriptor<types::property_set_t>();

        ensureDescriptor<types::timepoint_t>();
        ensureDescriptor<types::steady_timepoint_t>();
        ensureDescriptor<types::duration_t>();

        ensureDescriptor<types::uuid_t>();
        ensureDescriptor<types::string_t>();

        if (m_staticTypePolicy == StaticTypePolicy::None)
        {
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
        }
    }

    const Descriptor<>* Registry::findType(std::string_view name, bool assertNotNull/* = false*/) const
    {
        if (const Descriptor<>* descriptor = m_types.find(name); descriptor == nullptr)
        {
            if (m_staticTypePolicy != StaticTypePolicy::None)
            {
                if (descriptor = static_descriptors().find(name); descriptor == nullptr || (m_staticTypePolicy != StaticTypePolicy::All && IsUserType(*descriptor)))
                {
                    if (assertNotNull)
                    {
                        throw std::logic_error{std::string{"no type registered with name: "} + name.data()};
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
            else
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
        }
        else
        {
            return descriptor;
        }
    }

    const EnumDescriptor<>* Registry::findEnumType(std::string_view name, bool assertNotNull/* = false*/) const
    {
        auto descriptor = static_cast<const EnumDescriptor<>*>(findType(name, assertNotNull));

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

    const StructDescriptor<>* Registry::findStructType(std::string_view name, bool assertNotNull/* = false*/) const
    {
        auto descriptor = static_cast<const StructDescriptor<>*>(findType(name, assertNotNull));

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

    EnumDescriptor<>* Registry::findEnumType(std::string_view name, bool assertNotNull)
    {
        return const_cast<EnumDescriptor<>*>(std::as_const(*this).findEnumType(name, assertNotNull));
    }

    StructDescriptor<>* Registry::findStructType(std::string_view name, bool assertNotNull)
    {
        return const_cast<StructDescriptor<>*>(std::as_const(*this).findStructType(name, assertNotNull));
    }

    const Descriptor<>& Registry::getType(std::string_view name) const
    {
        return *findType(name, true);
    }

    const EnumDescriptor<>& Registry::getEnumType(std::string_view name) const
    {
        return *findEnumType(name, true);
    }

    const  StructDescriptor<>& Registry::getStructType(std::string_view name) const
    {
        return *findStructType(name, true);
    }

    Descriptor<>& Registry::getType(std::string_view name)
    {
        return const_cast<Descriptor<>&>(std::as_const(*this).getType(name));
    }

    EnumDescriptor<>& Registry::getEnumType(std::string_view name)
    {
        return const_cast<EnumDescriptor<>&>(std::as_const(*this).getEnumType(name));
    }

    StructDescriptor<>& Registry::getStructType(std::string_view name)
    {
        return const_cast<StructDescriptor<>&>(std::as_const(*this).getStructType(name));
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
        else if (auto enumDescriptor = descriptor.as<EnumDescriptor<>>(); enumDescriptor != nullptr)
        {
            registerType(enumDescriptor->underlyingDescriptor(), false);
        }
        else if (auto structDescriptor = descriptor.as<StructDescriptor<>>(); structDescriptor != nullptr)
        {
            for (PropertyDescriptor& propertyDescriptor : structDescriptor->propertyDescriptors())
            {
                registerType(propertyDescriptor.valueDescriptor(), false);
            }
        }

        if (m_newTypeHandler != nullptr)
        {
            m_newTypeHandler(descriptor);
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
        if (const auto* structDescriptor = descriptor.as<StructDescriptor<>>(); structDescriptor != nullptr)
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