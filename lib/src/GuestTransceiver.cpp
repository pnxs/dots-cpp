#include <dots/GuestTransceiver.h>
#include <dots/tools/logging.h>
#include <dots/serialization/AsciiSerialization.h>
#include <DotsMember.dots.h>
#include <DotsCacheInfo.dots.h>

namespace dots
{
    GuestTransceiver::GuestTransceiver(std::string selfName, boost::asio::io_context& ioContext/* = global_io_context()*/, bool staticUserTypes/* = true*/) :
        Transceiver(std::move(selfName), ioContext, staticUserTypes)
    {
        type::Descriptor<DotsCacheInfo>::Instance();
    }

    const Connection& GuestTransceiver::open(type::DescriptorMap preloadPublishTypes, type::DescriptorMap preloadSubscribeTypes, std::optional<std::string> authSecret, io::channel_ptr_t channel)
    {
        if (m_hostConnection != std::nullopt)
        {
            throw std::logic_error{ "already connected" };
        }

        m_preloadPublishTypes = std::move(preloadPublishTypes);
        m_preloadSubscribeTypes = std::move(preloadSubscribeTypes);

        m_hostConnection.emplace(std::move(channel), false, std::move(authSecret));
        m_hostConnection->asyncReceive(registry(), nullptr, selfName(),
            [this](Connection& connection, io::Transmission transmission){ return handleTransmission(connection, std::move(transmission)); },
            [this](Connection& connection, const std::exception_ptr& e){ handleTransition(connection, e); }
        );

        return *m_hostConnection;
    }

    const Connection& GuestTransceiver::open(io::channel_ptr_t channel)
    {
        return open({}, {}, std::nullopt, std::move(channel));
    }

    void GuestTransceiver::publish(const type::Struct& instance, std::optional<types::property_set_t> includedProperties/* = std::nullopt*/, bool remove/* = false*/)
    {
        const type::StructDescriptor<>& descriptor = instance._descriptor();

        if (descriptor.substructOnly())
        {
            throw std::logic_error{ "attempt to publish substruct-only type: " + descriptor.name() };
        }

        if (includedProperties == std::nullopt)
        {
            includedProperties = instance._validProperties();
        }

        if (!(descriptor.keyProperties() <= *includedProperties))
        {
            throw std::runtime_error("tried to publish instance with invalid key (not all key-fields are set) what=" + includedProperties->toString() + " tdkeys=" + descriptor.keyProperties().toString());
        }

        if (m_hostConnection == std::nullopt)
        {
            throw std::runtime_error{ "attempt to publish on closed connection" };
        }

        try
        {
            m_hostConnection->transmit(instance, includedProperties, remove);
        }
        catch (...)
        {
            m_hostConnection->handleError(std::current_exception());
        }
    }

    void GuestTransceiver::joinGroup(const std::string_view& name)
    {
        if (m_joinedGroups.count(std::string(name)) == 0)
        {
            publish(DotsMember{
                DotsMember::groupName_i{name},
                DotsMember::event_i{DotsMemberEvent::join}
            });
            m_joinedGroups.insert(std::string(name));
        }
    }

    void GuestTransceiver::leaveGroup(const std::string_view& name)
    {
        if (m_joinedGroups.count(std::string(name)))
        {
            publish(DotsMember{
                DotsMember::groupName_i{name},
                DotsMember::event_i{DotsMemberEvent::leave}
            });
            m_joinedGroups.erase(std::string(name));
        }
    }

    bool GuestTransceiver::handleTransmission(Connection&/* connection*/, io::Transmission transmission)
    {
        dispatcher().dispatch(transmission);
        return true;
    }

    void GuestTransceiver::handleTransition(Connection& connection, const std::exception_ptr&/* e*/) noexcept
    {
        try
        {
            if (connection.state() == DotsConnectionState::early_subscribe)
            {
                for (const auto& [name, descriptor] : m_preloadPublishTypes)
                {
                    (void)name;
                    connection.transmit(descriptor->to<type::StructDescriptor<>>());
                }

                for (const auto& [name, descriptor] : m_preloadSubscribeTypes)
                {
                    connection.transmit(descriptor->to<type::StructDescriptor<>>());
                    joinGroup(name);
                }

                m_preloadPublishTypes.clear();
                m_preloadSubscribeTypes.clear();
            }
            else if (connection.state() == DotsConnectionState::closed)
            {
                if (m_hostConnection != std::nullopt)
                {
                    m_hostConnection = std::nullopt;
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("error while handling transition for connection " << connection.peerDescription() << " -> " << e.what());
            m_hostConnection = std::nullopt;
        }
    }
}