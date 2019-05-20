#include "AnyStruct.h"

namespace dots::type
{
	AnyStruct::AnyStruct(const StructDescriptor& descriptor):
		_instance{ reinterpret_cast<Struct*>(::operator new(descriptor.sizeOf())) }
	{
		descriptor.construct(_instance.get());
	}

	AnyStruct::AnyStruct(const Struct& instance) :
		AnyStruct(instance._descriptor())
	{
		*this = instance;
	}

	AnyStruct::AnyStruct(const AnyStruct& other):
		AnyStruct(other->_descriptor())
	{
		*this = other;
	}

	AnyStruct::~AnyStruct()
	{
		if (_instance != nullptr)
		{
			_instance->_descriptor().destruct(_instance.get());
		}		
	}

	AnyStruct& AnyStruct::operator = (const AnyStruct& rhs)
	{
		return *this = rhs.get();
	}

	AnyStruct& AnyStruct::operator = (const Struct& rhs)
	{
		_instance->_descriptor().copy(_instance.get(), &rhs);
		return *this;
	}

	Struct& AnyStruct::operator * ()
	{
		return get();
	}

	const Struct& AnyStruct::operator * () const
	{
		return get();
	}

	Struct* AnyStruct::operator -> ()
	{
		return &get();
	}

	const Struct* AnyStruct::operator -> () const
	{
		return &get();
	}

	AnyStruct::operator Struct&()
	{
		return get();
	}

	AnyStruct::operator const Struct&() const
	{
		return const_cast<AnyStruct&>(*this).get();
	}

	Struct& AnyStruct::get()
	{
		return *_instance;
	}

	const Struct& AnyStruct::get() const
	{
		return *_instance;
	}
}