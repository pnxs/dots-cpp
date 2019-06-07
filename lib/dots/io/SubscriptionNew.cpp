#include <dots/io/SubscriptionNew.h>
#include <dots/io/DispatcherNew.h>

namespace dots
{
	PublishedType::PublishedType(const type::StructDescriptor* td)
	:td(td)
	{
	    LOG_DEBUG_S("PubType: " << td->name());
	}

	SubscribedType::SubscribedType(const type::StructDescriptor* td)
	:td(td)
	{
	    LOG_DEBUG_S("SubType: " << td->name());
	}

	SubscriptionNew::SubscriptionNew(std::weak_ptr<DispatcherNew*> dispatcher, const type::StructDescriptor& descriptor) :
		m_dispatcher(std::move(dispatcher)),
		m_descriptor(&descriptor),
		m_id(++M_lastId)
	{
		/* do nothing */
	}

	SubscriptionNew::SubscriptionNew(SubscriptionNew&& other) noexcept :
		m_dispatcher(std::move(other.m_dispatcher)),
		m_descriptor(other.m_descriptor),
		m_id(other.m_id)
	{
		other.m_dispatcher.reset();
		other.m_id = 0;
	}

	SubscriptionNew::~SubscriptionNew()
	{
		unsubscribe();
	}

	SubscriptionNew& SubscriptionNew::operator = (SubscriptionNew&& rhs) noexcept
	{
		m_dispatcher = std::move(rhs.m_dispatcher);
		m_descriptor = rhs.m_descriptor;
		m_id = rhs.m_id;

		rhs.m_dispatcher.reset();
		rhs.m_id = 0;

		return *this;
	}

	const type::StructDescriptor& SubscriptionNew::descriptor() const
	{
		return *m_descriptor;
	}

	auto SubscriptionNew::id() const -> id_t
	{
		return m_id;
	}

	void SubscriptionNew::unsubscribe()
	{
		if (auto dispatcher = m_dispatcher.lock())
		{
			(*dispatcher)->unsubscribe(*this);
			m_dispatcher.reset();
		}
	}

	void SubscriptionNew::discard()
	{
		m_dispatcher.reset();
	}
}
