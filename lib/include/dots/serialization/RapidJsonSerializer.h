#pragma once
#include <string>
#include <dots/serialization/Serializer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <dots/serialization/formats/RapidJsonReader.h>
#include <dots/serialization/formats/RapidJsonWriter.h>

namespace dots::serialization
{
    template <typename UnderlyingWriter = rapidjson::Writer<rapidjson::StringBuffer>>
    struct RapidJsonSerializerFormat
    {
        using reader_t = RapidJsonReader;
        using writer_t = RapidJsonWriter<UnderlyingWriter>;

        enum struct TimepointFormat : uint8_t
        {
            FractionalSeconds,
            ISO8601String
        };

        enum struct EnumFormat : uint8_t
        {
            Tag,
            Value,
            String
        };
        
        static constexpr TimepointFormat TimepointFormat = TimepointFormat::FractionalSeconds;
        static constexpr EnumFormat EnumFormat = EnumFormat::Tag;
    };

    template <typename Format = RapidJsonSerializerFormat<>>
    struct RapidJsonSerializer : type::TypeVisitor<RapidJsonSerializer<Format>>
    {
        using data_t = std::string;
        using value_t = data_t::value_type;

        using format_t = Format;
        using reader_t = typename format_t::reader_t;
        using writer_t = typename format_t::writer_t;

        using document_t = rapidjson::Document;
        using underlying_writer_t = typename writer_t::underlying_writer_t;

        RapidJsonSerializer() = default;

        RapidJsonSerializer(underlying_writer_t& underlyingWriter, const document_t::ValueType& inputValue) :
            m_reader{ inputValue },
            m_writer{ underlyingWriter }
        {
            /* do nothing */
        }

        RapidJsonSerializer(underlying_writer_t& underlyingWriter, document_t document) :
            m_reader{ std::move(document) },
            m_writer{ underlyingWriter }
        {
            /* do nothing */
        }

        RapidJsonSerializer(underlying_writer_t& underlyingWriter, std::string_view input) :
            m_reader{ input },
            m_writer{ underlyingWriter }
        {
            /* do nothing */
        }

        RapidJsonSerializer(underlying_writer_t& underlyingWriter) :
            m_writer{ underlyingWriter }
        {
            /* do nothing */
        }

        RapidJsonSerializer(document_t document) :
            m_reader{ std::move(document) }
        {
            /* do nothing */
        }

        RapidJsonSerializer(std::string_view input) :
            m_reader{ input }
        {
            /* do nothing */
        }

        RapidJsonSerializer(const RapidJsonSerializer& other) = default;
        RapidJsonSerializer(RapidJsonSerializer&& other) = default;
        ~RapidJsonSerializer() = default;

        RapidJsonSerializer& operator = (const RapidJsonSerializer& rhs) = default;
        RapidJsonSerializer& operator = (RapidJsonSerializer&& rhs) = default;

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

        void setUnderlyingWriter(underlying_writer_t& underlyingWriter)
        {
            m_writer = writer_t{ &underlyingWriter };
        }

        void setInput(const document_t::ValueType& inputValue)
        {
            m_reader.setInput(inputValue);
        }

        void setInput(document_t document)
        {
            m_reader.setInput(std::move(document));
        }

        void setInput(std::string_view input)
        {
            m_reader.setInput(input);
        }

