#pragma once
#include <map>
#include <functional>
#include <memory>
#include <algorithm>
#include <dots/type/AnyStruct.h>
#include <dots/io/Event.h>
#include <dots/io/SubscriptionNew.h>
#include <dots/io/ContainerPoolNew.h>

namespace dots
{
	struct DispatcherNew
	{
		template <typename T = type::Struct>
		using receive_handler_t = std::function<void(const DotsHeader&, const T&)>;
		template <typename T = type::Struct>
		using event_handler_t = std::function<void(const Event<T>&)>;

		DispatcherNew();
		DispatcherNew(const DispatcherNew& other) = delete;
		DispatcherNew(DispatcherNew&& other) noexcept;
		~DispatcherNew() = default;

		DispatcherNew& operator = (const DispatcherNew& rhs) = delete;
		DispatcherNew& operator = (DispatcherNew&& rhs) noexcept;

		const ContainerPoolNew& pool() const;
		ContainerPoolNew& pool();

		const ContainerNew<>& container(const type::StructDescriptor& descriptor) const;
		ContainerNew<>& container(const type::StructDescriptor& descriptor);

		SubscriptionNew subscribe(const type::StructDescriptor& descriptor, receive_handler_t<>&& handler);
		SubscriptionNew subscribe(const type::StructDescriptor& descriptor, event_handler_t<>&& handler);

		void unsubscribe(const SubscriptionNew& subscription);

		void dispatch(const DotsHeader& header, const type::AnyStruct& instance);

		template <typename T>
		const ContainerNew<T>& container() const
		{
			return m_containerPool.get<T>();
		}

		template <typename T>
		ContainerNew<T>& container()
		{
			return m_containerPool.get<T>();
		}

		template<typename T>
		SubscriptionNew subscribe(receive_handler_t<T>&& handler)
		{
			return subscribe(T::_Descriptor(), [_handler(std::move(handler))](const DotsHeader& header, const type::Struct& instance)
			{
				_handler(static_cast<const T&>(instance));
			});
		}	

		template<typename T>
		SubscriptionNew subscribe(event_handler_t<T>&& handler)
		{
			return subscribe(T::_Descriptor(), [_handler(std::move(handler))](const Event<>& e)
			{
				_handler(e.as<T>());
			});
		}		

	private:

		using receive_handlers_t = std::map<SubscriptionNew::id_t, receive_handler_t<>>;
		using receive_handler_pool_t = std::map<const type::StructDescriptor*, receive_handlers_t>;

		using event_handlers_t = std::map<SubscriptionNew::id_t, event_handler_t<>>;
		using event_handler_pool_t = std::map<const type::StructDescriptor*, event_handlers_t>;

		void dispatchReceive(const DotsHeader& header, const type::AnyStruct& instance);
		void dispatchEvent(const DotsHeader& header, const type::AnyStruct& instance);

		std::shared_ptr<DispatcherNew*> m_this;
		ContainerPoolNew m_containerPool;
		receive_handler_pool_t m_receiveHandlerPool;
		event_handler_pool_t m_eventHandlerPool;
	};
}