#pragma once
#include <dots/type/TypeVisitor.h>

namespace dots::serialization
{
    template <typename Format, typename Derived, bool Static = true>
    struct Serializer : type::TypeVisitor<std::conditional_t<Static, Derived, void>>
    {
        using format_t = Format;
        using reader_t = typename Format::reader_t;
        using writer_t = typename Format::writer_t;

        using data_t = typename reader_t::data_t;
        using value_t = typename data_t::value_type;

        Serializer() = default;

        Serializer(reader_t reader, writer_t writer) :
            m_serializeOffset(0),
            m_reader{ std::move(reader) },
            m_writer{ std::move(writer)}
        {
            /* do nothing */
        }

        const reader_t& reader() const
        {
            return m_reader;
        }

        reader_t& reader()
        {
            return m_reader;
        }

        const writer_t& writer() const
        {
            return m_writer;
        }

        writer_t& writer()
        {
            return m_writer;
        }

        const data_t& output() const
        {
            return m_writer.output();
        }

        data_t& output()
        {
            return m_writer.output();
        }

        void setInput(const value_t* inputData, size_t inputDataSize)
        {
            m_reader.setInput(inputData, inputDataSize);
        }

        void setInput(const data_t& input)
        {
            setInput(input.data(), input.size());
        }

        void setInput(data_t&& input) = delete;

        const value_t* inputData() const
        {
            return m_reader.inputData();
        }

        const value_t*& inputData()
        {
            return m_reader.inputData();
        }

        const value_t* inputDataBegin() const
        {
            return m_reader.inputDataBegin();
        }

        const value_t* inputDataEnd() const
        {
            return m_reader.inputDataEnd();
        }

        size_t inputOffset() const
        {
            return m_reader.inputOffset();
        }

        size_t lastSerializeSize() const
        {
            return m_writer.output().size() - m_serializeOffset;
        }

        size_t lastDeserializeSize() const
        {
            return static_cast<size_t>(m_reader.inputData() - m_reader.inputDataBegin());
        }

        size_t inputAvailable() const
        {
            return m_reader.inputDataEnd() - m_reader.inputData();
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

        template <typename T, std::enable_if_t<!std::is_const_v<T> && std::is_base_of_v<type::Struct, T>, int> = 0>
        size_t deserialize(T& instance, const property_set_t& includedProperties = property_set_t::All)
        {
            visit(instance, includedProperties);
            return lastDeserializeSize();
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        size_t deserialize(T& value, const type::Descriptor<T>& descriptor)
        {
            visit(value, descriptor);
            return lastDeserializeSize();
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_base_of_v<type::Struct, T>, int> = 0>
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

        template <bool Const>
        void visitBeginDerived()
        {
            if constexpr (Const)
            {
                m_serializeOffset = m_writer.output().size();
            }
            else
            {
                if (m_reader.inputData() >= m_reader.inputDataEnd())
                {
                    throw std::logic_error{ "attempt to deserialize from invalid or empty input buffer" };
                }

                m_reader.inputDataBegin() = m_reader.inputData();
            }
        }

    private:

        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }

        size_t m_serializeOffset;
        reader_t m_reader;
        writer_t m_writer;
    };
}
