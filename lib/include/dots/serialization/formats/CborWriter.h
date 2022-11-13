// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <vector>
#include <stdexcept>
#include <limits>
#include <cstring>
#include <dots/serialization/formats/Writer.h>
#include <dots/serialization/formats/CborFormat.h>

namespace dots::serialization
{
    struct CborWriter : Writer<std::vector<uint8_t>>
    {
        using cbor_t = CborFormat;

        void writeArraySize(size_t size)
        {
            writeHead(cbor_t::MajorType::Array, size);
        }

        void writeArrayBegin()
        {
            writeHead(cbor_t::MajorType::IndefiniteArray);
        }

        void writeArrayEnd()
        {
            writeHead(cbor_t::MajorType::IndefiniteArrayBreak);
        }

        void writeMapSize(size_t size)
        {
            writeHead(cbor_t::MajorType::Map, size);
        }

        void writeMapBegin()
        {
            writeHead(cbor_t::MajorType::IndefiniteMap);
        }

        void writeMapEnd()
        {
            writeHead(cbor_t::MajorType::IndefiniteMapBreak);
        }

        template <typename T, std::enable_if_t<std::is_unsigned_v<T> && !std::is_same_v<T, bool>, int> = 0>
        void write(T value)
        {
            writeHead(cbor_t::MajorType::UnsignedInt, value);
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, int> = 0>
        void write(T value)
        {
            if (value >= 0)
            {
                writeHead(cbor_t::MajorType::UnsignedInt, static_cast<uint64_t>(value));
            }
            else
            {
                writeHead(cbor_t::MajorType::SignedInt, static_cast<uint64_t>(-1 - value));
            }
        }

        template <typename T, size_t N, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) == 1, int> = 0>
        void write(const T(&bytes)[N])
        {
            writeHead(cbor_t::MajorType::ByteString, N);
            writeBytes(reinterpret_cast<const uint8_t*>(&bytes[0]), N);
        }

        template <typename T, size_t N, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) == 1, int> = 0>
        void write(const std::array<T, N>& bytes)
        {
            write(reinterpret_cast<const T(&)[N]>(*bytes.data()));
        }

        void write(std::string_view str)
        {
            writeHead(cbor_t::MajorType::TextString, str.size());
            writeBytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());
        }

        void write(bool value)
        {
            writeHead(cbor_t::MajorType::SimpleOrFloat | (value ? cbor_t::SimpleValue::True : cbor_t::SimpleValue::False));
        }

        enum struct FloatFormat : uint8_t
        {
            NativeSizeOnly,
            UseFloat16Zeroes
        };

        template <FloatFormat Format = FloatFormat::NativeSizeOnly, typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void write(T value)
        {
            static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>, "type not supported");

            if constexpr (Format == FloatFormat::UseFloat16Zeroes)
            {
                if (value == static_cast<T>(0))
                {
                    writeByte(cbor_t::MajorType::SimpleOrFloat | cbor_t::AdditionalInformation::FollowingBytes2);
                    writeContingentBytes(uint16_t{ 0 });
                    return;
                }
            }

            if constexpr (std::is_same_v<T, float>)
            {
                writeByte(cbor_t::MajorType::SimpleOrFloat | cbor_t::AdditionalInformation::FollowingBytes4);
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                writeByte(cbor_t::MajorType::SimpleOrFloat | cbor_t::AdditionalInformation::FollowingBytes8);
            }

            writePunnedContingentBytes(value);
        }

    private:

        uint8_t* ensureOutputAvailable(size_t size)
        {
            size_t previousSize = output().size();
            output().resize(output().size() + size);

            return output().data() + previousSize;
        }

        void writeByte(uint8_t byte)
        {
            output().emplace_back(byte);
        }

        void writeBytes(const uint8_t* begin, size_t size)
        {
            std::memcpy(ensureOutputAvailable(size), begin, size);
        }

        template <typename T, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) >= 1 && sizeof(T) <= 8, int> = 0>
        void writeContingentBytes(T value)
        {
            uint8_t* outputData = ensureOutputAvailable(sizeof(T));

            for (auto i = static_cast<ptrdiff_t>(sizeof(T) - 1); i >= 0; --i)
            {
                *outputData++ = static_cast<uint8_t>(value >> i * 8);
            }
        }

        template <typename T, std::enable_if_t<sizeof(T) >= 2 && sizeof(T) <= 8, int> = 0>
        void writePunnedContingentBytes(T value)
        {
            using integral_storage_t = std::conditional_t<sizeof(T) == 2, uint16_t, std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>>;
            integral_storage_t integralValue;
            std::memcpy(&integralValue, &value, sizeof(value));
            writeContingentBytes(integralValue);
        }

        void writeHead(uint8_t head)
        {
            writeByte(head);
        }

        template <typename T, std::enable_if_t<std::is_unsigned_v<T>, int> = 0>
        void writeHead(uint8_t majorType, T value)
        {
            uint64_t unsignedValue = static_cast<uint64_t>(value);

            if (unsignedValue <= cbor_t::AdditionalInformation::MaxInplaceValue)
            {
                writeByte(majorType | static_cast<uint8_t>(unsignedValue));
            }
            else
            {
                uint8_t numBytesExponent = static_cast<uint8_t>(unsignedValue > 0xFFFFFFFF) + static_cast<uint8_t>(unsignedValue > 0xFFFF) + static_cast<uint8_t>(unsignedValue > 0xFF);
                uint8_t numBytes = 1 << numBytesExponent;
                writeByte(majorType | (cbor_t::AdditionalInformation::FollowingBytes1 + numBytesExponent));

                for (int16_t i = numBytes - 1; i >= 0; --i)
                {
                    writeByte(static_cast<uint8_t>(unsignedValue >> i * 8));
                }
            }
        }
    };
}
