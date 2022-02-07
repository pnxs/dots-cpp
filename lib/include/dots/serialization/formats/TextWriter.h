#pragma once
#include <string>
#include <stack>
#include <dots/serialization/formats/Writer.h>
#include <dots/serialization/formats/TextFormat.h>
#include <dots/tools/type_traits.h>

namespace dots::serialization
{
    template <typename Format>
    struct TextWriter : Writer<std::string>
    {
        using format_t = Format;

        TextWriter(TextOptions options = {}) :
            m_options{ options }
        {
            /* do nothing */
        }

        size_t nestingLevel() const
        {
            return m_nesting.size();
        }

        void writeArrayBegin()
        {
            writeString(format_t::ArrayBegin);
            m_nesting.emplace(State::Array, true);
        }

        void writeArrayEnd()
        {
            m_nesting.pop();
            appendSuffixedNewLine(m_nesting.size(), format_t::ArrayEnd);
        }

        void writeObjectBegin()
        {
            writeString(format_t::ObjectBegin);
            m_nesting.emplace(State::Object, true);
        }

        void writeObjectBegin(std::string_view objectTypeName)
        {
            if constexpr (format_t::ObjectFormat == TextFormat::ObjectFormat::WithTypeName)
            {
                if (m_options.style == TextOptions::SingleLine)
                {
                    if (m_nesting.empty())
                    {
                        writeString(objectTypeName);
                    }
                }
                else if (m_options.style >= TextOptions::MultiLine)
                {
                    writeString(objectTypeName);
                }
            }
            else
            {
                (void)objectTypeName;
            }

            writeObjectBegin();
        }

        void writeObjectEnd()
        {
            m_nesting.pop();
            appendSuffixedNewLine(m_nesting.size(), format_t::ObjectEnd);
        }

        void writeObjectMemberName(std::string_view name)
        {
            if (bool& first = m_nesting.top().second)
            {
                appendNewLine(m_nesting.size());
                first = false;
            }
            else
            {
                appendPrefixedNewLine(m_nesting.size(), format_t::ObjectMemberValueEnd);
            }

            append(format_t::ObjectMemberNameBegin);
            append(name);

            if (!format_t::ObjectMemberNameEnd.empty())
            {
                append(format_t::ObjectMemberNameEnd);
            }

            if constexpr (!format_t::ObjectMemberValueBegin.empty())
            {
                if (m_options.style == TextOptions::Minified)
                {
                    append(format_t::ObjectMemberValueBegin);
                }
                else
                {
                    append(" ", format_t::ObjectMemberValueBegin, " ");
                }
            }
            else if (m_options.style >= TextOptions::SingleLine)
            {
                append(" ");
            }
        }

        void writeNull()
        {
            initiateWrite();
            append(format_t::NullValue);
        }

        template <typename T, std::enable_if_t<std::is_same_v<std::decay_t<T>, bool>, int> = 0>
        void write(T value)
        {
            if constexpr (format_t::BooleanFormat == TextFormat::BooleanFormat::Integer)
            {
                write(static_cast<uint8_t>(value));
            }
            else if constexpr (format_t::BooleanFormat == TextFormat::BooleanFormat::Literal)
            {
                writeString(value ? "true" : "false");
            }
            else if constexpr (format_t::BooleanFormat == TextFormat::BooleanFormat::String)
            {
                writeQuotedString(value ? "true" : "false");
            }
            else
            {
                static_assert(tools::always_false_v<T>, "unsupported boolean format");
            }
        }

