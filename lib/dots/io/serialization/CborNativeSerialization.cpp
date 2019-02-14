#include "CborNativeSerialization.h"
#include "StructDescriptorData.dots.h"
#include "cbor.h"
#include "dots/type/Registry.h"

#include <map>
#include <iostream>
#include <algorithm>
#include <string>

namespace dots {

static void to_cbor_recursive(DynamicInstance instance, property_set what, cbor::encoder &encoder);
static void write_array_to_cbor(const type::VectorDescriptor* vd, const void* data, cbor::encoder& encoder);
static void read_from_array_recursive(const type::VectorDescriptor* vd, void* data, cbor::decoder& decoder);
void from_cbor_recursive(const type::StructDescriptor* sd, void* data, cbor::decoder& decoder);

static void
write_atomic_types_to_cbor(const type::Descriptor* td, const void* data, cbor::encoder& encoder)
{
    //std::cout << "write atomic is_arithmetic:" << t.is_arithmetic() << " is_enum:" << t.is_enumeration() << " t:" << t.get_name() << "\n";
    //std::cout << "var ptr: " << var.get_ptr() << " type=" << var.get_type().get_name() << "\n";
    switch (td->dotsType()) {
        case type::DotsType::int8:            encoder.write_int(*(const int8_t *) data); break;
        case type::DotsType::int16:           encoder.write_int(*(const int16_t *) data); break;
        case type::DotsType::int32:           encoder.write_int(*(const int32_t *) data); break;
        case type::DotsType::int64:           encoder.write_int(*(const long long *) data); break;
        case type::DotsType::uint8:           encoder.write_int(*(const uint8_t *) data); break;
        case type::DotsType::uint16:          encoder.write_int(*(const uint16_t *) data); break;
        case type::DotsType::uint32:          encoder.write_int(*(const uint32_t *) data); break;
        case type::DotsType::uint64:          encoder.write_int(*(const unsigned long long *) data); break;
        case type::DotsType::boolean:         encoder.write_bool(*(const bool *) data); break;
        case type::DotsType::float16:         encoder.write_float(*(const float *) data); break;
        case type::DotsType::float32:         encoder.write_float(*(const float *) data);break;
        case type::DotsType::float64:         encoder.write_double(*(const double *) data);break;
        case type::DotsType::string:          encoder.write_string(*(const std::string*) data);break;
        case type::DotsType::property_set:    encoder.write_int(((const dots::property_set*)data)->value()); break;
        case type::DotsType::timepoint:       encoder.write_double(((const pnxs::TimePoint*)data)->value()); break;
        case type::DotsType::steady_timepoint:encoder.write_double(((const pnxs::SteadyTimePoint*)data)->value()); break;
        case type::DotsType::duration:        encoder.write_double(*(const pnxs::Duration*)data); break;
        case type::DotsType::uuid:            encoder.write_bytes(((const dots::uuid*)data)->data().data(), 16); break;
        case type::DotsType::Enum:
        {
            auto enumDescriptor = dynamic_cast<const type::EnumDescriptor*>(td);
            auto enumValue = enumDescriptor->to_int(data);
            encoder.write_int(enumDescriptor->value2key(enumValue));
        }
        break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("unknown type: " + td->name());

        case type::DotsType::pointer:
            throw std::runtime_error("unable to serialize pointer-type");
    }
}

static inline void write_cbor(const type::Descriptor* td, const void* data, cbor::encoder& encoder)
{
    if (td->dotsType() == type::DotsType::Vector)
    {
        auto vectorDescriptor = type::toVectorDescriptor(td);
        write_array_to_cbor(vectorDescriptor, data, encoder);
    }
    else if (isDotsBaseType(td->dotsType()))
    {
        write_atomic_types_to_cbor(td, data, encoder);
    }
    else if (td->dotsType() == type::DotsType::Struct) // object
    {
        auto structDescriptor = type::toStructDescriptor(td);
        to_cbor_recursive({structDescriptor, data}, PROPERTY_SET_ALL, encoder);
    }
    else
    {
        throw std::runtime_error("unable to decode array");
    }
}


static void write_array_to_cbor(const type::VectorDescriptor* vd, const void* data, cbor::encoder& encoder)
{
    auto vectorSize = vd->get_size(data);
    encoder.write_array(vectorSize);

    auto itemDescriptor = vd->vtd();

    for (unsigned int i = 0; i < vectorSize; ++i)
    {
        write_cbor(itemDescriptor, vd->get_data(data, i), encoder);
    }
}

static void to_cbor_recursive(DynamicInstance instance, property_set what, cbor::encoder &encoder)
{
    property_set validProperties = instance.td->validProperties(instance.obj);

    /// attributes which should be serialized 'and' are valid
    const property_set serializePropertySet = (what & validProperties);

    size_t nrElements = serializePropertySet.count();
    encoder.write_map(nrElements);

    auto prop_list = instance.td->properties();
    for (auto prop : prop_list)
    {
        auto tag = prop.tag();

        if (not serializePropertySet.test(tag))
            continue;

        auto propertyValue = prop.address(instance.obj);

        //std::cout << "cbor write property '" << prop.name() << "' tag: " << tag << ":\n";

        // Write Tag
        encoder.write_int(tag);

        write_cbor(prop.td(), propertyValue, encoder);
    }
}

//
//---------------------------------- Decoding
//


static void
read_atomic_types_from_cbor(const type::Descriptor* td, void* data, cbor::decoder& decoder)
{
    //std::cout << "write atomic is_arithmetic:" << t.is_arithmetic() << " is_enum:" << t.is_enumeration() << " t:" << t.get_name() << "\n";
    //std::cout << "var ptr: " << var.get_ptr() << " type=" << var.get_type().get_name() << "\n";
    auto ct = decoder.peekType().major();

    switch (td->dotsType()) {
        case type::DotsType::int8:            ::new (data) int8_t ((ct == cbor::majorType::unsignedInteger) ? decoder.read_uint() : decoder.read_int()); break;
        case type::DotsType::int16:           ::new (data) int16_t ((ct == cbor::majorType::unsignedInteger) ? decoder.read_uint() : decoder.read_int()); break;
        case type::DotsType::int32:           ::new (data) int32_t ((ct == cbor::majorType::unsignedInteger) ? decoder.read_uint() : decoder.read_int()); break;
        case type::DotsType::int64:           ::new (data) long long ((ct == cbor::majorType::unsignedInteger) ? decoder.read_ulong() : decoder.read_long()); break;
        case type::DotsType::uint8:           ::new (data) uint8_t (decoder.read_uint()); break;
        case type::DotsType::uint16:          ::new (data) uint16_t (decoder.read_uint()); break;
        case type::DotsType::uint32:          ::new (data) uint32_t (decoder.read_uint()); break;
        case type::DotsType::uint64:          ::new (data) unsigned long long (decoder.read_ulong()); break;
        case type::DotsType::boolean:         ::new (data) bool (decoder.read_bool()); break;
        case type::DotsType::float16:         ::new (data) float (decoder.read_float()); break;
        case type::DotsType::float32:         ::new (data) float (decoder.read_float()); break;
        case type::DotsType::float64:         ::new (data) double (decoder.read_double()); break;
        case type::DotsType::string:          ::new (data) std::string(decoder.read_string()); break;
        case type::DotsType::property_set:    ::new (data) dots::property_set(property_set(decoder.read_uint())); break;
        case type::DotsType::timepoint:       ::new (data) pnxs::TimePoint(pnxs::TimePoint(decoder.read_double())); break;
        case type::DotsType::steady_timepoint: ::new (data) pnxs::SteadyTimePoint(pnxs::SteadyTimePoint(decoder.read_double())); break;
        case type::DotsType::duration:        ::new (data) pnxs::Duration(pnxs::Duration(decoder.read_double())); break;
        case type::DotsType::uuid:            ::new (data) dots::uuid(dots::uuid(decoder.read_string())); break;
        case type::DotsType::Enum:
        {
            auto enumDescriptor = type::toEnumDescriptor(td);
            enumDescriptor->from_key(data, decoder.read_int());
        }
            break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("tried to deserialize Vector or Struct in read_atomic_types_from_cbor: " + td->name());
        case type::DotsType::pointer:
            throw std::runtime_error("unable to deserialize pointer-type");
    }
}

void read_cbor(const type::Descriptor* td, void* data, cbor::decoder& decoder)
{
    if (td->dotsType() == type::DotsType::Struct)
    {
        auto structDescriptor = type::toStructDescriptor(td);
        from_cbor_recursive(structDescriptor, data, decoder);
    }
    else if (td->dotsType() == type::DotsType::Vector)
    {
        auto vectorDescriptor = type::toVectorDescriptor(td);
        read_from_array_recursive(vectorDescriptor, data, decoder);
    }
    else if (isDotsBaseType(td->dotsType()))
    {
        read_atomic_types_from_cbor(td, data, decoder);
    }
    else
    {
        throw std::runtime_error("dotsType not in (Struct, Vector, <BaseType>): " + to_string((int)td->dotsType()));
    }
}

void from_cbor_recursive(const type::StructDescriptor* sd, void* data, cbor::decoder& decoder)
{
    property_set& validProperties = sd->validProperties(data);
    validProperties.clear();

    auto structProperties = sd->properties();

    auto structSize = decoder.read_map();
    //std::cout << "decode cbor struct '" << sd->name() << "'size " << structSize << "\n";

    for (size_t i = 0; i < structSize; ++i)
    {
        uint32_t tag = decoder.read_uint();

        // find structProperty with Tag <tag>
        auto propertyIter = std::find_if(structProperties.begin(), structProperties.end(), [&tag](auto prop) { return prop.tag() == tag; });
        if (propertyIter != structProperties.end())
        {
            auto property = *propertyIter;
            auto propertyValue = property.address(data);

            //auto cbor_type = decoder.peekType();

            //std::cout << "tag: " << tag << " descriptor-type:" << property.td()->name() << "\n";
            //std::cout << "cbor-type: " << (int) cbor_type.major() << "\n";

            if (1) // TODO: wireTypeCompatible
            {
                read_cbor(property.td(), propertyValue, decoder);
            }

            validProperties.set(tag);
        }
        else
        {
            LOG_INFO_P("%s: skip unkown attr tag %u or expection while deserializing", sd->name().c_str(), tag);
            //           att_tag, dots::toString(wire_type.category()).c_str());
            decoder.skip();
        }
    }
}

static void read_from_array_recursive(const type::VectorDescriptor* vd, void* data, cbor::decoder& decoder)
{
    size_t arrayLength = decoder.read_array();
    auto propertyDescriptor = vd->vtd();
    vd->resize(data, arrayLength);

    for (size_t i = 0; i < arrayLength; ++i)
    {
        auto propertyValue = vd->get_data(data, i);
        read_cbor(propertyDescriptor, propertyValue, decoder);
    }
}

//----------------------------------


std::string to_cbor(DynamicInstance instance, property_set properties)
{
    cbor::output_dynamic out;
    cbor::encoder encoder(out);

    to_cbor_recursive(instance, properties, encoder);

    return {reinterpret_cast<const char*>(out.data()), out.size()};
}

int from_cbor(const uint8_t* cborData, std::size_t cborSize, const dots::type::StructDescriptor* td, void* data)
{
    if (cborSize == 0) {
        throw std::runtime_error("unable to deserialize zero-sized CBOR object");
    }

    cbor::input input(cborData, cborSize);
    cbor::decoder decoder(input);

    from_cbor_recursive(td, data, decoder);

    return input.offset();
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