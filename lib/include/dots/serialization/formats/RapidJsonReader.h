#pragma once
#include <stack>
#include <rapidjson/document.h>

namespace dots::serialization
{
    struct RapidJsonReader
    {
        using data_t = std::string;
        using value_t = data_t::value_type;
        
        using document_t = rapidjson::Document;

        RapidJsonReader() = default;

        RapidJsonReader(const document_t::ValueType& inputValue)
        {
            setInput(inputValue);
        }

        RapidJsonReader(document_t document)
        {
            setInput(std::move(document));
        }

        RapidJsonReader(std::string_view input)
        {
            if (!input.empty())
            {
                setInput(input);
            }
        }

        RapidJsonReader(const RapidJsonReader& other) = delete;
        RapidJsonReader(RapidJsonReader&& other) = default;
        ~RapidJsonReader() = default;

        RapidJsonReader& operator = (const RapidJsonReader& rhs) = delete;
        RapidJsonReader& operator = (RapidJsonReader&& rhs) = default;

        void setInput(const document_t::ValueType& inputValue)
        {
            m_input = &inputValue;
            m_nesting = {};
        }

        void setInput(document_t document)
        {
            m_document.emplace(std::move(document));

            if (m_document->HasParseError())
            {
                throw std::runtime_error("RapidJSON parse error at '" + std::to_string(m_document->GetErrorOffset()) + "' -> " + parseErrorToString(m_document->GetParseError()));
            }

            setInput(static_cast<const document_t::ValueType&>(*m_document));
        }

        void setInput(std::string_view input)
        {
            document_t document;
            document.Parse(input.data(), input.size());
            setInput(std::move(document));
        }

        bool hasInput() const
        {
            return m_input != nullptr;
        }

        size_t nestingLevel() const
        {
            return m_nesting.size();
        }

        void readArrayBegin()
        {
            assertType(rapidjson::kArrayType);
            document_t::ConstArray array = m_input->GetArray();
            m_nesting.emplace(m_input);
            m_input = array.Begin();
        }

        void readArrayEnd()
        {
            const document_t::ValueType* elementValue = m_input;
            m_input = m_nesting.top();
            assertType(rapidjson::kArrayType);
            m_nesting.pop();

            if (elementValue != m_input->GetArray().End())
            {
                throw std::runtime_error{ "attempt to read array end with remaining elements" };
            }
            
            finalizeRead();
        }

        bool tryReadArrayEnd()
        {
            const document_t::ValueType* elementValue = m_input;
            document_t::ConstArray array = m_nesting.top()->GetArray();

            if (elementValue == array.End())
            {
                m_input = m_nesting.top();
                assertType(rapidjson::kArrayType);
                m_nesting.pop();
                finalizeRead();

                return true;
            }
            else
            {
                return false;
            }
        }
        
        void readObjectBegin()
        {
            assertType(rapidjson::kObjectType);
            document_t::ConstObject obj = m_input->GetObject();
            m_nesting.emplace(m_input);
            m_input = &obj.MemberBegin()->name;
        }

        void readObjectEnd()
        {
            const document_t::ValueType* memberValue = m_input;
            m_input = m_nesting.top();
            assertType(rapidjson::kObjectType);
            m_nesting.pop();

            if (memberValue != &m_input->GetObject().MemberEnd()->name)
            {
                throw std::runtime_error{ "attempt to read object end with remaining members" };
            }
            
            finalizeRead();
        }

        bool tryReadObjectEnd()
        {
            const document_t::ValueType* memberValue = m_input;

            if (memberValue == &m_nesting.top()->GetObject().MemberEnd()->name)
            {
                m_input = m_nesting.top();
                assertType(rapidjson::kObjectType);
                m_nesting.pop();
                finalizeRead();

                return true;
            }
            else
            {
                return false;
            }
        }

        std::string_view readObjectMemberName()
        {
            return read<std::string_view>();
        }

        bool tryReadNull()
        {
            if (m_input->IsNull())
            {
                finalizeRead();
                return true;
            }
            else
            {
                return false;
            }
        }

        template <typename T, std::enable_if_t<std::is_default_constructible_v<T>, int> = 0>
        T read()
        {
            T value;
            read(value);
            finalizeRead();

            return value;
        }

        void skip()
        {
            finalizeRead();
        }

        void assertHasInputValue() const
        {
            if (!hasInput())
            {
                throw std::logic_error{ "serializer does not have RapidJSON input value" };
            }
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

        void read(bool& value)
        {
            assertType(rapidjson::kFalseType, rapidjson::kTrueType);
            value = m_input->GetBool();
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        void read(T& value)
        {
            assertType(rapidjson::kNumberType);

            if constexpr (std::is_signed_v<T>)
            {
                if constexpr (sizeof(T) <= sizeof(int))
                {
                    value = static_cast<T>(m_input->GetInt());
                }
                else/* if (sizeof(T) == sizeof(int64_t))*/
                {
                    value = static_cast<T>(m_input->GetInt64());
                }
            }
            else/* if constexpr (std::is_unsigned_v<T>)*/
            {
                if constexpr (sizeof(T) <= sizeof(unsigned))
                {
                    value = static_cast<T>(m_input->GetUint());
                }
                else/* if (sizeof(T) == sizeof(uint64_t))*/
                {
                    value = static_cast<T>(m_input->GetUint64());
                }
            }
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void read(T& value)
        {
            assertType(rapidjson::kNumberType);

            if constexpr (sizeof(T) == sizeof(float))
            {
                value = m_input->GetFloat();
            }
            else/* if (sizeof(T) == sizeof(double))*/
            {
                value = m_input->GetDouble();
            }
        }

        void read(std::string& value)
        {
            assertType(rapidjson::kStringType);
            value = m_input->GetString();
        }

        void read(std::string_view& value)
        {
            assertType(rapidjson::kStringType);
            value = m_input->GetString();
        }

        void finalizeRead()
        {
            if (!m_nesting.empty())
            {
                ++m_input;
            }
            else
            {
                m_input = nullptr;
            }
        }

        template <size_t N>
        void assertType(std::array<rapidjson::Type, N> expectedTypes)
        {
            if (rapidjson::Type actualType = m_input->GetType(); std::find(expectedTypes.begin(), expectedTypes.end(), actualType) == expectedTypes.end())
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

        std::string typeToString(rapidjson::Type type)
        {
            return std::string{ TypeNames[type] };
        }

        std::string parseErrorToString(rapidjson::ParseErrorCode parseError)
        {
            return std::string{ ParseErrors[parseError] };
        }

        std::stack<const document_t::ValueType*> m_nesting;
        std::optional<document_t> m_document = std::nullopt;
        const document_t::ValueType* m_input = nullptr;
    };
}
