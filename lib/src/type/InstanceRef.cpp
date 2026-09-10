// SPDX-License-Identifier: LGPL-3.0-only
#include <dots/type/InstanceRef.h>
#include <dots/serialization/formats/CborWriter.h>
#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/writer.h>
#include <charconv>
#include <limits>

namespace dots::type
{
    namespace
    {
        void validateText(const uint8_t* data, size_t size)
        {
            for (size_t i = 0; i < size;)
            {
                uint32_t code = data[i++];
                if (code < 0x80) continue;
                unsigned continuation;
                uint32_t minimum;
                if (code >= 0xc2 && code <= 0xdf) { continuation = 1; minimum = 0x80; code &= 0x1f; }
                else if (code >= 0xe0 && code <= 0xef) { continuation = 2; minimum = 0x800; code &= 0x0f; }
                else if (code >= 0xf0 && code <= 0xf4) { continuation = 3; minimum = 0x10000; code &= 0x07; }
                else throw std::invalid_argument{"invalid UTF-8 in instance reference"};
                if (continuation > size - i) throw std::invalid_argument{"truncated UTF-8 in instance reference"};
                while (continuation--)
                {
                    auto byte = data[i++];
                    if ((byte & 0xc0) != 0x80) throw std::invalid_argument{"invalid UTF-8 continuation"};
                    code = (code << 6) | (byte & 0x3f);
                }
                if (code < minimum || code > 0x10ffff || (code >= 0xd800 && code <= 0xdfff))
                    throw std::invalid_argument{"invalid UTF-8 code point"};
            }
        }

        std::string quote(std::string_view text)
        {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer{buffer};
            writer.String(text.data(), static_cast<rapidjson::SizeType>(text.size()));
            return {buffer.GetString(), buffer.GetSize()};
        }

        struct KeyReader
        {
            const uint8_t* data;
            size_t size;
            size_t offset = 0;
            bool allowNull;
            std::string* text = nullptr;

            uint8_t byte()
            {
                if (offset == size) throw std::invalid_argument{"truncated instance key"};
                return data[offset++];
            }

            void item(size_t depth, bool array = false)
            {
                if (depth > 64) throw std::invalid_argument{"instance key nesting exceeds 64"};
                auto head = byte();
                auto major = head >> 5;
                auto info = head & 31;
                if (array && major != 4) throw std::invalid_argument{"instance key must be an array"};
                if (major == 7 && (head == 0xf4 || head == 0xf5 || (allowNull && head == 0xf6)))
                {
                    if (text) *text += head == 0xf4 ? "false" : head == 0xf5 ? "true" : "null";
                    return;
                }
                if (major > 4 || info > 27) throw std::invalid_argument{"invalid instance key value"};
                uint64_t argument = info;
                if (info >= 24)
                {
                    unsigned count = 1u << (info - 24);
                    argument = 0;
                    for (unsigned i = 0; i < count; ++i) argument = (argument << 8) | byte();
                    uint64_t minimum = count == 1 ? 24 : uint64_t{1} << (count * 4);
                    if (argument < minimum) throw std::invalid_argument{"non-canonical instance key"};
                }
                if (text && major <= 1)
                {
                    if (major == 0) *text += std::to_string(argument);
                    else if (argument == std::numeric_limits<uint64_t>::max()) *text += "-18446744073709551616";
                    else *text += "-" + std::to_string(argument + 1);
                }
                if (major == 2 || major == 3)
                {
                    if (argument > size - offset) throw std::invalid_argument{"truncated instance key string"};
                    if (major == 3) validateText(data + offset, static_cast<size_t>(argument));
                    if (text)
                    {
                        if (major == 3) *text += quote({reinterpret_cast<const char*>(data + offset), static_cast<size_t>(argument)});
                        else
                        {
                            static constexpr char Hex[] = "0123456789abcdef";
                            *text += "h'";
                            for (size_t i = 0; i < argument; ++i)
                            {
                                auto value = data[offset + i];
                                *text += Hex[value >> 4];
                                *text += Hex[value & 15];
                            }
                            *text += "'";
                        }
                    }
                    offset += static_cast<size_t>(argument);
                }
                else if (major == 4)
                {
                    if (argument > size - offset) throw std::invalid_argument{"truncated instance key array"};
                    if (text) *text += '[';
                    for (uint64_t i = 0; i < argument; ++i)
                    {
                        if (text && i) *text += ',';
                        item(depth + 1);
                    }
                    if (text) *text += ']';
                }
            }
        };

        struct TextKeyReader
        {
            std::string_view text;
            size_t offset = 0;
            serialization::CborWriter writer;

            static bool isWhitespace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
            void whitespace() { while (offset < text.size() && isWhitespace(text[offset])) ++offset; }
            char peek() const { return offset < text.size() ? text[offset] : '\0'; }
            bool consume(char c)
            {
                whitespace();
                if (offset == text.size() || text[offset] != c) return false;
                ++offset;
                return true;
            }

