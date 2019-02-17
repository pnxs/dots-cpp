#include "JsonSerialization.h"

#include "StructDescriptorData.dots.h"
#include "dots/type/Registry.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <dots/dots_base.h>

#include <iostream>
#include <algorithm>

using namespace rapidjson;

namespace dots {

template<class WRITER>
static void write_array_to_json(const type::VectorDescriptor *vd, const void *data, WRITER& writer);

template<class WRITER>
static void to_json_recursive(const dots::type::StructDescriptor *td, const void *data, property_set what,
                              WRITER& writer, bool allFields);

static void from_json_recursive(const type::StructDescriptor* sd, void* data, const rapidjson::Document::ConstObject& object);
static void read_from_json_array_recursive(const type::VectorDescriptor* vd, void* data, const rapidjson::Document::ValueType& value);

template<class WRITER>
static void write_atomic_types_to_json(const type::Descriptor *td, const void *data, WRITER& writer)
{
    //std::cout << "write atomic is_arithmetic:" << t.is_arithmetic() << " is_enum:" << t.is_enumeration() << " t:" << t.get_name() << "\n";
    //std::cout << "var ptr: " << var.get_ptr() << " type=" << var.get_type().get_name() << "\n";
    switch (td->dotsType())
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
        case type::DotsType::float16: writer.Double(*(const float *) data);
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
        case type::DotsType::property_set: writer.Int(((const dots::property_set *) data)->value());
            break;
        case type::DotsType::timepoint: writer.Double(((const pnxs::TimePoint *) data)->value());
            break;
        case type::DotsType::steady_timepoint:writer.Double(((const pnxs::SteadyTimePoint *) data)->value());
            break;
        case type::DotsType::duration: writer.Double(*(const pnxs::Duration *) data);
            break;
        case type::DotsType::uuid: writer.String(((const dots::uuid *) data)->toString());
            break;
        case type::DotsType::Enum:
        {
            auto enumDescriptor = dynamic_cast<const type::EnumDescriptor *>(td);
            auto enumValue = enumDescriptor->to_int(data);
            writer.Int(enumDescriptor->value2key(enumValue));
        }
            break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("unknown type: " + td->name());
        case type::DotsType::pointer:throw std::runtime_error("unable to serialize pointer-type");

    }
}

template<class WRITER>
static inline void write_json(const type::Descriptor *td, const void *data, WRITER& writer)
{
    if (td->dotsType() == type::DotsType::Vector)
    {
        auto vectorDescriptor = type::toVectorDescriptor(td);
        write_array_to_json(vectorDescriptor, data, writer);
    } else if (isDotsBaseType(td->dotsType()))
    {
        write_atomic_types_to_json(td, data, writer);
    } else if (td->dotsType() == type::DotsType::Struct) // object
    {
        auto structDescriptor = type::toStructDescriptor(td);
        to_json_recursive(structDescriptor, data, PROPERTY_SET_ALL, writer, false);
    } else
    {
        throw std::runtime_error("unable to decode array");
    }
}

template<class WRITER>
static void write_array_to_json(const type::VectorDescriptor *vd, const void *data, WRITER& writer)
{
    auto vectorSize = vd->get_size(data);
    writer.StartArray();

    auto itemDescriptor = vd->vtd();

    for (unsigned int i = 0; i < vectorSize; ++i)
    {
        write_json(itemDescriptor, vd->get_data(data, i), writer);
    }

    writer.EndArray();
}

template<class WRITER>
static void to_json_recursive(const dots::type::StructDescriptor *td, const void *data, property_set what,
                              WRITER& writer, bool allFields)
{
    property_set validProperties = td->validProperties(data);

    /// attributes which should be serialized 'and' are valid
    const property_set serializePropertySet = (what & validProperties);

    //size_t nrElements = serializePropertySet.count();

    writer.StartObject();

    auto prop_list = td->properties();
    for (auto prop : prop_list)
    {
        auto tag = prop.tag();
        const auto name = prop.name();

        if (not serializePropertySet.test(tag))
        {
            if (allFields)
            {
                writer.String(name.data(), static_cast<rapidjson::SizeType>(name.length()), false);
                writer.Null();
            }
        }
        else
        {
            auto propertyValue = prop.address(data);
            //std::cout << "cbor write property '" << prop.name() << "' tag: " << tag << ":\n";
            writer.String(name.data(), static_cast<rapidjson::SizeType>(name.length()), false);
            write_json(prop.td(), propertyValue, writer);
        }
    }

    writer.EndObject();
}


std::string to_json(const dots::type::StructDescriptor *td, const void *data, property_set properties, const ToJsonOptions& opts)
{
    StringBuffer strbuf;
    if (opts.prettyPrint)
    {
        PrettyWriter<StringBuffer> writer(strbuf);
        to_json_recursive(td, data, properties, writer, opts.allFields);
    }
    else
    {
        Writer<StringBuffer> writer(strbuf);
        to_json_recursive(td, data, properties, writer, opts.allFields);
    }

    return strbuf.GetString();
}

void to_json(const dots::type::StructDescriptor *td, const void *data, Writer<StringBuffer>& writer, property_set properties, bool allFields)
{
    to_json_recursive(td, data, properties, writer, allFields);
}

void to_json(const dots::type::StructDescriptor *td, const void *data, PrettyWriter<StringBuffer>& writer, property_set properties, bool allFields)
{
    to_json_recursive(td, data, properties, writer, allFields);
}

