# DOTS Wire Format

**Status: DRAFT** — this document is the beginning of a normative wire-format
definition for DOTS. Sections 2–5 are *descriptive*: they document the
encoding as implemented by dots-cpp (the reference implementation) and must
not be changed without a format version bump. Section 6 (canonical instance
key and the `instance_ref` field types) is a *proposed extension*; dots-rust
is the first implementation.

The key words MUST, MUST NOT, SHOULD, and MAY are to be interpreted as
described in RFC 2119. CBOR refers to RFC 8949.

---

## 1. Overview

DOTS exchanges *instances* of *struct types* between clients and a caching
server. A struct type declares properties, each with a numeric **tag** that is
the sole identity of the property on the wire. Property *names* and the
*declaration order* of properties never appear in serialized instances;
compatibility between programs compiled against different revisions of a type
rests exclusively on tags.

Everything on the wire is CBOR. A transmission consists of a length-framed
pair of (header instance, payload instance), both encoded by the rules in
section 3.

---

## 2. Transmission framing

Two framing formats exist. The format is a property of the channel and is not
negotiated in-band.

### 2.1 Format v1 (legacy)

```
+----------------------+----------------------------+---------------------+
| header size (uint16) | DotsTransportHeader (CBOR) | instance (CBOR)     |
+----------------------+----------------------------+---------------------+
```

1. `header size` — the byte length of the serialized `DotsTransportHeader`,
   as a raw (non-CBOR) 16-bit unsigned integer. **Caution:** dots-cpp writes
   this value in host byte order; on all deployed platforms this is
   little-endian, but it was never formally specified. New implementations of
   v1 MUST write little-endian and MUST NOT run v1 across byte-order
   boundaries.
2. `DotsTransportHeader` — a struct instance (section 3). Its `payloadSize`
   property gives the byte length of the following instance.
3. `instance` — the payload struct instance.

### 2.2 Format v2 (current)

```
+--------------------------------+-------------------+-------------------+
| transmission size (CBOR, 5 B)  | DotsHeader (CBOR) | instance (CBOR)   |
+--------------------------------+-------------------+-------------------+
```

1. `transmission size` — the combined byte length of header and instance,
   encoded as a **fixed-width** CBOR unsigned integer: the initial byte
   `0x1A` (major type 0, 4-byte argument) followed by the value as a 32-bit
   big-endian integer. Writers MUST use this fixed 5-byte form even for small
   values, so the frame length can be patched in place after serialization.
2. `DotsHeader` — a struct instance (section 3) carrying `typeName`,
   `attributes` (the property set contained in the payload), `sentTime`,
   `sender`, and `removeObj`.
3. `instance` — the payload struct instance. Only the properties named by
   `DotsHeader.attributes` are meaningful; a remove transmission
   (`removeObj = true`) SHOULD carry only the key properties, with
   `attributes` set to the type's key property set.

The header struct definitions live in [`../model/`](../model/) and in the
overview document; they are internal DOTS types that every implementation
compiles in.

---

## 3. Struct encoding

A struct instance is encoded as a **CBOR map** (major type 5):

* The map size is the number of *included* properties: the intersection of
  the instance's valid properties with the property set selected by the
  caller (for payload instances: `DotsHeader.attributes`).
* Each map entry is `tag → value`: the property's tag as a CBOR unsigned
  integer, followed by the property's value encoded per section 4.
* Properties that are not valid (unset) are **absent** from the map. CBOR
  `null` is not used to represent an unset property.

Rules:

* Writers SHOULD emit entries in declaration order of the type. This is not
  normative for general transmissions; readers MUST accept entries in any
  order. (For the canonical form in section 6, order IS normative.)
* Readers MUST skip entries whose tag is unknown to their descriptor,
  consuming the value per CBOR skip rules. Unknown tags are not an error;
  they are the mechanism that lets differently-versioned programs
  interoperate.
* Duplicate tags in one map are malformed; a reader MAY reject the
  transmission or use the last occurrence.
* Sub-struct properties are encoded by the same rules, recursively (a nested
  CBOR map of tag → value).

### 3.1 Tags and the property set

Tags are integers in the range **1..31**. This bound comes from the
`property_set` wire type (section 4): a 32-bit mask whose bit *n* corresponds
to tag *n*. Bit 0 is reserved. A type therefore has at most 31 properties
over its whole lifetime, including `removed` ones (whose tags stay burned).

Compatibility rules:

* A tag, once published, is permanently bound to its property's meaning.
  Renaming a property is allowed; re-purposing its tag is not.
* Adding a property with a fresh tag is backward compatible.
* Removing a property requires marking it `removed` so its tag cannot be
  reused.
* Reordering property declarations does not affect serialized instances —
  key properties included (identity artifacts are sorted by tag,
  section 6.3).

