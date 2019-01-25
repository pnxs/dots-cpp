struct_templates     = ["struct.dots.cs.dotsT"]
enum_templates       = ["enum.dots.cs.dotsT"]
vector_format        = "List<%s>"

# C# mapping    
type_mapping         = {
    "bool": "bool",
    "int8": "sbyte",
    "int16": "short",
    "int32": "int",
    "int64": "long",
    "uint8": "byte",
    "uint16": "ushort",
    "uint32": "uint",
    "uint64": "ulong",
    "float32": "float",
    "float64": "double",
    "float128": "Decimal",
    "duration": "TimeSpan",
    "timepoint": "DateTime",
    "steady_timepoint": "DateTime",
    "string": "string",
    "property_set": "dots::property_set",
    "uuid": "Guid"
}

