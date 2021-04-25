#pragma once
#include <string>
#include <stack>
#include <vector>
#include <dots/io/serialization/SerializerBase.h>
#include <dots/tools/string_tools.h>

namespace dots::io
{
    struct StringSerializerTraits
    {
        static constexpr std::string_view StructBegin = "{";
        static constexpr std::string_view StructEnd = "}";

        static constexpr std::string_view PropertyNameBegin = ".";
        static constexpr std::string_view PropertyNameEnd = "";
        static constexpr std::string_view PropertyValueBegin = "=";
        static constexpr std::string_view PropertyValueEnd = ",";
        static constexpr std::string_view PropertyInvalidValue = "<invalid>";

        static constexpr std::string_view VectorBegin = "{";
        static constexpr std::string_view VectorValueSeparator = ",";
        static constexpr std::string_view VectorEnd = "}";

        static constexpr std::string_view TupleBegin = "{";
        static constexpr std::string_view TupleElementSeperator = ",";
        static constexpr std::string_view TupleEnd = "}";

        static constexpr std::string_view StringDelimiter = "\"";
        static constexpr std::string_view StringEscape = "\\";

        struct string_escape_mapping
        {
            std::string_view from;
            std::string_view to;
        };
        static constexpr std::array<string_escape_mapping, 8> StringEscapeMapping{
            string_escape_mapping{ "\"", "\\\"" },
            string_escape_mapping{ "\\", "\\\\" },
            string_escape_mapping{ "/", "\\/" },
            string_escape_mapping{ "\b", "\\b" },
            string_escape_mapping{ "\f", "\\f" },
            string_escape_mapping{ "\n", "\\n" },
            string_escape_mapping{ "\r", "\\r" },
            string_escape_mapping{ "\t", "\\t" }
        };

        static constexpr bool UserTypeNames = true;

        static constexpr bool NumericBooleans = false;
        static constexpr bool NumericPropertySets = false;
        static constexpr bool NumericTimePoints = false;
        static constexpr bool NumericEnums = false;

        static constexpr bool IntegerBasePrefixes = true;
        static constexpr bool IntegerSignSuffixes = true;

        static constexpr bool FloatSizeSuffix = true;
    };

    struct StringSerializerOptions
    {
        bool compact = false;
        bool multiLine = false;
        size_t indentSize = 4;
    };

    template <typename Derived, typename Traits>
    struct StringSerializerBase : SerializerBase<std::string, Derived>
    {
        using serializer_base_t = SerializerBase<std::string, Derived>;

        using traits_t = Traits;
        using data_t = typename serializer_base_t::data_t;

        StringSerializerBase(StringSerializerOptions options = {}) :
            m_indentLevel(0),
            m_options{ std::move(options) }
        {
            /* do nothing */
        }
        StringSerializerBase(const StringSerializerBase& other) = default;
        StringSerializerBase(StringSerializerBase&& other) = default;
        ~StringSerializerBase() = default;

        StringSerializerBase& operator = (const StringSerializerBase& rhs) = default;
        StringSerializerBase& operator = (StringSerializerBase&& rhs) = default;

        using serializer_base_t::output;
        using serializer_base_t::lastSerializeSize;
        using serializer_base_t::lastDeserializeSize;

        size_t serializeTupleBegin()
        {
            m_outputTupleInfo.push(false);
            serializer_base_t::template visitBeginDerived<true>();
            incrementIndentLevel();
            writePrefixedNewLine(traits_t::TupleBegin);

            return lastSerializeSize();
        }

        size_t serializeTupleEnd()
        {
            if (m_outputTupleInfo.empty())
            {
                throw std::logic_error{ "attempt to serialize tuple end without previous begin" };
            }

            m_outputTupleInfo.pop();

            serializer_base_t::template visitBeginDerived<true>();
            decrementIndentLevel();
            writeSuffixedNewLine(traits_t::TupleEnd);

            return lastSerializeSize();
        }

        size_t deserializeTupleBegin()
        {
            m_inputTupleInfo.push(false);
            serializer_base_t::template visitBeginDerived<false>();
            readTokenAfterWhitespace(traits_t::TupleBegin);
            inputData() = m_input.data();

            return lastDeserializeSize();
        }

