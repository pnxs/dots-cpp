#include "CborNativeSerialization.h"
#include "StructDescriptorData.dots.h"
#undef major
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "cbor.h"
#pragma GCC diagnostic pop

#include <map>
#include <iostream>
#include <algorithm>
#include <string>

namespace dots {

static void write_array_to_cbor(const type::NewVectorDescriptor& vd, const type::NewVector<>& data, cbor::encoder& encoder);
static void read_from_array_recursive(const type::NewVectorDescriptor& vd, type::NewVector<>& data, cbor::decoder& decoder);
void from_cbor_recursive(const type::NewStructDescriptor<>& td, type::NewStruct& instance, cbor::decoder& decoder);
static void to_cbor_recursive(const type::NewStruct& instance, types::property_set_t what, cbor::encoder &encoder);

static void
write_atomic_types_to_cbor(const type::NewDescriptor<>& td, const type::NewTypeless& data, cbor::encoder& encoder)
{
    //std::cout << "write atomic is_arithmetic:" << t.is_arithmetic() << " is_enum:" << t.is_enumeration() << " t:" << t.get_name() << "\n";
    //std::cout << "var ptr: " << var.get_ptr() << " type=" << var.get_type().get_name() << "\n";
    switch (td.dotsType()) {
        case type::DotsType::int8:            encoder.write_int(data.to<types::int8_t>()); break;
        case type::DotsType::int16:           encoder.write_int(data.to<types::int16_t>()); break;
        case type::DotsType::int32:           encoder.write_int(data.to<types::int32_t>()); break;
        case type::DotsType::int64:           encoder.write_int(static_cast<long long>(data.to<types::int64_t>())); break;
        case type::DotsType::uint8:           encoder.write_int(data.to<types::uint8_t>()); break;
        case type::DotsType::uint16:          encoder.write_int(data.to<types::uint16_t>()); break;
        case type::DotsType::uint32:          encoder.write_int(data.to<types::uint32_t>()); break;
        case type::DotsType::uint64:          encoder.write_int(static_cast<unsigned long long>(data.to<types::uint64_t>())); break;
        case type::DotsType::boolean:         encoder.write_bool(data.to<types::bool_t>()); break;
        case type::DotsType::float32:         encoder.write_float(data.to<types::float32_t>());break;
        case type::DotsType::float64:         encoder.write_double(data.to<types::float64_t>());break;
        case type::DotsType::string:          encoder.write_string(data.to<types::string_t>());break;
        case type::DotsType::property_set:    encoder.write_int(data.to<types::property_set_t>().toValue()); break;
        case type::DotsType::timepoint:       encoder.write_double(data.to<types::timepoint_t>().value()); break;
        case type::DotsType::steady_timepoint:encoder.write_double(data.to<types::steady_timepoint_t>().value()); break;
        case type::DotsType::duration:        encoder.write_double(data.to<types::duration_t>()); break;
        case type::DotsType::uuid:            encoder.write_bytes(data.to<types::uuid_t>().data().data(), 16); break;
        case type::DotsType::Enum:
        {
            encoder.write_int(static_cast<const type::NewEnumDescriptor<>&>(td).enumeratorFromValue(data).tag());
        }
        break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("unknown type: " + td.name());
    }
}

static inline void write_cbor(const type::NewDescriptor<>& td, const type::NewTypeless& data, cbor::encoder& encoder)
{
    if (td.dotsType() == type::DotsType::Vector)
    {
        write_array_to_cbor(static_cast<const type::NewVectorDescriptor&>(td), data.to<const type::NewVector<>>(), encoder);
    }
    else if (isDotsBaseType(td.dotsType()))
    {
        write_atomic_types_to_cbor(td, data, encoder);
    }
    else if (td.dotsType() == type::DotsType::Struct) // object
    {
        to_cbor_recursive(data.to<const type::NewStruct>(), types::property_set_t::All, encoder);
    }
    else
    {
        throw std::runtime_error("unable to decode array");
    }
}


static void write_array_to_cbor(const type::NewVectorDescriptor& vd, const type::NewVector<>& data, cbor::encoder& encoder)
{
    encoder.write_array(data.typelessSize());

    for (unsigned int i = 0; i < data.typelessSize(); ++i)
    {
        write_cbor(vd.valueDescriptor(), data.typelessAt(i), encoder);
    }
}

static void to_cbor_recursive(const type::NewStruct& instance, types::property_set_t what, cbor::encoder &encoder)
{
    types::property_set_t validProperties = instance._validProperties();

    /// attributes which should be serialized 'and' are valid
    const types::property_set_t serializePropertySet = (what ^ validProperties);

    size_t nrElements = serializePropertySet.count();
    encoder.write_map(nrElements);

    const auto& prop_list = instance._descriptor().propertyDescriptors();
    for (const auto& prop : prop_list)
    {
        if (!(prop->set() <= serializePropertySet))
            continue;

        auto propertyValue = prop->address(&instance);

        //std::cout << "cbor write property '" << prop.name() << "' tag: " << tag << ":\n";

        // Write Tag
        encoder.write_int(prop->tag());

        write_cbor(prop->valueDescriptor(), *type::NewTypeless::From(propertyValue), encoder);
    }
}

//
//---------------------------------- Decoding
//


static void
read_atomic_types_from_cbor(const type::NewDescriptor<>& td, type::NewTypeless& data, cbor::decoder& decoder)
{
    //std::cout << "write atomic is_arithmetic:" << t.is_arithmetic() << " is_enum:" << t.is_enumeration() << " t:" << t.get_name() << "\n";
    //std::cout << "var ptr: " << var.get_ptr() << " type=" << var.get_type().get_name() << "\n";
    auto ct = decoder.peekType().major();

    switch (td.dotsType()) {
		case type::DotsType::int8:             static_cast<const type::NewDescriptor<types::int8_t>&>(td).construct(data.to<types::int8_t>(), ct == cbor::majorType::unsignedInteger ? decoder.read_uint() : decoder.read_int()); break;
        case type::DotsType::int16:            static_cast<const type::NewDescriptor<types::int16_t>&>(td).construct(data.to<types::int16_t>(), ct == cbor::majorType::unsignedInteger ? decoder.read_uint() : decoder.read_int()); break;
        case type::DotsType::int32:            static_cast<const type::NewDescriptor<types::int32_t>&>(td).construct(data.to<types::int32_t>(), ct == cbor::majorType::unsignedInteger ? decoder.read_uint() : decoder.read_int()); break;
        case type::DotsType::int64:            static_cast<const type::NewDescriptor<types::int64_t>&>(td).construct(data.to<types::int64_t>(), ct == cbor::majorType::unsignedInteger ? decoder.read_ulong() : decoder.read_long()); break;
        case type::DotsType::uint8:            static_cast<const type::NewDescriptor<types::uint8_t>&>(td).construct(data.to<types::uint8_t>(), decoder.read_uint()); break;
        case type::DotsType::uint16:           static_cast<const type::NewDescriptor<types::uint16_t>&>(td).construct(data.to<types::uint16_t>(), decoder.read_uint()); break;
        case type::DotsType::uint32:           static_cast<const type::NewDescriptor<types::uint32_t>&>(td).construct(data.to<types::uint32_t>(), decoder.read_uint()); break;
        case type::DotsType::uint64:           static_cast<const type::NewDescriptor<types::uint64_t>&>(td).construct(data.to<types::uint64_t>(), decoder.read_ulong()); break;
        case type::DotsType::boolean:          static_cast<const type::NewDescriptor<types::bool_t>&>(td).construct(data.to<types::bool_t>(), decoder.read_bool()); break;
        case type::DotsType::float32:          static_cast<const type::NewDescriptor<types::float32_t>&>(td).construct(data.to<types::float32_t>(), decoder.read_float()); break;
        case type::DotsType::float64:          static_cast<const type::NewDescriptor<types::float64_t>&>(td).construct(data.to<types::float64_t>(), decoder.read_double()); break;
        case type::DotsType::string:           static_cast<const type::NewDescriptor<types::string_t>&>(td).construct(data.to<types::string_t>(), decoder.read_string()); break;
		case type::DotsType::property_set:     static_cast<const type::NewDescriptor<types::property_set_t>&>(td).construct(data.to<types::property_set_t>(), decoder.read_uint()); break;
        case type::DotsType::timepoint:        static_cast<const type::NewDescriptor<types::timepoint_t>&>(td).construct(data.to<types::timepoint_t>(), decoder.read_double()); break;
        case type::DotsType::steady_timepoint: static_cast<const type::NewDescriptor<types::steady_timepoint_t>&>(td).construct(data.to<types::steady_timepoint_t>(), decoder.read_double()); break;
        case type::DotsType::duration:         static_cast<const type::NewDescriptor<types::duration_t>&>(td).construct(data.to<types::duration_t>(), decoder.read_double()); break;
        case type::DotsType::uuid:             static_cast<const type::NewDescriptor<types::uuid_t>&>(td).construct(data.to<types::uuid_t>(), decoder.read_string()); break;
        case type::DotsType::Enum:
        {
            const auto& enumDescriptor = static_cast<const type::NewEnumDescriptor<>&>(td);
            enumDescriptor.construct(data, enumDescriptor.enumeratorFromTag(decoder.read_int()).valueTypeless());
        }
            break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("tried to deserialize Vector or Struct in read_atomic_types_from_cbor: " + td.name());
    }
}

void read_cbor(const type::NewDescriptor<>& td, type::NewTypeless& data, cbor::decoder& decoder)
{
    if (td.dotsType() == type::DotsType::Struct)
    {
    	const auto& structDescriptor = static_cast<const type::NewStructDescriptor<>&>(td);
    	structDescriptor.construct(data);
    	
        from_cbor_recursive(structDescriptor, data.to<type::NewStruct>(), decoder);
    }
    else if (td.dotsType() == type::DotsType::Vector)
    {
    	const auto& vectorDescriptor = static_cast<const type::NewVectorDescriptor&>(td);
    	vectorDescriptor.construct(data);
    	
        read_from_array_recursive(vectorDescriptor, data.to<type::NewVector<>>(), decoder);
    }
    else if (isDotsBaseType(td.dotsType()))
    {
        read_atomic_types_from_cbor(td, data, decoder);
    }
    else
    {
        throw std::runtime_error("dotsType not in (Struct, Vector, <BaseType>): " + std::to_string((int)td.type()));
    }
}

void from_cbor_recursive(const type::NewStructDescriptor<>& td, type::NewStruct& instance, cbor::decoder& decoder)
{
    const auto& structProperties = td.propertyDescriptors();

    auto structSize = decoder.read_map();
    //std::cout << "decode cbor struct '" << sd->name() << "'size " << structSize << "\n";

    for (size_t i = 0; i < structSize; ++i)
    {
        uint32_t tag = decoder.read_uint();

        // find structProperty with Tag <tag>
        auto propertyIter = std::find_if(structProperties.begin(), structProperties.end(), [&tag](auto prop) { return prop->tag() == tag; });
        if (propertyIter != structProperties.end())
        {
            auto property = *propertyIter;
            auto propertyValue = property->address(&instance);

            //auto cbor_type = decoder.peekType();

            //std::cout << "tag: " << tag << " descriptor-type:" << property.td()->name() << "\n";
            //std::cout << "cbor-type: " << (int) cbor_type.major() << "\n";

            if (1) // TODO: wireTypeCompatible
            {
                read_cbor(property->valueDescriptor(), *type::NewTypeless::From(propertyValue), decoder);
            }

            //td.propertyArea(instance).validProperties() += property->set();
        	type::NewPropertySet& validProperties = td.propertyArea(instance).validProperties();
        	validProperties += property->set();
        }
        else
        {
            LOG_INFO_P("%s: skip unkown attr tag %u or expection while deserializing", td.name().c_str(), tag);
            //           att_tag, dots::toString(wire_type.category()).c_str());
            decoder.skip();
        }
    }
}

static void read_from_array_recursive(const type::NewVectorDescriptor& vd, type::NewVector<>& data, cbor::decoder& decoder)
{
	data.typelessResize(decoder.read_array());

    for (size_t i = 0; i < data.typelessSize(); ++i)
    {
        read_cbor(vd.valueDescriptor(), data.typelessAt(i), decoder);
    }
}

//----------------------------------


std::string to_cbor(const type::NewStruct& instance, types::property_set_t properties)
{
    cbor::output_dynamic out;
    cbor::encoder encoder(out);

    to_cbor_recursive(instance, properties, encoder);

    return {reinterpret_cast<const char*>(out.data()), out.size()};
}

std::string to_cbor(DynamicInstance instance, types::property_set_t properties)
{
	return to_cbor(*reinterpret_cast<const type::NewStruct*>(instance.obj), properties);
}

int from_cbor(const uint8_t* cborData, std::size_t cborSize, type::NewStruct& instance)
{
    if (cborSize == 0) {
        throw std::runtime_error("unable to deserialize zero-sized CBOR object");
    }

    cbor::input input(cborData, cborSize);
    cbor::decoder decoder(input);

    from_cbor_recursive(instance._descriptor(), instance, decoder);

    return input.offset();
}

int from_cbor(const uint8_t* cborData, std::size_t cborSize, const dots::type::NewStructDescriptor<>* /*td*/, void* data)
{
	return from_cbor(cborData, cborSize, *reinterpret_cast<type::NewStruct*>(data));
}

int skip_cbor(const uint8_t* cborData, std::size_t cborSize)
{
    if (cborSize == 0) {
        throw std::runtime_error("unable to deserialize zero-sized CBOR object");
    }

    cbor::input input(cborData, cborSize);
    cbor::decoder decoder(input);

    decoder.skip();

    return input.offset();
}

}