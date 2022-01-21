#pragma once
#include <string>
#include <dots/serialization/SerializerBase.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

namespace dots::serialization
{
    struct RapidJsonSerializerTraits
    {
        static constexpr bool NumericTimePoints = true;
        static constexpr bool NumericEnums = true;
    };

    template <typename Writer = rapidjson::Writer<rapidjson::StringBuffer>, typename Traits = RapidJsonSerializerTraits>
    struct RapidJsonSerializer : type::TypeVisitor<RapidJsonSerializer<Writer, Traits>>
    {
        using data_t = std::string;
        using value_t = data_t::value_type;

        using writer_t = Writer;
        using document_t = rapidjson::Document;
        using traits_t = Traits;

        RapidJsonSerializer() = default;

        RapidJsonSerializer(Writer& writer, const document_t::ValueType& inputValue) :
            RapidJsonSerializer(&writer, &inputValue)
        {
            /* do nothing */
        }

        RapidJsonSerializer(Writer& writer, document_t document) :
            RapidJsonSerializer(&writer, std::move(document))
        {
            /* do nothing */
        }

        RapidJsonSerializer(Writer& writer, std::string_view input) :
            RapidJsonSerializer(&writer, input)
        {
            /* do nothing */
        }

        RapidJsonSerializer(Writer& writer) :
            RapidJsonSerializer(&writer, std::nullopt)
        {
            /* do nothing */
        }

        RapidJsonSerializer(document_t document) :
            RapidJsonSerializer(nullptr, std::move(document))
        {
            /* do nothing */
        }

        RapidJsonSerializer(std::string_view input) :
            RapidJsonSerializer(nullptr, input)
        {
            /* do nothing */
        }

        RapidJsonSerializer(const RapidJsonSerializer& other) = default;
        RapidJsonSerializer(RapidJsonSerializer&& other) = default;
        ~RapidJsonSerializer() = default;

        RapidJsonSerializer& operator = (const RapidJsonSerializer& rhs) = default;
        RapidJsonSerializer& operator = (RapidJsonSerializer&& rhs) = default;

        void setWriter(Writer& writer)
        {
            m_writer = &writer;
        }

        void setInputValue(const document_t::ValueType& inputValue)
        {
            m_inputValue = &inputValue;
            m_tupleEnd = nullptr;
        }

        void setInputValue(document_t document)
        {
            m_document.emplace(std::move(document));

            if (m_document->HasParseError())
            {
                throw std::runtime_error("RapidJSON parse error at '" + std::to_string(m_document->GetErrorOffset()) + "' -> " + parseErrorToString(m_document->GetParseError()));
            }

            setInputValue(static_cast<const document_t::ValueType&>(*m_document));
        }

        void setInputValue(std::string_view input)
        {
            document_t document;
            document.Parse(input.data(), input.size());
            setInputValue(std::move(document));
        }

        bool hasInputValue() const
        {
            return m_inputValue != nullptr && (m_tupleEnd == nullptr || m_inputValue != m_tupleEnd);
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

            if (m_tupleEnd != nullptr)
            {
                ++m_inputValue;
            }
        }

        template <typename T, std::enable_if_t<!std::is_const_v<T> && !std::is_reference_v<T>, int> = 0>
        T deserialize()
        {
            T value;
            deserialize(value);

            return value;
        }

        void serializeTupleBegin()
        {
            assertHasWriter();
            m_writer->StartArray();
        }

        void serializeTupleEnd()
        {
            assertHasWriter();
            m_writer->EndArray();
        }

        void deserializeTupleBegin()
        {
            assertType(rapidjson::kArrayType);
            document_t::ConstArray array = m_inputValue->GetArray();
            m_tupleEnd = array.End();
            m_inputValue = array.Begin();
        }

        void deserializeTupleEnd()
        {
            if (m_inputValue != m_tupleEnd)
            {
                throw std::runtime_error{ "attempt to deserialize tuple end with remaining elements" };
            }
            
            m_inputValue = nullptr;
            m_tupleEnd = nullptr;
        }

        template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
        static data_t Serialize(const T& instance, const property_set_t& includedProperties)
        {
            rapidjson::StringBuffer buffer;
            writer_t writer{ buffer };
            RapidJsonSerializer serializer{ writer };
            serializer.serialize(instance, includedProperties);

            return buffer.GetString();
        }