        size_t deserializeTupleEnd()
        {
            if (m_inputTupleInfo.empty())
            {
                throw std::logic_error{ "attempt to deserialize tuple end without previous begin" };
            }

            m_inputTupleInfo.pop();

            serializer_base_t::template visitBeginDerived<false>();
            readTokenAfterWhitespace(traits_t::TupleEnd);
            inputData() = m_input.data();

            return lastDeserializeSize();
        }

        template <typename T, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, StringSerializerOptions> && std::is_base_of_v<type::Struct, T>, int> = 0>
        static data_t Serialize(const T& instance, const property_set_t& includedProperties, StringSerializerOptions options = {})
        {
            Derived serializer{ options };
            serializer.serialize(instance, includedProperties);

            return std::move(serializer.output());
        }

        template <typename T, typename D = Derived, std::enable_if_t<std::is_constructible_v<D, StringSerializerOptions>, int> = 0>
        static data_t Serialize(const T& value, StringSerializerOptions options = {})
        {
            Derived serializer{ options };
            serializer.serialize(value);

            return std::move(serializer.output());
        }

    protected:

        using visitor_base_t = type::TypeVisitor<Derived>;

        friend visitor_base_t;
        friend serializer_base_t;

        using serializer_base_t::visit;
        using serializer_base_t::inputData;
        using serializer_base_t::inputDataEnd;

        template <bool Const>
        void visitBeginDerived()
        {
            serializer_base_t::template visitBeginDerived<Const>();

            if constexpr (Const)
            {
                if (!m_outputTupleInfo.empty() && m_outputTupleInfo.top())
                {
                    writePrefixedNewLine(traits_t::TupleElementSeperator);
                }
            }
            else
            {
                if (!m_inputTupleInfo.empty() && m_inputTupleInfo.top())
                {
                    readTokenAfterWhitespace(traits_t::TupleElementSeperator);
                }
            }
        }

        template <bool Const>
        void visitEndDerived()
        {
            if constexpr (Const)
            {
                if (!m_outputTupleInfo.empty())
                {
                    m_outputTupleInfo.top() = true;
                }
            }
            else
            {
                if (!m_inputTupleInfo.empty())
                {
                    m_inputTupleInfo.top() = true;
                }

                inputData() = m_input.data();
            }

            serializer_base_t::template visitEndDerived<Const>();
        }

        //
        // serialization
        //

        template <typename T>
        bool visitStructBeginDerived(const T& instance, property_set_t&/* includedProperties*/)
        {
            if constexpr (traits_t::UserTypeNames)
            {
                write(instance._descriptor().name());
            }

            incrementIndentLevel();
            writePrefixedNewLine(traits_t::StructBegin);

            return true;
        }

        template <typename T>
        void visitStructEndDerived(const T&/* instance*/, const property_set_t&/* includedProperties*/)
        {
            decrementIndentLevel();
            writeSuffixedNewLine(traits_t::StructEnd);
        }

        template <typename T>
        bool visitPropertyBeginDerived(const T& property, bool first)
        {
            if (!first)
            {
                writePrefixedNewLine(traits_t::PropertyValueEnd);
            }

            write(traits_t::PropertyNameBegin);
            write(property.descriptor().name());

            if (!traits_t::PropertyNameEnd.empty())
            {
                write(traits_t::PropertyNameEnd);
            }

            if constexpr (!traits_t::PropertyValueBegin.empty())
            {
                writeSeparator(traits_t::PropertyValueBegin);
            }
            else if (!m_options.compact)
            {
                write(" ");
            }

            return true;
        }

        template <typename T>
        void visitPropertyEndDerived(const T& property, bool/* first*/)
        {
            if (!property.isValid())
            {
                write(traits_t::PropertyInvalidValue);
            }
        }

        template <typename T>
        bool visitVectorBeginDerived(const vector_t<T>& vector, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            incrementIndentLevel();

            if (vector.typelessSize() == 0)
            {
                write(traits_t::VectorBegin);
                return false;
            }
            else
            {
                writePrefixedNewLine(traits_t::VectorBegin);
                return true;
            }
        }

