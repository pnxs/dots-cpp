#include <dots/io/Subscription.h>
#include <dots/io/Dispatcher.h>
#include <dots/common/logging.h>

namespace dots
{
	PublishedType::PublishedType(const type::NewStructDescriptor<>* td)
	:td(td)
	{
	    LOG_DEBUG_S("PubType: " << td->name());
	}

	SubscribedType::SubscribedType(const type::NewStructDescriptor<>* td)
	:td(td)
	{
	    LOG_DEBUG_S("SubType: " << td->name());
	}

	Subscription::Subscription(std::weak_ptr<Dispatcher*> dispatcher, const type::NewStructDescriptor<>& descriptor) :
		m_dispatcher(std::move(dispatcher)),
		m_descriptor(&descriptor),
		m_id(++M_lastId)
	{
		/* do nothing */
	}

	Subscription::Subscription(Subscription&& other) noexcept :
		m_dispatcher(std::move(other.m_dispatcher)),
		m_descriptor(other.m_descriptor),
		m_id(other.m_id)
	{
		other.m_dispatcher.reset();
		other.m_id = 0;
	}

	Subscription::~Subscription()
	{
		unsubscribe();
	}

	Subscription& Subscription::operator = (Subscription&& rhs) noexcept
	{
		m_dispatcher = std::move(rhs.m_dispatcher);
		m_descriptor = rhs.m_descriptor;
		m_id = rhs.m_id;

		rhs.m_dispatcher.reset();
		rhs.m_id = 0;

		return *this;
	}

	const type::NewStructDescriptor<>& Subscription::descriptor() const
	{
		return *m_descriptor;
	}

	auto Subscription::id() const -> id_t
	{
		return m_id;
	}

	void Subscription::unsubscribe()
	{
		if (auto dispatcher = m_dispatcher.lock())
		{
			(*dispatcher)->unsubscribe(*this);
			m_dispatcher.reset();
		}
	}

	void Subscription::discard()
	{
		m_dispatcher.reset();
	}
}
