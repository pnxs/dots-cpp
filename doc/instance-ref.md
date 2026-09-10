# Instance references

`instance_ref` carries an instance's identity without its state. The C++ model
mapping is `dots::instance_ref_t` for a generic reference and
`dots::typed_instance_ref_t<T>` for `instance_ref<T>`. The target must be a DOTS
struct. References can themselves be key properties, and both forms can be
used in vectors.

```dots
struct Device {
    1: [key] string name;
    2: bool enabled;
}

struct Status {
    1: [key] instance_ref<Device> subject;
    2: bool applied;
}
```

Include `<dots/serialization/CborSerializer.h>` to construct references and
canonical keys:

```cpp
Device device{.name = "eth0", .enabled = true};
auto generic = dots::to_instance_ref(device);
auto typed = dots::to_typed_instance_ref(device);
Status status{.subject = typed, .applied = true};

auto bytes = dots::to_cbor(status);
auto restored = dots::from_cbor<Status>(bytes);

// Produces a partial instance containing exactly the referenced key properties.
dots::type::Registry registry;
auto keysOnly = dots::from_instance_ref(generic, registry);
```

`dots::canonical_key(instance)` returns the canonical CBOR key array. Keys are
sorted by property tag, independent of declaration order. Unset keys appear as
CBOR null in this standalone artifact. `to_instance_ref` and
`to_typed_instance_ref` reject unset keys. A keyless type has the key `[]` and
can be referenced as a singleton.

`instance_ref_t{typeName, keyBytes}` constructs an opaque reference without a
target descriptor. `typed_instance_ref_t<T>{generic}` checks the target name;
`typed_instance_ref_t<T>{keyBytes}` supplies the target name from `T`. These
constructors validate the deterministic encoding and reject nulls, forbidden
CBOR types, malformed UTF-8, trailing bytes, and nesting deeper than 64 levels.
Default-constructed references are empty placeholders and cannot be serialized.
`from_instance_ref` additionally checks the key against the registered target
schema before returning the partial instance.

Equality, ordering, and `std::hash` use the target type name and canonical key
bytes. CBOR encodes generic references as `[typeName, keyArray]`; typed
references encode only `keyArray`. JSON, string, and ASCII output use the opaque
text form `TypeName["key",42]`, with key values in ascending tag order. Strings
use JSON quoting and escaping, integers and booleans are written directly,
and nested keys remain arrays. Binary keys (including UUIDs) use CBOR diagnostic
notation such as `h'00ff'` to distinguish them from text. Keyless references are
written as `TypeName[]`. Text deserialization checks the target name for
typed fields. Descriptor exchange uses `instance_ref` and
`instance_ref<TypeName>` as the type strings and supports opaque references when
the target descriptor is unavailable.

The C++ generator configuration requires a `dots-code-generator` version that
supports `instance_ref` and `instance_ref_format`. Set CMake's `DOTS-CG` to the
appropriate `dcg.py` when multiple generator versions are installed.

See [the wire-format definition](../external/dots/doc/dots-wire-format.md#6-canonical-instance-key-proposed)
for the canonical encoding and golden vectors.
