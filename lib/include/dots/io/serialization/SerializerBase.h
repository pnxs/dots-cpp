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

        template <typename T>
        const data_t& serializeStruct(const T& instance, const property_set_t& includedProperties = property_set_t::All)
        {
            initSerialize();
            visitStruct(instance, includedProperties);

            return m_output;
        }

        template <typename T>
        const data_t& serializeProperty(const T& property)
        {
            initSerialize();
            visitProperty(property);

            return m_output;
        }

        template <typename T>
        const data_t& serializeVector(const type::Vector<T>& vector)
        {
            initSerialize();
            visitVector(vector, type::Descriptor<type::Vector<T>>::InstanceRef());

            return m_output;
        }

        template <typename T>
        const data_t& serialize(const T& value)
        {
            initSerialize();
            visitType(value);

            return m_output;
        }

        template <typename T>
        void deserializeStruct(const value_t* data, size_t size, T& instance)
        {
            initDeserialize(data, size);
            visitStruct(instance, property_set_t::None);
        }

        template <typename T>
        void deserializeStruct(const data_t& data, T& instance)
        {
            deserializeStruct(data.data(), data.size(), instance);
        }

        template <typename T>
        void deserializeProperty(const value_t* data, size_t size, T& property)
        {
            initDeserialize(data, size);
            visitProperty(property);
        }

        template <typename T>
        void deserializeProperty(const data_t& data, T& property)
        {
            deserializeProperty(data.data(), data.size(), property);
        }

        template <typename T>
        void deserializeVector(const value_t* data, size_t size, type::Vector<T>& vector)
        {
            initDeserialize(data, size);
            visitVector(vector, type::Descriptor<type::Vector<T>>::InstanceRef());
        }

        template <typename T>
        void deserializeVector(const data_t& data, type::Vector<T>& vector)
        {
            deserializeVector(data.data(), data.size(), vector);
        }

        template <typename T>
        void deserialize(const value_t* data, size_t size, T& value)
        {
            initDeserialize(data, size);
            visitType(value);
        }

        template <typename T>
        void deserialize(const data_t& data, T& value)
        {
            deserialize(data.data(), data.size(), value);
        }

        template <typename T>
        T deserialize(const value_t* data, size_t size)
        {
            T value;
            deserialize(data, size, value);

            return value;
        }

        template <typename T>
        T deserialize(const data_t& data)
        {
            return deserialize<T>(data.data(), data.size());
        }

    protected:

        using visitor_base_t = type::TypeVisitor<std::conditional_t<Static, Derived, void>>;

        using visitor_base_t::visitStruct;
        using visitor_base_t::visitProperty;
        using visitor_base_t::visitVector;
        using visitor_base_t::visitType;

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