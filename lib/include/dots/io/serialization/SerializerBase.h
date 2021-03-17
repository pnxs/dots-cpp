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
            visit(instance, includedProperties);

            return m_output;
        }

        template <typename T>
        const data_t& serializeProperty(const T& property)
        {
            initSerialize();
            visit(property);

            return m_output;
        }

        template <typename T>
        const data_t& serializeVector(const type::Vector<T>& vector)
        {
            initSerialize();
            visit(vector, type::Descriptor<type::Vector<T>>::InstanceRef());

            return m_output;
        }

        template <typename T>
        const data_t& serialize(const T& value)
        {
            initSerialize();
            visit(value);

            return m_output;
        }

        template <typename T>
        size_t deserializeStruct(const value_t* data, size_t size, T& instance)
        {
            initDeserialize(data, size);
            visit(instance, property_set_t::None);

            return static_cast<size_t>(m_inputData - data);
        }

        template <typename T>
        size_t deserializeStruct(const data_t& data, T& instance)
        {
            return deserializeStruct(data.data(), data.size(), instance);
        }

        template <typename T>
        size_t deserializeProperty(const value_t* data, size_t size, T& property)
        {
            initDeserialize(data, size);
            visit(property);

            return static_cast<size_t>(m_inputData - data);
        }

        template <typename T>
        size_t deserializeProperty(const data_t& data, T& property)
        {
            return deserializeProperty(data.data(), data.size(), property);
        }

        template <typename T>
        size_t deserializeVector(const value_t* data, size_t size, type::Vector<T>& vector)
        {
            initDeserialize(data, size);
            visit(vector, type::Descriptor<type::Vector<T>>::InstanceRef());

            return static_cast<size_t>(m_inputData - data);
        }

        template <typename T>
        size_t deserializeVector(const data_t& data, type::Vector<T>& vector)
        {
            return deserializeVector(data.data(), data.size(), vector);
        }

        template <typename T>
        size_t deserialize(const value_t* data, size_t size, T& value)
        {
            initDeserialize(data, size);
            visit(value);

            return static_cast<size_t>(m_inputData - data);
        }

        template <typename T>
        size_t deserialize(const data_t& data, T& value)
        {
            return deserialize(data.data(), data.size(), value);
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