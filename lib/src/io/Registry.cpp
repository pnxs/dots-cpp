#include <dots/io/Registry.h>
#include <dots/io/DescriptorConverter.h>

namespace dots::io
{
    Registry::Registry(new_type_handler_t newTypeHandler/* = nullptr*/, bool staticUserTypes/* = true*/) :
        m_newTypeHandler(std::move(newTypeHandler)),
        m_staticUserTypes(staticUserTypes)
    {
        // ensure fundamental types are instantiated and added to static descriptor map
        type::Descriptor<types::bool_t>::Instance();

        type::Descriptor<types::int8_t>::Instance();
        type::Descriptor<types::uint8_t>::Instance();
        type::Descriptor<types::int16_t>::Instance();
        type::Descriptor<types::uint16_t>::Instance();
        type::Descriptor<types::int32_t>::Instance();
        type::Descriptor<types::uint32_t>::Instance();
        type::Descriptor<types::int64_t>::Instance();
        type::Descriptor<types::uint64_t>::Instance();

        type::Descriptor<types::float32_t>::Instance();
        type::Descriptor<types::float64_t>::Instance();

        type::Descriptor<types::property_set_t>::Instance();

        type::Descriptor<types::timepoint_t>::Instance();
        type::Descriptor<types::steady_timepoint_t>::Instance();
        type::Descriptor<types::duration_t>::Instance();

        type::Descriptor<types::uuid_t>::Instance();
        type::Descriptor<types::string_t>::Instance();

        // ensure fundamental vector types are instantiated and added to static descriptor map
        type::Descriptor<types::vector_t<types::bool_t>>::Instance();

        type::Descriptor<types::vector_t<types::int8_t>>::Instance();
        type::Descriptor<types::vector_t<types::uint8_t>>::Instance();
        type::Descriptor<types::vector_t<types::int16_t>>::Instance();
        type::Descriptor<types::vector_t<types::uint16_t>>::Instance();
        type::Descriptor<types::vector_t<types::int32_t>>::Instance();
        type::Descriptor<types::vector_t<types::uint32_t>>::Instance();
        type::Descriptor<types::vector_t<types::int64_t>>::Instance();
        type::Descriptor<types::vector_t<types::uint64_t>>::Instance();

        type::Descriptor<types::vector_t<types::float32_t>>::Instance();
        type::Descriptor<types::vector_t<types::float64_t>>::Instance();

        type::Descriptor<types::vector_t<types::property_set_t>>::Instance();

        type::Descriptor<types::vector_t<types::timepoint_t>>::Instance();
        type::Descriptor<types::vector_t<types::steady_timepoint_t>>::Instance();
        type::Descriptor<types::vector_t<types::duration_t>>::Instance();

        type::Descriptor<types::vector_t<types::uuid_t>>::Instance();
        type::Descriptor<types::vector_t<types::string_t>>::Instance();
    }

