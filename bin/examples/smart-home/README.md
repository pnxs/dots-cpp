# Example: smart-home

This is an intermediate example that demonstrates how to use the dots-cpp library in a way that resembles simple real-life DOTS applications.

## Description

The example is based on a fictional scenario in which a home owner has set up several *smart* devices and wants to program them in a particular way.

It is assumed that the applications for the smart devices already exist and that they define the types contained in [model.dots](./src/model.dots).

The desired logic is as follows:

* Living room: Control the brightness of one lamp at the ceiling and one next to the couch simultaneously with a single smart dimmer device.
* Stairwell: Toggle a single lamp by two stateless smart switches located at each end of the stairwell.
* Basement: Control a single lamp with a smart motion sensor. The light must also kept being turned on for some time after no motion is detected anymore.

## Focus

* Structuring an application with several components.
* Subscribing to a DOTS struct type with a member function handler.
* Accessing instances of a DOTS struct type in the local cache (i.e. container).
* Adding and removing timers.
* Using uncached DOTS types.
* Testing components using the Google Test integration.

## Usage

The example requires a running DOTS host (e.g. [dotsd](../../dotsd)) to connect to.

The application can then be started from the build directory as follows:

```sh
./bin/examples/smart-home/smart-home
```

To control any of the fictitious smart devices, corresponding objects can be published via the [object-reader](../object-reader) example:

```sh
./bin/examples/object-reader/object-reader 'Dimmer{ .id = "LivingRoom_MasterDimmer", .brightness = 42 }'
./bin/examples/object-reader/object-reader 'StatelessSwitch{ .id = "Stairwell_LowerSwitch" }'
./bin/examples/object-reader/object-reader 'StatelessSwitch{ .id = "Stairwell_UpperSwitch" }'
./bin/examples/object-reader/object-reader 'Switch{ .id = "Basement_MotionSwitch", .enabled = true }'
...
```

To have a better idea of what is going on, the [object-trace](../object-trace) example can be run in parallel:

```sh
./bin/examples/object-trace/object-trace
```

## Unit tests

The example also features unit tests, which can be run from the build directory as follows:

```
./bin/examples/smart-home/smart-home-tests
```
