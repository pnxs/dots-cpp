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
