#pragma once
#include <map>
#include <unordered_map>
#include <functional>
#include <memory>
#include <algorithm>
#include <dots/type/AnyStruct.h>
#include <dots/io/Event.h>
#include <dots/io/Subscription.h>
#include <dots/io/ContainerPool.h>

namespace dots
{
	struct Dispatcher
	{
		template <typename T = type::Struct>
		using receive_handler_t = std::function<void(const DotsHeader&, const T&, bool)>;
		template <typename T = type::Struct>
		using event_handler_t = std::function<void(const Event<T>&)>;

		Dispatcher();
		Dispatcher(const Dispatcher& other) = delete;
		Dispatcher(Dispatcher&& other) noexcept;
		~Dispatcher() = default;

		Dispatcher& operator = (const Dispatcher& rhs) = delete;
		Dispatcher& operator = (Dispatcher&& rhs) noexcept;

		const ContainerPool& pool() const;
		ContainerPool& pool();

		const Container<>& container(const type::StructDescriptor<>& descriptor) const;
		Container<>& container(const type::StructDescriptor<>& descriptor);

		Subscription subscribe(const type::StructDescriptor<>& descriptor, receive_handler_t<>&& handler);
		Subscription subscribe(const type::StructDescriptor<>& descriptor, event_handler_t<>&& handler);

		void unsubscribe(const Subscription& subscription);

		void dispatch(const DotsHeader& header, const type::AnyStruct& instance, bool isFromMyself);

		template <typename T>
		const Container<T>& container() const
		{
			return m_containerPool.get<T>();
		}

		template <typename T>
		Container<T>& container()
		{
			return m_containerPool.get<T>();
		}

		template<typename T>
		Subscription subscribe(receive_handler_t<T>&& handler)
		{
			return subscribe(T::_Descriptor(), [_handler(std::move(handler))](const DotsHeader& header, const type::Struct& instance, bool isFromMyself)
			{
				_handler(header, static_cast<const T&>(instance), isFromMyself);
			});
		}

		template<typename T>
		Subscription subscribe(event_handler_t<T>&& handler)
		{
			return subscribe(T::_Descriptor(), [_handler(std::move(handler))](const Event<>& e)
			{
				_handler(e.as<T>());
			});
		}		

	private:

		using receive_handlers_t = std::map<Subscription::id_t, receive_handler_t<>>;
		using receive_handler_pool_t = std::unordered_map<const type::StructDescriptor<>*, receive_handlers_t>;

		using event_handlers_t = std::map<Subscription::id_t, event_handler_t<>>;
		using event_handler_pool_t = std::unordered_map<const type::StructDescriptor<>*, event_handlers_t>;

		void dispatchReceive(const DotsHeader& header, const type::AnyStruct& instance, bool isFromMyself);
		void dispatchEvent(const DotsHeader& header, const type::AnyStruct& instance, bool isFromMyself);

		std::shared_ptr<Dispatcher*> m_this;
		ContainerPool m_containerPool;
		receive_handler_pool_t m_receiveHandlerPool;
		event_handler_pool_t m_eventHandlerPool;
	};
}