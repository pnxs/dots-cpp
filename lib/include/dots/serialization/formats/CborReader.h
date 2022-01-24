#pragma once
#include <vector>
#include <stdexcept>
#include <string>
#include <limits>
#include <cstring>
#include <cmath>
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
            readHead(cbor_t::MajorType::IndefiniteMapBreak);
        }

        void readMapEnd()
        {
            readHead(cbor_t::MajorType::IndefiniteMapBreak);
        }

        template <typename T, std::enable_if_t<std::is_default_constructible_v<T>, int> = 0>
        T read()
        {
            T value;
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

            if (uint8_t majorType = *inputData() & cbor_t::MajorType::Mask; majorType == cbor_t::MajorType::SignedInt)
            {
                unsigned_t unsignedValue = readHead<unsigned_t>(cbor_t::MajorType::SignedInt);
                value = -1 - reinterpret_cast<const T&>(unsignedValue);
            }
            else
            {
                unsigned_t unsignedValue = readHead<unsigned_t>(cbor_t::MajorType::UnsignedInt);
                value = reinterpret_cast<const T&>(unsignedValue);
            }
        }

        template <typename T, size_t N, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) == 1, int> = 0>
        void read(T(&bytes)[N])
        {
            size_t size = readHead<size_t>(cbor_t::MajorType::ByteString);

            if (size != N)
            {
                throw std::runtime_error{ "byte string size does not match size. expected '" + std::to_string(N) + "' but got '" + std::to_string(size) + "'" };
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
            size_t size = readHead<size_t>(cbor_t::MajorType::TextString);
            assertInputAvailable(size);
            str.resize(size);
            readBytes(reinterpret_cast<uint8_t*>(str.data()), size);
        }

        template <typename T, std::enable_if_t<std::is_same_v<T, bool>, int> = 0>
        void read(T& value)
        {
            if (uint8_t simpleValue = readHead<uint8_t>(cbor_t::MajorType::SimpleOrFloat); simpleValue == cbor_t::SimpleValue::False)
            {
                value = false;
            }
            else if (simpleValue == cbor_t::SimpleValue::True)
            {
                value = true;
            }
            else
            {
                throw std::runtime_error{ "encountered unexpected simple value when deserializing bool: '" + std::to_string(simpleValue) + "'" };
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
                        uint16_t float16 = readPunnedContingentBytes<uint16_t>();
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
                        throw std::runtime_error{ "encountered unsupported additional information value: " + std::to_string(additionalInformation) };
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

                consume(numBytes);

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

        void assertInputAvailable(size_t size)
        {
            if (size > static_cast<size_t>(inputDataEnd() - inputData()))
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

        template <typename T, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) >= 1 && sizeof(T) <= 8, int> = 0>
        T readContingentBytes()
        {
            T value = {};

            for (auto i = static_cast<ptrdiff_t>(sizeof(T) - 1); i >= 0; --i)
            {
                value |= static_cast<T>(readByte()) << i * 8;
            }

            return value;
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

        std::pair<uint8_t, uint8_t> readInitialByte(uint8_t expectedMajorType, size_t valueSize)
        {
            assertInputAvailable(1);
            uint8_t initialByte = readByte();
            uint8_t majorType = initialByte & cbor_t::MajorType::Mask;
            uint8_t additionalInformation = initialByte & cbor_t::AdditionalInformation::Mask;

            if (majorType != expectedMajorType)
            {
                throw std::runtime_error{ "encountered unexpected major type. expected '" + std::to_string(expectedMajorType) + "' but got '" + std::to_string(majorType) + "'" };
            }

            if (additionalInformation <= cbor_t::AdditionalInformation::MaxInplaceValue)
            {
                return { uint8_t{ 0 }, additionalInformation };
            }
            else
            {
                if (additionalInformation > cbor_t::AdditionalInformation::FollowingBytes8)
                {
                    throw std::runtime_error{ "encountered unsupported additional information value: " + std::to_string(additionalInformation) };
                }

                uint8_t numBytes = 1  << (additionalInformation - cbor_t::AdditionalInformation::FollowingBytes1);

                if (numBytes > valueSize)
                {
                    throw std::runtime_error{ "encountered value exceeds value size: expected at most '" + std::to_string(valueSize) + "' but got '" + std::to_string(numBytes) + "'" };
                }

                assertInputAvailable(numBytes);

                return { numBytes, additionalInformation };
            }
        }

        uint8_t readHead(uint8_t expectedHead)
        {
            assertInputAvailable(1);
            uint8_t head = readByte();

            if (head != expectedHead)
            {
                throw std::runtime_error{ "encountered unexpected head. expected '" + std::to_string(expectedHead) + "' but got '" + std::to_string(head) + "'" };
            }

            return head;
        }

        template <typename T, std::enable_if_t<std::is_unsigned_v<T>, int> = 0>
        T readHead(uint8_t expectedMajorType)
        {
            T value;
            auto [numBytes, additionalInformation] = readInitialByte(expectedMajorType, sizeof(T));

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
        }
    };
}
