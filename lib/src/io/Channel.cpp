#include <dots/io/Channel.h>
#include <dots/type/Registry.h>
#include <dots/io/DescriptorConverter.h>
#include <DotsMsgError.dots.h>

namespace dots::io
{
    Channel::Channel(key_t key) :
        shared_ptr_only(key),
        m_asyncReceiving(false),
        m_initialized(false),
        m_registry(nullptr)
    {
        /* do nothing */
    }

    const Endpoint& Channel::localEndpoint()
    {
        if (m_localEndpoint == std::nullopt)
        {
            throw std::runtime_error{ "endpoints have not been initialized" };
        }

        return *m_localEndpoint;
    }

    const Endpoint& Channel::remoteEndpoint()
    {
        if (m_remoteEndpoint == std::nullopt)
        {
            throw std::runtime_error{ "endpoints have not been initialized" };
        }

        return *m_remoteEndpoint;
    }

    void Channel::init(type::Registry& registry)
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
        if (m_asyncReceiving)
        {
            throw std::logic_error{ "only one async receive can be active at any given time" };
        }

        verifyInitialized();

        if (receiveHandler == nullptr || errorHandler == nullptr)
        {
            throw std::logic_error{ "both a receive and an error handler must be set" };
        }

        m_receiveHandler = std::move(receiveHandler);
        m_errorHandler = std::move(errorHandler);
        m_asyncReceiving = true;
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

    void Channel::transmit(const type::Descriptor<>& descriptor)
    {
        exportDependencies(descriptor);
    }

    void Channel::initEndpoints(Endpoint localEndpoint, Endpoint remoteEndpoint)
    {
        if (m_localEndpoint != std::nullopt)
        {
            throw std::runtime_error{ "endpoints have already been initialized" };
        }

        m_localEndpoint.emplace(std::move(localEndpoint));
        m_remoteEndpoint.emplace(std::move(remoteEndpoint));
    }

    const type::Registry& Channel::registry() const
    {
        verifyInitialized();
        return *m_registry;
    }

    type::Registry& Channel::registry()
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

            // note: if the receive handler yields 'false', the channel must no
            // longer be accessed afterwards, because it might have already been
            // deleted by the callee prior to returning
            if (m_receiveHandler(std::move(transmission)))
            {
                asyncReceiveImpl();
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
            m_asyncReceiving = false;
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
            if (const auto* vectorDescriptor = descriptor.as<type::VectorDescriptor>(); vectorDescriptor != nullptr)
            {
                exportDependencies(vectorDescriptor->valueDescriptor());
            }
            else if (const auto* enumDescriptor = descriptor.as<type::EnumDescriptor<>>(); enumDescriptor != nullptr)
            {
                exportDependencies(enumDescriptor->underlyingDescriptor());
                transmit(io::DescriptorConverter{ registry() }(*enumDescriptor));
            }
            else if (const auto* structDescriptor = descriptor.as<type::StructDescriptor<>>(); structDescriptor != nullptr)
            {
                if (!structDescriptor->internal())
                {
                    for (const type::PropertyDescriptor& propertyDescriptor : structDescriptor->propertyDescriptors())
                    {
                        exportDependencies(propertyDescriptor.valueDescriptor());
                    }

                    transmit(io::DescriptorConverter{ registry() }(*structDescriptor));
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