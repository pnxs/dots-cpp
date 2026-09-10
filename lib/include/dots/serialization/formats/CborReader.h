// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <vector>
#include <stdexcept>
#include <string>
#include <limits>
#include <cstring>
#include <cmath>
#include <array>
#include <bit>
#include <dots/serialization/formats/Reader.h>
#include <dots/serialization/formats/CborFormat.h>

namespace dots::serialization
{
    struct CborReader : Reader<std::vector<uint8_t>>
    {
        using cbor_t = CborFormat;

        size_t readArraySize()
        {
            return readHead<size_t>(cbor_t::MajorType::Array);
        }

        void readArrayBegin()
        {
            readHead(cbor_t::MajorType::IndefiniteArray);
        }

        void readArrayEnd()
        {
            readHead(cbor_t::MajorType::IndefiniteArrayBreak);
        }

        size_t readMapSize()
        {
            return readHead<size_t>(cbor_t::MajorType::Map);
        }

        void readMapBegin()
        {
            readHead(cbor_t::MajorType::IndefiniteMap);
        }

        void readMapEnd()
        {
            readHead(cbor_t::MajorType::IndefiniteMapBreak);
        }

        template <typename T, std::enable_if_t<std::is_default_constructible_v<T>, int> = 0>
        T read()
        {
            T value = {}; // "= {}" is a workaround for a bug in GCC 11.4, which triggers a false warning (-Werror=maybe-uninitialized)
            read(value);

            return value;
        }

        template <typename T, std::enable_if_t<std::is_unsigned_v<T> && !std::is_same_v<T, bool>, int> = 0>
        void read(T& value)
        {
            value = readHead<T>(cbor_t::MajorType::UnsignedInt);
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, int> = 0>
        void read(T& value)
        {
            using unsigned_t = std::make_unsigned_t<T>;

            assertInputAvailable(1);
            uint8_t initialByte = *inputData();
            uint8_t majorType = initialByte & cbor_t::MajorType::Mask;

            if (majorType != cbor_t::MajorType::SignedInt && majorType != cbor_t::MajorType::UnsignedInt) [[unlikely]]
            {
                throwUnexpectedMajorTypeException(cbor_t::MajorType::UnsignedInt, majorType);
            }

            auto [numBytes, additionalInformation] = consumeInitialByte(initialByte, sizeof(unsigned_t));
            unsigned_t unsignedValue = decodeHeadTail<unsigned_t>(numBytes, additionalInformation);

            if (majorType == cbor_t::MajorType::SignedInt)
            {
                value = -1 - reinterpret_cast<const T&>(unsignedValue);
            }
            else
            {
                value = reinterpret_cast<const T&>(unsignedValue);
            }
        }

        template <typename T, size_t N, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) == 1, int> = 0>
        void read(T(&bytes)[N])
        {
            auto size = readHead<size_t>(cbor_t::MajorType::ByteString);

            if (size != N)
            {
                throwInvalidSize("byte string size does not match size", N, size);
            }

            assertInputAvailable(N);
            readBytes(reinterpret_cast<uint8_t*>(&bytes[0]), size);
        }

        template <typename T, size_t N, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) == 1, int> = 0>
        void read(std::array<T, N>& bytes)
        {
            read(reinterpret_cast<T(&)[N]>(*bytes.data()));
        }

        void read(std::string& str)
        {
            auto size = readHead<size_t>(cbor_t::MajorType::TextString);
            assertInputAvailable(size);
#ifdef __cpp_lib_string_resize_and_overwrite
            const uint8_t* src = inputData();
            str.resize_and_overwrite(size, [src, size](char* dst, size_t) {
                std::memcpy(dst, src, size);
                return size;
            });
            inputData() += size;
#else
            str.resize(size);
            readBytes(reinterpret_cast<uint8_t*>(str.data()), size);
#endif
        }

