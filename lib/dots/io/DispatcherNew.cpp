#include <dots/io/DispatcherNew.h>

namespace dots
{
	DispatcherNew::DispatcherNew() :
		m_this(std::make_shared<DispatcherNew*>(this))
	{
		/* do nothing*/
	}

	DispatcherNew::DispatcherNew(DispatcherNew&& other) noexcept :
		m_this(std::move(other.m_this)),
		m_containerPool(std::move(other.m_containerPool)),
		m_receiveHandlerPool(std::move(other.m_receiveHandlerPool)),
		m_eventHandlerPool(std::move(other.m_eventHandlerPool))
	{
		*m_this = this;
	}

	DispatcherNew& DispatcherNew::operator = (DispatcherNew&& rhs) noexcept
	{
		m_this = std::move(rhs.m_this);
		m_containerPool = std::move(rhs.m_containerPool);
		m_receiveHandlerPool = std::move(rhs.m_receiveHandlerPool);
		m_eventHandlerPool = std::move(rhs.m_eventHandlerPool);

		*m_this = this;

		return *this;
	}

	const ContainerPoolNew& DispatcherNew::pool() const
	{
		return m_containerPool;
	}

	ContainerPoolNew& DispatcherNew::pool()
	{
		return m_containerPool;
	}

	const ContainerNew<>& DispatcherNew::container(const type::StructDescriptor& descriptor) const
	{
		return m_containerPool.get(descriptor);
	}

	ContainerNew<>& DispatcherNew::container(const type::StructDescriptor& descriptor)
	{
		return m_containerPool.get(descriptor);
	}

	SubscriptionNew DispatcherNew::subscribe(const type::StructDescriptor& descriptor, receive_handler_t<>&& handler)
	{
		SubscriptionNew subscription{ m_this, descriptor };
		m_receiveHandlerPool[&descriptor].emplace(subscription.id(), std::move(handler));

		return subscription;
	}

	SubscriptionNew DispatcherNew::subscribe(const type::StructDescriptor& descriptor, event_handler_t<>&& handler)
	{
		SubscriptionNew subscription{ m_this, descriptor };
		const event_handler_t<>& handler_ = m_eventHandlerPool[&descriptor].emplace(subscription.id(), std::move(handler)).first->second;

		const ContainerNew<>& container = m_containerPool.get(descriptor);

		if (!container.empty())
		{
			DotsHeader header{
				DotsHeader::typeName_t_i{ descriptor.name() },
				DotsHeader::removeObj_t_i{ false },
				DotsHeader::fromCache_t_i{ container.size() }
			};

			for (const auto& [instance, cloneInfo] : container)
			{
				header.attributes = instance->_validProperties();
				--* header.fromCache;
				handler_(Event<>{ header, instance, instance, cloneInfo });
			}
		}

		return subscription;
	}

	void DispatcherNew::unsubscribe(const SubscriptionNew& subscription)
	{
		auto itHandlers = m_eventHandlerPool.find(&subscription.descriptor());

		if (itHandlers == m_eventHandlerPool.end())
		{
			throw std::logic_error{ "cannot unsubscribe unknown subscription for type: " + subscription.descriptor().name() };
		}

		event_handlers_t& handlers = itHandlers->second;
		auto itHandler = handlers.find(subscription.id());

		if (itHandler == handlers.end())
		{
			throw std::logic_error{ "cannot unsubscribe unknown subscription for type: " + subscription.descriptor().name() };
		}

		handlers.erase(itHandler);
	}

	void DispatcherNew::dispatch(const DotsHeader& header, const type::AnyStruct& instance)
	{
		dispatchReceive(header, instance);
		dispatchEvent(header, instance);
	}

	void DispatcherNew::dispatchReceive(const DotsHeader& header, const type::AnyStruct& instance)
	{
		const type::StructDescriptor& descriptor = instance->_descriptor();

		auto itHandlers = m_receiveHandlerPool.find(&descriptor);

		if (itHandlers == m_receiveHandlerPool.end())
		{
			return;
		}

		const receive_handlers_t& handlers = itHandlers->second;

		for (const auto& [id, handler] : handlers)
		{
			(void)id;
			handler(header, instance);
		}
	}

	void DispatcherNew::dispatchEvent(const DotsHeader& header, const type::AnyStruct& instance)
	{
		const type::StructDescriptor& descriptor = instance->_descriptor();

		auto itHandlers = m_eventHandlerPool.find(&descriptor);

		if (itHandlers == m_eventHandlerPool.end())
		{
			return;
		}

		const event_handlers_t& handlers = itHandlers->second;

		auto dispatchEvent = [&](const Event<>& e)
		{
			for (const auto& [id, handler] : handlers)
			{
				(void)id;
				handler(e);
			}
		};

		if (descriptor.cached())
		{
			ContainerNew<>& container = m_containerPool.get(descriptor);

			if (header.removeObj == true)
			{
				ContainerNew<>::node_t removed = container.remove(header, instance);
				dispatchEvent(Event<>{ header, instance, removed.key(), removed.mapped() });
			}
			else
			{
				const auto& [updated, cloneInfo] = container.insert(header, instance);
				dispatchEvent(Event<>{ header, instance, updated, cloneInfo });
			}
		}
		else
		{
			if (header.removeObj == true)
			{
				throw std::logic_error{ "cannot remove uncached instance for type: " + descriptor.name() };
			}

			dispatchEvent(Event<>{ header, instance, instance,
				DotsCloneInformation{
					DotsCloneInformation::lastOperation_t_i{ DotsMt::create },
					DotsCloneInformation::createdFrom_t_i{ header.sender },
					DotsCloneInformation::created_t_i{ header.sentTime },
					DotsCloneInformation::localUpdateTime_t_i{ pnxs::SystemNow{} }
				}
			});
		}
	}
}