    const type::Descriptor<>* Registry::findType(const std::string_view& name, bool assertNotNull/* = false*/) const
    {
        if (const type::Descriptor<>* descriptor = m_types.find(name); descriptor == nullptr)
        {
            if (descriptor = type::static_descriptors().find(name); descriptor == nullptr || (!m_staticUserTypes && IsUserType(*descriptor)))
            {
                if (assertNotNull)
                {
                    throw std::logic_error{ std::string{ "no type registered with name: " } + name.data() };
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
            return descriptor;
        }
    }

    const type::EnumDescriptor<>* Registry::findEnumType(const std::string_view& name, bool assertNotNull/* = false*/) const
    {
        auto descriptor = static_cast<const type::EnumDescriptor<>*>(findType(name, assertNotNull));

        if (descriptor != nullptr && descriptor->type() != type::Type::Enum)
        {
            descriptor = nullptr;
        }

        if (assertNotNull && descriptor == nullptr)
        {
            throw std::logic_error{ std::string{ "registered type with name '" } + name.data() + "' is not an enum type" };
        }

        return descriptor;
    }

    const type::StructDescriptor<>* Registry::findStructType(const std::string_view& name, bool assertNotNull/* = false*/) const
    {
        auto descriptor = static_cast<const type::StructDescriptor<>*>(findType(name, assertNotNull));

        if (descriptor != nullptr && descriptor->type() != type::Type::Struct)
        {
            descriptor = nullptr;
        }

        if (assertNotNull && descriptor == nullptr)
        {
            throw std::logic_error{ std::string{ "registered type with name '" } + name.data() + "' is not a struct type" };
        }

        return descriptor;
    }

    type::Descriptor<>* Registry::findType(const std::string_view& name, bool assertNotNull)
    {
        return const_cast<type::Descriptor<>*>(std::as_const(*this).findType(name, assertNotNull));
    }

    type::EnumDescriptor<>* Registry::findEnumType(const std::string_view& name, bool assertNotNull)
    {
        return const_cast<type::EnumDescriptor<>*>(std::as_const(*this).findEnumType(name, assertNotNull));
    }

    type::StructDescriptor<>* Registry::findStructType(const std::string_view& name, bool assertNotNull)
    {
        return const_cast<type::StructDescriptor<>*>(std::as_const(*this).findStructType(name, assertNotNull));
    }

    const type::Descriptor<>& Registry::getType(const std::string_view& name) const
    {
        return *findType(name, true);
    }

    const type::EnumDescriptor<>& Registry::getEnumType(const std::string_view& name) const
    {
        return *findEnumType(name, true);
    }

    const type:: StructDescriptor<>& Registry::getStructType(const std::string_view& name) const
    {
        return *findStructType(name, true);
    }

    type::Descriptor<>& Registry::getType(const std::string_view& name)
    {
        return const_cast<type::Descriptor<>&>(std::as_const(*this).getType(name));
    }

    type::EnumDescriptor<>& Registry::getEnumType(const std::string_view& name)
    {
        return const_cast<type::EnumDescriptor<>&>(std::as_const(*this).getEnumType(name));
    }

    type::StructDescriptor<>& Registry::getStructType(const std::string_view& name)
    {
        return const_cast<type::StructDescriptor<>&>(std::as_const(*this).getStructType(name));
    }

    bool Registry::hasType(const std::string_view& name) const
    {
        return findType(name) != nullptr;
    }

    type::Descriptor<>& Registry::registerType(type::Descriptor<>& descriptor, bool assertNewType/* = true*/)
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

        if (auto vectorDescriptor = descriptor.as<type::VectorDescriptor>(); vectorDescriptor != nullptr)
        {
            registerType(vectorDescriptor->valueDescriptor(), false);
        }
        else if (auto enumDescriptor = descriptor.as<type::EnumDescriptor<>>(); enumDescriptor != nullptr)
        {
            registerType(enumDescriptor->underlyingDescriptor(), false);
        }
        else if (auto structDescriptor = descriptor.as<type::StructDescriptor<>>(); structDescriptor != nullptr)
        {
            for (type::PropertyDescriptor& propertyDescriptor : structDescriptor->propertyDescriptors())
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

    type::Descriptor<>& Registry::registerType(std::shared_ptr<type::Descriptor<>> descriptor, bool assertNewType)
    {
        return registerType(*descriptor, assertNewType);
    }

    void Registry::deregisterType(const type::Descriptor<>& descriptor, bool assertRegisteredType/* = true*/)
    {
        m_types.erase(descriptor.name(), assertRegisteredType);
    }

    void Registry::deregisterType(const std::string_view& name, bool assertRegisteredType/* = true*/)
    {
        m_types.erase(name, assertRegisteredType);
    }

    const type::Descriptor<>* Registry::findDescriptor(const std::string& name) const
    {
        return findType(name);
    }

    const type::StructDescriptor<>* Registry::findStructDescriptor(const std::string& name) const
    {
        return findStructType(name);
    }

    const std::map<std::string_view, std::shared_ptr<type::Descriptor<>>>& Registry::getTypes()
    {
        return m_types.data();
    }

    bool Registry::IsUserType(const type::Descriptor<>& descriptor)
    {
        if (const auto* structDescriptor = descriptor.as<type::StructDescriptor<>>(); structDescriptor != nullptr)
        {
            return !structDescriptor->internal();
        }
        else if (const auto* vectorDescriptor = descriptor.as<type::VectorDescriptor>(); vectorDescriptor != nullptr)
        {
            return IsUserType(vectorDescriptor->valueDescriptor());
        }
        else
        {
            return false;
        }
    }
}