        template <typename T, std::enable_if_t<std::is_same_v<T, bool>, int> = 0>
        void read(T& value)
        {
            if (auto simpleValue = readHead<uint8_t>(cbor_t::MajorType::SimpleOrFloat); simpleValue == cbor_t::SimpleValue::False)
            {
                value = false;
            }
            else if (simpleValue == cbor_t::SimpleValue::True)
            {
                value = true;
            }
            else
            {
                throwException("encountered unexpected simple value when deserializing bool", simpleValue);
            }
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void read(T& value)
        {
            if (auto [numBytes, additionalInformation] = readInitialByte(cbor_t::MajorType::SimpleOrFloat, sizeof(T)); numBytes == sizeof(T))
            {
                value = readPunnedContingentBytes<T>();
            }
            else/* if (numBytes < sizeof(T))*/
            {
                if (numBytes == sizeof(uint16_t))
                {
                    if (reinterpret_cast<const uint16_t&>(*inputData()) == 0)
                    {
                        value = T{ 0 };
                        inputData() += sizeof(uint16_t);
                    }
                    else
                    {
                        auto float16 = readPunnedContingentBytes<uint16_t>();
                        uint16_t sign = float16 & 0x8000;
                        uint16_t exponent = (float16 >> 10) & 0x1F;
                        uint16_t mantissa = float16 & 0x3FF;
                        double float64;

                        if (exponent == 0)
                        {
                            float64 = std::ldexp(mantissa, -24);
                        }
                        else if (exponent != 31)
                        {
                            float64 = std::ldexp(mantissa + 1024, exponent - 25);
                        }
                        else
                        {
                            float64 = mantissa == 0 ? std::numeric_limits<double>::infinity() : std::numeric_limits<double>::quiet_NaN();
                        }

                        value = static_cast<T>(static_cast<bool>(sign) ? -float64 : float64);
                    }
                }
                else/* if (numBytes == sizeof(float))*/
                {
                    value = static_cast<T>(readPunnedContingentBytes<float>());
                }
            }
        }

        void readByteString(std::vector<uint8_t>& out)
        {
            auto size = readHead<size_t>(cbor_t::MajorType::ByteString);
            assertInputAvailable(size);
            out.resize(size);
            readBytes(out.data(), size);
        }

        void skip()
        {
            assertInputAvailable(1);
            uint8_t initialByte = readByte();
            uint8_t majorType = initialByte & cbor_t::MajorType::Mask;
            uint8_t additionalInformation = initialByte & cbor_t::AdditionalInformation::Mask;

            auto any_of = [majorType](auto... majorTypes)
            {
                return (... || (majorType == majorTypes));
            };

            auto consume = [&](size_t numBytes)
            {
                assertInputAvailable(numBytes);
                inputData() += numBytes;
            };

            auto get_num_bytes = [&]() -> uint8_t
            {
                if (additionalInformation <= cbor_t::AdditionalInformation::MaxInplaceValue)
                {
                    return 0;
                }
                else
                {
                    if (additionalInformation > cbor_t::AdditionalInformation::FollowingBytes8)
                    {
                        throwException("encountered unsupported additional information value", additionalInformation);
                    }

                    uint8_t numBytes = 1 << (additionalInformation - cbor_t::AdditionalInformation::FollowingBytes1);
                    return numBytes;
                }
            };

            auto read_size = [&]
            {
                size_t value;
                uint8_t numBytes = get_num_bytes();

                if (additionalInformation <= cbor_t::AdditionalInformation::MaxInplaceValue)
                {
                    value = additionalInformation;
                }
                else
                {
                    value = 0;

                    for (int16_t i = numBytes - 1; i >= 0; --i)
                    {
                        value |= static_cast<uint64_t>(readByte()) << i * 8;
                    }
                }

                return value;
            };

            if (any_of(cbor_t::MajorType::UnsignedInt, cbor_t::MajorType::SignedInt, cbor_t::MajorType::SimpleOrFloat))
            {
                size_t numBytes = get_num_bytes();
                consume(numBytes);
            }
            else if (any_of(cbor_t::MajorType::ByteString, cbor_t::MajorType::TextString))
            {
                size_t size = read_size();
                consume(size);
            }
            else if (any_of(cbor_t::MajorType::Array))
            {
                size_t size = read_size();

                if (size > 0)
                {
                    while (size--)
                    {
                        skip();
                    }
                }
            }
            else if (any_of(cbor_t::MajorType::Map))
            {
                size_t size = read_size();

                if (size > 0)
                {
                    while (size--)
                    {
                        skip();
                        skip();
                    }
                }
            }
        }

    private:

        void throwInvalidSize(const std::string& msg, std::size_t expected, std::size_t size) const;
        void throwUnexpectedHeadException(uint8_t expectedHead, uint8_t head) const;
        void throwUnexpectedMajorTypeException(uint8_t expectedMajorType, uint8_t majorType) const;
        void throwException(const std::string& msg, int value) const;
        void throwException(const std::string& msg, const std::string& details) const;

        void assertInputAvailable(size_t size)
        {
            if (size > static_cast<size_t>(inputDataEnd() - inputData())) [[unlikely]]
            {
                throw std::runtime_error{ "out of data" };
            }
        }

        uint8_t readByte()
        {
            return *inputData()++;
        }

        void readBytes(uint8_t* begin, size_t size)
        {
            std::memcpy(begin, inputData(), size);
            inputData() += size;
        }

        template <size_t N>
        uint64_t loadBigEndian()
        {
            uint64_t result;
            if constexpr (N == 1)
            {
                result = *inputData();
            }
            else if constexpr (N == 2)
            {
                uint16_t v;
                std::memcpy(&v, inputData(), 2);
                if constexpr (std::endian::native == std::endian::little) v = __builtin_bswap16(v);
                result = v;
            }
            else if constexpr (N == 4)
            {
                uint32_t v;
                std::memcpy(&v, inputData(), 4);
                if constexpr (std::endian::native == std::endian::little) v = __builtin_bswap32(v);
                result = v;
            }
            else
            {
                static_assert(N == 8);
                uint64_t v;
                std::memcpy(&v, inputData(), 8);
                if constexpr (std::endian::native == std::endian::little) v = __builtin_bswap64(v);
                result = v;
            }
            inputData() += N;
            return result;
        }

        template <typename T, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) >= 1 && sizeof(T) <= 8, int> = 0>
        T readContingentBytes()
        {
            return static_cast<T>(loadBigEndian<sizeof(T)>());
        }

