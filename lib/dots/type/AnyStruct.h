#pragma once
#include <memory>
#include "Struct.h"

namespace dots::type
{
	struct AnyStruct
	{
		AnyStruct(const StructDescriptor& descriptor);
		AnyStruct(const Struct& instance);
		AnyStruct(const AnyStruct& other);
		AnyStruct(AnyStruct&& other) = default;
		~AnyStruct();

		AnyStruct& operator = (const AnyStruct& rhs);
		AnyStruct& operator = (AnyStruct&& rhs) = default;
		AnyStruct& operator = (const Struct& rhs);

		Struct& operator * ();
		const Struct& operator *() const;

		Struct* operator -> ();
		const Struct* operator -> () const;

		operator Struct&();
		operator const Struct&() const;

		Struct& get();
		const Struct& get() const;

	private:

		std::unique_ptr<Struct> _instance;
	};
}