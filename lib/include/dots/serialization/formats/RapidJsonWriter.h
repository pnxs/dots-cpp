// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <stack>
#include <rapidjson/writer.h>

namespace dots::serialization
{
    template <typename UnderlyingWriter = rapidjson::Writer<rapidjson::StringBuffer>>
    struct RapidJsonWriter
    {
        using underlying_writer_t = UnderlyingWriter;

        RapidJsonWriter() :
            m_underlyingWriter(nullptr)
        {
            /* do nothing */
        }

        RapidJsonWriter(underlying_writer_t& writer) :
            m_underlyingWriter(&writer)
        {
            /* do nothing */
        }

        void setWriter(underlying_writer_t& writer)
        {
            m_underlyingWriter = &writer;
        }

        size_t nestingLevel() const
        {
            return m_nesting.size();
        }

        void writeArrayBegin()
        {
            initiateWrite();
            m_underlyingWriter->StartArray();
            m_nesting.emplace(State::Array);
        }

        void writeArrayEnd()
        {
            initiateWrite();
            m_nesting.pop();
            m_underlyingWriter->EndArray();
        }

        void writeObjectBegin()
        {
            initiateWrite();
            m_underlyingWriter->StartObject();
            m_nesting.emplace(State::Object);
        }

        void writeObjectEnd()
        {
            initiateWrite();
            m_nesting.pop();
            m_underlyingWriter->EndObject();
        }

        void writeObjectMemberName(std::string_view name)
        {
            write(name);
        }

        void writeNull()
        {
            initiateWrite();
            m_underlyingWriter->Null();
        }

        void write(bool value)
        {
            m_underlyingWriter->Bool(value);
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
        void write(T value)
        {
            if constexpr (std::is_signed_v<T>)
            {
                if constexpr (sizeof(T) <= sizeof(int))
                {
                    m_underlyingWriter->Int(value);
                }
                else/* if (sizeof(T) == sizeof(int64_t))*/
                {
                    m_underlyingWriter->Int64(value);
                }
            }
            else/* if constexpr (std::is_unsigned_v<T>)*/
            {
                if constexpr (sizeof(T) <= sizeof(unsigned))
                {
                    m_underlyingWriter->Uint(value);
                }
                else/* if (sizeof(T) == sizeof(uint64_t))*/
                {
                    m_underlyingWriter->Uint64(value);
                }
            }
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void write(T value)
        {
            m_underlyingWriter->Double(value);
        }
        
        void write(std::string_view sv)
        {
            m_underlyingWriter->String(sv.data(), static_cast<rapidjson::SizeType>(sv.size()));
        }

        void assertHasWriter() const
        {
            if (m_underlyingWriter == nullptr)
            {
                throw std::logic_error{ "serializer does not have RapidJSON writer" };
            }
        }

    private:

        enum struct State : uint8_t
        {
            Array,
            Object
        };

        void initiateWrite()
        {
            assertHasWriter();
        }
        
        std::stack<State> m_nesting;
        underlying_writer_t* m_underlyingWriter = nullptr;
    };
}
