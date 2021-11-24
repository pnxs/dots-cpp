# Example: object-reader

This is an intermediate example that demonstrates how to dynamically publish objects of DOTS struct types.

The term *dynamic* in the context of DOTS refers to working with types without knowing their definitions *statically* (i.e. at compile time). This is often required for generic tools and utilities that do not have any prior knowledge about the types they encounter.

## Description

The example consists of a small utility application that publishes a DOTS object that is provided as a command line argument.

The given object can be of any type that is known to the DOTS host.

If the type is unknown when the application is started, the application will wait until the type becomes available.

## Focus

* Creating a *single-shot* application that automatically exits when the task is done.
* Creating an instance of a DOTS struct type from a string representation.
* Publishing an object of a DOTS struct type dynamically (i.e without knowing the type statically).
