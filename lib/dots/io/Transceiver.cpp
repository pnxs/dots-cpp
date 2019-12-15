#include "Transceiver.h"
#include <set>
#include "DotsMsgConnect.dots.h"
#include <dots/common/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>
#include <DotsMember.dots.h>

namespace dots
{
	Publisher* onPublishObject = nullptr;

	Transceiver::Transceiver()
	{
		connection().onConnected.connect(FUN(*this, onConnect));
		connection().onEarlyConnect.connect(FUN(*this, onEarlySubscribe));
		connection().onReceiveMessage.connect(FUN(m_dispatcher, dispatch));

		onPublishObject = this;
	}

	bool Transceiver::start(const std::string& name, channel_ptr_t channel, descriptor_map_t preloadPublishTypes, descriptor_map_t preloadSubscribeTypes)
	{
		LOG_DEBUG_S("start transceiver");
		m_preloadPublishTypes = std::move(preloadPublishTypes);
		m_preloadSubscribeTypes = std::move(preloadSubscribeTypes);

		// start communication
		if (connection().start(name, channel))
		{
			 // publish types
			return true;
		}

		return false;
	}

	void Transceiver::stop()
	{
		// stop communication
		connection().stop();
	}

	const io::Registry& Transceiver::registry() const
	{
		return m_registry;
	}

	io::Registry& Transceiver::registry()
	{
		return m_registry;
	}

	const ContainerPool& Transceiver::pool() const
	{
		return m_dispatcher.pool();
	}

	const Container<>& Transceiver::container(const type::StructDescriptor<>& descriptor)
	{
		return m_dispatcher.container(descriptor);
	}

	Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, receive_handler_t<>&& handler)
	{
		if (descriptor.substructOnly())
		{
			throw std::logic_error{ "attempt to subscribe to substruct-only type" };
		}

		joinGroup(descriptor.name());
		return m_dispatcher.subscribe(descriptor, std::move(handler));
	}

	Subscription Transceiver::subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler)
	{
		if (descriptor.substructOnly())
		{
			throw std::logic_error{ "attempt to subscribe to substruct-only type" };
		}

		joinGroup(descriptor.name());
		return m_dispatcher.subscribe(descriptor, std::move(handler));
	}

	Subscription Transceiver::subscribe(const std::string_view& name, receive_handler_t<>&& handler)
	{
		return subscribe(m_registry.getStructType(name), std::move(handler));
	}

	Subscription Transceiver::subscribe(const std::string_view& name, event_handler_t<>&& handler)
	{
		return subscribe(m_registry.getStructType(name), std::move(handler));
	}

	ServerConnection& Transceiver::connection()
	{
		return m_serverConnection;
	}

	void Transceiver::publish(const type::Struct& instance, types::property_set_t includedProperties/*t = types::property_set_t::All*/, bool remove/* = false*/)
	{
		publish(instance._descriptor(), includedProperties, remove);
	}
	
	void Transceiver::publish(const type::StructDescriptor<>* td, const type::Struct& instance, types::property_set_t what, bool remove)
	{
		if (td->substructOnly())
		{
			throw std::logic_error{ "attempt to publish substruct-only type" };
		}

		const type::StructDescriptor<>& descriptor = instance._descriptor();
		
		if (remove)
	    {
	        what ^= descriptor.keyProperties();
	    }

	    if (!(what <= descriptor.keyProperties()))
	    {
	        throw std::runtime_error("tried to publish instance with invalid key (not all key-fields are set) what=" + what.toString() + " tdkeys=" + descriptor.keyProperties().toString());
	    }
		
		DotsTransportHeader header{
            DotsTransportHeader::destinationGroup_i{ descriptor.name() },
            DotsTransportHeader::dotsHeader_i{
                DotsHeader::typeName_i{ descriptor.name() },
                DotsHeader::sentTime_i{ pnxs::SystemNow() },
                DotsHeader::attributes_i{ what ==  types::property_set_t::All ? instance._validProperties() : what },
                DotsHeader::removeObj_i{ remove },
            }
        };
		
		if (descriptor.internal())
		{
			header.nameSpace("SYS");
			LOG_DEBUG_S("publish ns=" << *header.nameSpace << " type=" << descriptor.name());
		}
		
		LOG_DATA_S("data:" << to_ascii(&descriptor, &instance, what));
		m_serverConnection.channel().transmit(header, instance);
	}

	void Transceiver::onConnect()
	{
		m_connected = true;
	}

	bool Transceiver::connected() const
	{
		return m_connected;
	}

	void Transceiver::joinGroup(const std::string_view& name)
	{
		LOG_DEBUG_S("send DotsMember (join " << name << ")");
		publish(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::join }
        });
	}

	void Transceiver::leaveGroup(const std::string_view& name)
	{
		LOG_INFO_S("send DotsMember (leave " << name << ")");
		publish(DotsMember{
            DotsMember::groupName_i{ name },
            DotsMember::event_i{ DotsMemberEvent::leave }
        });
	}

	void Transceiver::onEarlySubscribe()
	{
		TD_Traversal traversal;

		for (const auto& [name, descriptor] : m_preloadPublishTypes)
		{
			if (!descriptor->internal())
			{
				traversal.traverseDescriptorData(descriptor, [this](auto td, auto body) 
				{
					publish(*reinterpret_cast<const type::Struct*>(body), td->validProperties(body), false);
				});
			}
		}

		for (const auto& [name, descriptor] : m_preloadSubscribeTypes)
		{
			(void)name;
			
			if (!descriptor->internal())
			{
				traversal.traverseDescriptorData(descriptor, [this](auto td, auto body) 
				{
					publish(*reinterpret_cast<const type::Struct*>(body), td->validProperties(body), false);
				});
			}
			
			joinGroup(descriptor->name());
		}

		publish(DotsMsgConnect{
			DotsMsgConnect::preloadClientFinished_i{ true }
		});
	}
}