#include <dots/io/Dispatcher.h>

namespace dots
{
	Dispatcher::Dispatcher() :
		m_this(std::make_shared<Dispatcher*>(this))
	{
		/* do nothing*/
	}

	Dispatcher::Dispatcher(Dispatcher&& other) noexcept :
		m_this(std::move(other.m_this)),
		m_containerPool(std::move(other.m_containerPool)),
		m_receiveHandlerPool(std::move(other.m_receiveHandlerPool)),
		m_eventHandlerPool(std::move(other.m_eventHandlerPool))
	{
		*m_this = this;
	}

	Dispatcher& Dispatcher::operator = (Dispatcher&& rhs) noexcept
	{
		m_this = std::move(rhs.m_this);
		m_containerPool = std::move(rhs.m_containerPool);
		m_receiveHandlerPool = std::move(rhs.m_receiveHandlerPool);
		m_eventHandlerPool = std::move(rhs.m_eventHandlerPool);

		*m_this = this;

		return *this;
	}

	const ContainerPool& Dispatcher::pool() const
	{
		return m_containerPool;
	}

	ContainerPool& Dispatcher::pool()
	{
		return m_containerPool;
	}

	const Container<>& Dispatcher::container(const type::StructDescriptor<>& descriptor) const
	{
		return m_containerPool.get(descriptor);
	}

	Container<>& Dispatcher::container(const type::StructDescriptor<>& descriptor)
	{
		return m_containerPool.get(descriptor);
	}

	Subscription Dispatcher::subscribe(const type::StructDescriptor<>& descriptor, receive_handler_t<>&& handler)
	{
		Subscription subscription{ m_this, descriptor };
		m_receiveHandlerPool[&descriptor].emplace(subscription.id(), std::move(handler));

		return subscription;
	}

	Subscription Dispatcher::subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler)
	{
		Subscription subscription{ m_this, descriptor };
		const event_handler_t<>& handler_ = m_eventHandlerPool[&descriptor].emplace(subscription.id(), std::move(handler)).first->second;

		const Container<>& container = m_containerPool.get(descriptor);

		if (!container.empty())
		{
			DotsHeader header{
				DotsHeader::typeName_i{ descriptor.name() },
				DotsHeader::removeObj_i{ false },
				DotsHeader::fromCache_i{ container.size() }
			};

			for (const auto& [instance, cloneInfo] : container)
			{
				header.attributes = instance->_validProperties();
				--* header.fromCache;
				handler_(Event<>{ header, instance, instance, cloneInfo, false });
			}
		}

		return subscription;
	}

	void Dispatcher::unsubscribe(const Subscription& subscription)
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

	void Dispatcher::dispatch(const DotsHeader& header, const type::AnyStruct& instance, bool isFromMyself)
	{
		dispatchReceive(header, instance, isFromMyself);
		dispatchEvent(header, instance, isFromMyself);
	}

	void Dispatcher::dispatchReceive(const DotsHeader& header, const type::AnyStruct& instance, bool isFromMyself)
	{
		const type::StructDescriptor<>& descriptor = instance->_descriptor();

		auto itHandlers = m_receiveHandlerPool.find(&descriptor);

		if (itHandlers == m_receiveHandlerPool.end())
		{
			return;
		}

		const receive_handlers_t& handlers = itHandlers->second;

		for (const auto& [id, handler] : handlers)
		{
			(void)id;
			handler(header, instance, isFromMyself);
		}
	}

	void Dispatcher::dispatchEvent(const DotsHeader& header, const type::AnyStruct& instance, bool isFromMyself)
	{
		const type::StructDescriptor<>& descriptor = instance->_descriptor();
		const event_handlers_t& handlers = m_eventHandlerPool[&descriptor];

		auto dispatchEventToHandlers = [&](const Event<>& e)
		{
			for (const auto& [id, handler] : handlers)
			{
				(void)id;
				handler(e);
			}
		};

		if (descriptor.cached())
		{
			Container<>& container = m_containerPool.get(descriptor);

			if (header.removeObj == true)
			{
				Container<>::node_t removed = container.remove(header, instance);
				dispatchEventToHandlers(Event<>{ header, instance, removed.key(), removed.mapped(), isFromMyself });
			}
			else
			{
				const auto& [updated, cloneInfo] = container.insert(header, instance);
				dispatchEventToHandlers(Event<>{ header, instance, updated, cloneInfo, isFromMyself });
			}
		}
		else
		{
			if (header.removeObj == true)
			{
				throw std::logic_error{ "cannot remove uncached instance for type: " + descriptor.name() };
			}

			dispatchEventToHandlers(Event<>{ header, instance, instance,
				DotsCloneInformation{
					DotsCloneInformation::lastOperation_i{ DotsMt::create },
					DotsCloneInformation::createdFrom_i{ header.sender },
					DotsCloneInformation::created_i{ header.sentTime },
					DotsCloneInformation::localUpdateTime_i{ types::timepoint_t::Now() }
				},
				isFromMyself
			});
		}
	}
}