        template <typename T>
        static data_t Serialize(const T& value, const type::Descriptor<T>& descriptor)
        {
            rapidjson::StringBuffer buffer;
            writer_t writer{ buffer };
            RapidJsonSerializer serializer{ writer };
            serializer.serialize(value, descriptor);

            return buffer.GetString();
        }

        template <typename T>
        static data_t Serialize(const T& value)
        {
            rapidjson::StringBuffer buffer;
            writer_t writer{ buffer };
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

        using visitor_base_t = type::TypeVisitor<RapidJsonSerializer<Writer, Traits>>;
        using visitor_base_t::visit;

        friend visitor_base_t;

        template <bool Const>
        void visitBeginDerived()
        {
            if constexpr (Const)
            {
                assertHasWriter();
            }
            else
            {
                assertHasInputValue();
            }
        }

        //
        // serialization
        //

        template <typename T>
        bool visitStructBeginDerived(const T&/* instance*/, property_set_t&/* includedProperties*/)
        {
            m_writer->StartObject();
            return true;
        }

        template <typename T>
        void visitStructEndDerived(const T&/* instance*/, const property_set_t&/* includedProperties*/)
        {
            m_writer->EndObject();
        }

        template <typename T>
        bool visitPropertyBeginDerived(const T& property, bool/* first*/)
        {
            if (visitor_base_t::template visitingLevel<true>() > 0)
            {
                write(property.descriptor().name());
            }

            if (property.isValid())
            {
                return true;
            }
            else
            {
                m_writer->Null();
                return false;
            }
        }

        template <typename T>
        bool visitVectorBeginDerived(const vector_t<T>&/* vector*/, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            m_writer->StartArray();
            return true;
        }

        template <typename T>
        void visitVectorEndDerived(const vector_t<T>&/* vector*/, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            m_writer->EndArray();
        }

        template <typename T>
        void visitEnumDerived(const T& value, const type::EnumDescriptor<T>& descriptor)
        {
            if constexpr (traits_t::NumericEnums)
            {
                write(descriptor.enumeratorFromValue(value).tag());
            }
            else
            {
                write(descriptor.enumeratorFromValue(value).name());
            }
        }

        template <typename T>
        void visitFundamentalTypeDerived(const T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                write(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                write(value.toValue());
            }
            else if constexpr (std::is_same_v<T, timepoint_t>)
            {
                if constexpr (traits_t::NumericTimePoints)
                {
                    write(value.duration().toFractionalSeconds());
                }
                else
                {
                    write(value.toString());
                }
            }
            else if constexpr (std::is_same_v<T, steady_timepoint_t>)
            {
                write(value.duration().toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, duration_t>)
            {
                write(value.toFractionalSeconds());
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                write(value.toString());
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                write(value);
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
            
            assertType(rapidjson::kObjectType);
            const document_t::ValueType* objValue = m_inputValue;
            document_t::ConstObject obj = objValue->GetObject();

            for (const auto& member : obj)
            {
                m_inputValue = &member.value;

                if (!member.value.IsNull())
                {
                    std::string_view propertyName = member.name.GetString();

                    if (auto it = std::find_if(propertyDescriptors.begin(), propertyDescriptors.end(), [propertyName](const auto& p) { return p.name() == propertyName; }); it != propertyDescriptors.end())
                    {
                        const type::PropertyDescriptor& propertyDescriptor = *it;
                        type::ProxyProperty<> property{ instance, propertyDescriptor };
                        visit(property);
                    }
                }
            }

            m_inputValue = objValue;

            return false;
        }

        template <typename T>
        bool visitPropertyBeginDerived(T& property, bool/* first*/)
        {
            property.constructOrValue();
            return true;
        }

        template <typename T>
        bool visitVectorBeginDerived(vector_t<T>& vector, const type::Descriptor<vector_t<T>>& descriptor)
        {
            assertType(rapidjson::kArrayType);
            const document_t::ValueType* arrayValue = m_inputValue;
            document_t::ConstArray array = arrayValue->GetArray();

            for (const auto& value : array)
            {
                descriptor.resize(vector, vector.typelessSize() + 1);
                m_inputValue = &value;

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

            m_inputValue = arrayValue;

            return false;
        }

        template <typename T>
        void visitEnumDerived(T& value, const type::EnumDescriptor<T>& descriptor)
        {
            if constexpr (traits_t::NumericEnums)
            {
                descriptor.construct(value, descriptor.enumeratorFromTag(read<uint32_t>()).value());
            }
            else
            {
                descriptor.construct(value, descriptor.enumeratorFromName(read<string_t>()).value());
            }
        }

        template <typename T>
        void visitFundamentalTypeDerived(T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr(std::is_arithmetic_v<T>)
            {
                read(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                value = property_set_t{ read<uint32_t>() };
            }
            else if constexpr (std::is_same_v<T, timepoint_t>)
            {
                if constexpr (traits_t::NumericTimePoints)
                {
                    value = T{ duration_t{ read<float64_t>() } };
                }
                else
                {
                    value = timepoint_t::FromString(read<string_t>());
                }
            }
            else if constexpr (std::is_same_v<T, steady_timepoint_t> || std::is_same_v<T, duration_t>)
            {
                value = T{ duration_t{ read<float64_t>() } };
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                value = uuid_t::FromString(read<string_t>());
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                value = read<string_t>();
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }

        //
        // writing
        //
        
        void write(bool value)
        {
            m_writer->Bool(value);
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        void write(T value)
        {
            if constexpr (std::is_signed_v<T>)
            {
                if constexpr (sizeof(T) <= sizeof(int))
                {
                    m_writer->Int(value);
                }
                else/* if (sizeof(T) == sizeof(int64_t))*/
                {
                    m_writer->Int64(value);
                }
            }
            else/* if constexpr (std::is_unsigned_v<T>)*/
            {
                if constexpr (sizeof(T) <= sizeof(unsigned))
                {
                    m_writer->Uint(value);
                }
                else/* if (sizeof(T) == sizeof(uint64_t))*/
                {
                    m_writer->Uint64(value);
                }
            }
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void write(T value)
        {
            m_writer->Double(value);
        }
        
        void write(std::string_view sv)
        {
            m_writer->String(sv.data(), static_cast<rapidjson::SizeType>(sv.size()));
        }

        //
        // reading
        //

        template <typename T, std::enable_if_t<std::is_default_constructible_v<T>, int> = 0>
        T read()
        {
            T value;
            read(value);

            return value;
        }

        void read(bool& value)
        {
            assertType(rapidjson::kFalseType, rapidjson::kTrueType);
            value = m_inputValue->GetBool();
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        void read(T& value)
        {
            assertType(rapidjson::kNumberType);

            if constexpr (std::is_signed_v<T>)
            {
                if constexpr (sizeof(T) <= sizeof(int))
                {
                    value = static_cast<T>(m_inputValue->GetInt());
                }
                else/* if (sizeof(T) == sizeof(int64_t))*/
                {
                    value = static_cast<T>(m_inputValue->GetInt64());
                }
            }
            else/* if constexpr (std::is_unsigned_v<T>)*/
            {
                if constexpr (sizeof(T) <= sizeof(unsigned))
                {
                    value = static_cast<T>(m_inputValue->GetUint());
                }
                else/* if (sizeof(T) == sizeof(uint64_t))*/
                {
                    value = static_cast<T>(m_inputValue->GetUint64());
                }
            }
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void read(T& value)
        {
            assertType(rapidjson::kNumberType);

            if constexpr (sizeof(T) == sizeof(float))
            {
                value = m_inputValue->GetFloat();
            }
            else/* if (sizeof(T) == sizeof(double))*/
            {
                value = m_inputValue->GetDouble();
            }
        }

        void read(std::string& value)
        {
            assertType(rapidjson::kStringType);
            value = m_inputValue->GetString();
        }

    private:

        inline static std::map<rapidjson::Type, std::string_view> TypeNames = {
            { rapidjson::kNullType, "Null" },
            { rapidjson::kFalseType, "False" },
            { rapidjson::kTrueType, "True" },
            { rapidjson::kObjectType, "Object" },
            { rapidjson::kArrayType, "Array" },
            { rapidjson::kStringType, "String" },
            { rapidjson::kNumberType, "Number" }
        };

        inline static std::map<rapidjson::ParseErrorCode, std::string_view> ParseErrors = {
            { rapidjson::kParseErrorNone, "No error" },

            { rapidjson::kParseErrorDocumentEmpty, "The document is empty" },
            { rapidjson::kParseErrorDocumentRootNotSingular, "The document root must not follow by other values" },

            { rapidjson::kParseErrorValueInvalid, "Invalid value" },

            { rapidjson::kParseErrorObjectMissName, "Missing a name for object member" },
            { rapidjson::kParseErrorObjectMissColon, "Missing a colon after a name of object member" },
            { rapidjson::kParseErrorObjectMissCommaOrCurlyBracket, "Missing a comma or '}' after an object member" },

            { rapidjson::kParseErrorArrayMissCommaOrSquareBracket, "Missing a comma or ']' after an array element" },

            { rapidjson::kParseErrorStringUnicodeEscapeInvalidHex, "Incorrect hex digit after \\u escape in string" },
            { rapidjson::kParseErrorStringUnicodeSurrogateInvalid, "The surrogate pair in string is invalid" },
            { rapidjson::kParseErrorStringEscapeInvalid, "Invalid escape character in string" },
            { rapidjson::kParseErrorStringMissQuotationMark, "Missing a closing quotation mark in string" },
            { rapidjson::kParseErrorStringInvalidEncoding, "Invalid encoding in string" },

            { rapidjson::kParseErrorNumberTooBig, "Number too big to be stored in double" },
            { rapidjson::kParseErrorNumberMissFraction, "Miss fraction part in number" },
            { rapidjson::kParseErrorNumberMissExponent, "Miss exponent in number" },

            { rapidjson::kParseErrorTermination, "Parsing was terminated" },
            { rapidjson::kParseErrorUnspecificSyntaxError, "Unspecific syntax error" }
        };

        RapidJsonSerializer(Writer* writer, const document_t::ValueType* inputValue) :
            m_writer(writer)
        {
            if (inputValue != nullptr)
            {
                setInputValue(*inputValue);
            }
        }

        RapidJsonSerializer(Writer* writer, std::optional<document_t> document) :
            m_writer(writer)
        {
            if (document != std::nullopt)
            {
                setInputValue(std::move(*document));
            }
        }

        RapidJsonSerializer(Writer* writer, std::string_view input) :
            m_writer(writer)
        {
            if (!input.empty())
            {
                setInputValue(input);
            }
        }

        std::string typeToString(rapidjson::Type type)
        {
            return std::string{ TypeNames[type] };
        }

        std::string parseErrorToString(rapidjson::ParseErrorCode parseError)
        {
            return std::string{ ParseErrors[parseError] };
        }

        void assertHasWriter() const
        {
            if (m_writer == nullptr)
            {
                throw std::logic_error{ "serializer does not have RapidJSON writer" };
            }
        }

        void assertHasInputValue() const
        {
            if (!hasInputValue())
            {
                throw std::logic_error{ "serializer does not have RapidJSON input value" };
            }
        }

        template <size_t N>
        void assertType(std::array<rapidjson::Type, N> expectedTypes)
        {
            if (rapidjson::Type actualType = m_inputValue->GetType(); std::find(expectedTypes.begin(), expectedTypes.end(), actualType) == expectedTypes.end())
            {
                std::string what = "encountered unexpected JSON type: expected '";

                for (rapidjson::Type expectedType : expectedTypes)
                {
                    what += typeToString(expectedType);
                    what += "' or '";
                }

                what.resize(what.size() - 5);
                what += " but got '";
                what += typeToString(actualType);
                what += '\'';

                throw std::runtime_error{ what };
            }
        }

        template <typename... Ts, std::enable_if_t<std::conjunction_v<std::is_same<Ts, rapidjson::Type>...>, int> = 0>
        void assertType(Ts&&... expectedTypes)
        {
            assertType(std::array{ std::forward<Ts>(expectedTypes)... });
        }
        
        Writer* m_writer = nullptr;
        std::optional<document_t> m_document = std::nullopt;
        const document_t::ValueType* m_inputValue = nullptr;
        document_t::ConstValueIterator m_tupleEnd = nullptr;
    };
}