        template <typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, bool> && std::is_integral_v<T>, int> = 0>
        void write(T value, int base = 10)
        {
            initiateWrite();
            
            {
                if (base == 2)
                {
                    append("0b");
                }
                else if (base == 16)
                {
                    append("0x");
                }
            }

            char buffer[32];
            auto [last, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, base);
            append(std::string_view{ buffer, static_cast<size_t>(last - buffer) });

            if constexpr (format_t::IntegerFormat == TextFormat::IntegerFormat::WithSignSuffix && std::is_unsigned_v<T>)
            {
                if (!m_nesting.empty() && m_options.style >= TextOptions::SingleLine)
                {
                    append("u");
                }
            }
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void write(T value)
        {
            initiateWrite();

            #if (defined _MSC_VER) && (!defined __INTEL_COMPILER)
            constexpr bool HasFpToChars = true;
            #else
            constexpr bool HasFpToChars = false;
            #endif

            if (value == 0)
            {
                append("0.0");
            }
            else
            {
                if constexpr (HasFpToChars)
                {
                    char buffer[32];
                    auto [last, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
                    append(std::string_view{ buffer, static_cast<size_t>(last - buffer) });
                }
                else
                {
                    std::ostringstream oss;
                    oss << std::setprecision(std::numeric_limits<T>::digits10 + 1) << value;
                    append(oss.str());
                }
            }

            if constexpr (format_t::FloatFormat == TextFormat::FloatFormat::WithSizeSuffix && std::is_same_v<T, float>)
            {
                if (!m_nesting.empty() && m_options.style >= TextOptions::SingleLine)
                {
                    append("f");
                }
            }
        }

        template <typename... Ts, std::enable_if_t<std::conjunction_v<std::is_constructible<std::string, Ts>...>, int> = 0>
        void writeString(Ts&&... strs)
        {
            initiateWrite();
            append(std::forward<Ts>(strs)...);
        }

        template <typename T, std::enable_if_t<std::is_constructible_v<std::string, T>, int> = 0>
        void writeIdentifierString(T&& identifier)
        {
            initiateWrite();
            append(std::forward<T>(identifier));
        }

        template <typename... Ts, std::enable_if_t<std::conjunction_v<std::bool_constant<sizeof...(Ts) >= 1>, std::is_constructible<std::string, Ts>...>, int> = 0>
        void writeIdentifierString(Ts&&... prefixedIdentifier)
        {
            initiateWrite();

            if constexpr (format_t::ObjectFormat == TextFormat::ObjectFormat::WithTypeName)
            {
                if (!m_nesting.empty() && m_options.style >= TextOptions::MultiLine)
                {
                    append(std::forward<Ts>(prefixedIdentifier)...);
                    return;
                }
            }

            auto prefixedIdentifierTuple = std::forward_as_tuple(std::forward<Ts>(prefixedIdentifier)...);
            append(std::get<sizeof...(Ts) - 1>(prefixedIdentifierTuple));
        }

        void writeQuotedString(const std::string& str)
        {
            initiateWrite();

            if (m_nesting.empty())
            {
                append(str);
            }
            else
            {
                append(format_t::StringDelimiter, str, format_t::StringDelimiter);
            }
        }

        void writeEscapedString(const std::string& str)
        {
            initiateWrite();

            if (m_nesting.empty())
            {
                append(str);
            }
            else
            {
                append(format_t::StringDelimiter);

                {
                    std::string_view strRemaining = str;

                    while (!strRemaining.empty())
                    {
                        auto it = std::find_if(format_t::StringEscapeMapping.begin(), format_t::StringEscapeMapping.end(), [&strRemaining](const auto& escapeMapping)
                        {
                            return tools::starts_with(strRemaining, escapeMapping.from);
                        });

                        if (it == format_t::StringEscapeMapping.end())
                        {
                            append(strRemaining.substr(0, 1));
                            strRemaining.remove_prefix(1);
                        }
                        else
                        {
                            const auto& [from, to] = *it;
                            append(to);
                            strRemaining.remove_prefix(from.size());
                        }
                    }
                }

                append(format_t::StringDelimiter);
            }

        }

    private:

        enum struct State : uint8_t
        {
            Array,
            Object
        };

        using nesting_t = std::pair<State, bool>;

        template <typename... Ts, std::enable_if_t<std::conjunction_v<std::is_constructible<std::string, Ts>...>, int> = 0>
        void append(Ts&&... strs)
        {
            (output().operator+=(std::forward<Ts>(strs)), ...);
        }

        void initiateWrite()
        {
            if (!m_nesting.empty())
            {
                if (auto& [state, first] = m_nesting.top(); state == State::Array)
                {
                    if (first)
                    {
                        appendNewLine(m_nesting.size());
                        first = false;
                    }
                    else
                    {
                        appendPrefixedNewLine(m_nesting.size(), format_t::ArrayElementSeparator);
                    }
                }
            }
        }

        void appendNewLine(size_t level)
        {
            if (m_options.style == TextOptions::MultiLine)
            {
                append("\n");

                for (size_t i = 0; i < level * m_options.indentSize; ++i)
                {
                    append(" ");
                }
            }
            else if (m_options.style >= TextOptions::SingleLine)
            {
                append(" ");
            }
        }

        void appendPrefixedNewLine(size_t level, std::string_view prefix)
        {
            append(prefix);
            appendNewLine(level);
        }

        void appendSuffixedNewLine(size_t level, std::string_view suffix)
        {
            appendNewLine(level);
            append(suffix);
        }
        
        std::stack<nesting_t> m_nesting;
        TextOptions m_options;
    };
}
