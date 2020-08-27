#pragma once

#include <dots/type/StructDescriptor.h>
#include <dots/type/FundamentalTypes.h>

namespace dots
{

struct ToJsonOptions
{
    bool allFields = false; ///< serialize also properties, that are not valid (value will be NULL)
    bool prettyPrint = true; ///< should the JSON be pretty printed (with indentation and newlines)?
};

/**
 * Serializes a DOTS-object into JSON
 * @param td StructDescriptor of the type
 * @param data Pointer to DOTS-object
 * @param properties allows to serialize only a subset of the properties (default ALL)
 * @param opts allows to set options that change the JSON output
 * @return the JSON-serialized string of the DOTS-object
 */
std::string to_json(const type::Struct& instance, types::property_set_t properties = types::property_set_t::All, const ToJsonOptions& opts = {});

[[deprecated("only available for backwards compatibility")]]
std::string to_json(const dots::type::StructDescriptor<>* td, const void* data, types::property_set_t properties = types::property_set_t::All, const ToJsonOptions& opts = {});

/**
 * Deserializes a DOTS-object, encoded in JSON
 * @param jsonString
 * @param td StructDescriptor
 * @param data Pointer to an allocated Object of the correct type
 * @return nr of bytes read from input-data (size of JSON object)
 */
int from_json(const std::string& jsonString, type::Struct& instance);

[[deprecated("only available for backwards compatibility")]]
int from_json(const std::string& jsonString, const dots::type::StructDescriptor<>* td, void* data);

}