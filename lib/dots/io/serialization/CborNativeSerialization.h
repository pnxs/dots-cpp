#pragma once

#include "dots/cpp_config.h"
#include <dots/type/StructDescriptor.h>
#include "dots/dots_base.h"

namespace dots {

std::string to_cbor(DynamicInstance instance, property_set properties = PROPERTY_SET_ALL);

/**
 * Deserializes DOTS-object encoded in CBOR using a StructDescriptor and pointer
 * @param cborData binaryCbor
 * @param cborSize size of cborData-Buffer
 * @param td StructDescritpor
 * @param data Pointer to an allocated Object of the correct type
 * @return nr of bytes read from input-data (size of CBOR object)
 */
int from_cbor(const uint8_t* cborData, std::size_t cborSize, const dots::type::StructDescriptor* td, void* data);

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

    from_cbor(data.data(), data.size(), obj._td(), &obj);
    return obj;
}

}

