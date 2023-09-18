# dots-cpp

The *dots-cpp* project is a C++20 library for type-oriented *interprocess communication (IPC)* with a focus on closed environments. It is based on the *DOTS (Distributed Object in Time and Space)* specification and is its reference implementation.

# TL;DR

DOTS communication is based on objects of custom user data types that are defined in a simple modelling language:

```cpp
struct Foobar {
    1: [key] uint32 foo;
    2: string bar;
    3: timepoint baz;
    4: float64 qux;
}
```

The modelled types are automatically generated during build time and can be used explicitly within an application. Instances of these types can be shared via a publish-subscribe event system:

```cpp
// Application A: Publish a partial instance of type 'Foobar'
dots::publish(Foobar{
    .foo = 42,
    .bar = "foobar",
    .qux = 3.1415f
});
```

Published instances are then automatically distributed to all subscribers of that type:

```cpp
// Application B: Create a subscription to type 'Foobar' with a corresponding event handler
dots::Subscription subscription = dots::subscribe<Foobar>([](const dots::Event<Foobar>& event)
{
    if (const Foobar& instance = event.updated(); instance.foo == 42)
    {
        // ...
    }
    else if (instance.baz.isValid())
    {
        // ...
    }
});
```

# Overview

The dots-cpp library is a complete publish-subscribe IPC solution for both local and distributed applications based on the (currently informal) DOTS specification.

## Features: DOTS

* **Type-oriented**: DOTS uses a self-describing type system that makes it always unambiguous how data is structured and what types are expected. Authors can define types in a simple modeling language and leverage static type support or write dynamic applications using type introspection.
* **Stateful**: By default, objects of a DOTS type have a *cached* state. This state is known and can be inspected by interested applications at any time, even if they subscribe to a particular type after objects were already published.
* **Object-driven**: DOTS is all about the state of objects. Subscribers can react to object changes via events or just use the current state of an object when desired.
* **Efficient**: Objects can be partially updated and by default are serialized using CBOR, which in many cases results in payloads of only "a few bytes" in size.
* **Origin-agnostic**: Subscribers do not have to care about how data is produced, where it is coming from or how it is serialized. This makes it easy to change producers, inject objects for testing, or reproduce system states for debugging or analysis.
* **Transport-agnostic**: Even though DOTS specifies a default binary encoding via CBOR, it can be built upon any transport protocol and serialization method. For example: Processes running on the same machine might want to use UNIX domain sockets, while remote applications could be connected via TCP.
* **Language-agnostic**: DOTS can be implemented in any programming language. DOTS libraries and applications have been written in C#, TypeScript and Python in addition to C++.

## Features: Library

* Modern C++20 library with CMake support.
* Portable with officially maintained support for Linux (with GCC 9.4.0 or Clang 10.0.0) and Windows (with latest MSVC).
* Variety of available transport protocols (TCP, UNIX domain sockets, WebSocket) and serialization methods (CBOR, JSON).
* Small base API that is sufficient for most common uses cases.
* Modular design that allows advanced users to configure, extend or switch components, such as serializers and transport protocols.
* Single-threaded asynchronous execution approach based on Boost Asio.
* Sufficiently low resource usage to be suitable for embedded applications with monolithic kernels (e.g. Yocto Linux).
* Google Test integration.

# Examples

The repository includes a set of example applications:

Introductory examples:
* [roundtrip](./bin/examples/roundtrip/README.md): Performs a simple roundtrip of a static DOTS struct type.
* [object-trace](./bin/examples/object-trace/README.md): Traces "all" DOTS struct objects within a DOTS system dynamically.

Intermediate examples:
* [smart-home](./bin/examples/smart-home/README.md): Controls the logic of multiple fictitious smart-devices.
* [object-reader](./bin/examples/object-reader/README.md): Creates and publishes an object of a DOTS struct type dynamically.

# Dependencies

* CMake >= 3.12
* Boost Libraries[^1] >= 1.74.0 (including program-options)
* Python >= 3.8
* dots-code-generator >= 0.0.4 (https://github.com/pnxs/dots-code-generator, also available via pip (https://pypi.org/project/dots-code-generator))
* C++ compiler supporting at least C++20 (such as GCC 11.4 or MSVC 19.26 (VS 2019 16.6))

[^1]: Boost is linked static by default, change cmake-option 'Boost_USE_STATIC_LIBS' to link it shared.

# Build and Run

This is how dots-cpp can be built and run based on the example of Ubuntu 20.04.

Install general dependencies:

```sh
sudo apt-get update -yq && \
sudo apt-get install -yq \
    build-essential \
    cmake \
    ninja-build \
    python3 \
    python3-pip \
    git
```

Install build dependencies:

```sh
sudo apt-get update -yq && \
sudo apt-get install -yq \
    libboost-dev=1.71.0.0ubuntu2 \
    libboost-program-options-dev=1.71.0.0ubuntu2 \
&& \
pip3 install dots-code-generator
```

Clone repository including submodules:

```sh
git clone --recurse-submodules https://github.com/pnxs/dots-cpp.git && cd dots-cpp
```

Build library:

```sh
cmake -G Ninja -B ./build/ && cmake --build ./build/
```

Run an [example](#examples) (e.g. roundtrip):

```sh
./build/bin/dotsd/dotsd &
./build/bin/examples/roundtrip/roundtrip
```

# License

This project is currently licensed under the GPL-3.0.
