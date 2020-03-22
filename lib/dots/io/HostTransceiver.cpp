#include "HostTransceiver.h"
#include <vector>
#include <dots/dots.h>
#include <dots/common/logging.h>
#include <DotsCacheInfo.dots.h>
#include <DotsClient.dots.h>

namespace dots
{
    HostTransceiver::HostTransceiver(std::string selfName, transition_handler_t transitionHandler) :
        Transceiver(std::move(selfName)),
        m_transitionHandler{ std::move(transitionHandler) }
    {
        /* do nothing */
    }

    void HostTransceiver::listen(listener_ptr_t&& listener)
    {
        Listener* listenerPtr = listener.get();
        m_listeners.emplace(listenerPtr, std::move(listener));

        listenerPtr->asyncAccept(
            [this](Listener& listener, channel_ptr_t channel){ return handleListenAccept(listener, std::move(channel)); },
            [this](Listener& listener, const std::exception_ptr& e){ handleListenError(listener, e); }
        );
    }

    void HostTransceiver::publish(const type::Struct& instance, types::property_set_t includedProperties, bool remove)
    {
        const type::StructDescriptor<>& descriptor = instance._descriptor();

        DotsTransportHeader header{
            DotsTransportHeader::destinationGroup_i{ descriptor.name() },
            DotsTransportHeader::dotsHeader_i{
                DotsHeader::typeName_i{ descriptor.name() },
                DotsHeader::sentTime_i{ types::timepoint_t::Now() },
                DotsHeader::serverSentTime_i{ types::timepoint_t::Now() },
                DotsHeader::attributes_i{ includedProperties ==  types::property_set_t::All ? instance._validProperties() : includedProperties },
				DotsHeader::sender_i{ io::Connection::HostId },
                DotsHeader::removeObj_i{ remove }
            }
        };

        if (descriptor.internal() && !instance._is<DotsClient>() && !instance._is<DotsDescriptorRequest>())
        {
            header.nameSpace("SYS");
        }

        // TODO: avoid local copy
        Transmission transmission{ type::AnyStruct{ instance } };

        dispatcher().dispatch(header.dotsHeader, transmission.instance(), true);
        transmit(nullptr, header.destinationGroup, header, std::move(transmission));
    }

    void HostTransceiver::joinGroup(const std::string_view&/* name*/)
    {
        /* do nothing */
    }

	void HostTransceiver::leaveGroup(const std::string_view&/* name*/)
    {
        /* do nothing */
    }

    void HostTransceiver::transmit(io::Connection* origin, const std::string& group, const DotsTransportHeader& header, Transmission&& transmission)
    {
        using dirty_connection_t = std::pair<io::Connection*, std::exception_ptr>;
        std::vector<dirty_connection_t> dirtyConnections;

        for (io::Connection* destinationConnection : m_groups[group])
        {
            LOG_DEBUG_S("deliver message group:" << this << "(" << group << ")");

            if (destinationConnection->state() != DotsConnectionState::closed)
            {
                try
                {
                    destinationConnection->transmit(header, transmission);
                }
                catch (...)
                {
                    dirtyConnections.emplace_back(destinationConnection, std::current_exception());
                }
            }
        }

        if (!dirtyConnections.empty())
        {
            std::exception_ptr originError;

            for (const auto& [connection, e] : dirtyConnections)
            {
                if (connection == origin)
                {
                    originError = e;
                }
                else
                {
                    connection->handleError(e);
                }
            }

            if (originError != nullptr)
            {
                std::rethrow_exception(originError);
            }
        }
    }

    bool HostTransceiver::handleListenAccept(Listener&/* listener*/, channel_ptr_t channel)
    {
        auto connection = std::make_shared<io::Connection>(std::move(channel), true);
        connection->asyncReceive(registry(), selfName(),
            [this](io::Connection& connection, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself) { return handleReceive(connection, header, std::move(transmission), isFromMyself); },
            [this](io::Connection& connection, const std::exception_ptr& e) { handleTransition(connection, e); }
        );
        m_guestConnections.emplace(connection.get(), connection);

        return true;
    }

