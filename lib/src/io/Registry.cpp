#include <dots/io/Registry.h>
#include <dots/io/DescriptorConverter.h>

namespace dots::io
{
    Registry::Registry(new_type_handler_t newTypeHandler/* = nullptr*/) :
        m_newTypeHandler(std::move(newTypeHandler))
    {
        // ensure fundamental types are instantiated and added to static descriptor map
        type::Descriptor<types::bool_t>::InstancePtr();

        type::Descriptor<types::int8_t>::InstancePtr();
        type::Descriptor<types::uint8_t>::InstancePtr();
        type::Descriptor<types::int16_t>::InstancePtr();
        type::Descriptor<types::uint16_t>::InstancePtr();
        type::Descriptor<types::int32_t>::InstancePtr();
        type::Descriptor<types::uint32_t>::InstancePtr();
        type::Descriptor<types::int64_t>::InstancePtr();
        type::Descriptor<types::uint64_t>::InstancePtr();

        type::Descriptor<types::float32_t>::InstancePtr();
        type::Descriptor<types::float64_t>::InstancePtr();

        type::Descriptor<types::property_set_t>::InstancePtr();

        type::Descriptor<types::timepoint_t>::InstancePtr();
        type::Descriptor<types::steady_timepoint_t>::InstancePtr();
        type::Descriptor<types::duration_t>::InstancePtr();

        type::Descriptor<types::uuid_t>::InstancePtr();
        type::Descriptor<types::string_t>::InstancePtr();

        // ensure fundamental vector types are instantiated and added to static descriptor map
        type::Descriptor<types::vector_t<types::bool_t>>::InstancePtr();

        type::Descriptor<types::vector_t<types::int8_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::uint8_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::int16_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::uint16_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::int32_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::uint32_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::int64_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::uint64_t>>::InstancePtr();

        type::Descriptor<types::vector_t<types::float32_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::float64_t>>::InstancePtr();

        type::Descriptor<types::vector_t<types::property_set_t>>::InstancePtr();

        type::Descriptor<types::vector_t<types::timepoint_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::steady_timepoint_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::duration_t>>::InstancePtr();

        type::Descriptor<types::vector_t<types::uuid_t>>::InstancePtr();
        type::Descriptor<types::vector_t<types::string_t>>::InstancePtr();
    }

    std::shared_ptr<type::Descriptor<>> Registry::findType(const std::string_view& name, bool assertNotNull/* = false*/) const
    {
        if (const std::shared_ptr<type::Descriptor<>>& descriptor = type::StaticDescriptorMap::Find(name); descriptor == nullptr)
        {
            if (auto it = m_types.find(name); it == m_types.end())
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
                return it->second;
            }
        }
        else
        {
            return descriptor;
        }
    }

    std::shared_ptr<type::EnumDescriptor<>> Registry::findEnumType(const std::string_view& name, bool assertNotNull/* = false*/) const
    {
        const auto& descriptor = std::static_pointer_cast<type::EnumDescriptor<>>(findType(name, assertNotNull));
        return descriptor == nullptr ? nullptr : (descriptor->type() == type::Type::Enum ? descriptor : nullptr);
    }

    std::shared_ptr<type::StructDescriptor<>> Registry::findStructType(const std::string_view& name, bool assertNotNull/* = false*/) const
    {
        const auto& descriptor = std::static_pointer_cast<type::StructDescriptor<>>(findType(name, assertNotNull));
        return descriptor == nullptr ? nullptr : (descriptor->type() == type::Type::Struct ? descriptor : nullptr);
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

    bool Registry::hasType(const std::string_view& name) const
    {
        return findType(name) != nullptr;
    }

    std::shared_ptr<type::Descriptor<>> Registry::registerType(std::shared_ptr<type::Descriptor<>> descriptor, bool assertNewType/* = true*/)
    {
        auto [it, emplaced] = m_types.try_emplace(descriptor->name(), descriptor);

        if (!emplaced)
        {
            if (assertNewType)
            {
                throw std::logic_error{ "there already is a type with name: " + descriptor->name() };
            }
            else
            {
                return it->second;
            }
        }

        if (descriptor->type() == type::Type::Vector)
        {
            auto vectorDescriptor = std::static_pointer_cast<type::VectorDescriptor>(descriptor);
            registerType(vectorDescriptor->valueDescriptorPtr(), false);
        }
        else if (descriptor->type() == type::Type::Enum)
        {
            auto enumDescriptor = std::static_pointer_cast<type::EnumDescriptor<>>(descriptor);
            registerType(enumDescriptor->underlyingDescriptorPtr(), false);
        }
        else if (descriptor->type() == type::Type::Struct)
        {
            auto structDescriptor = std::static_pointer_cast<type::StructDescriptor<>>(descriptor);

            for (const type::PropertyDescriptor& propertyDescriptor : structDescriptor->propertyDescriptors())
            {
                registerType(propertyDescriptor.valueDescriptorPtr(), false);
            }
        }

        if (m_newTypeHandler != nullptr)
        {
            m_newTypeHandler(*it->second);
        }

        return it->second;
    }

    void Registry::deregisterType(const std::shared_ptr<type::Descriptor<>>& descriptor, bool assertRegisteredType/* = true*/)
    {
        deregisterType(descriptor->name(), assertRegisteredType);
    }

    void Registry::deregisterType(const type::Descriptor<>& descriptor, bool assertRegisteredType/* = true*/)
    {
        deregisterType(descriptor.name(), assertRegisteredType);
    }

    void Registry::deregisterType(const std::string_view& name, bool assertRegisteredType/* = true*/)
    {
        auto it = m_types.find(name);

        if (it == m_types.end())
        {
            if (assertRegisteredType)
            {
                throw std::logic_error{ std::string{ "no type registered with name: " } + name.data() };
            }
        }
        else
        {
            m_types.erase(it);
        }
    }

    const type::Descriptor<>* Registry::findDescriptor(const std::string& name) const
    {
        return findType(name).get();
    }

    const type::StructDescriptor<>* Registry::findStructDescriptor(const std::string& name) const
    {
        return findStructType(name).get();
    }

    const std::map<std::string_view, std::shared_ptr<type::Descriptor<>>>& Registry::getTypes()
    {
        return m_types;
    }
}
