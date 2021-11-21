# Example: roundtrip

This is a simple introductory example that demonstrates basic usage of the dots-cpp library.

## Description

The example consists of a small application that performs a publishing roundtrip.

The application subscribes to a specific type and then publishes a corresponding instance itself. After receiving the publish event, another instance will be published and the process repeated until a specific limit is reached. After that, the application will exit automatically.

## Focus

* Creating and executing a DOTS application.
* Modeling a simple DOTS struct type.
* Creating instances of a DOTS struct type including initialization.
* Publishing instances of a DOTS struct type via the global API.
* Subscribing to a DOTS struct type via the global API.
* Handling events of a DOTS struct type.
