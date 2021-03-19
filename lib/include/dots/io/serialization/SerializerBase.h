#pragma once
#include <dots/type/TypeVisitor.h>

namespace dots::io
{
    template <typename Data, typename Derived, bool Static = true>
    struct SerializerBase : type::TypeVisitor<std::conditional_t<Static, Derived, void>>
    {
        using data_t = Data;
        using value_t = typename data_t::value_type;

        SerializerBase() = default;
        SerializerBase(const SerializerBase& other) = default;
        SerializerBase(SerializerBase&& other) = default;
        ~SerializerBase() = default;

        SerializerBase& operator = (const SerializerBase& rhs) = default;
        SerializerBase& operator = (SerializerBase&& rhs) = default;

        template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        const data_t& serialize(const T& instance, const property_set_t& includedProperties = property_set_t::All)
        {
            initSerialize();
            visit(instance, includedProperties);

            return m_output;
        }

        template <typename T, std::enable_if_t<!std::is_base_of_v<type::Struct, T>, int> = 0>
        const data_t& serialize(const T& value)
        {
            initSerialize();
            visit(value);

            return m_output;
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        size_t deserialize(const value_t* data, size_t size, T& value)
        {
            initDeserialize(data, size);
            visit(value);

            return static_cast<size_t>(m_inputData - data);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        size_t deserialize(const data_t& data, T& value)
        {
            return deserialize(data.data(), data.size(), value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        T deserialize(const value_t* data, size_t size)
        {
            T value;
            deserialize(data, size, value);

            return value;
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        T deserialize(const data_t& data)
        {
            return deserialize<T>(data.data(), data.size());
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        static data_t Serialize(const T& instance, const property_set_t& includedProperties = property_set_t::All)
        {
            Derived serializer;
            return serializer.serialize(instance, includedProperties);
        }

        template <typename T, std::enable_if_t<!std::is_base_of_v<type::Struct, T>, int> = 0>
        static data_t Serialize(const T& value)
        {
            Derived serializer;
            return serializer.serialize(value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        static size_t Deserialize(const value_t* data, size_t size, T& value)
        {
            Derived serializer;
            return serializer.deserialize(data, size, value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        static size_t Deserialize(const data_t& data, T& value)
        {
            Derived serializer;
            return serializer.deserialize(data, value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        static T Deserialize(const value_t* data, size_t size)
        {
            Derived serializer;
            return serializer.template deserialize<T>(data, size);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        static T Deserialize(const data_t& data)
        {
            Derived serializer;
            return serializer.template deserialize<T>(data);
        }

    protected:

        using visitor_base_t = type::TypeVisitor<std::conditional_t<Static, Derived, void>>;
        using visitor_base_t::visit;

        data_t& output()
        {
            return m_output;
        }

        const value_t*& inputData()
        {
            return m_inputData;
        }

        const value_t*& inputDataEnd()
        {
            return m_inputDataEnd;
        }

        void initSerializeDerived()
        {
            /* do nothing */
        }

        void initDeserializeDerived()
        {
            /* do nothing */
        }

    private:

        void initSerialize()
        {
            m_output.clear();
            derived().initSerializeDerived();
        }

        void initDeserialize(const value_t* data, size_t size)
        {
            m_inputData = data;
            m_inputDataEnd = m_inputData + size;
            derived().initDeserializeDerived();
        }

        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }

        data_t m_output;
        const value_t* m_inputData = nullptr;
        const value_t* m_inputDataEnd = nullptr;
    };
}