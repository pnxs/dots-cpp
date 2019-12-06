#include "AnyStruct.h"

namespace dots::type
{
	AnyStruct::AnyStruct(const NewStructDescriptor<>& descriptor):
		_instance{ reinterpret_cast<NewStruct*>(::operator new(descriptor.size())) }
	{
		descriptor.construct(NewTypeless::From(*_instance));
	}

	AnyStruct::AnyStruct(const NewStruct& instance) :
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
			_instance->_descriptor().destruct(NewTypeless::From(*_instance));
		}		
	}

	AnyStruct& AnyStruct::operator = (const AnyStruct& rhs)
	{
		return *this = rhs.get();
	}

	AnyStruct& AnyStruct::operator = (const NewStruct& rhs)
	{
		_instance->_assign(rhs);
		return *this;
	}

	NewStruct& AnyStruct::operator * ()
	{
		return get();
	}

	const NewStruct& AnyStruct::operator * () const
	{
		return get();
	}

	NewStruct* AnyStruct::operator -> ()
	{
		return &get();
	}

	const NewStruct* AnyStruct::operator -> () const
	{
		return &get();
	}

	AnyStruct::operator NewStruct&()
	{
		return get();
	}

	AnyStruct::operator const NewStruct&() const
	{
		return const_cast<AnyStruct&>(*this).get();
	}

	NewStruct& AnyStruct::get()
	{
		return *_instance;
	}

	const NewStruct& AnyStruct::get() const
	{
		return *_instance;
	}
}