    void HostTransceiver::handleListenError(Listener& listener, const std::exception_ptr& e)
    {
        try
        {
            std::rethrow_exception(e);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("error while listening for incoming channels -> " << e.what());
        }
        
        m_listeners.erase(&listener);
    }

    bool HostTransceiver::handleReceive(io::Connection& connection, const DotsTransportHeader& header, Transmission&& transmission, bool isFromMyself)
    {
        if (const type::Struct& instance = transmission.instance(); instance._descriptor().internal())
        {
            if (auto* member = instance._as<DotsMember>())
            {
                handleMemberMessage(connection, *member);
            }
            else if (auto* descriptorRequest = instance._as<DotsDescriptorRequest>())
            {
                handleDescriptorRequest(connection, *descriptorRequest);
            }
            else if (auto* clearCache = instance._as<DotsClearCache>())
            {
                handleClearCache(connection, *clearCache);
            }
        }

        dispatcher().dispatch(header.dotsHeader, transmission.instance(), isFromMyself);
        transmit(&connection, header.destinationGroup, header, std::move(transmission));

        return true;
    }

    void HostTransceiver::handleTransition(io::Connection& connection, const std::exception_ptr& e) noexcept
    {
        if (m_transitionHandler)
        {
            try
            {
                m_transitionHandler(connection);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR_S("error in connection transition handler -> " << e.what());
            }
        }

        try
        {
            if (connection.state() == DotsConnectionState::closed)
            {
                if (e != nullptr)
                {
                    try
                    {
                        std::rethrow_exception(e);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR_S("connection error: " << e.what());
                    }
                }

                for (auto& [groupName, group] : m_groups)
                {
                    group.erase(&connection);
                }

                std::vector<const type::Struct*> cleanupInstances;

                for (const auto& [descriptor, container] : pool())
                {
                    if (descriptor->cleanup())
                    {
                        for (const auto& [instance, cloneInfo] : container)
                        {
                            if (connection.peerId() == cloneInfo.lastUpdateFrom)
                            {
                                cleanupInstances.emplace_back(&*instance);
                            }
                        }
                    }
                }

                for (const type::Struct* instance : cleanupInstances)
                {
                    remove(*instance);
                }

                LOG_INFO_S("connection closed -> peerId: " << connection.peerId() << ", name: " << connection.peerName());
                m_guestConnections.erase(&connection);
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_S("error while handling connection transition -> peerId: " << connection.peerId() << ", name: " << connection.peerName() << " -> " << e.what());
        }
    }

    void HostTransceiver::handleMemberMessage(io::Connection& connection, const DotsMember& member)
    {
        member._assertHasProperties(DotsMember::groupName_p + DotsMember::event_p);
        const std::string& groupName = member.groupName;

        if (member.event == DotsMemberEvent::kill)
        {
            LOG_WARN_S("guest '" << connection.peerName() << "' requested unsupported kill event");
        }
        else if (member.event == DotsMemberEvent::leave)
        {
            if (size_t removed = m_groups[groupName].erase(&connection); removed == 0)
            {
                LOG_WARN_S("guest '" << connection.peerName() << "' is not a member of group '" << groupName << "'");
            }
        }
        else if (member.event == DotsMemberEvent::join)
        {
            if (auto [it, emplaced] = m_groups[groupName].emplace(&connection); emplaced)
            {
                LOG_INFO_S("guest '" << connection.peerName() << "' is now a member of group '" << groupName << "'");
            }
            else
            {
                LOG_WARN_S("guest '" << connection.peerName() << "' is already member of group '" << groupName << "'");
            }

            // note: transmitting the container content even when the guest has already joined the group is currently
            // necessary to retain backwards compatibility
            if (const Container<>* container = pool().find(groupName); container != nullptr)
            {
                if (container->descriptor().cached())
                {
                    transmitContainer(connection, *container);
                }

                connection.transmit(DotsCacheInfo{
                    DotsCacheInfo::typeName_i{ member.groupName },
                    DotsCacheInfo::endTransmission_i{ true }
                });
            }
        }
    }

    void HostTransceiver::handleDescriptorRequest(io::Connection& connection, const DotsDescriptorRequest& descriptorRequest)
    {
        const types::vector_t<types::string_t>& whiteList = descriptorRequest.whitelist.isValid() ? *descriptorRequest.whitelist : types::vector_t<types::string_t>{};
        const types::vector_t<types::string_t>& blacklist = descriptorRequest.blacklist.isValid() ? *descriptorRequest.blacklist : types::vector_t<types::string_t>{};

        LOG_INFO_S("received DescriptorRequest from " << connection.peerName() << "(" << connection.peerId() << ")");

        for (const auto& [descriptor, container] : pool())
        {
            if (descriptor->internal())
            {
                continue;
            }

            if (!whiteList.empty() && std::find(whiteList.begin(), whiteList.end(), descriptor->name()) == whiteList.end())
            {
                continue;
            }

            if (!blacklist.empty() && std::find(blacklist.begin(), blacklist.end(), descriptor->name()) != blacklist.end())
            {
                continue;
            }

            LOG_DEBUG_S("sending descriptor for type '" << descriptor->name() << "' to " << connection.peerId());

            connection.transmit(*descriptor);
        }

        connection.transmit(DotsCacheInfo{ DotsCacheInfo::endDescriptorRequest_i{ true } });
    }

    void HostTransceiver::handleClearCache(io::Connection&/* connection*/, const DotsClearCache& clearCache)
    {
        clearCache._assertHasProperties(DotsClearCache::typeNames_p);
        const types::vector_t<types::string_t>& typeNames = clearCache.typeNames;

        for (auto& [descriptor, container] : pool())
        {
            if (std::find(typeNames.begin(), typeNames.end(), container.descriptor().name()) != typeNames.end())
            {
                LOG_INFO_S("clear container '" << container.descriptor().name() << "' (" << container.size() << " elements)");
                std::vector<const type::Struct*> removeInstances;

                for (const auto& [instance, cloneInformation] : container)
                {
                    (void)cloneInformation;
                    removeInstances.emplace_back(&instance.get());
                }

                for (const type::Struct* instance : removeInstances)
                {
                    remove(*instance);
                }
            }
        }
    }

    void HostTransceiver::transmitContainer(io::Connection& connection, const Container<>& container)
    {
        const auto& td = container.descriptor();

        LOG_DEBUG_S("send cache for " << td.name() << " size=" << container.size());

        if (container.empty())
        {
            return;
        }

        DotsTransportHeader header{
            DotsTransportHeader::destinationGroup_i{ container.descriptor().name() },
            DotsTransportHeader::dotsHeader_i{
                DotsHeader::typeName_i{ container.descriptor().name() },
                DotsHeader::fromCache_i{ container.size() },
                DotsHeader::removeObj_i{ false }
            }
        };

        for (const auto& [instance, cloneInfo] : container)
        {
            const char* lop = "";
            switch (cloneInfo.lastOperation)
            {
                case DotsMt::create: lop = "C";
                    break;
                case DotsMt::update: lop = "U";
                    break;
                case DotsMt::remove: lop = "R";
                    break;
            }

            LOG_DATA_S("clone-info: lastOp=" << lop << ", lastUpdateFrom=" << cloneInfo.lastUpdateFrom
                << ", created=" << cloneInfo.created->toString() << ", creator=" << cloneInfo.createdFrom
                << ", modified=" << cloneInfo.modified->toString() << ", localUpdateTime=" << cloneInfo.localUpdateTime->toString());

            DotsHeader& dotsHeader = header.dotsHeader;
            dotsHeader.sentTime = *cloneInfo.modified;
            dotsHeader.serverSentTime = types::timepoint_t::Now();
            dotsHeader.attributes = instance->_validProperties();
            dotsHeader.sender = *cloneInfo.lastUpdateFrom;
			--*dotsHeader.fromCache;

            connection.transmit(header, instance);
        }
    }
}