---

## 4. Value encodings

| DOTS type          | CBOR encoding |
| ------------------ | ------------- |
| `bool`             | simple value `true` (0xF5) / `false` (0xF4) |
| `int8..int64`      | major type 0 (non-negative) or 1 (negative) |
| `uint8..uint64`    | major type 0 |
| `float16/32/64`    | major type 7, additional info 25/26/27 |
| `string`           | major type 3 (UTF-8 text string) |
| `bytes`            | major type 2 (byte string) |
| `uuid`             | major type 2, exactly 16 bytes (RFC 4122 binary form) |
| `timepoint`        | float64: seconds since the Unix epoch, fractional (µs resolution) |
| `steady_timepoint` | float64: seconds on the sender's monotonic clock, fractional |
| `duration`         | float64: seconds, fractional |
| `property_set`     | major type 0: the 32-bit mask value (bit *n* = tag *n*) |
| enum               | major type 0: the **tag** of the enumerator (NOT its declared value) |
| `vector<T>`        | major type 4 (array); each element encoded as `T` |
| struct             | major type 5 (map), per section 3 |
| `any` (AnyObject)  | array of 2: `[ typeName (text string), payload (byte string) ]`, where payload is the referenced instance encoded per section 3 |
| `instance_ref`     | array of 2: `[ typeName (text string), canonical key (array, section 6) ]` — an identity-only reference to an instance of any type (section 6.5) |
| `instance_ref<T>`  | the canonical key (section 6) of the referenced `T` instance, as a bare array — the target type is fixed by the schema and is not transmitted |

Notes:

* **Enums travel by tag.** Like struct properties, an enumerator's tag — not
  its numeric value — is the wire identity. Enumerator values may be changed
  or reordered without breaking the wire; tags may not.
* Integer width on the wire is independent of the declared width: writers
  SHOULD use the shortest CBOR form that represents the value; readers MUST
  accept any well-formed width and range-check against the declared type.
* `timepoint`/`duration` as float64 means sub-microsecond precision degrades
  for far-future dates; this is an accepted property of the format.
* `instance_ref` is to `any` what identity is to state: `any` carries a full
  serialized instance, `instance_ref` carries only what is needed to *name*
  an instance. Both forms of `instance_ref` are encoded under the
  deterministic profile of section 6.4 so that byte equality of the encoded
  value is instance identity.

---

## 5. Instance identity and lifecycle

Instances of a type are distinguished by the type's **key properties**
(declared `[key]`). Two instances denote the same object exactly when all
key properties compare equal. The server's cache, client containers, and
remove semantics (section 2.2) are all defined over this identity.

Key properties are structural identity, so they carry stricter rules than
ordinary properties:

* Key properties MUST be valid (set) in every published instance of a cached
  type.
* The key property set of a type MUST NOT change once published: adding,
  removing, or re-typing a key property changes the identity of every
  instance and is a new type in all but name.

Nothing in sections 2–4 serializes "the key" as a distinct artifact — key
properties travel inside the instance map like any other property, and
identity is established by structural comparison. Section 6 defines a
canonical byte form of the key for contexts that need identity *as a value*.

---

## 6. Canonical instance key (PROPOSED)

### 6.1 Motivation

Several consumers need an instance's identity as a single comparable,
storable value rather than as a structural predicate: cache indexes, wrapper
types that reference instances of other types (e.g. a generic `Status`
acknowledging arbitrary set-points), REST-style URIs, and log/display
handles. Absent a definition here, each layer invents its own encoding
(JSON arrays, path segments, ad-hoc concatenation) and equality of those
encodings becomes an accidental cross-implementation contract.

### 6.2 Definition

The **canonical key** of an instance is:

> A CBOR array containing the values of the instance's key properties, in
> **ascending tag order**, each encoded per section 4 under the
> deterministic-encoding profile of section 6.4. A key property that is not
> valid encodes as CBOR `null` (0xF6). A type with no key properties has the
> canonical key `[]` (0x80).

The **instance reference** of an instance is:

> A CBOR array of 2: `[ typeName (text string), canonical key ]`, encoded
> under the same deterministic profile.

The instance reference is also the value encoding of the generic
`instance_ref` field type (section 6.5); the canonical key alone is the
value encoding of the typed `instance_ref<T>` field type.

Two canonical keys (or instance references) denote the same instance if and
only if their encoded bytes are equal. Byte equality MUST be equivalent to
structural key equality; the deterministic profile exists to guarantee this.

A key property of reference type (section 6.5) contributes its section 4
encoding to the canonical key like any other key property — canonical keys
may therefore contain nested arrays.

### 6.3 Constraint on key declarations

