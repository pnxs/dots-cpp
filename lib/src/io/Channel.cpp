// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/io/Channel.h>
#include <cassert>
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

    void Channel::asyncReceive(receive_handler_t receiveHandler, error_handler_t errorHandler)
    {
        if (m_asyncReceiving)
        {
            throw std::logic_error{ "only one async receive can be active at any given time" };
        }

        assert(m_initialized);

        m_receiveHandler = std::move(receiveHandler);
        m_errorHandler = std::move(errorHandler);
        m_asyncReceiving = true;
        asyncReceiveImpl();
    }

    void Channel::transmit(const type::Struct& instance)
    {
      transmit(DotsHeader{
            .typeName = instance._descriptor().name(),
            .attributes = instance._validProperties()
        }, instance);
    }

    void Channel::transmit(const DotsHeader& header, const type::Struct& instance)
    {
        assert(m_initialized);
        exportDependencies(instance._descriptor());
        transmitImpl(header, instance);
    }

    void Channel::transmit(const Transmission& transmission)
    {
        assert(m_initialized);
        exportDependencies(transmission.instance());
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
        assert(m_initialized);
        return *m_registry;
    }

    type::Registry& Channel::registry()
    {
        assert(m_initialized);
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
            if ((*m_receiveHandler)(std::move(transmission)))
            {
                asyncReceiveImpl();
            }
        }
        catch (...)
        {
            processError(std::current_exception());
        }
    }

    void Channel::processError(std::exception_ptr ePtr)
    {
        try
        {
            m_asyncReceiving = false;
            (*m_errorHandler)(ePtr);
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

    void Channel::verifyErrorCode(std::error_code errorCode)
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
            if (bool isNewSharedType = m_sharedTypeNames.count(*structDescriptorData->name) == 0; isNewSharedType)
            {
                m_sharedTypeNames.emplace(*structDescriptorData->name);
                m_sharedTypeDescriptors.emplace(&DescriptorConverter{ registry() }(*structDescriptorData));
            }
        }
        else if (auto* enumDescriptorData = instance._as<EnumDescriptorData>())
        {
            if (bool isNewSharedType = m_sharedTypeNames.count(*enumDescriptorData->name) == 0; isNewSharedType)
            {
                m_sharedTypeNames.emplace(*enumDescriptorData->name);
                m_sharedTypeDescriptors.emplace(&DescriptorConverter{ registry() }(*enumDescriptorData));
            }
        }
    }

    void Channel::exportDependencies(const type::Descriptor<>& descriptor)
    {
        if (bool isNewSharedType = m_sharedTypeDescriptors.count(&descriptor) == 0; isNewSharedType)
        {
            m_sharedTypeDescriptors.emplace(&descriptor);

            if (const auto* vectorDescriptor = descriptor.as<type::VectorDescriptor>(); vectorDescriptor != nullptr)
            {
                exportDependencies(vectorDescriptor->valueDescriptor());
            }
            else if (const auto* enumDescriptor = descriptor.as<type::EnumDescriptor>(); enumDescriptor != nullptr)
            {
                exportDependencies(enumDescriptor->underlyingDescriptor());
                transmit(DescriptorConverter{ registry() }(*enumDescriptor));
            }
            else if (const auto* structDescriptor = descriptor.as<type::StructDescriptor>(); structDescriptor != nullptr)
            {
                if (!structDescriptor->internal())
                {
                    for (const type::PropertyDescriptor& propertyDescriptor : structDescriptor->propertyDescriptors())
                    {
                        exportDependencies(propertyDescriptor.valueDescriptor());
                    }

                    transmit(DescriptorConverter{ registry() }(*structDescriptor));
                }
            }
        }

    }

    void Channel::exportDependencies(const type::Struct& instance)
    {
        // If descriptor is for type StructDescriptorData or EnumDescriptorData, check if dependent types has to be exported first.
        // This can happen if e.g. a dynamic client sends a DescriptorRequest (and is therefore joined for StructDescriptorData
        // and EnumDescriptorData) and another client is just in the process sending it descriptors, but the dynamic client
        // did not receive the start of the transmission-sequence. In that case it could otherwise not resolve the dependent
        // types, because it may not have received them yet.
        if (auto* structDescriptorData = instance._as<StructDescriptorData>())
        {
            auto* embeddedDescriptor = registry().findStructType(*structDescriptorData->name);
            if (embeddedDescriptor) {
                exportDependencies(*embeddedDescriptor);
            }
        }
        else if (auto* enumDescriptorData = instance._as<EnumDescriptorData>())
        {
            auto* embeddedDescriptor = m_registry->findEnumType(*enumDescriptorData->name);
            if (embeddedDescriptor) {
                exportDependencies(*embeddedDescriptor);
            }
        }
    }
}
