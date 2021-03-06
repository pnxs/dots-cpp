#pragma once
#include <memory>
#include <dots/type/Struct.h>

namespace dots::type
{
    struct AnyStruct
    {
        AnyStruct(const StructDescriptor<>& descriptor);
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

        operator const PropertyArea&() const;
        operator PropertyArea&();

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

        Struct& get();
        const Struct& get() const;

    private:

        std::unique_ptr<Struct> _instance;
    };
}