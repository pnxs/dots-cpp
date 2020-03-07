#pragma once

#include <dots/type/StructDescriptor.h>
#include <dots/type/FundamentalTypes.h>
#include "dots/dots_base.h"

namespace dots {

struct [[deprecated("only available for backwards compatibility")]] DynamicInstance
{
    DynamicInstance(const type::StructDescriptor<>* _td, const void* _obj): td(_td), obj(_obj) {}

    template<class T>
    DynamicInstance(const T& objReference)
    :td(&T::_Descriptor()), obj(&objReference) { }

    const type::StructDescriptor<>* td;
    const void* obj;
};


template<class T>
struct [[deprecated("only available for backwards compatibility")]] DynamicInstanceHelper: public DynamicInstance
{
    DynamicInstanceHelper(const T& objReference)
    :DynamicInstance(T::_td(), &objReference)
    { }
};

namespace type {
    struct Struct;
}

std::string to_cbor(const type::Struct& instance, types::property_set_t properties = types::property_set_t::All);

[[deprecated("only available for backwards compatibility")]]
std::string to_cbor(DynamicInstance instance, types::property_set_t properties = types::property_set_t::All);

/**
 * Deserializes DOTS-object encoded in CBOR using a StructDescriptor and pointer
 * @param cborData binaryCbor
 * @param cborSize size of cborData-Buffer
 * @param td StructDescritpor
 * @param data Pointer to an allocated Object of the correct type
 * @return nr of bytes read from input-data (size of CBOR object)
 */
int from_cbor(const uint8_t* cborData, std::size_t cborSize, type::Struct& instance);

[[deprecated("only available for backwards compatibility")]]
int from_cbor(const uint8_t* cborData, std::size_t cborSize, const dots::type::StructDescriptor<>* td, void* data);

/**
 * Skips over a CBOR element
 * @param cborData binaryCbor
 * @param cborSize size of cborData-Buffer
 * @return nr of bytes read from input-data (size of CBOR object)
 */
int skip_cbor(const uint8_t* cborData, std::size_t cborSize);

template<class T>
T decodeInto_cbor(const std::vector<uint8_t> &data)
{
    T obj;

    from_cbor(data.data(), data.size(), obj);
    return obj;
}

}