        template <typename T>
        bool visitVectorValueBeginDerived(const vector_t<T>&/* vector*/, const type::Descriptor<vector_t<T>>&/* descriptor*/, size_t/* index*/)
        {
            return true;
        }

        template <typename T>
        void visitVectorValueEndDerived(const vector_t<T>& vector, const type::Descriptor<vector_t<T>>&/* descriptor*/, size_t index)
        {
            if (index < vector.typelessSize() - 1)
            {
                writePrefixedNewLine(traits_t::VectorValueSeparator);
            }
        }

        template <typename T>
        void visitVectorEndDerived(const vector_t<T>&/* vector*/, const type::Descriptor<vector_t<T>>&/* descriptor*/)
        {
            decrementIndentLevel();
            writeSuffixedNewLine(traits_t::VectorEnd);
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
                if constexpr (traits_t::UserTypeNames)
                {
                    write(descriptor.name());
                    write("::");
                }

                write(descriptor.enumeratorFromValue(value).name());
            }
        }

        template <typename T>
        void visitFundamentalTypeDerived(const T& value, const type::Descriptor<T>&/* descriptor*/)
        {
            if constexpr (std::is_same_v<T, bool>)
            {
                if constexpr (traits_t::NumericBooleans)
                {
                    write(static_cast<uint8_t>(value));
                }
                else
                {
                    write(value ? "true" : "false");
                }
            }
            else if constexpr(std::is_arithmetic_v<T>)
            {
                write(value);
            }
            else if constexpr(std::is_same_v<T, property_set_t>)
            {
                if constexpr (traits_t::NumericPropertySets)
                {
                    write(value.toValue());
                }
                else
                {
                    write("0b");
                    write(value.toString());
                }
            }
            else if constexpr (std::is_same_v<T, timepoint_t>)
            {
                if constexpr (traits_t::NumericTimePoints)
                {
                    write(value.duration().toFractionalSeconds());
                }
                else
                {
                    writeEnclosed(value.toString(), traits_t::StringDelimiter);
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
                writeEnclosed(value.toString(), traits_t::StringDelimiter);
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                writeEscaped(value);
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

            if constexpr (traits_t::UserTypeNames)
            {
                readTokenAfterWhitespace(descriptor.name());
            }

            readTokenAfterWhitespace(traits_t::StructBegin);

            for (;;)
            {
                readTokenAfterWhitespace(traits_t::PropertyNameBegin);
                std::string_view propertyName = readIdentifier();

                if constexpr (!traits_t::PropertyNameEnd.empty())
                {
                    readTokenAfterWhitespace(traits_t::PropertyNameEnd);
                }

                readTokenAfterWhitespace(traits_t::PropertyValueBegin);

                if (auto it = std::find_if(propertyDescriptors.begin(), propertyDescriptors.end(), [propertyName](const auto& p) { return p.name() == propertyName; }); it != propertyDescriptors.end())
                {
                    const type::PropertyDescriptor& propertyDescriptor = *it;
                    type::ProxyProperty<> property{ instance, propertyDescriptor };
                    visit(property);
                }

                if (readAnyTokenAfterWhitespace(std::array{ traits_t::PropertyValueEnd, traits_t::StructEnd }) == traits_t::StructEnd)
                {
                    break;
                }
            }

            return false;
        }

        template <typename T>
        bool visitPropertyBeginDerived(T& property, bool/* first*/)
        {
            if constexpr (!traits_t::PropertyInvalidValue.empty())
            {
                if (tryReadTokenAfterWhitespace(traits_t::PropertyInvalidValue))
                {
                    return false;
                }
            }

            property.constructOrValue();
            return true;
        }

        template <typename T>
        bool visitVectorBeginDerived(vector_t<T>& vector, const type::Descriptor<vector_t<T>>& descriptor)
        {
            readTokenAfterWhitespace(traits_t::VectorBegin);

            for (;;)
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

                if (readAnyTokenAfterWhitespace(std::array{ traits_t::VectorValueSeparator, traits_t::VectorEnd }) == traits_t::VectorEnd)
                {
                    break;
                }
            }

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
                if constexpr (traits_t::UserTypeNames)
                {
                    readTokenAfterWhitespace(descriptor.name());
                    readTokenAfterWhitespace("::");
                }

                descriptor.construct(value, descriptor.enumeratorFromName(readIdentifier()).value());
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
                value = readEscapedToken();
            }
            else
            {
                static_assert(!std::is_same_v<T, T>, "type not supported");
            }
        }