// ---------------- Deserialization ------------------------
static void read_atomic_types_from_json(const type::Descriptor* td, void* data, const rapidjson::Document::ValueType& value)
{
    switch (td->dotsType()) {
        case type::DotsType::int8:            ::new (data) int8_t  (value.GetInt()); break;
        case type::DotsType::int16:           ::new (data) int16_t  (value.GetInt()); break;
        case type::DotsType::int32:           ::new (data) int32_t  (value.GetInt()); break;
        case type::DotsType::int64:           ::new (data) long long  (value.GetInt64()); break;
        case type::DotsType::uint8:           ::new (data) uint8_t  (value.GetUint()); break;
        case type::DotsType::uint16:          ::new (data) uint16_t (value.GetUint()); break;
        case type::DotsType::uint32:          ::new (data) uint32_t (value.GetUint()); break;
        case type::DotsType::uint64:          ::new (data) unsigned long long  (value.GetUint64()); break;
        case type::DotsType::boolean:         ::new (data) bool  (value.GetBool()); break;
        case type::DotsType::float16:         ::new (data) float  (value.GetFloat()); break;
        case type::DotsType::float32:         ::new (data) float (value.GetFloat()); break;
        case type::DotsType::float64:         ::new (data) double (value.GetDouble()); break;
        case type::DotsType::string:          ::new (data) std::string (value.GetString()); break;
        case type::DotsType::property_set:    ::new (data) dots::property_set (property_set(value.GetUint())); break;
        case type::DotsType::timepoint:       ::new (data) pnxs::TimePoint (pnxs::TimePoint(value.GetDouble())); break;
        case type::DotsType::steady_timepoint: ::new (data) pnxs::SteadyTimePoint (pnxs::SteadyTimePoint(value.GetDouble())); break;
        case type::DotsType::duration:        ::new (data) pnxs::Duration (pnxs::Duration(value.GetDouble())); break;
        case type::DotsType::uuid:
        {
            dots::uuid uuid;
            uuid.fromString(value.GetString());
            ::new (data) dots::uuid(uuid);
        }
        break;
        case type::DotsType::Enum:
        {
            auto enumDescriptor = type::toEnumDescriptor(td);
            enumDescriptor->from_key(data, value.GetInt());
        }
            break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("tried to deserialize Vector or Struct in read_atomic_types_from_cbor: " + td->name());
        case type::DotsType::pointer:
            throw std::runtime_error("unable to deserialize pointer-type");
    }
}

static void read_json(const type::Descriptor* td, void* data, const rapidjson::Document::ValueType& value)
{
    if (td->dotsType() == type::DotsType::Struct)
    {
        auto structDescriptor = type::toStructDescriptor(td);
        auto object = value.GetObject();
        from_json_recursive(structDescriptor, data, object);
    }
    else if (td->dotsType() == type::DotsType::Vector)
    {
        auto vectorDescriptor = type::toVectorDescriptor(td);
        read_from_json_array_recursive(vectorDescriptor, data, value);
    }
    else if (isDotsBaseType(td->dotsType()))
    {
        read_atomic_types_from_json(td, data, value);
    }
    else
    {
        throw std::runtime_error("dotsType not in (Struct, Vector, <BaseType>): " + to_string((int)td->dotsType()));
    }
}

void read_from_json_array_recursive(const type::VectorDescriptor* vd, void* data, const rapidjson::Document::ValueType& value)
{
    if (not value.IsArray()) {
        throw std::runtime_error("JSON value is not an array");
    }
    size_t arrayLength = value.Size();
    auto propertyDescriptor = vd->vtd();
	vd->construct(data);
    vd->resize(data, arrayLength);

    auto item = value.Begin();
    for (int i = 0; item != value.End(); ++item, ++i)
    {
        auto propertyValue = vd->get_data(data, i);
        read_json(propertyDescriptor, propertyValue, *item);
    }
}

void from_json_recursive(const type::StructDescriptor* sd, void* data, const rapidjson::Document::ConstObject& object)
{
	sd->construct(data);
    property_set& validProperties = sd->validProperties(data);
    validProperties.clear();

    auto structProperties = sd->properties();

    auto members = object.MemberBegin();
    for(; members != object.MemberEnd(); ++members)
    {
        auto& jsonPropertyValue = members->value;
        string name = members->name.GetString();

        auto propertyIter = std::find_if(structProperties.begin(), structProperties.end(), [&name](auto prop) { return prop.name() == name; });
        if (propertyIter != structProperties.end())
        {
            auto property = *propertyIter;
            auto propertyValue = property.address(data);

            read_json(property.td(), propertyValue, jsonPropertyValue);
            validProperties.set(property.tag());
        }
        else
        {
            LOG_INFO_P("%s: skip unkown property %s or expection while deserializing", sd->name().c_str(), name.c_str());
        }
    }
}

int from_json(const std::string &jsonString, const dots::type::StructDescriptor *td, void *data)
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
    else if (not document.IsObject())
    {
        throw std::runtime_error("Not a JSON object");
    }

    const auto& constDoc = document;
    auto object = constDoc.GetObject();

    from_json_recursive(td, data, object);

    return jsonString.size();
}

void from_json(const rapidjson::Document::ConstObject& jsonDocument, const dots::type::StructDescriptor *td, void *data)
{
    from_json_recursive(td, data, jsonDocument);
}

}
