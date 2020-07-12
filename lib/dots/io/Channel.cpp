#include <dots/io/Channel.h>
#include <dots/io/Registry.h>
#include <dots/io/DescriptorConverter.h>
#include <DotsMsgError.dots.h>

namespace dots::io
{
    Channel::Channel(key_t key) :
        shared_ptr_only(key),
        m_initialized(false),
        m_registry(nullptr)
    {
        /* do nothing */
    }

    void Channel::init(io::Registry& registry)
    {
        if (m_initialized)
        {
            throw std::logic_error{ "channel has already been initialized" };
        }

        m_registry = &registry;
        m_initialized = true;
    }

    void Channel::asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&& errorHandler)
    {
        verifyInitialized();

        if (receiveHandler == nullptr || errorHandler == nullptr)
        {
            throw std::logic_error{ "both a receive and an error handler must be set" };
        }
        
        m_receiveHandler = std::move(receiveHandler);
        m_errorHandler = std::move(errorHandler);
        asyncReceiveImpl();
    }

    void Channel::transmit(const type::Struct& instance)
    {
      transmit(DotsHeader{
            DotsHeader::typeName_i{ instance._descriptor().name() },
            DotsHeader::attributes_i{ instance._validProperties() }
        }, instance);
    }

    void Channel::transmit(const DotsHeader& header, const type::Struct& instance)
    {
        verifyInitialized();
        exportDependencies(instance._descriptor());
        transmitImpl(header, instance);
    }

    void Channel::transmit(const Transmission& transmission)
    {
        verifyInitialized();
        exportDependencies(transmission.instance()->_descriptor());
        transmitImpl(transmission);
    }

    void Channel::transmit(const type::StructDescriptor<>& descriptor)
    {
        exportDependencies(descriptor);
    }

    const Medium& Channel::medium() const
    {
        return DefaultMedium;
    }

    const io::Registry& Channel::registry() const
    {
        verifyInitialized();
        return *m_registry;
    }
    
    io::Registry& Channel::registry()
    {
        verifyInitialized();
        return *m_registry;
    }

    void Channel::transmitImpl(const Transmission& transmission)
    {
        transmitImpl(transmission.header(), transmission.instance());
    }

    void Channel::processReceive(Transmission transmission) noexcept
    {
        try
        {
            importDependencies(transmission.instance());

            if (m_receiveHandler(std::move(transmission)))
            {
                asyncReceiveImpl();
            }
            else
            {
                m_receiveHandler = nullptr;
                m_errorHandler = nullptr;
            }
        }
        catch (...)
        {
            processError(std::current_exception());
        }
    }
    
    void Channel::processError(const std::exception_ptr& e)
    {
        try
        {
            m_errorHandler(e);
        }
        catch (const std::exception& e)
        {
            throw std::logic_error{ std::string{ "exception in channel error handler -> " } + e.what() };
        }       
    }

    void Channel::processError(const std::string& what)
    {
        processError(std::make_exception_ptr(std::runtime_error{ what }));
    }

    void Channel::verifyErrorCode(const std::error_code& errorCode)
    {
        if (errorCode)
        {
            throw std::system_error{ errorCode };
        }
    }

    void Channel::importDependencies(const type::Struct& instance)
    {
        if (auto* structDescriptorData = instance._as<StructDescriptorData>())
        {
            if (bool isNewSharedType = m_sharedTypes.emplace(structDescriptorData->name).second; isNewSharedType)
            {
                io::DescriptorConverter{ registry() }(*structDescriptorData);
            }
        }
        else if (auto* enumDescriptorData = instance._as<EnumDescriptorData>())
        {
            if (bool isNewSharedType = m_sharedTypes.emplace(enumDescriptorData->name).second; isNewSharedType)
            {
                io::DescriptorConverter{ registry() }(*enumDescriptorData);
            }
        }
    }

    void Channel::exportDependencies(const type::Descriptor<>& descriptor)
    {
        if (bool isNewSharedType = m_sharedTypes.emplace(descriptor.name()).second; isNewSharedType)
        {
            if (descriptor.type() == type::Type::Vector)
            {
                auto& vectorDescriptor = static_cast<const type::VectorDescriptor&>(descriptor);
                exportDependencies(vectorDescriptor.valueDescriptor());
            }
            else if (descriptor.type() == type::Type::Enum)
            {
                auto& enumDescriptor = static_cast<const type::EnumDescriptor<>&>(descriptor);
                exportDependencies(enumDescriptor.underlyingDescriptor());
                transmit(io::DescriptorConverter{ registry() }(enumDescriptor));
            }
            else if (descriptor.type() == type::Type::Struct)
            {
                if (auto& structDescriptor = static_cast<const type::StructDescriptor<>&>(descriptor); !structDescriptor.internal())
                {
                    for (const type::PropertyDescriptor& propertyDescriptor : structDescriptor.propertyDescriptors())
                    {
                        exportDependencies(propertyDescriptor.valueDescriptor());
                    }
                    
                    transmit(io::DescriptorConverter{ registry() }(structDescriptor));
                }
            }
        }
    }

    void Channel::verifyInitialized() const
    {
        if (!m_initialized)
        {
            throw std::logic_error{ "channel has not been initialized" };
        }
    }
}