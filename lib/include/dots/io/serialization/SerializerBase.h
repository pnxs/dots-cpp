#pragma once
#include <stack>
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

        const data_t& output() const
        {
            return m_output;
        }

        data_t& output()
        {
            return m_output;
        }

        void setInput(const value_t* inputData, size_t inputDataSize)
        {
            m_inputData = inputData;
            m_inputDataEnd = m_inputData + inputDataSize;
            derived().setInputDerived();
        }

        void setInput(const data_t& input)
        {
            setInput(input.data(), input.size());
        }

        void setInput(data_t&& input) = delete;

        size_t inputAvailable() const
        {
            return m_inputDataEnd - m_inputData;
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        size_t serialize(const T& instance, const property_set_t& includedProperties)
        {
            visit(instance, includedProperties);
            return m_output.size() - m_outputSizeBegin;
        }

        template <typename T>
        size_t serialize(const T& value)
        {
            visit(value);
            return m_output.size() - m_outputSizeBegin;
        }

        size_t serializePackBegin()
        {
            serializePackBeginInternal();
            return static_cast<size_t>(m_inputData - m_inputDataBegin);
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        size_t serializePackElement(const T& instance, const property_set_t& includedProperties)
        {
            serializePackElementBeginInternal();
            visit(instance, includedProperties);
            serializePackElementEndInternal();

            return m_output.size() - m_outputSizeBegin;
        }

        template <typename T>
        size_t serializePackElement(const T& value)
        {
            serializePackElementBeginInternal();
            visit(value);
            serializePackElementEndInternal();

            return m_output.size() - m_outputSizeBegin;
        }

        size_t serializePackEnd()
        {
            serializePackEndInternal();
            return static_cast<size_t>(m_inputData - m_inputDataBegin);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        size_t deserialize(T& value)
        {
            visit(value);
            return static_cast<size_t>(m_inputData - m_inputDataBegin);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        T deserialize()
        {
            T value;
            deserialize(value);

            return value;
        }

        size_t deserializePackBegin()
        {
            deserializePackBeginInternal();
            return static_cast<size_t>(m_inputData - m_inputDataBegin);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        size_t deserializePackElement(T& value)
        {
            deserializePackElementBeginInternal();
            visit(value);
            deserializePackElementEndInternal();

            return static_cast<size_t>(m_inputData - m_inputDataBegin);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        T deserializePackElement()
        {
            T value;
            deserializePackElement(value);

            return value;
        }

        size_t deserializePackEnd()
        {
            deserializePackEndInternal();
            return static_cast<size_t>(m_inputData - m_inputDataBegin);
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        static data_t Serialize(const T& instance, const property_set_t& includedProperties = property_set_t::All)
        {
            Derived serializer;
            serializer.serialize(instance, includedProperties);

            return std::move(serializer.output());
        }

        template <typename T, std::enable_if_t<!std::is_base_of_v<type::Struct, T>, int> = 0>
        static data_t Serialize(const T& value)
        {
            Derived serializer;
            serializer.serialize(value);

            return std::move(serializer.output());
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        static size_t Deserialize(const value_t* data, size_t size, T& value)
        {
            Derived serializer;
            serializer.setInput(data, size);

            return serializer.deserialize(value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        static size_t Deserialize(const data_t& data, T& value)
        {
            return Deserialize(data.data(), data.size(), value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        static T Deserialize(const value_t* data, size_t size)
        {
            Derived serializer;
            serializer.setInput(data, size);

            return serializer.template deserialize<T>();
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        static T Deserialize(const data_t& data)
        {
            return Deserialize<T>(data.data(), data.size());
        }

    protected:

        using visitor_base_t = type::TypeVisitor<std::conditional_t<Static, Derived, void>>;
        using visitor_base_t::visit;

        friend visitor_base_t;

        template <bool Const>
        void visitBeginDerived()
        {
            if constexpr (Const)
            {
                m_outputSizeBegin = m_output.size();
            }
            else
            {
                if (m_inputData >= m_inputDataEnd)
                {
                    throw std::logic_error{ "attempt to deserialize from invalid or empty input buffer" };
                }

                m_inputDataBegin = m_inputData;
            }
        }

        const value_t*& inputData()
        {
            return m_inputData;
        }

        const value_t*& inputDataEnd()
        {
            return m_inputDataEnd;
        }

        void setInputDerived()
        {
            /* do nothing */
        }

        void serializePackBeginDerived()
        {
            /* do nothing */
        }

        void serializePackElementBeginDerived(size_t/* index*/)
        {
            /* do nothing */
        }

        void serializePackElementEndDerived(size_t/* index*/)
        {
            /* do nothing */
        }

        void serializePackEndDerived()
        {
            /* do nothing */
        }

        void deserializePackBeginDerived()
        {
            /* do nothing */
        }

        void deserializePackElementBeginDerived(size_t/* index*/)
        {
            /* do nothing */
        }

        void deserializePackElementEndDerived(size_t/* index*/)
        {
            /* do nothing */
        }

        void deserializePackEndDerived()
        {
            /* do nothing */
        }

    private:

        void serializePackBeginInternal()
        {
            m_outputPackIndices.emplace(0);
            derived().serializePackBeginDerived();
        }

        void serializePackElementBeginInternal()
        {
            derived().serializePackElementBeginDerived(m_outputPackIndices.top());
        }

        void serializePackElementEndInternal()
        {
            derived().serializePackElementEndDerived(m_outputPackIndices.top());
        }

        void serializePackEndInternal()
        {
            m_outputPackIndices.pop();
            derived().serializePackEndDerived();
        }

        void deserializePackBeginInternal()
        {
            m_inputPackIndices.emplace(0);
            derived().deserializePackBeginDerived();
        }

        void deserializePackElementBeginInternal()
        {
            derived().deserializePackElementBeginDerived(m_inputPackIndices.top());
        }

        void deserializePackElementEndInternal()
        {
            derived().deserializePackElementEndDerived(m_inputPackIndices.top());
        }

        void deserializePackEndInternal()
        {
            m_inputPackIndices.pop();
            derived().deserializePackEndDerived();
        }

        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }

        data_t m_output;
        size_t m_outputSizeBegin = 0;
        const value_t* m_inputData = nullptr;
        const value_t* m_inputDataBegin = nullptr;
        const value_t* m_inputDataEnd = nullptr;
        std::stack<size_t> m_outputPackIndices;
        std::stack<size_t> m_inputPackIndices;
    };
}