        void setInputDerived()
        {
            m_input = std::string_view{ inputData(), static_cast<size_t>(inputDataEnd() - inputData()) };
        }

        //
        // indentation
        //

        size_t& indentLevel()
        {
            return m_indentLevel;
        }

        void incrementIndentLevel()
        {
            ++m_indentLevel;
        }

        void decrementIndentLevel()
        {
            --m_indentLevel;
        }

        //
        // writing
        //

        void writeSeparator(std::string_view infix)
        {
            if (m_options.compact)
            {
                write(infix);
            }
            else
            {
                writeEnclosed(std::string{ infix }, " ");
            }
        }

        void writeNewLine()
        {
            if (m_options.multiLine)
            {
                write("\n");
                output().resize(output().size() + m_indentLevel * m_options.indentSize, ' ');
            }
            else if (!m_options.compact)
            {
                write(" ");
            }
        }

        void writePrefixedNewLine(std::string_view prefix)
        {
            write(prefix);
            writeNewLine();
        }

        void writeSuffixedNewLine(std::string_view suffix)
        {
            writeNewLine();
            write(suffix);
        }

        void writeEnclosed(const std::string& str, std::string_view delimiter)
        {
            write(delimiter, str, delimiter);
        }

        void writeEscaped(const std::string& str)
        {
            write(traits_t::StringDelimiter);

            {
                std::string_view strRemaining = str;

                while (!strRemaining.empty())
                {
                    auto it = std::find_if(traits_t::StringEscapeMapping.begin(), traits_t::StringEscapeMapping.end(), [&strRemaining](const auto& escapeMapping)
                    {
                        return tools::starts_with(strRemaining, escapeMapping.from);
                    });

                    if (it == traits_t::StringEscapeMapping.end())
                    {
                        output() += strRemaining.front();
                        strRemaining.remove_prefix(1);
                    }
                    else
                    {
                        const auto& [from, to] = *it;
                        output() += to;
                        strRemaining.remove_prefix(from.size());
                    }
                }
            }

            write(traits_t::StringDelimiter);
        }