        template <typename T, std::enable_if_t<sizeof(T) >= 2 && sizeof(T) <= 8, int> = 0>
        T readPunnedContingentBytes()
        {
            using integral_storage_t = std::conditional_t<sizeof(T) == 2, uint16_t, std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>>;
            auto integralValue = readContingentBytes<integral_storage_t>();

            T value;
            std::memcpy(&value, &integralValue, sizeof(value));

            return value;
        }

        // Decodes an already-peeked initial byte: consumes it, validates the
        // following-bytes count, and ensures the tail is available. The caller is
        // responsible for validating the major type.
        std::pair<uint8_t, uint8_t> consumeInitialByte(uint8_t initialByte, size_t valueSize)
        {
            ++inputData();
            uint8_t additionalInformation = initialByte & cbor_t::AdditionalInformation::Mask;

            if (additionalInformation <= cbor_t::AdditionalInformation::MaxInplaceValue) [[likely]]
            {
                return { uint8_t{ 0 }, additionalInformation };
            }

            if (additionalInformation > cbor_t::AdditionalInformation::FollowingBytes8) [[unlikely]]
            {
                throwException("encountered unsupported additional information", additionalInformation);
            }

            auto numBytes = static_cast<uint8_t>(1u << (additionalInformation - cbor_t::AdditionalInformation::FollowingBytes1));

            if (numBytes > valueSize) [[unlikely]]
            {
                throwInvalidSize("encountered value exceeds value size", valueSize, numBytes);
            }

            assertInputAvailable(numBytes);
            return { numBytes, additionalInformation };
        }

        std::pair<uint8_t, uint8_t> readInitialByte(uint8_t expectedMajorType, size_t valueSize)
        {
            assertInputAvailable(1);
            uint8_t initialByte = *inputData();
            uint8_t majorType = initialByte & cbor_t::MajorType::Mask;

            if (majorType != expectedMajorType) [[unlikely]]
            {
                throwUnexpectedMajorTypeException(expectedMajorType, majorType);
            }

            return consumeInitialByte(initialByte, valueSize);
        }

        template <typename T, std::enable_if_t<std::is_unsigned_v<T>, int> = 0>
        T decodeHeadTail(uint8_t numBytes, uint8_t additionalInformation)
        {
            if (additionalInformation <= cbor_t::AdditionalInformation::MaxInplaceValue) [[likely]]
            {
                return static_cast<T>(additionalInformation);
            }

            switch (numBytes)
            {
                case 1: return static_cast<T>(loadBigEndian<1>());
                case 2: return static_cast<T>(loadBigEndian<2>());
                case 4: return static_cast<T>(loadBigEndian<4>());
                default: return static_cast<T>(loadBigEndian<8>());
            }
        }

        uint8_t readHead(uint8_t expectedHead)
        {
            assertInputAvailable(1);
            uint8_t head = readByte();

            if (head != expectedHead) [[unlikely]]
            {
                throwUnexpectedHeadException(expectedHead, head);
            }

            return head;
        }

        template <typename T, std::enable_if_t<std::is_unsigned_v<T>, int> = 0>
        T readHead(uint8_t expectedMajorType)
        {
            auto [numBytes, additionalInformation] = readInitialByte(expectedMajorType, sizeof(T));
            return decodeHeadTail<T>(numBytes, additionalInformation);
        }
    };
}
