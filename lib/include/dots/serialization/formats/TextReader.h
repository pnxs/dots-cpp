#pragma once
#include <string>
#include <vector>
#include <stack>
#include <dots/serialization/formats/Reader.h>
#include <dots/serialization/formats/TextFormat.h>
#include <dots/tools/string_tools.h>
#include <dots/tools/type_traits.h>

namespace dots::serialization
{
    template <typename Format>
    struct TextReader : Reader<std::string>
    {
        using format_t = Format;

        TextReader(TextOptions options = {}) :
            m_options{ options }
        {
            /* do nothing */
        }

        size_t nestingLevel() const
        {
            return m_nesting.size();
        }

        void readArrayBegin()
        {
            consumeTokenAfterWhitespace(format_t::ArrayBegin);
            m_nesting.emplace(State::Array, true);
        }

        void readArrayEnd()
        {
            m_nesting.pop();
            consumeTokenAfterWhitespace(format_t::ArrayEnd);
            finalizeRead();
        }

        bool tryReadArrayEnd()
        {
            if (tryConsumeTokenAfterWhitespace(format_t::ArrayEnd))
            {
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
            consumeTokenAfterWhitespace(format_t::ObjectBegin);
            m_nesting.emplace(State::Object, true);
        }

        void readObjectBegin(std::string_view objectTypeName)
        {
            if constexpr (format_t::ObjectFormat == TextFormat::ObjectFormat::WithTypeName)
            {
                if (m_options.policy == TextOptions::Strict)
                {
                    consumeTokenAfterWhitespace(objectTypeName);
                }
                else
                {
                    tryConsumeTokenAfterWhitespace(objectTypeName);
                }
            }
            else
            {
                (void)objectTypeName;
            }

            readObjectBegin();
        }

        void readObjectEnd()
        {
            m_nesting.pop();
            consumeTokenAfterWhitespace(format_t::ObjectEnd);
            finalizeRead();
        }

        bool tryReadObjectEnd()
        {
            if (tryConsumeTokenAfterWhitespace(format_t::ObjectEnd))
            {
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
            consumeTokenAfterWhitespace(format_t::ObjectMemberNameBegin);
            std::string_view propertyName = readIdentifierString();

            if constexpr (!format_t::ObjectMemberNameEnd.empty())
            {
                consumeTokenAfterWhitespace(format_t::ObjectMemberNameEnd);
            }

            consumeTokenAfterWhitespace(format_t::ObjectMemberValueBegin);

            return propertyName;
        }

        bool tryReadNull()
        {
            if constexpr (format_t::NullValue.empty())
            {
                return false;
            }
            else
            {
                bool isNull = tryConsumeTokenAfterWhitespace(format_t::NullValue);

                if (isNull)
                {
                    finalizeRead();
                }

                return isNull;
            }
        }

        template <typename T, std::enable_if_t<!std::is_same_v<T, std::string> && std::is_default_constructible_v<T>, int> = 0>
        T read()
        {
            T value;
            read(value);
            finalizeRead();

            return value;
        }

        std::string readEscapedString()
        {
            if (m_nesting.empty())
            {
                consumeWhitespace();
                std::string token = std::string{ inputData(), inputAvailable() };
                inputAdvance(input().size());
                finalizeRead();

                return token;
            }
            else
            {
                consumeTokenAfterWhitespace(format_t::StringDelimiter);
                std::string token;

                while (!input().empty())
                {
                    if (tryConsumeToken(format_t::StringDelimiter))
                    {
                        finalizeRead();
                        return token;
                    }
                    else if (tools::starts_with(input(), format_t::StringEscape))
                    {
                        auto it = std::find_if(format_t::StringEscapeMapping.begin(), format_t::StringEscapeMapping.end(), [this](const auto& escapeMapping)
                        {
                            return tools::starts_with(input(), escapeMapping.to);
                        });

                        if (it == format_t::StringEscapeMapping.end())
                        {
                            throw makeTokenError("<valid-escape-sequence>");
                        }

                        const auto& [from, to] = *it;
                        token += from;
                        inputAdvance(to.size());
                    }
                    else
                    {
                        token += input().front();
                        inputAdvance(1);
                    }
                }

                throw makeTokenError(format_t::StringDelimiter);
            }
        }

        std::string_view readQuotedString()
        {
            if (m_nesting.empty())
            {
                consumeWhitespace();
                std::string_view token{ inputData(), inputAvailable() };
                inputAdvance(input().size());
                finalizeRead();

                return token;
            }
            else
            {
                constexpr std::string_view Delimiter = format_t::StringDelimiter;

                consumeTokenAfterWhitespace(Delimiter);
                std::string_view tokenInput = input();

                while (!tokenInput.empty())
                {
                    if (tools::starts_with(tokenInput, Delimiter))
                    {
                        std::string_view value = input().substr(0, static_cast<size_t>(tokenInput.data() - input().data()));
                        tokenInput.remove_prefix(Delimiter.size());
                        inputAdvance(tokenInput.data() - input().data());
                        finalizeRead();

                        return value;
                    }

                    tokenInput.remove_prefix(1);
                }

                throw makeTokenError(Delimiter);
            }
        }

        std::string_view readIdentifierString()
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

            auto it = std::find_if_not(input().begin(), input().end(), [](char c){ return IdentifierSymbols[static_cast<size_t>(c)]; });
            size_t identifierSize = it == input().end() ? input().size() : std::distance(input().begin(), it);
            std::string_view identifier = input().substr(0, identifierSize);
            inputAdvance(identifierSize);

            finalizeRead();

            return identifier;
        }

        template <typename T, typename... Ts, std::enable_if_t<std::conjunction_v<std::is_constructible<std::string, Ts>...>, int> = 0>
        std::string_view readIdentifierString(T&& prefixHead, Ts&&... prefixTail)
        {
            consumeWhitespace();

            if (!m_nesting.empty() && m_options.policy == TextOptions::Strict)
            {
                consumeToken(std::forward<T>(prefixHead));
                (consumeToken(std::forward<Ts>(prefixTail)), ...);
            }
            else
            {
                if (tryConsumeToken(std::forward<T>(prefixHead)))
                {
                    (consumeToken(std::forward<Ts>(prefixTail)), ...);
                }
            }

            return readIdentifierString();
        }

    private:

        enum struct State : uint8_t
        {
            Array,
            Object
        };

        using nesting_t = std::pair<State, bool>;

        template <typename T, std::enable_if_t<std::is_same_v<std::decay_t<T>, bool>, int> = 0>
        void read(T& value)
        {
            consumeWhitespace();

            if constexpr (format_t::BooleanFormat == TextFormat::BooleanFormat::Integer)
            {
                if (tryConsumeToken("1"))
                {
                    value = true;
                    return;
                }
                else if (tryConsumeToken("0"))
                {
                    value = false;
                    return;
                }
            }
            else if constexpr (format_t::BooleanFormat == TextFormat::BooleanFormat::Literal)
            {
                if (tryConsumeToken("true"))
                {
                    value = true;
                    return;
                }
                else if (tryConsumeToken("false"))
                {
                    value = false;
                    return;
                }
            }
            else if constexpr (format_t::BooleanFormat == TextFormat::BooleanFormat::String)
            {
                std::string_view token = readQuotedString();

                if (token == "true")
                {
                    value = true;
                    return;
                }
                else if (token == "false")
                {
                    value = false;
                    return;
                }
            }
            else
            {
                static_assert(tools::always_false_v<T>, "unsupported boolean format");
            }

            throw makeTokenError("<boolean-string>");
        }

        template <typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, bool> && std::is_integral_v<T>, int> = 0>
        void read(T& value)
        {
            int base = 10;
            consumeWhitespace();
            
            {
                if (tryConsumeToken("0b"))
                {
                    base = 2;
                }
                else if (tryConsumeToken("0x"))
                {
                    base = 16;
                }
            }

            std::from_chars_result result = std::from_chars(input().data(), input().data() + input().size(), value, base);

            if (result.ec != std::errc{})
            {
                throw makeTokenError("<integer-value>");
            }

            inputAdvance(static_cast<size_t>(result.ptr - input().data()));

            if constexpr (format_t::IntegerFormat == TextFormat::IntegerFormat::WithSignSuffix && std::is_unsigned_v<T>)
            {
                if (!m_nesting.empty() && m_options.policy == TextOptions::Strict)
                {
                    consumeToken("u");
                }
                else
                {
                    tryConsumeToken("u");
                }
            }
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
                consumeWhitespace();
                std::from_chars_result result = std::from_chars(input().data(), input().data() + input().size(), value);

                if (result.ec != std::errc{})
                {
                    throw makeTokenError("<floating-point-value>");
                }

                inputAdvance(static_cast<size_t>(result.ptr - input().data()));
            }
            else
            {
                char* end;

                if constexpr (std::is_same_v<T, float>)
                {
                    value = std::strtof(input().data(), &end);
                }
                else/* if constexpr (std::is_same_v<T, double>)*/
                {
                    value = std::strtod(input().data(), &end);
                }

                if (end == input().data())
                {
                    throw makeTokenError("<floating point value>");
                }

                inputAdvance(static_cast<size_t>(end - input().data()));
            }

            if constexpr (format_t::FloatFormat == TextFormat::FloatFormat::WithSizeSuffix && std::is_same_v<T, float>)
            {
                if (!m_nesting.empty() && m_options.policy == TextOptions::Strict)
                {
                    consumeToken("f");
                }
                else
                {
                    tryConsumeToken("f");
                }
            }
        }

