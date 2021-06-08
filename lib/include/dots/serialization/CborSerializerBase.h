#pragma once
#include <vector>
#include <stdexcept>
#include <string>
#include <limits>
#include <cstring>
#include <dots/type/TypeVisitor.h>
#include <dots/type/StaticProperty.h>
#include <dots/serialization/SerializerBase.h>

namespace dots::serialization
{
    template <typename Derived, bool UseFloat16Zero = true>
    struct CborSerializerBase : SerializerBase<std::vector<uint8_t>, Derived>
    {
        using serializer_base_t = SerializerBase<std::vector<uint8_t>, Derived>;

        CborSerializerBase() = default;
        CborSerializerBase(const CborSerializerBase& other) = default;
        CborSerializerBase(CborSerializerBase&& other) = default;
        ~CborSerializerBase() = default;

        CborSerializerBase& operator = (const CborSerializerBase& rhs) = default;
        CborSerializerBase& operator = (CborSerializerBase&& rhs) = default;

        using serializer_base_t::output;

    protected:

        friend serializer_base_t;

        using serializer_base_t::inputData;
        using serializer_base_t::inputDataEnd;

        struct Cbor
        {
            struct MajorType
            {
                static constexpr uint8_t UnsignedInt = 0x00;
                static constexpr uint8_t SignedInt = 0x20;
                static constexpr uint8_t ByteString = 0x40;
                static constexpr uint8_t TextString = 0x60;
                static constexpr uint8_t Array = 0x80;
                static constexpr uint8_t Map = 0xA0;
                static constexpr uint8_t SimpleOrFloat = 0xE0;

                static constexpr uint8_t IndefiniteArray = Array | 31;
                static constexpr uint8_t IndefiniteMap = Map | 31;

                static constexpr uint8_t IndefiniteArrayBreak = SimpleOrFloat | 31;
                static constexpr uint8_t IndefiniteMapBreak = SimpleOrFloat | 31;

                static constexpr uint8_t Mask = 0xE0;
            };

            struct AdditionalInformation
            {
                static constexpr uint8_t MaxInplaceValue = 23;

                static constexpr uint8_t FollowingBytes1 = 24;
                static constexpr uint8_t FollowingBytes2 = 25;
                static constexpr uint8_t FollowingBytes4 = 26;
                static constexpr uint8_t FollowingBytes8 = 27;

                static constexpr uint8_t Reserved1 = 28;
                static constexpr uint8_t Reserved2 = 29;
                static constexpr uint8_t Reserved3 = 30;

                static constexpr uint8_t FollowingBytesIndefinite = 31;

                static constexpr uint8_t Mask = 0x1F;
            };

            struct SimpleValue
            {
                static constexpr uint8_t False = 20;
                static constexpr uint8_t True = 21;

                static constexpr uint8_t Null = 22;
                static constexpr uint8_t Undefined = 23;

                static constexpr uint8_t Float16 = 25;
                static constexpr uint8_t Float32 = 26;
                static constexpr uint8_t Float64 = 27;

                static constexpr uint8_t Break = 31;

                static constexpr uint8_t Mask = 0x1F;
            };
        };

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

            if (unsignedValue <= Cbor::AdditionalInformation::MaxInplaceValue)
            {
                writeByte(majorType | static_cast<uint8_t>(unsignedValue));
            }
            else
            {
                uint8_t numBytesExponent = static_cast<uint8_t>(unsignedValue > 0xFFFFFFFF) + static_cast<uint8_t>(unsignedValue > 0xFFFF) + static_cast<uint8_t>(unsignedValue > 0xFF);
                uint8_t numBytes = 1 << numBytesExponent;
                writeByte(majorType | (Cbor::AdditionalInformation::FollowingBytes1 + numBytesExponent));

                for (int16_t i = numBytes - 1; i >= 0; --i)
                {
                    writeByte(static_cast<uint8_t>(unsignedValue >> i * 8));
                }
            }
        }

        template <typename T, std::enable_if_t<std::is_unsigned_v<T> && !std::is_same_v<T, bool>, int> = 0>
        void write(T value)
        {
            writeHead(Cbor::MajorType::UnsignedInt, value);
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, int> = 0>
        void write(T value)
        {
            if (value >= 0)
            {
                writeHead(Cbor::MajorType::UnsignedInt, static_cast<uint64_t>(value));
            }
            else
            {
                writeHead(Cbor::MajorType::SignedInt, static_cast<uint64_t>(-1 - value));
            }
        }

        template <typename T, size_t N, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) == 1, int> = 0>
        void write(const T(&bytes)[N])
        {
            writeHead(Cbor::MajorType::ByteString, N);
            writeBytes(reinterpret_cast<const uint8_t*>(&bytes[0]), N);
        }

