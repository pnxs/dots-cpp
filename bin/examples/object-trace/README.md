# Example: object-trace

This is an introductory example that demonstrates how to write a basic dynamic DOTS application.

The term *dynamic* in the context of DOTS refers to working with types without knowing their definitions *statically* (i.e. at compile time). This is often required for generic tools and utilities that do not have any prior knowledge about the types they encounter.

## Description

The example consists of a small utility application that traces "all" objects within a DOTS system. This is done by subscribing to to every DOTS struct type that is known to the DOTS host and writing corresponding events to the standard output.

Because subscribing always results in a *create* event for every currently existing object if the type is *cached*, this will also effectively print the current cache of all such types when the application is started.

Note that objects will also be traced when their types are made available while the application is already running

## Focus

* Creating and executing a DOTS application.
* Subscribing to DOTS type descriptors.
* Subscribing dynamically to a DOTS struct type (i.e to a type that is not statically known).
* Checking flags of a DOTS struct type.
* Retrieving meta information from a DOTS event (e.g. at which point in time the object was published).
* Getting a string representation of a DOTS struct type instance.

## Usage

The example requires a running DOTS host (e.g. [dotsd](../../dotsd)) to connect to.

The application can then be started from the build directory as follows:

```sh
./bin/examples/object-trace/object-trace
```

To actually have something to trace, another example (e.g. [roundtrip](../roundtrip/)) can be run in parallel:

```sh
./bin/examples/roundtrip/roundtrip
```
