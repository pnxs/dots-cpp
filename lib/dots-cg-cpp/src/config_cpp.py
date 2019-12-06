struct_templates = ["struct.dots.h.dotsT"]
enum_templates   = ["enum.dots.h.dotsT"]
vector_format    = "vector_t<%s>"

type_mapping     = {
    "bool": "bool_t",
    "int8": "int8_t",
    "int16": "int16_t",
    "int32": "int32_t",
    "int64": "int64_t",
    "uint8": "uint8_t",
    "uint16": "uint16_t",
    "uint32": "uint32_t",
    "uint64": "uint64_t",
    "float32": "float32_t",
    "float64": "float64_t",
    "float128": "float64_t",
    "duration": "duration_t",
    "timepoint": "timepoint_t",
    "steady_timepoint": "steady_timepoint_t",
    "string": "string_t",
    "property_set": "property_set_t",
    "uuid": "uuid_t",
}

