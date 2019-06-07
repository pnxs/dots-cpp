#pragma once
#include <functional>
#include <type_traits>
#include <atomic>
#include <memory>
#include <dots/io/Chained.h>
#include <dots/type/StructDescriptor.h>

namespace dots
{
	struct PublishedType : public Chained<PublishedType>
	{
		const type::StructDescriptor* td;
		PublishedType(const type::StructDescriptor* td);
	};

	struct SubscribedType : public Chained<SubscribedType>
	{
		const type::StructDescriptor* td;
		SubscribedType(const type::StructDescriptor* td);
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

	struct DispatcherNew;

	struct [[nodiscard]] SubscriptionNew
	{
		using id_t = uint64_t;

		SubscriptionNew(std::weak_ptr<DispatcherNew*> dispatcher, const type::StructDescriptor& descriptor);
		SubscriptionNew(const SubscriptionNew& other) = delete;
		SubscriptionNew(SubscriptionNew&& other) noexcept;
		~SubscriptionNew();

		SubscriptionNew& operator = (const SubscriptionNew& rhs) = delete;
		SubscriptionNew& operator = (SubscriptionNew&& rhs) noexcept;

		const type::StructDescriptor& descriptor() const;
		id_t id() const;
		void unsubscribe();

		void discard();

	private:

		inline static std::atomic<id_t> M_lastId = 0;
		std::weak_ptr<DispatcherNew*> m_dispatcher;
		const type::StructDescriptor* m_descriptor;
		id_t m_id;
	};
}