Canonical-key element order is **ascending tag order**, never declaration
order. Implementations MUST sort by tag when producing a canonical key;
they MUST NOT rely on the declaration order of key properties. Declaration
order therefore stays free for *all* properties, keys included — reordering
declarations never changes an instance's identity (this extends the general
reordering freedom of section 3.1 to key properties).

Consequently, wherever an ordered view of the key matters across programs
or languages — REST-style URI segment mappings, the element order of a
typed reference (section 6.5), display of composite keys — that order is
tag order. Purely program-local orders (e.g. the parameter order of a
generated constructor) MAY follow declaration order, since they never leave
the program that was compiled against that declaration.

Key properties MUST be of scalar type — integers, `string`, `bytes`, `uuid`,
`bool`, or enum — or of reference type (`instance_ref` / `instance_ref<T>`,
section 6.5), whose deterministic encoding makes byte equality valid
identity. Floating-point, `timepoint`, `duration`, vectors, structs, and
`any` MUST NOT be key properties (equality of these is not byte-stable).

### 6.4 Deterministic encoding profile

Within a canonical key or instance reference:

* Integers (including enum tags and array lengths) MUST use the shortest
  CBOR form that represents the value.
* Indefinite-length items MUST NOT be used.
* Text and byte strings MUST use definite length with the shortest length
  encoding.
* No CBOR tags (major type 6) are used.

This is the "core deterministic encoding" of RFC 8949 §4.2.1 restricted to
the types allowed in keys (map ordering rules do not apply, as keys contain
no maps).

### 6.5 Reference field types (`instance_ref`)

The description language gains two reference field types, spelled lower-case
like all built-in types:

* `instance_ref` — the **generic reference**: may reference an instance of
  any type. Its value encoding is the instance reference of section 6.2:
  `[ typeName (text string), canonical key ]`.
* `instance_ref<TypeName>` — the **typed reference**: may only reference
  instances of `TypeName`, which MUST be a struct type. Because the schema
  fixes the target type, the value encoding is the canonical key alone — the
  type name is not transmitted.

Rules:

* The canonical key contained in a reference value MUST NOT contain `null`:
  a reference denotes a complete identity. (`null` elements are legal only
  in the standalone canonical-key artifact of a locally constructed partial
  instance.)
* A reference to a keyless type is legal and denotes the singleton instance;
  its canonical key is `[]` (`0x80`).
* The elements of a typed reference's canonical key correspond one-to-one to
  the target type's key properties in ascending tag order (declaration
  order is irrelevant, section 6.3). A reader that knows the target
  type's descriptor MAY therefore materialize the reference as a partial
  instance containing exactly the key properties; a reader MAY equally treat
  the key as opaque bytes — comparison never requires the descriptor.
* Both forms are valid key property types (section 6.3).
* In descriptor exchange (`StructDescriptorData`), the type strings are
  `instance_ref` and `instance_ref<TypeName>`, analogous to `vector<T>`.

### 6.6 Examples (golden vectors)

For:

```
struct ExampleType {
    1: [key] string name;
    2: bool enabled;
}
```

instance `{ name: "eth0", enabled: true }`:

| Artifact | Bytes (hex) | Diagnostic |
| -------- | ----------- | ---------- |
| instance (both properties) | `A2 01 64 65746830 02 F5` | `{1: "eth0", 2: true}` |
| canonical key | `81 64 65746830` | `["eth0"]` |
| `instance_ref` value | `82 6B 4578616D706C6554797065 81 64 65746830` | `["ExampleType", ["eth0"]]` |
| `instance_ref<ExampleType>` value | `81 64 65746830` | `["eth0"]` |

For a composite key:

```
struct RouteType {
    1: [key] string parent;
    2: [key] string name;
    3: string via;
}
```

instance `{ parent: "eth0", name: "r1", via: "192.0.2.1" }`:

| Artifact | Bytes (hex) | Diagnostic |
| -------- | ----------- | ---------- |
| canonical key | `82 64 65746830 62 7231` | `["eth0", "r1"]` |

Keyless singleton (`struct TimeType { 1: string tz; }`): canonical key `80`
(`[]`).

For a reference used as a key (canonical keys nest):

```
struct StatusType {
    1: [key] instance_ref subject;
    2: bool applied;
}
```

instance with `subject` referencing the `ExampleType` instance above:

| Artifact | Bytes (hex) | Diagnostic |
| -------- | ----------- | ---------- |
| canonical key | `81 82 6B 4578616D706C6554797065 81 64 65746830` | `[["ExampleType", ["eth0"]]]` |

Implementations MUST reproduce these vectors byte-for-byte; each library
SHOULD carry them as conformance tests.

---

## 7. Open points

* Formal definition of the connection handshake (currently prose in
  [dots-overview.md](dots-overview.md)) as part of this document.
* Whether format v1 should be declared historic.
