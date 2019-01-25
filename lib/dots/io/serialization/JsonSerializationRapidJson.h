#pragma once

#include "dots/cpp_config.h"
#include <dots/type/StructDescriptor.h>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>

namespace dots
{

void to_json(const dots::type::StructDescriptor *td, const void *data, rapidjson::Writer<rapidjson::StringBuffer>& writer, property_set properties = PROPERTY_SET_ALL, bool allFields = false);
void to_json(const dots::type::StructDescriptor *td, const void *data, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, property_set properties = PROPERTY_SET_ALL, bool allFields = false);

/**
 * Deserializes a DOTS-object, encoded in JSON
 * @param RapidJson Document-Object
 * @param td StructDescriptor
 * @param data Pointer to an allocated Object of the correct type
 */
void from_json(const rapidjson::Document::ConstObject& jsonDocument, const dots::type::StructDescriptor *td, void *data);

}
