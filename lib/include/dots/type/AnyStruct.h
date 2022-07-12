// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <memory>
#include <dots/type/Struct.h>

namespace dots::type
{
    struct AnyStruct
    {
        AnyStruct(const StructDescriptor& descriptor);

        AnyStruct(const Struct& instance) :
            AnyStruct(instance._descriptor())
        {
            *this = instance;
        }

        AnyStruct(const AnyStruct& other):
            AnyStruct(other->_descriptor())
        {
            *this = other;
        }

        AnyStruct(AnyStruct&& other) = default;

        ~AnyStruct()
        {
            if (_instance != nullptr)
            {
                _instance->_descriptor().destruct(Typeless::From(*_instance));
            }
        }

        AnyStruct& operator = (const AnyStruct& rhs)
        {
            return *this = rhs.get();
        }

        AnyStruct& operator = (AnyStruct&& rhs) = default;

        AnyStruct& operator = (const Struct& rhs)
        {
            _instance->_assign(rhs);
            return *this;
        }

        Struct& operator * ()
        {
            return get();
        }

        const Struct& operator * () const
        {
            return get();
        }

        Struct* operator -> ()
        {
            return &get();
        }

        const Struct* operator -> () const
        {
            return &get();
        }

        operator Struct&()
        {
            return get();
        }

        operator const Struct&() const
        {
            return const_cast<AnyStruct&>(*this).get();
        }

        operator const PropertyArea&() const
        {
            return *_instance;
        }

        operator PropertyArea&()
        {
            return *_instance;
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

        Struct& get()
        {
            return *_instance;
        }

        const Struct& get() const
        {
            return *_instance;
        }

    private:

        std::unique_ptr<Struct> _instance;
    };
    
    inline property_iterator begin(AnyStruct& instance)
    {
        return instance->_begin();
    }

    inline const_property_iterator begin(const AnyStruct& instance)
    {
        return instance->_begin();
    }

    inline property_iterator end(AnyStruct& instance)
    {
        return instance->_end();
    }

    inline const_property_iterator end(const AnyStruct& instance)
    {
        return instance->_end();
    }

    inline reverse_property_iterator rbegin(AnyStruct& instance)
    {
        return instance->_rbegin();
    }

    inline const_reverse_property_iterator rbegin(const AnyStruct& instance)
    {
        return instance->_rbegin();
    }

    inline reverse_property_iterator rend(AnyStruct& instance)
    {
        return instance->_rend();
    }

    inline const_reverse_property_iterator rend(const AnyStruct& instance)
    {
        return instance->_rend();
    }
}
