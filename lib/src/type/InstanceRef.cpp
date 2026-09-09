// SPDX-License-Identifier: LGPL-3.0-only
#include <dots/type/InstanceRef.h>

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

        struct KeyReader
        {
            const uint8_t* data;
            size_t size;
            size_t offset = 0;
            bool allowNull;

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
                if (major == 7 && (head == 0xf4 || head == 0xf5 || (allowNull && head == 0xf6))) return;
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
                if (major == 2 || major == 3)
                {
                    if (argument > size - offset) throw std::invalid_argument{"truncated instance key string"};
                    if (major == 3) validateText(data + offset, static_cast<size_t>(argument));
                    offset += static_cast<size_t>(argument);
                }
                else if (major == 4)
                {
                    if (argument > size - offset) throw std::invalid_argument{"truncated instance key array"};
                    for (uint64_t i = 0; i < argument; ++i) item(depth + 1);
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

    InstanceRef InstanceRef::FromString(std::string_view text)
    {
        auto separator = text.rfind('#');
        if (separator == std::string_view::npos || (text.size() - separator - 1) % 2 != 0)
            throw std::invalid_argument{"invalid instance_ref text"};
        for (char c : text.substr(separator + 1))
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
                throw std::invalid_argument{"invalid instance_ref hex digit"};
        auto any = AnyObject::FromString(text);
        return InstanceRef{std::string{any.typeName()}, any.payload()};
    }
}