            std::string string()
            {
                rapidjson::MemoryStream stream{text.data() + offset, text.size() - offset};
                rapidjson::Document document;
                document.ParseStream<rapidjson::kParseStopWhenDoneFlag | rapidjson::kParseValidateEncodingFlag>(stream);
                if (document.HasParseError() || !document.IsString()) throw std::invalid_argument{"invalid instance_ref string"};
                offset += stream.Tell();
                return {document.GetString(), document.GetStringLength()};
            }

            void item(size_t depth, bool array = false)
            {
                if (depth > 64) throw std::invalid_argument{"instance key nesting exceeds 64"};
                whitespace();
                if (array && peek() != '[') throw std::invalid_argument{"instance_ref requires a key array"};
                if (consume('['))
                {
                    auto begin = writer.output().size();
                    size_t count = 0;
                    if (!consume(']'))
                    {
                        do { item(depth + 1); ++count; } while (consume(','));
                        if (!consume(']')) throw std::invalid_argument{"unterminated instance_ref array"};
                    }
                    serialization::CborWriter head;
                    head.writeArraySize(count);
                    writer.output().insert(writer.output().begin() + begin, head.output().begin(), head.output().end());
                }
                else if (peek() == '"') writer.write(string());
                else if (text.substr(offset, 2) == "h'")
                {
                    offset += 2;
                    std::vector<uint8_t> bytes;
                    auto nibble = [](char c) -> unsigned
                    {
                        if (c >= '0' && c <= '9') return c - '0';
                        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                        throw std::invalid_argument{"invalid instance_ref byte string"};
                    };
                    while (peek() != '\'')
                    {
                        if (text.size() - offset < 2) throw std::invalid_argument{"unterminated instance_ref byte string"};
                        bytes.push_back(static_cast<uint8_t>((nibble(text[offset]) << 4) | nibble(text[offset + 1])));
                        offset += 2;
                    }
                    ++offset;
                    writer.writeByteString(bytes.data(), bytes.size());
                }
                else if (text.substr(offset, 4) == "true") { offset += 4; writer.write(true); }
                else if (text.substr(offset, 5) == "false") { offset += 5; writer.write(false); }
                else
                {
                    bool negative = consume('-');
                    auto begin = offset;
                    while (peek() >= '0' && peek() <= '9') ++offset;
                    auto digits = text.substr(begin, offset - begin);
                    if (digits.empty() || (digits.size() > 1 && digits.front() == '0'))
                        throw std::invalid_argument{"invalid instance_ref integer"};
                    uint64_t value = 0;
                    if (negative && digits == "18446744073709551616") value = std::numeric_limits<uint64_t>::max();
                    else
                    {
                        auto result = std::from_chars(digits.data(), digits.data() + digits.size(), value);
                        if (result.ec != std::errc{} || (negative && value == 0))
                            throw std::invalid_argument{"instance_ref integer out of range"};
                        if (negative) --value;
                    }
                    auto head = writer.output().size();
                    writer.write(value);
                    if (negative) writer.output()[head] |= 0x20;
                }
            }
        };
    }

    size_t instance_key_size(const uint8_t* data, size_t size, bool allowNull)
    {
        KeyReader reader{data, size, 0, allowNull};
        reader.item(0, true);
        return reader.offset;
    }

    InstanceRef::InstanceRef(std::string typeName, std::vector<uint8_t> key) :
        m_typeName(std::move(typeName)), m_key(std::move(key))
    {
        validateText(reinterpret_cast<const uint8_t*>(m_typeName.data()), m_typeName.size());
        if (m_typeName.empty()) throw std::invalid_argument{"instance_ref requires a type name"};
        if (instance_key_size(m_key.data(), m_key.size()) != m_key.size())
            throw std::invalid_argument{"trailing data in instance key"};
    }

    std::string InstanceRef::toString() const
    {
        // Normal DOTS type names are bare; quote unusual opaque names so they
        // cannot be confused with the opening key-array delimiter.
        std::string text = m_typeName.find_first_of("[\" \t\r\n") == std::string::npos ? m_typeName : quote(m_typeName);
        KeyReader reader{m_key.data(), m_key.size(), 0, false, &text};
        reader.item(0, true);
        return text;
    }

    InstanceRef InstanceRef::FromString(std::string_view text)
    {
        TextKeyReader reader{text, 0, {}};
        reader.whitespace();
        std::string name;
        if (reader.peek() == '"') name = reader.string();
        else
        {
            auto end = text.find('[', reader.offset);
            if (end == std::string_view::npos) throw std::invalid_argument{"instance_ref requires a key array"};
            name = text.substr(reader.offset, end - reader.offset);
            while (!name.empty() && TextKeyReader::isWhitespace(name.back())) name.pop_back();
            reader.offset = end;
        }
        reader.item(0, true);
        reader.whitespace();
        if (reader.offset != text.size()) throw std::invalid_argument{"trailing instance_ref text"};
        return InstanceRef{std::move(name), std::move(reader.writer.output())};
    }
}
