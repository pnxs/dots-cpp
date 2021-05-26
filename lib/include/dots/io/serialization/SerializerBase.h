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
            m_inputDataBegin = nullptr;
            derived().setInputDerived();
        }

        void setInput(const data_t& input)
        {
            setInput(input.data(), input.size());
        }

        void setInput(data_t&& input) = delete;

        size_t lastSerializeSize() const
        {
            return m_output.size() - m_outputSizeBegin;
        }

        size_t lastDeserializeSize() const
        {
            return static_cast<size_t>(m_inputData - m_inputDataBegin);
        }

        size_t inputAvailable() const
        {
            return m_inputDataEnd - m_inputData;
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        size_t serialize(const T& instance, const property_set_t& includedProperties)
        {
            visit(instance, includedProperties);
            return lastSerializeSize();
        }

        template <typename T>
        size_t serialize(const T& value, const type::Descriptor<T>& descriptor)
        {
            visit(value, descriptor);
            return lastSerializeSize();
        }

        template <typename T>
        size_t serialize(const T& value)
        {
            visit(value);
            return lastSerializeSize();
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        size_t deserialize(T& value, const type::Descriptor<T>& descriptor)
        {
            visit(value, descriptor);
            return lastDeserializeSize();
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        size_t deserialize(T& value)
        {
            visit(value);
            return lastDeserializeSize();
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        T deserialize()
        {
            T value;
            deserialize(value);

            return value;
        }

        size_t serializeTupleBegin()
        {
            derived().template visitBeginDerived<true, false>();
            m_outputTupleInfo.push(true);
            derived().serializeTupleBeginDerived();
            derived().template visitEndDerived<true, false>();

            return lastSerializeSize();
        }

        size_t serializeTupleEnd()
        {
            if (m_outputTupleInfo.empty())
            {
                throw std::logic_error{ "attempt to serialize tuple end without previous begin" };
            }

            derived().template visitBeginDerived<true, false>();
            derived().serializeTupleEndDerived();
            derived().template visitEndDerived<true, false>();
            m_outputTupleInfo.pop();

            return lastSerializeSize();
        }

        size_t deserializeTupleBegin()
        {
            derived().template visitBeginDerived<false, false>();
            m_inputTupleInfo.push(true);
            derived().deserializeTupleBeginDerived();
            derived().template visitEndDerived<false, false>();

            return lastDeserializeSize();
        }

        size_t deserializeTupleEnd()
        {
            if (m_inputTupleInfo.empty())
            {
                throw std::logic_error{ "attempt to deserialize tuple end without previous begin" };
            }

            derived().template visitBeginDerived<false, false>();
            derived().deserializeTupleEndDerived();
            m_inputTupleInfo.pop();
            derived().template visitEndDerived<false, false>();

            return lastDeserializeSize();
        }

        template <typename T, typename... Args, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, Args...> && std::is_base_of_v<type::Struct, T>, int> = 0>
        static data_t Serialize(const T& instance, const property_set_t& includedProperties, Args&&... args)
        {
            Derived serializer{ std::forward<Args>(args)... };
            serializer.serialize(instance, includedProperties);

            return std::move(serializer.output());
        }

        template <typename T, typename... Args, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, Args...>, int> = 0>
        static data_t Serialize(const T& value, const type::Descriptor<T>& descriptor, Args&&... args)
        {
            Derived serializer{ std::forward<Args>(args)... };
            serializer.serialize(value, descriptor);

            return std::move(serializer.output());
        }

        template <typename T, typename... Args, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, Args...>, int> = 0>
        static data_t Serialize(const T& value, Args&&... args)
        {
            Derived serializer{ std::forward<Args>(args)... };
            serializer.serialize(value);

            return std::move(serializer.output());
        }

        template <typename T, typename... Args, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, Args...> && !std::is_const_v<T>, int> = 0>
        static size_t Deserialize(const value_t* data, size_t size, T& value, const type::Descriptor<T>& descriptor, Args&&... args)
        {
            Derived serializer{ std::forward<Args>(args)... };
            serializer.setInput(data, size);

            return serializer.deserialize(value, descriptor);
        }

        template <typename T, typename... Args, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, Args...> && !std::is_const_v<T>, int> = 0>
        static size_t Deserialize(const value_t* data, size_t size, T& value, Args&&... args)
        {
            Derived serializer{ std::forward<Args>(args)... };
            serializer.setInput(data, size);

            return serializer.deserialize(value);
        }

        template <typename T, typename... Args, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, Args...> && !std::is_const_v<T>, int> = 0>
        static size_t Deserialize(const data_t& data, T& value, const type::Descriptor<T>& descriptor, Args&&... args)
        {
            return Deserialize(data.data(), data.size(), value, descriptor, std::forward<Args>(args)...);
        }

        template <typename T, typename... Args, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, Args...> && !std::is_const_v<T>, int> = 0>
        static size_t Deserialize(const data_t& data, T& value, Args&&... args)
        {
            return Deserialize(data.data(), data.size(), value, std::forward<Args>(args)...);
        }

        template <typename T, typename... Args, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, Args...> && !std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        static T Deserialize(const value_t* data, size_t size, Args&&... args)
        {
            Derived serializer{ std::forward<Args>(args)... };
            serializer.setInput(data, size);

            return serializer.template deserialize<T>();
        }

        template <typename T, typename... Args, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, Args...> && !std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        static T Deserialize(const data_t& data, Args&&... args)
        {
            return Deserialize<T>(data.data(), data.size(), std::forward<Args>(args)...);
        }

    protected:

        using visitor_base_t = type::TypeVisitor<std::conditional_t<Static, Derived, void>>;
        using visitor_base_t::visit;

        friend visitor_base_t;

        template <bool Const, bool ConsiderTupleValue = true>
        void visitBeginDerived()
        {
            if constexpr (Const)
            {
                m_outputSizeBegin = m_output.size();

                if constexpr (ConsiderTupleValue)
                {
                    if (!m_outputTupleInfo.empty())
                    {
                        derived().serializeTupleValueBeginDerived(m_outputTupleInfo.top());
                    }
                }
            }
            else
            {
                if (m_inputData >= m_inputDataEnd)
                {
                    throw std::logic_error{ "attempt to deserialize from invalid or empty input buffer" };
                }

                m_inputDataBegin = m_inputData;

                if constexpr (ConsiderTupleValue)
                {
                    if (!m_inputTupleInfo.empty())
                    {
                        derived().deserializeTupleValueBeginDerived(m_inputTupleInfo.top());
                    }
                }
            }
        }

        template <bool Const, bool ConsiderTupleValue = true>
        void visitEndDerived()
        {
            if constexpr (Const)
            {
                if constexpr (ConsiderTupleValue)
                {
                    if (!m_outputTupleInfo.empty())
                    {
                        m_outputTupleInfo.top() = false;
                        derived().serializeTupleValueEndDerived();
                    }
                }
            }
            else
            {
                if constexpr (ConsiderTupleValue)
                {
                    if (!m_inputTupleInfo.empty())
                    {
                        m_inputTupleInfo.top() = false;
                        derived().deserializeTupleValueEndDerived();
                    }
                }
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

        void serializeTupleBeginDerived()
        {
            /* do nothing */
        }

        void serializeTupleValueBeginDerived(bool/* first*/)
        {
            /* do nothing */
        }

        void serializeTupleValueEndDerived()
        {
            /* do nothing */
        }

        void serializeTupleEndDerived()
        {
            /* do nothing */
        }

        void deserializeTupleBeginDerived()
        {
            /* do nothing */
        }

        void deserializeTupleValueBeginDerived(bool/* first*/)
        {
            /* do nothing */
        }

        void deserializeTupleValueEndDerived()
        {
            /* do nothing */
        }

        void deserializeTupleEndDerived()
        {
            /* do nothing */
        }

        void setInputDerived()
        {
            /* do nothing */
        }

    private:

        using tuple_info_t = std::stack<bool, std::vector<bool>>;

        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }

        data_t m_output;
        size_t m_outputSizeBegin = 0;
        const value_t* m_inputData = nullptr;
        const value_t* m_inputDataBegin = nullptr;
        const value_t* m_inputDataEnd = nullptr;
        tuple_info_t m_outputTupleInfo;
        tuple_info_t m_inputTupleInfo;
    };
}