        void finalizeRead()
        {
            if (!m_nesting.empty())
            {
                if (auto& [state, first] = m_nesting.top(); state == State::Array)
                {
                    tryConsumeTokenAfterWhitespace(format_t::ArrayElementSeparator);
                }
                else if (state == State::Object)
                {
                    tryConsumeTokenAfterWhitespace(format_t::ObjectMemberValueEnd);
                }
            }

            inputData() += input().data() - inputData();
        }

        void consumeWhitespace()
        {
            while (!input().empty() && input().front() < '!')
            {
                inputAdvance(1);
            }
        }

        void consumeToken(std::string_view token)
        {
            if (!tryConsumeToken(token))
            {
                throw makeTokenError(token);
            }
        }

        void consumeTokenAfterWhitespace(std::string_view token)
        {
            consumeWhitespace();
            consumeToken(token);
        }

        bool tryConsumeToken(std::string_view token)
        {
            if (tools::starts_with(input(), token))
            {
                inputAdvance(token.size());
                return true;
            }
            else
            {
                return false;
            }
        }

        bool tryConsumeTokenAfterWhitespace(std::string_view token)
        {
            consumeWhitespace();
            return tryConsumeToken(token);
        }

        template <size_t N>
        std::runtime_error makeTokenError(std::array<std::string_view, N> expected)
        {
            ptrdiff_t inputPos = inputData() - inputDataBegin();
            std::string what = "encountered unexpected token at " + std::to_string(inputPos) + ": expected '";

            for (std::string_view token : expected)
            {
                what += token;
                what += "' or '";
            }

            what.resize(what.size() - 5);
            what += " but got '";
            what += input().substr(0, std::min<size_t>(input().size(), 10));
            what += '\'';

            return std::runtime_error{ what };
        }

        void inputAdvance(size_t n)
        {
            inputData() += n;
        }

        std::string_view input() const
        {
            return std::string_view{ inputData(), inputAvailable() };
        }

        std::runtime_error makeTokenError(std::string_view expected)
        {
            return makeTokenError(std::array{ expected });
        }

        std::stack<nesting_t> m_nesting;
        TextOptions m_options;
    };
}
