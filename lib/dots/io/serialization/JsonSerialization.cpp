#include "JsonSerialization.h"

#include "StructDescriptorData.dots.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <dots/dots_base.h>

#include <iostream>
#include <algorithm>

using namespace rapidjson;

namespace dots {

template<class WRITER>
static void write_array_to_json(const type::NewVectorDescriptor& vd, const type::NewVector<>& data, WRITER& writer);

template<class WRITER>
static void to_json_recursive(const type::NewStruct& instance, types::property_set_t what,
                              WRITER& writer, bool allFields);

static void from_json_recursive(const type::NewStructDescriptor<>& sd, type::NewStruct& instance, const rapidjson::Document::ConstObject& object);
static void read_from_json_array_recursive(const type::NewVectorDescriptor& vd, type::NewVector<>& data, const rapidjson::Document::ValueType& value);

template<class WRITER>
static void write_atomic_types_to_json(const type::NewDescriptor<>& td, const type::NewTypeless& typeless, WRITER& writer)
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
            writer.Int(static_cast<const type::NewEnumDescriptor<>&>(td).enumeratorFromValue(typeless).tag());
        }
            break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("unknown type: " + td.name());
    }
}

template<class WRITER>
static inline void write_json(const type::NewDescriptor<>& td, const type::NewTypeless& data, WRITER& writer)
{
    if (td.dotsType() == type::DotsType::Vector)
    {
        write_array_to_json(static_cast<const type::NewVectorDescriptor&>(td), data.to<const type::NewVector<>>(), writer);
    } else if (isDotsBaseType(td.dotsType()))
    {
        write_atomic_types_to_json(td, data, writer);
    } else if (td.dotsType() == type::DotsType::Struct) // object
    {
        to_json_recursive(data.to<const type::NewStruct>(), types::property_set_t::All, writer, false);
    } else
    {
        throw std::runtime_error("unable to decode array");
    }
}

template<class WRITER>
static void write_array_to_json(const type::NewVectorDescriptor& vd, const type::NewVector<>& data, WRITER& writer)
{
	writer.StartArray();

    for (unsigned int i = 0; i < data.typelessSize(); ++i)
    {
        write_json(vd.valueDescriptor(), data.typelessAt(i), writer);
    }

	writer.EndArray();
}

template<class WRITER>
static void to_json_recursive(const type::NewStruct& instance, types::property_set_t what,
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
        const auto name = prop->name();

        if (!(prop->set() <= serializePropertySet))
        {
            if (allFields)
            {
                writer.String(name.data(), static_cast<rapidjson::SizeType>(name.length()), false);
                writer.Null();
            }
        }
        else
        {
            auto propertyValue = prop->address(&instance);
            //std::cout << "cbor write property '" << prop.name() << "' tag: " << tag << ":\n";
            writer.String(name.data(), static_cast<rapidjson::SizeType>(name.length()), false);
            write_json(prop->valueDescriptor(), *type::NewTypeless::From(propertyValue), writer);
        }
    }

    writer.EndObject();
}


std::string to_json(const type::NewStruct& instance, types::property_set_t properties, const ToJsonOptions& opts)
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

std::string to_json(const dots::type::NewStructDescriptor<>* /*td*/, const void* data, types::property_set_t properties, const ToJsonOptions& opts)
{
	return to_json(*reinterpret_cast<const type::NewStruct*>(data), properties, opts);
}

void to_json(const type::NewStruct& instance, Writer<StringBuffer>& writer, types::property_set_t properties, bool allFields)
{
    to_json_recursive(instance, properties, writer, allFields);
}

void to_json(const type::NewStruct& instance, PrettyWriter<StringBuffer>& writer, types::property_set_t properties, bool allFields)
{
    to_json_recursive(instance, properties, writer, allFields);
}

void to_json(const dots::type::NewStructDescriptor<> */*td*/, const void *data, rapidjson::Writer<rapidjson::StringBuffer>& writer, types::property_set_t properties, bool allFields)
{
	to_json(*reinterpret_cast<const type::NewStruct*>(data), writer, properties, allFields);
}

void to_json(const dots::type::NewStructDescriptor<> */*td*/, const void *data, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, types::property_set_t properties, bool allFields)
{
	to_json(*reinterpret_cast<const type::NewStruct*>(data), writer, properties, allFields);
}

