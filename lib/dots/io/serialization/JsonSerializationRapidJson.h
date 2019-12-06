#pragma once

#include "dots/cpp_config.h"
#include <dots/type/NewStructDescriptor.h>
#include <dots/type/NewFundamentalTypes.h>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>

namespace dots
{

void to_json(const type::Struct& instance, rapidjson::Writer<rapidjson::StringBuffer>& writer, types::property_set_t properties = types::property_set_t::All, bool allFields = false);
void to_json(const type::Struct& instance, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, types::property_set_t properties = types::property_set_t::All, bool allFields = false);

[[deprecated("only available for backwards compatibility")]]
void to_json(const dots::type::StructDescriptor<> *td, const void *data, rapidjson::Writer<rapidjson::StringBuffer>& writer, types::property_set_t properties = types::property_set_t::All, bool allFields = false);

[[deprecated("only available for backwards compatibility")]]
void to_json(const dots::type::StructDescriptor<> *td, const void *data, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, types::property_set_t properties = types::property_set_t::All, bool allFields = false);

/**
 * Deserializes a DOTS-object, encoded in JSON
 * @param RapidJson Document-Object
 * @param td StructDescriptor
 * @param data Pointer to an allocated Object of the correct type
 */
void from_json(const rapidjson::Document::ConstObject& jsonDocument, type::Struct& instance);

[[deprecated("only available for backwards compatibility")]]
void from_json(const rapidjson::Document::ConstObject& jsonDocument, const dots::type::StructDescriptor<> *td, void *data);
}