        template <typename T, size_t N, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) == 1, int> = 0>
        void write(const std::array<T, N>& bytes)
        {
            write(reinterpret_cast<const T(&)[N]>(*bytes.data()));
        }

        void write(std::string_view str)
        {
            writeHead(Cbor::MajorType::TextString, str.size());
            writeBytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());
        }

        void write(bool value)
        {
            writeHead(Cbor::MajorType::SimpleOrFloat | (value ? Cbor::SimpleValue::True : Cbor::SimpleValue::False));
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void write(T value)
        {
            static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>, "type not supported");

            if constexpr (UseFloat16Zero)
            {
                if (value == static_cast<T>(0))
                {
                    writeByte(Cbor::MajorType::SimpleOrFloat | Cbor::AdditionalInformation::FollowingBytes2);
                    writeContingentBytes(uint16_t{ 0 });
                    return;
                }
            }

            if constexpr (std::is_same_v<T, float>)
            {
                writeByte(Cbor::MajorType::SimpleOrFloat | Cbor::AdditionalInformation::FollowingBytes4);
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                writeByte(Cbor::MajorType::SimpleOrFloat | Cbor::AdditionalInformation::FollowingBytes8);
            }

            writePunnedContingentBytes(value);
        }

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
            uint8_t majorType = initialByte & Cbor::MajorType::Mask;
            uint8_t additionalInformation = initialByte & Cbor::AdditionalInformation::Mask;

            if (majorType != expectedMajorType)
            {
                throw std::runtime_error{ "encountered unexpected major type. expected '" + std::to_string(expectedMajorType) + "' but got '" + std::to_string(majorType) + "'" };
            }

            if (additionalInformation <= Cbor::AdditionalInformation::MaxInplaceValue)
            {
                return { uint8_t{ 0 }, additionalInformation };
            }
            else
            {
                if (additionalInformation > Cbor::AdditionalInformation::FollowingBytes8)
                {
                    throw std::runtime_error{ "encountered unsupported additional information value: " + std::to_string(additionalInformation) };
                }

                uint8_t numBytes = 1  << (additionalInformation - Cbor::AdditionalInformation::FollowingBytes1);

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

            if (additionalInformation <= Cbor::AdditionalInformation::MaxInplaceValue)
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
            value = readHead<T>(Cbor::MajorType::UnsignedInt);
        }

        template <typename T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, int> = 0>
        void read(T& value)
        {
            using unsigned_t = std::make_unsigned_t<T>;

            if (uint8_t majorType = *inputData() & Cbor::MajorType::Mask; majorType == Cbor::MajorType::SignedInt)
            {
                unsigned_t unsignedValue = readHead<unsigned_t>(Cbor::MajorType::SignedInt);
                value = -1 - reinterpret_cast<const T&>(unsignedValue);
            }
            else
            {
                unsigned_t unsignedValue = readHead<unsigned_t>(Cbor::MajorType::UnsignedInt);
                value = reinterpret_cast<const T&>(unsignedValue);
            }
        }

        template <typename T, size_t N, std::enable_if_t<std::is_unsigned_v<T> && sizeof(T) == 1, int> = 0>
        void read(T(&bytes)[N])
        {
            size_t size = readHead<size_t>(Cbor::MajorType::ByteString);

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
            size_t size = readHead<size_t>(Cbor::MajorType::TextString);
            assertInputAvailable(size);
            str.resize(size);
            readBytes(reinterpret_cast<uint8_t*>(str.data()), size);
        }

        template <typename T, std::enable_if_t<std::is_same_v<T, bool>, int> = 0>
        void read(T& value)
        {
            if (uint8_t simpleValue = readHead<uint8_t>(Cbor::MajorType::SimpleOrFloat); simpleValue == Cbor::SimpleValue::False)
            {
                value = false;
            }
            else if (simpleValue == Cbor::SimpleValue::True)
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
            if (auto [numBytes, additionalInformation] = readInitialByte(Cbor::MajorType::SimpleOrFloat, sizeof(T)); numBytes == sizeof(T))
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
    };
}