// ---------------- Deserialization ------------------------
static void read_atomic_types_from_json(const type::NewDescriptor<>& td, type::NewTypeless& data, const rapidjson::Document::ValueType& value)
{
    switch (td.dotsType()) {
        case type::DotsType::int8:            static_cast<const type::NewDescriptor<types::int8_t>&>(td).construct(data.to<types::int8_t>(), value.GetInt()); break;
        case type::DotsType::int16:           static_cast<const type::NewDescriptor<types::int16_t>&>(td).construct(data.to<types::int16_t>(), value.GetInt()); break;
        case type::DotsType::int32:           static_cast<const type::NewDescriptor<types::int32_t>&>(td).construct(data.to<types::int32_t>(), value.GetInt()); break;
        case type::DotsType::int64:           static_cast<const type::NewDescriptor<types::int64_t>&>(td).construct(data.to<types::int64_t>(), value.GetInt64()); break;
        case type::DotsType::uint8:           static_cast<const type::NewDescriptor<types::uint8_t>&>(td).construct(data.to<types::uint8_t>(), value.GetUint()); break;
        case type::DotsType::uint16:          static_cast<const type::NewDescriptor<types::uint16_t>&>(td).construct(data.to<types::uint16_t>(), value.GetUint()); break;
        case type::DotsType::uint32:          static_cast<const type::NewDescriptor<types::uint32_t>&>(td).construct(data.to<types::uint32_t>(), value.GetUint()); break;
        case type::DotsType::uint64:          static_cast<const type::NewDescriptor<types::uint64_t>&>(td).construct(data.to<types::uint64_t>(), value.GetUint64()); break;
        case type::DotsType::boolean:         static_cast<const type::NewDescriptor<types::bool_t>&>(td).construct(data.to<types::bool_t>(), value.GetBool()); break;
        case type::DotsType::float32:         static_cast<const type::NewDescriptor<types::float32_t>&>(td).construct(data.to<types::float32_t>(), value.GetFloat()); break;
        case type::DotsType::float64:         static_cast<const type::NewDescriptor<types::float64_t>&>(td).construct(data.to<types::float64_t>(), value.GetDouble()); break;
        case type::DotsType::string:          static_cast<const type::NewDescriptor<types::string_t>&>(td).construct(data.to<types::string_t>(), value.GetString()); break;
        case type::DotsType::property_set:    static_cast<const type::NewDescriptor<types::property_set_t>&>(td).construct(data.to<types::property_set_t>(), types::property_set_t(value.GetUint())); break;
        case type::DotsType::timepoint:       static_cast<const type::NewDescriptor<types::timepoint_t>&>(td).construct(data.to<types::timepoint_t>(), value.GetDouble()); break;
        case type::DotsType::steady_timepoint:static_cast<const type::NewDescriptor<types::steady_timepoint_t>&>(td).construct(data.to<types::steady_timepoint_t>(), value.GetDouble()); break;
        case type::DotsType::duration:        static_cast<const type::NewDescriptor<types::duration_t>&>(td).construct(data.to<types::duration_t>(), pnxs::Duration(value.GetDouble())); break;
		case type::DotsType::uuid:            static_cast<const type::NewDescriptor<types::uuid_t>&>(td).construct(data.to<types::uuid_t>()).fromString(value.GetString()); break;
        break;
        case type::DotsType::Enum:
        {
            const auto& enumDescriptor = static_cast<const type::NewEnumDescriptor<>&>(td);
        	enumDescriptor.construct(data, enumDescriptor.enumeratorFromTag(value.GetInt()).valueTypeless());
        }
            break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("tried to deserialize Vector or Struct in read_atomic_types_from_cbor: " + td.name());
    }
}

static void read_json(const type::NewDescriptor<>& td, type::NewTypeless& data, const rapidjson::Document::ValueType& value)
{
    if (td.dotsType() == type::DotsType::Struct)
    {
    	const auto& structDescriptor = static_cast<const type::NewStructDescriptor<>&>(td);
    	structDescriptor.construct(data);

    	from_json_recursive(structDescriptor, data.to<type::NewStruct>(), value.GetObject());
    }
    else if (td.dotsType() == type::DotsType::Vector)
    {
    	const auto& vectorDescriptor = static_cast<const type::NewVectorDescriptor&>(td);
    	vectorDescriptor.construct(data);
        read_from_json_array_recursive(vectorDescriptor, data.to<type::NewVector<>>(), value);
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

void read_from_json_array_recursive(const type::NewVectorDescriptor& vd, type::NewVector<>& data, const rapidjson::Document::ValueType& value)
{
    if (!value.IsArray()) {
        throw std::runtime_error("JSON value is not an array");
    }
	
	data.typelessResize(value.Size());

	auto item = value.Begin();
    for (size_t i = 0; i < data.typelessSize(); ++i, ++item)
    {
        read_json(vd.valueDescriptor(), data.typelessAt(i), *item);
    }
}

void from_json_recursive(const type::NewStructDescriptor<>& sd, type::NewStruct& instance, const rapidjson::Document::ConstObject& object)
{
    auto structProperties = sd.propertyDescriptors();

    auto members = object.MemberBegin();
    for(; members != object.MemberEnd(); ++members)
    {
        auto& jsonPropertyValue = members->value;
        string name = members->name.GetString();

        auto propertyIter = std::find_if(structProperties.begin(), structProperties.end(), [&name](auto prop) { return prop->name() == name; });
        if (propertyIter != structProperties.end())
        {
            auto property = *propertyIter;
            auto propertyValue = property->address(&instance);

            read_json(property->valueDescriptor(), *type::NewTypeless::From(propertyValue), jsonPropertyValue);
            sd.propertyArea(instance).validProperties() += property->set();
        }
        else
        {
            LOG_INFO_P("%s: skip unkown property %s or expection while deserializing", sd.name().c_str(), name.c_str());
        }
    }
}

int from_json(const std::string &jsonString, type::NewStruct& instance)
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

int from_json(const std::string& jsonString, const dots::type::NewStructDescriptor<>* /*td*/, void* data)
{
	return from_json(jsonString, *reinterpret_cast<type::NewStruct*>(data));
}

void from_json(const rapidjson::Document::ConstObject& jsonDocument, type::NewStruct& instance)
{
    from_json_recursive(instance._descriptor(), instance, jsonDocument);
}

void from_json(const rapidjson::Document::ConstObject& jsonDocument, const dots::type::NewStructDescriptor<> */*td*/, void *data)
{
	from_json(jsonDocument, *reinterpret_cast<type::NewStruct*>(data));
}

}