        template <typename... Ts, std::enable_if_t<std::is_constructible_v<std::string>, int> = 0>
        void write(Ts&&... strs)
        {
            (output().operator+=(std::forward<Ts>(strs)), ...);
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        void write(T value, int base = 10)
        {
            if constexpr (traits_t::IntegerBasePrefixes)
            {
                if (base == 2)
                {
                    write("0b");
                }
                else if (base == 16)
                {
                    write("0x");
                }
            }
            else
            {
                (void)base;
            }

            char buffer[32];
            auto [last, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
            write(std::string_view{ buffer, static_cast<size_t>(last - buffer) });

            if constexpr (traits_t::IntegerSignSuffixes && std::is_unsigned_v<T>)
            {
                write("u");
            }
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void write(T value)
        {
            #if (defined _MSC_VER) && (!defined __INTEL_COMPILER)
            constexpr bool HasFpToChars = true;
            #else
            constexpr bool HasFpToChars = false;
            #endif

            if (value == 0)
            {
                write("0.0");
            }
            else
            {
                if constexpr (HasFpToChars)
                {
                    char buffer[32];
                    auto [last, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
                    write(std::string_view{ buffer, static_cast<size_t>(last - buffer) });
                }
                else
                {
                    std::ostringstream oss;
                    oss << std::setprecision(std::numeric_limits<T>::digits10 + 1) << value;
                    write(oss.str());
                }
            }

            if constexpr (traits_t::FloatSizeSuffix && std::is_same_v<T, float>)
            {
                write("f");
            }
        }

        //
        // reading
        //

        void readWhitespace()
        {
            while (!m_input.empty() && m_input.front() < '!')
            {
                m_input.remove_prefix(1);
            }
        }

        bool tryReadToken(std::string_view token)
        {
            if (tools::starts_with(m_input, token))
            {
                m_input.remove_prefix(token.size());
                return true;
            }
            else
            {
                return false;
            }
        }

        bool tryReadTokenAfterWhitespace(std::string_view token)
        {
            readWhitespace();
            return tryReadToken(token);
        }

        void readToken(std::string_view token)
        {
            if (!tryReadToken(token))
            {
                throw makeTokenError(token);
            }
        }

        void readTokenAfterWhitespace(std::string_view token)
        {
            readWhitespace();
            readToken(token);
        }

        template <size_t N>
        std::string_view readAnyToken(std::array<std::string_view, N> tokens)
        {
            for (std::string_view token : tokens)
            {
                if (tryReadToken(token))
                {
                    return token;
                }
            }

            throw makeTokenError(tokens);
        }

        template <size_t N>
        std::string_view readAnyTokenAfterWhitespace(std::array<std::string_view, N> tokens)
        {
            readWhitespace();
            return readAnyToken(tokens);
        }

        template <size_t N>
        auto readDelimitedToken(std::array<std::string_view, N> delimiters) -> std::pair<std::string_view, std::string_view>
        {
            readWhitespace();
            std::string_view tokenInput = m_input;

            while (!tokenInput.empty())
            {
                for (std::string_view delimiter : delimiters)
                {
                    if (tools::starts_with(tokenInput, delimiter))
                    {
                        std::string_view token = m_input.substr(0, static_cast<size_t>(tokenInput.data() - m_input.data()));
                        tokenInput.remove_prefix(delimiter.size());
                        m_input = tokenInput;

                        return { token, delimiter };
                    }
                }

                tokenInput.remove_prefix(1);
            }            

            throw makeTokenError(delimiters);
        }

        auto readDelimitedToken(std::string_view delimiter) -> std::string_view
        {
            return readDelimitedToken(std::array{ delimiter }).first;
        }

        std::string_view readEnclosedToken(std::string_view beginDelimiter, std::optional<std::string_view> endDelimiter = std::nullopt)
        {
            readTokenAfterWhitespace(beginDelimiter);
            return readDelimitedToken(endDelimiter == std::nullopt ? beginDelimiter : *endDelimiter);
        }

        std::string readEscapedToken()
        {
            readTokenAfterWhitespace(traits_t::StringDelimiter);
            std::string token;

            while (!m_input.empty())
            {
                if (tryReadToken(traits_t::StringDelimiter))
                {
                    return token;
                }
                else if (tools::starts_with(m_input, traits_t::StringEscape))
                {
                    auto it = std::find_if(traits_t::StringEscapeMapping.begin(), traits_t::StringEscapeMapping.end(), [this](const auto& escapeMapping)
                    {
                        return tools::starts_with(m_input, escapeMapping.to);
                    });

                    if (it != traits_t::StringEscapeMapping.end())
                    {
                        const auto& [from, to] = *it;
                        token += from;
                        m_input.remove_prefix(to.size());
                    }
                }
                else
                {
                    token += m_input.front();
                    m_input.remove_prefix(1);
                }
            }

            throw makeTokenError(traits_t::StringDelimiter);
        }

        std::string_view readIdentifier()
        {
            static /*constexpr */std::vector<bool> IdentifierSymbols = []()
            {
                std::vector<bool> identifierSymbols(2 << (sizeof(char) * 8 - 1), false);
                for (char c = '0'; c <= '9'; ++c){ identifierSymbols[static_cast<size_t>(c)] = true; }
                for (char c = 'A'; c <= 'Z'; ++c){ identifierSymbols[static_cast<size_t>(c)] = true; }
                for (char c = 'a'; c <= 'z'; ++c){ identifierSymbols[static_cast<size_t>(c)] = true; }
                identifierSymbols[static_cast<size_t>('_')] = true;

                return identifierSymbols;
            }();

            auto it = std::find_if_not(m_input.begin(), m_input.end(), [](char c){ return IdentifierSymbols[static_cast<size_t>(c)]; });
            size_t identifierSize = it == m_input.end() ? m_input.size() : std::distance(m_input.begin(), it);
            std::string_view identifier = m_input.substr(0, identifierSize);
            m_input.remove_prefix(identifierSize);

            return identifier;
        }

        template <typename T, std::enable_if_t<std::is_default_constructible_v<T>, int> = 0>
        T read()
        {
            T value;
            read(value);

            return value;
        }

        void read(bool& value)
        {
            readWhitespace();

            if constexpr (traits_t::NumericBooleans)
            {
                if (tryReadToken("1"))
                {
                    value = true;
                    return;
                }
                else if (tryReadToken("0"))
                {
                    value = false;
                    return;
                }
            }
            else
            {
                if (tryReadToken("true"))
                {
                    value = true;
                    return;
                }
                else if (tryReadToken("false"))
                {
                    value = false;
                    return;
                }
            }

            throw makeTokenError("<boolean-string>");
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        void read(T& value)
        {
            int base = 10;
            readWhitespace();

            if constexpr (traits_t::IntegerBasePrefixes)
            {
                if (tryReadToken("0b"))
                {
                    base = 2;
                }
                else if (tryReadToken("0x"))
                {
                    base = 16;
                }
            }

            std::from_chars_result result = std::from_chars(m_input.data(), m_input.data() + m_input.size(), value, base);

            if (result.ec != std::errc{})
            {
                throw makeTokenError("<integer-value>");
            }

            m_input.remove_prefix(static_cast<size_t>(result.ptr - m_input.data()));
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void read(T& value)
        {
            #if (defined _MSC_VER) && (!defined __INTEL_COMPILER)
            constexpr bool HasFpFromChars = true;
            #else
            constexpr bool HasFpFromChars = false;
            #endif

            if constexpr (HasFpFromChars)
            {
                readWhitespace();
                std::from_chars_result result = std::from_chars(m_input.data(), m_input.data() + m_input.size(), value);

                if (result.ec != std::errc{})
                {
                    throw makeTokenError("<floating-point-value>");
                }

                m_input.remove_prefix(static_cast<size_t>(result.ptr - m_input.data()));
            }
            else
            {
                char* end;

                if constexpr (std::is_same_v<T, float>)
                {
                    value = std::strtof(m_input.data(), &end);
                }
                else/* if constexpr (std::is_same_v<T, double>)*/
                {
                    value = std::strtod(m_input.data(), &end);
                }

                if (end == m_input.data())
                {
                    throw makeTokenError("<floating point value>");
                }

                m_input.remove_prefix(static_cast<size_t>(end - m_input.data()));
            }

            if constexpr (traits_t::FloatSizeSuffix && std::is_same_v<T, float>)
            {
                tryReadTokenAfterWhitespace("f");
            }
        }

        void read(std::string& value)
        {
            value = readEnclosedToken(traits_t::StringDelimiter);
        }


    private:

        using tuple_info_t = std::stack<bool, std::vector<bool>>;

        template <size_t N>
        std::runtime_error makeTokenError(std::array<std::string_view, N> expected)
        {
            ptrdiff_t inputPos = m_input.data() - inputData();
            std::string what = "encountered unexpected token at " + std::to_string(inputPos) + ": expected '";

            for (std::string_view token : expected)
            {
                what += token;
                what += "' or '";
            }

            what.resize(what.size() - 5);
            what += " but got '";
            what += m_input.substr(0, std::min<size_t>(m_input.size(), 10));
            what += '\'';

            return std::runtime_error{ what };
        }

        std::runtime_error makeTokenError(std::string_view expected)
        {
            return makeTokenError(std::array{ expected });
        }

        tuple_info_t m_outputTupleInfo;
        tuple_info_t m_inputTupleInfo;
        size_t m_indentLevel;
        StringSerializerOptions m_options;
        std::string_view m_input;
    };
}