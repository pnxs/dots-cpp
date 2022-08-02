// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <cstdint>

namespace dots::serialization
{
    struct CborFormat
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
}
