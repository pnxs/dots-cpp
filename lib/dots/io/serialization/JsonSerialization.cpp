#include "JsonSerialization.h"

#include "StructDescriptorData.dots.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <dots/tools/logging.h>

#include <iostream>
#include <algorithm>
#include <cstddef>

using namespace rapidjson;

namespace dots {

template<class WRITER>
static void write_array_to_json(const type::VectorDescriptor& vd, const type::Vector<>& data, WRITER& writer);

template<class WRITER>
static void to_json_recursive(const type::Struct& instance, types::property_set_t what,
                              WRITER& writer, bool allFields);

static void from_json_recursive(const type::StructDescriptor<>& sd, type::Struct& instance, const rapidjson::Document::ConstObject& object);
static void read_from_json_array_recursive(const type::VectorDescriptor& vd, type::Vector<>& data, const rapidjson::Document::ValueType& value);

template<class WRITER>
static void write_atomic_types_to_json(const type::Descriptor<>& td, const type::Typeless& typeless, WRITER& writer)
{
    //std::cout << "write atomic is_arithmetic:" << t.is_arithmetic() << " is_enum:" << t.is_enumeration() << " t:" << t.get_name() << "\n";
    //std::cout << "var ptr: " << var.get_ptr() << " type=" << var.get_type().get_name() << "\n";

    const void* data = &typeless;

    switch (td.dotsType())
    {
        case type::DotsType::int8: writer.Int(*(const int8_t *) data);
            break;
        case type::DotsType::int16: writer.Int(*(const int16_t *) data);
            break;
        case type::DotsType::int32: writer.Int(*(const int32_t *) data);
            break;
        case type::DotsType::int64: writer.Int64(*(const long long *) data);
            break;
        case type::DotsType::uint8: writer.Uint(*(const uint8_t *) data);
            break;
        case type::DotsType::uint16: writer.Uint(*(const uint16_t *) data);
            break;
        case type::DotsType::uint32: writer.Uint(*(const uint32_t *) data);
            break;
        case type::DotsType::uint64: writer.Uint64(*(const unsigned long long *) data);
            break;
        case type::DotsType::boolean: writer.Bool(*(const bool *) data);
            break;
        case type::DotsType::float32:
            writer.Double(*(const float *) data);
            break;
        case type::DotsType::float64:
            writer.Double(*(const double *) data);
            break;
        case type::DotsType::string:
            writer.String(*(const std::string *) data);
            break;
        case type::DotsType::property_set: writer.Uint(((const dots::types::property_set_t *) data)->toValue());
            break;
        case type::DotsType::timepoint: writer.Double(((const type::TimePoint *) data)->duration().toFractionalSeconds());
            break;
        case type::DotsType::steady_timepoint:writer.Double(((const type::SteadyTimePoint *) data)->duration().toFractionalSeconds());
            break;
        case type::DotsType::duration: writer.Double(((const type::Duration *) data)->toFractionalSeconds());
            break;
        case type::DotsType::uuid: writer.String(((const dots::types::uuid_t *) data)->toString());
            break;
        case type::DotsType::Enum:
        {
            writer.Int(static_cast<const type::EnumDescriptor<>&>(td).enumeratorFromValue(typeless).tag());
        }
            break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("unknown type: " + td.name());
    }
}

template<class WRITER>
static inline void write_json(const type::Descriptor<>& td, const type::Typeless& data, WRITER& writer)
{
    if (td.dotsType() == type::DotsType::Vector)
    {
        write_array_to_json(static_cast<const type::VectorDescriptor&>(td), data.to<const type::Vector<>>(), writer);
    } else if (isDotsBaseType(td.dotsType()))
    {
        write_atomic_types_to_json(td, data, writer);
    } else if (td.dotsType() == type::DotsType::Struct) // object
    {
        to_json_recursive(data.to<const type::Struct>(), types::property_set_t::All, writer, false);
    } else
    {
        throw std::runtime_error("unable to decode array");
    }
}

template<class WRITER>
static void write_array_to_json(const type::VectorDescriptor& vd, const type::Vector<>& data, WRITER& writer)
{
    writer.StartArray();

    for (unsigned int i = 0; i < data.typelessSize(); ++i)
    {
        write_json(vd.valueDescriptor(), data.typelessAt(i), writer);
    }

    writer.EndArray();
}

template<class WRITER>
static void to_json_recursive(const type::Struct& instance, types::property_set_t what,
                              WRITER& writer, bool allFields)
{
    types::property_set_t validProperties = instance._validProperties();

    /// attributes which should be serialized 'and' are valid
    const types::property_set_t serializePropertySet = (what ^ validProperties);

    //size_t nrElements = serializePropertySet.count();

    writer.StartObject();

    const auto& prop_list = instance._descriptor().propertyDescriptors();
    for (const auto& prop : prop_list)
    {
        const auto name = prop.name();

        if (!(prop.set() <= serializePropertySet))
        {
            if (allFields)
            {
                writer.String(name.data(), static_cast<rapidjson::SizeType>(name.length()), false);
                writer.Null();
            }
        }
        else
        {
            auto propertyValue = prop.address(&instance);
            //std::cout << "cbor write property '" << prop.name() << "' tag: " << tag << ":\n";
            writer.String(name.data(), static_cast<rapidjson::SizeType>(name.length()), false);
            write_json(prop.valueDescriptor(), *type::Typeless::From(propertyValue), writer);
        }
    }

    writer.EndObject();
}


std::string to_json(const type::Struct& instance, types::property_set_t properties, const ToJsonOptions& opts)
{
    StringBuffer strbuf;
    if (opts.prettyPrint)
    {
        PrettyWriter<StringBuffer> writer(strbuf);
        to_json_recursive(instance, properties, writer, opts.allFields);
    }
    else
    {
        Writer<StringBuffer> writer(strbuf);
        to_json_recursive(instance, properties, writer, opts.allFields);
    }

    return strbuf.GetString();
}

std::string to_json(const dots::type::StructDescriptor<>* /*td*/, const void* data, types::property_set_t properties, const ToJsonOptions& opts)
{
    return to_json(*reinterpret_cast<const type::Struct*>(data), properties, opts);
}

void to_json(const type::Struct& instance, Writer<StringBuffer>& writer, types::property_set_t properties, bool allFields)
{
    to_json_recursive(instance, properties, writer, allFields);
}

void to_json(const type::Struct& instance, PrettyWriter<StringBuffer>& writer, types::property_set_t properties, bool allFields)
{
    to_json_recursive(instance, properties, writer, allFields);
}

void to_json(const dots::type::StructDescriptor<> */*td*/, const void *data, rapidjson::Writer<rapidjson::StringBuffer>& writer, types::property_set_t properties, bool allFields)
{
    to_json(*reinterpret_cast<const type::Struct*>(data), writer, properties, allFields);
}

void to_json(const dots::type::StructDescriptor<> */*td*/, const void *data, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, types::property_set_t properties, bool allFields)
{
    to_json(*reinterpret_cast<const type::Struct*>(data), writer, properties, allFields);
}

// ---------------- Deserialization ------------------------
static void read_atomic_types_from_json(const type::Descriptor<>& td, type::Typeless& data, const rapidjson::Document::ValueType& value)
{
    switch (td.dotsType()) {
        case type::DotsType::int8:            static_cast<const type::Descriptor<types::int8_t>&>(td).construct(data.to<types::int8_t>(), value.GetInt()); break;
        case type::DotsType::int16:           static_cast<const type::Descriptor<types::int16_t>&>(td).construct(data.to<types::int16_t>(), value.GetInt()); break;
        case type::DotsType::int32:           static_cast<const type::Descriptor<types::int32_t>&>(td).construct(data.to<types::int32_t>(), value.GetInt()); break;
        case type::DotsType::int64:           static_cast<const type::Descriptor<types::int64_t>&>(td).construct(data.to<types::int64_t>(), value.GetInt64()); break;
        case type::DotsType::uint8:           static_cast<const type::Descriptor<types::uint8_t>&>(td).construct(data.to<types::uint8_t>(), value.GetUint()); break;
        case type::DotsType::uint16:          static_cast<const type::Descriptor<types::uint16_t>&>(td).construct(data.to<types::uint16_t>(), value.GetUint()); break;
        case type::DotsType::uint32:          static_cast<const type::Descriptor<types::uint32_t>&>(td).construct(data.to<types::uint32_t>(), value.GetUint()); break;
        case type::DotsType::uint64:          static_cast<const type::Descriptor<types::uint64_t>&>(td).construct(data.to<types::uint64_t>(), value.GetUint64()); break;
        case type::DotsType::boolean:         static_cast<const type::Descriptor<types::bool_t>&>(td).construct(data.to<types::bool_t>(), value.GetBool()); break;
        case type::DotsType::float32:         static_cast<const type::Descriptor<types::float32_t>&>(td).construct(data.to<types::float32_t>(), value.GetFloat()); break;
        case type::DotsType::float64:         static_cast<const type::Descriptor<types::float64_t>&>(td).construct(data.to<types::float64_t>(), value.GetDouble()); break;
        case type::DotsType::string:          static_cast<const type::Descriptor<types::string_t>&>(td).construct(data.to<types::string_t>(), value.GetString()); break;
        case type::DotsType::property_set:    static_cast<const type::Descriptor<types::property_set_t>&>(td).construct(data.to<types::property_set_t>(), types::property_set_t(value.GetUint())); break;
        case type::DotsType::timepoint:       static_cast<const type::Descriptor<types::timepoint_t>&>(td).construct(data.to<types::timepoint_t>(), types::duration_t{ value.GetDouble() }); break;
        case type::DotsType::steady_timepoint:static_cast<const type::Descriptor<types::steady_timepoint_t>&>(td).construct(data.to<types::steady_timepoint_t>(), types::duration_t{ value.GetDouble() }); break;
        case type::DotsType::duration:        static_cast<const type::Descriptor<types::duration_t>&>(td).construct(data.to<types::duration_t>(), type::Duration(value.GetDouble())); break;
        case type::DotsType::uuid:            static_cast<const type::Descriptor<types::uuid_t>&>(td).construct(data.to<types::uuid_t>(), types::uuid_t::FromString(value.GetString())); break;
        break;
        case type::DotsType::Enum:
        {
            const auto& enumDescriptor = static_cast<const type::EnumDescriptor<>&>(td);
            enumDescriptor.construct(data, enumDescriptor.enumeratorFromTag(value.GetInt()).valueTypeless());
        }
            break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("tried to deserialize Vector or Struct in read_atomic_types_from_cbor: " + td.name());
    }
}

static void read_json(const type::Descriptor<>& td, type::Typeless& data, const rapidjson::Document::ValueType& value)
{
    if (td.dotsType() == type::DotsType::Struct)
    {
        const auto& structDescriptor = static_cast<const type::StructDescriptor<>&>(td);
        structDescriptor.construct(data);

        from_json_recursive(structDescriptor, data.to<type::Struct>(), value.GetObject());
    }
    else if (td.dotsType() == type::DotsType::Vector)
    {
        const auto& vectorDescriptor = static_cast<const type::VectorDescriptor&>(td);
        vectorDescriptor.construct(data);
        read_from_json_array_recursive(vectorDescriptor, data.to<type::Vector<>>(), value);
    }
    else if (isDotsBaseType(td.dotsType()))
    {
        read_atomic_types_from_json(td, data, value);
    }
    else
    {
        throw std::runtime_error("dotsType not in (Struct, Vector, <BaseType>): " + std::to_string((int)td.dotsType()));
    }
}

void read_from_json_array_recursive(const type::VectorDescriptor& vd, type::Vector<>& data, const rapidjson::Document::ValueType& value)
{
    if (!value.IsArray()) {
        throw std::runtime_error("JSON value is not an array");
    }

    std::byte staticBuffer[1024];
    std::unique_ptr<std::byte[]> dynamicBuffer;
    std::byte& valueData = [&]() -> std::byte&
    {
        if (vd.valueDescriptor().size() <= sizeof(staticBuffer))
        {
            return staticBuffer[0];
        }
        else
        {
            dynamicBuffer = std::make_unique<std::byte[]>(vd.valueDescriptor().size());
            return dynamicBuffer[0];
        }
    }();

    for (auto item = value.Begin(); item != value.End(); ++item)
    {
        read_json(vd.valueDescriptor(), type::Typeless::From(valueData), *item);
        data.typelessPushBack(std::move(type::Typeless::From(valueData)));
        vd.valueDescriptor().destruct(type::Typeless::From(valueData));
    }
}

void from_json_recursive(const type::StructDescriptor<>& sd, type::Struct& instance, const rapidjson::Document::ConstObject& object)
{
    const auto& structProperties = sd.propertyDescriptors();

    auto members = object.MemberBegin();
    for(; members != object.MemberEnd(); ++members)
    {
        auto& jsonPropertyValue = members->value;
        std::string name = members->name.GetString();

        auto propertyIter = std::find_if(structProperties.begin(), structProperties.end(), [&name](const auto& prop) { return prop.name() == name; });
        if (propertyIter != structProperties.end())
        {
            auto& property = *propertyIter;
            auto propertyValue = property.address(&instance);

            read_json(property.valueDescriptor(), *type::Typeless::From(propertyValue), jsonPropertyValue);
            sd.propertyArea(instance).validProperties() += property.set();
        }
        else
        {
            LOG_INFO_P("%s: skip unkown property %s or expection while deserializing", sd.name().c_str(), name.c_str());
        }
    }
}

int from_json(const std::string &jsonString, type::Struct& instance)
{
    if (jsonString.empty())
    {
        throw std::runtime_error("unable to deserialize empty JSON object");
    }

    rapidjson::Document document;
    document.Parse(jsonString.c_str());

    if (document.HasParseError())
    {
        throw std::runtime_error("JSON parse error at " + std::to_string(document.GetErrorOffset()));
    }
    else if (!document.IsObject())
    {
        throw std::runtime_error("Not a JSON object");
    }

    const auto& constDoc = document;
    auto object = constDoc.GetObject();

    from_json_recursive(instance._descriptor(), instance, object);

    return jsonString.size();
}

int from_json(const std::string& jsonString, const dots::type::StructDescriptor<>* /*td*/, void* data)
{
    return from_json(jsonString, *reinterpret_cast<type::Struct*>(data));
}

void from_json(const rapidjson::Document::ConstObject& jsonDocument, type::Struct& instance)
{
    from_json_recursive(instance._descriptor(), instance, jsonDocument);
}

void from_json(const rapidjson::Document::ConstObject& jsonDocument, const dots::type::StructDescriptor<> */*td*/, void *data)
{
    from_json(jsonDocument, *reinterpret_cast<type::Struct*>(data));
}

}
