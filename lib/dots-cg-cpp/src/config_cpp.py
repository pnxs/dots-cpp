struct_templates = ["struct.dots.h.dotsT"]
enum_templates   = ["enum.dots.h.dotsT"]
vector_format    = "dots::Vector<%s>"

type_mapping     = {
    "bool": "bool",
    "int8": "int8_t",
    "int16": "int16_t",
    "int32": "int32_t",
    "int64": "int64_t",
    "uint8": "uint8_t",
    "uint16": "uint16_t",
    "uint32": "uint32_t",
    "uint64": "uint64_t",
    "float32": "float",
    "float64": "double",
    "float128": "long double",
    "duration": "Duration",
    "timepoint": "TimePoint",
    "steady_timepoint": "SteadyTimePoint",
    "string": "std::string",
    "property_set": "dots::property_set",
    "uuid": "dots::uuid",
}