        bool hasInput() const
        {
            return m_reader.hasInput();
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        void serialize(const T& instance, const property_set_t& includedProperties)
        {
            visit(instance, includedProperties);
        }

        template <typename T>
        void serialize(const T& value, const type::Descriptor<T>& descriptor)
        {
            visit(value, descriptor);
        }

        template <typename T>
        void serialize(const T& value)
        {
            visit(value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        void deserialize(T& value, const type::Descriptor<T>& descriptor)
        {
            visit(value, descriptor);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        void deserialize(T& value)
        {
            visit(value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        T deserialize()
        {
            T value;
            deserialize(value);

            return value;
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        static data_t Serialize(const T& instance, const property_set_t& includedProperties)
        {
            rapidjson::StringBuffer buffer;
            underlying_writer_t writer{ buffer };
            RapidJsonSerializer serializer{ writer };
            serializer.serialize(instance, includedProperties);

            return buffer.GetString();
        }

        template <typename T>
        static data_t Serialize(const T& value, const type::Descriptor<T>& descriptor)
        {
            rapidjson::StringBuffer buffer;
            underlying_writer_t writer{ buffer };
            RapidJsonSerializer serializer{ writer };
            serializer.serialize(value, descriptor);

            return buffer.GetString();
        }

        template <typename T>
        static data_t Serialize(const T& value)
        {
            rapidjson::StringBuffer buffer;
            underlying_writer_t writer{ buffer };
            RapidJsonSerializer serializer{ writer };
            serializer.serialize(value);

            return buffer.GetString();
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        static void Deserialize(const value_t* data, size_t size, T& value, const type::Descriptor<T>& descriptor)
        {
            RapidJsonSerializer serializer{ std::string_view{ data, size } };
            serializer.deserialize(value, descriptor);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        static void Deserialize(const value_t* data, size_t size, T& value)
        {
            RapidJsonSerializer serializer{ std::string_view{ data, size } };
            serializer.deserialize(value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        static void Deserialize(const data_t& data, T& value, const type::Descriptor<T>& descriptor)
        {
            return Deserialize(data.data(), data.size(), value, descriptor);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
        static void Deserialize(const data_t& data, T& value)
        {
            return Deserialize(data.data(), data.size(), value);
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        static T Deserialize(const value_t* data, size_t size)
        {
            RapidJsonSerializer serializer{ std::string_view{ data, size } };
            return serializer.deserialize<T>();
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        static T Deserialize(const data_t& data)
        {
            return Deserialize<T>(data.data(), data.size());
        }

    protected:

        using visitor_base_t = type::TypeVisitor<RapidJsonSerializer<Format>>;
        using visitor_base_t::visit;

        friend visitor_base_t;

        template <bool Const>
        void visitBeginDerived()
        {
            if constexpr (Const)
            {
                m_writer.assertHasWriter();
            }
            else
            {
                m_reader.assertHasInputValue();
            }
        }

        //
        // serialization
        //

        template <typename T>
        bool visitStructBeginDerived(const T&/* instance*/, property_set_t&/* includedProperties*/)
        {
            m_writer.writeObjectBegin();
            return true;
        }

        template <typename T>
        void visitStructEndDerived(const T&/* instance*/, const property_set_t&/* includedProperties*/)
        {
            m_writer.writeObjectEnd();
        }

        template <typename T>
        bool visitPropertyBeginDerived(const T& property, bool/* first*/)
        {
            if (m_writer.nestingLevel() > 0)
            {
                m_writer.writeObjectMemberName(property.descriptor().name());
            }

            if (property.isValid())
            {
                return true;
            }
            else
            {
                m_writer.writeNull();
                return false;
            }
        }

        template <typename T>
        bool visitVectorBeginDerived(const vector_t<T>&/* vector*/, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            m_writer.writeArrayBegin();
            return true;
        }

        template <typename T>
        void visitVectorEndDerived(const vector_t<T>&/* vector*/, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            m_writer.writeArrayEnd();
        }

        template <typename T>
        void visitEnumDerived(const T& value, const type::EnumDescriptor<T>& descriptor)
        {
            if constexpr (format_t::EnumFormat == format_t::EnumFormat::Tag)
            {
                m_writer.write(descriptor.enumeratorFromValue(value).tag());
            }
            else if constexpr (format_t::EnumFormat == format_t::EnumFormat::Value)
            {
                m_writer.write(type::Typeless::From(value).template to<int32_t>());
            }
            else if constexpr (format_t::EnumFormat == format_t::EnumFormat::String)
            {
                m_writer.write(descriptor.enumeratorFromValue(value).name());
            }
            else
            {
                static_assert(tools::always_false_v<T>, "unsupported enum format");
            }
        }

        template <typename T>
        void visitFundamentalTypeDerived(const T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                m_writer.write(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                m_writer.write(value.toValue());
            }
            else if constexpr (std::is_same_v<T, timepoint_t>)
            {
                if constexpr (format_t::TimepointFormat == format_t::TimepointFormat::FractionalSeconds)
                {
                    m_writer.write(value.duration().toFractionalSeconds());
                }
                else if constexpr (format_t::TimepointFormat == format_t::TimepointFormat::ISO8601String)
                {
                    m_writer.write(value.toString());
                }
                else
                {
                    static_assert(tools::always_false_v<T>, "unsupported timepoint format");
                }
            }
            else if constexpr (std::is_same_v<T, steady_timepoint_t>)
            {
                m_writer.write(value.duration().toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, duration_t>)
            {
                m_writer.write(value.toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                m_writer.write(value.toString());
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                m_writer.write(value);
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }

        //
        // deserialization
        //

        template <typename T>
        bool visitStructBeginDerived(T& instance, property_set_t&/* includedProperties*/)
        {
            const type::StructDescriptor<>& descriptor = instance._descriptor();
            const type::property_descriptor_container_t& propertyDescriptors = descriptor.propertyDescriptors();

            m_reader.readObjectBegin();

            while (!m_reader.tryReadObjectEnd())
            {
                std::string_view propertyName = m_reader.readObjectMemberName();

                if (auto it = std::find_if(propertyDescriptors.begin(), propertyDescriptors.end(), [propertyName](const auto& p) { return p.name() == propertyName; }); it != propertyDescriptors.end())
                {
                    const type::PropertyDescriptor& propertyDescriptor = *it;
                    type::ProxyProperty<> property{ instance, propertyDescriptor };
                    visit(property);
                }
                else
                {
                    m_reader.skip();
                }
            }

            return false;
        }

        template <typename T>
        bool visitPropertyBeginDerived(T& property, bool/* first*/)
        {
            if (m_reader.tryReadNull())
            {
                return false;
            }
            else
            {
                property.constructOrValue();
                return true;
            }
        }

        template <typename T>
        bool visitVectorBeginDerived(vector_t<T>& vector, const type::Descriptor<vector_t<T>>& descriptor)
        {
            m_reader.readArrayBegin();

            while (!m_reader.tryReadArrayEnd())
            {
                descriptor.resize(vector, vector.typelessSize() + 1);

                // TODO: avoid special case for bool
                if constexpr (std::is_same_v<T, bool_t>)
                {
                    visit(reinterpret_cast<bool_t&>(vector.back()), descriptor.valueDescriptor());
                }
                else
                {
                    if constexpr (std::is_same_v<T, type::Typeless>)
                    {
                        visit(vector.typelessAt(vector.typelessSize() - 1), descriptor.valueDescriptor());
                    }
                    else
                    {
                        visit(vector.back(), descriptor.valueDescriptor());
                    }
                }
            }

            return false;
        }

        template <typename T>
        void visitEnumDerived(T& value, const type::EnumDescriptor<T>& descriptor)
        {
            if constexpr (format_t::EnumFormat == format_t::EnumFormat::Tag)
            {
                descriptor.construct(value, descriptor.enumeratorFromTag(reader().template read<uint32_t>()).value());
            }
            else if constexpr (format_t::EnumFormat == format_t::EnumFormat::Value)
            {
                auto underlyingValue = m_reader.template read<uint32_t>();
                descriptor.construct(value, descriptor.enumeratorFromValue(type::Typeless::From(underlyingValue)).value());
            }
            else if constexpr (format_t::EnumFormat == format_t::EnumFormat::String)
            {
                std::string_view identifier = m_reader.template read<std::string_view>();
                descriptor.construct(value, descriptor.enumeratorFromName(identifier).value());
            }
            else
            {
                static_assert(tools::always_false_v<T>, "unsupported enum format");
            }
        }

        template <typename T>
        void visitFundamentalTypeDerived(T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                value = reader().template read<T>();
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                value = property_set_t{ reader().template read<uint32_t>() };
            }
            else if constexpr (std::is_same_v<T, timepoint_t>)
            {
                if constexpr (format_t::TimepointFormat == format_t::TimepointFormat::FractionalSeconds)
                {
                    value = T{ duration_t{ reader().template read<float64_t>() } };
                }
                else if constexpr (format_t::TimepointFormat == format_t::TimepointFormat::ISO8601String)
                {
                    value = timepoint_t::FromString(m_reader.template read<string_t>());
                }
                else
                {
                    static_assert(tools::always_false_v<T>, "unsupported timepoint format");
                }
            }
            else if constexpr (std::is_same_v<T, steady_timepoint_t> || std::is_same_v<T, duration_t>)
            {
                value = T{ duration_t{ reader().template read<float64_t>() } };
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                value = uuid_t::FromString(m_reader.template read<string_t>());
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                value = m_reader.template read<string_t>();
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }

    private:

        reader_t m_reader;
        writer_t m_writer;
    };
}