// SPDX-License-Identifier: LGPL-3.0-only
#pragma once
#include <dots/type/InstanceRef.h>
#include <dots/serialization/formats/CborReader.h>
#include <dots/serialization/formats/CborWriter.h>

namespace dots::serialization
{
    inline void validate_instance_ref(const type::InstanceRef& value, const type::Descriptor<type::InstanceRef>& descriptor)
    {
        if (value.empty()) throw std::invalid_argument{"cannot serialize an empty instance_ref"};
        if (!descriptor.targetTypeName().empty() && descriptor.targetTypeName() != value.typeName())
            throw std::invalid_argument{"instance_ref target type mismatch"};
    }

    inline void write_instance_ref(CborWriter& writer, const type::InstanceRef& value, const type::Descriptor<type::InstanceRef>& descriptor)
    {
        validate_instance_ref(value, descriptor);
        if (descriptor.targetTypeName().empty())
        {
            writer.writeArraySize(2);
            writer.write(value.typeName());
        }
        writer.output().insert(writer.output().end(), value.key().begin(), value.key().end());
    }

    inline type::InstanceRef read_instance_ref(CborReader& reader, const type::Descriptor<type::InstanceRef>& descriptor)
    {
        std::string name = descriptor.targetTypeName();
        if (name.empty())
        {
            // Validate the whole envelope under the deterministic profile too.
            type::instance_key_size(reader.inputData(), reader.inputAvailable());
            if (reader.readArraySize() != 2) throw std::invalid_argument{"instance_ref must be an array of 2"};
            reader.read(name);
        }
        size_t size = type::instance_key_size(reader.inputData(), reader.inputAvailable());
        std::vector<uint8_t> key(reader.inputData(), reader.inputData() + size);
        reader.inputData() += size;
        return type::InstanceRef{std::move(name), std::move(key)};
    }
}
