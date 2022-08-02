// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/type/AnyStruct.h>

namespace dots::type
{
    AnyStruct::AnyStruct(const StructDescriptor& descriptor):
        _instance{ static_cast<Struct*>(::operator new(descriptor.size())) }
    {
        descriptor.constructInPlace(Typeless::From(*_instance));
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
            _instance->_descriptor().destruct(Typeless::From(*_instance));
        }
    }

    AnyStruct& AnyStruct::operator = (const AnyStruct& rhs)
    {
        return *this = rhs.get();
    }

    AnyStruct& AnyStruct::operator = (const Struct& rhs)
    {
        _instance->_assign(rhs);
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

    AnyStruct::operator const PropertyArea&() const
    {
        return *_instance;
    }

    AnyStruct::operator PropertyArea&()
    {
        return *_instance;
    }

    Struct& AnyStruct::get()
    {
        return *_instance;
    }

    const Struct& AnyStruct::get() const
    {
        return *_instance;
    }

    property_iterator begin(AnyStruct& instance)
    {
        return instance->_begin();
    }

    const_property_iterator begin(const AnyStruct& instance)
    {
        return instance->_begin();
    }

    property_iterator end(AnyStruct& instance)
    {
        return instance->_end();
    }

    const_property_iterator end(const AnyStruct& instance)
    {
        return instance->_end();
    }

    reverse_property_iterator rbegin(AnyStruct& instance)
    {
        return instance->_rbegin();
    }

    const_reverse_property_iterator rbegin(const AnyStruct& instance)
    {
        return instance->_rbegin();
    }

    reverse_property_iterator rend(AnyStruct& instance)
    {
        return instance->_rend();
    }

    const_reverse_property_iterator rend(const AnyStruct& instance)
    {
        return instance->_rend();
    }
}
