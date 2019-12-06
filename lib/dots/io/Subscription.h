#pragma once
#include <functional>
#include <type_traits>
#include <atomic>
#include <memory>
#include <dots/io/Chained.h>
#include <dots/type/NewStructDescriptor.h>

namespace dots
{
	struct PublishedType : public Chained<PublishedType>
	{
		const type::StructDescriptor<>* td;
		PublishedType(const type::StructDescriptor<>* td);
	};

	struct SubscribedType : public Chained<SubscribedType>
	{
		const type::StructDescriptor<>* td;
		SubscribedType(const type::StructDescriptor<>* td);
	};

	/**
	* Helper template to create a instance of Listname-Class (S)
	* for every type T
	* @tparam T
	* @tparam S
	*/
	template<class T, class S>
	class RegisterTypeUsage
	{
	public:
		static S& get()
		{
			return m_obj;
		}
	private:
		static S m_obj;
	};

	template<class T, class S>
	S RegisterTypeUsage<T, S>::m_obj(&T::_Descriptor());

	/**
	* Registeres usage of type T in the given Chained-List
	*
	* @tparam T DOTS-Type to register
	* @tparam S Chained-List name
	*/
	template<class T, class S>
	void registerTypeUsage() {
		RegisterTypeUsage<T, S>::get();
	}

	struct Dispatcher;

	struct [[nodiscard]] Subscription
	{
		using id_t = uint64_t;

		Subscription(std::weak_ptr<Dispatcher*> dispatcher, const type::StructDescriptor<>& descriptor);
		Subscription(const Subscription& other) = delete;
		Subscription(Subscription&& other) noexcept;
		~Subscription();

		Subscription& operator = (const Subscription& rhs) = delete;
		Subscription& operator = (Subscription&& rhs) noexcept;

		const type::StructDescriptor<>& descriptor() const;
		id_t id() const;
		void unsubscribe();

		void discard();

	private:

		inline static std::atomic<id_t> M_lastId = 0;
		std::weak_ptr<Dispatcher*> m_dispatcher;
		const type::StructDescriptor<>* m_descriptor;
		id_t m_id;
	};
}