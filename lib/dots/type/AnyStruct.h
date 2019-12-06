#pragma once
#include <memory>
#include <dots/type/NewStruct.h>

namespace dots::type
{
	struct AnyStruct
	{
		AnyStruct(const NewStructDescriptor<>& descriptor);
		AnyStruct(const NewStruct& instance);
		AnyStruct(const AnyStruct& other);
		AnyStruct(AnyStruct&& other) = default;
		~AnyStruct();

		AnyStruct& operator = (const AnyStruct& rhs);
		AnyStruct& operator = (AnyStruct&& rhs) = default;
		AnyStruct& operator = (const NewStruct& rhs);

		NewStruct& operator * ();
		const NewStruct& operator *() const;

		NewStruct* operator -> ();
		const NewStruct* operator -> () const;

		operator NewStruct&();
		operator const NewStruct&() const;

		template <typename T, std::enable_if_t<std::is_base_of_v<NewStruct, T>, int> = 0>
		explicit operator const T&() const
		{
			return _instance->_to<T>();
		}

		template <typename T, std::enable_if_t<std::is_base_of_v<NewStruct, T>, int> = 0>
		explicit operator T&()
		{
			return _instance->_to<T>();
		}

		template <typename T>
        bool is() const
        {
            return _instance->_is<T>();
        }

        template <typename T>
        const T* as() const
        {
            return _instance->_as<T>();
        }

        template <typename T>
        T* as()
        {
            return _instance->_as<T>();
        }

		template <typename T, bool Safe = false>
        const T& to() const
        {
			return _instance->_to<T, Safe>();
        }

        template <typename T, bool Safe = false>
        T& to()
        {
            return _instance->_to<T, Safe>();
        }

		NewStruct& get();
		const NewStruct& get() const;

	private:

		std::unique_ptr<NewStruct> _instance;
	};
}