// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <dots/serialization/TextSerializer.h>
#include <dots/serialization/formats/StringReader.h>
#include <dots/serialization/formats/StringWriter.h>

namespace dots::serialization
{
    struct StringSerializerFormat : TextSerializerFormat
    {
        using reader_t = StringReader;
        using writer_t = StringWriter;

        static constexpr PropertySetFormat PropertySetFormat = PropertySetFormat::BinaryValue;
        static constexpr TimepointFormat TimepointFormat = TimepointFormat::ISO8601String;
        static constexpr EnumFormat EnumFormat = EnumFormat::Identifier;
    };

    struct StringSerializer : TextSerializer<StringSerializerFormat, StringSerializer>
    {
        using base_t = TextSerializer<StringSerializerFormat, StringSerializer>;
        using data_t = std::string;

        using base_t::base_t;
    };
}

namespace dots
{
    template <typename T, std::enable_if_t<std::is_base_of_v<type::Struct, T>, int> = 0>
    std::string to_string(const T& instance, const property_set_t& includedProperties, serialization::TextOptions options = {})
    {
        return serialization::StringSerializer::Serialize(instance, includedProperties, options);
    }

    template <typename T>
    std::string to_string(const T& value, const type::Descriptor<T>& descriptor, serialization::TextOptions options = {})
    {
        return serialization::StringSerializer::Serialize(value, descriptor, options);
    }

    template <typename T>
    std::string to_string(const T& value, serialization::TextOptions options = {})
    {
        return serialization::StringSerializer::Serialize(value, options);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_string(const std::string& data, T& value, const type::Descriptor<T>& descriptor, serialization::TextOptions options = {})
    {
        return serialization::StringSerializer::Deserialize(data, value, descriptor, options);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    size_t from_string(const std::string& data, T& value, serialization::TextOptions options = {})
    {
        return serialization::StringSerializer::Deserialize(data, value, options);
    }

    template <typename T, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    T from_string(const std::string& data, serialization::TextOptions options = {})
    {
        return serialization::StringSerializer::Deserialize<T>(data, options);
    }
}
