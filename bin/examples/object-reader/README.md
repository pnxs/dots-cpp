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

## Usage

This example is intended to be used in conjunction with other applications to manually manipulate the state of objects.

For example, after starting the [smart-home](../smart-home) application, the object-reader can be used to control the fictitious smart devices.

### Reading single objects

The object format is based on *designated initialization*, which is available for DOTS struct types in dots-cpp itself when using C++20.

#### Unix shell:

```sh
./bin/examples/object-reader/object-reader 'Dimmer{ .id = "LivingRoom_MasterDimmer", .brightness = 42 }'
./bin/examples/object-reader/object-reader 'StatelessSwitch{ .id = "Stairwell_LowerSwitch" }'
./bin/examples/object-reader/object-reader 'StatelessSwitch{ .id = "Stairwell_UpperSwitch" }'
./bin/examples/object-reader/object-reader 'Switch{ .id = "Basement_MotionSwitch", .enabled = true }'
...
```

#### PowerShell:

```powershell
.\bin\examples\object-reader\object-reader.exe 'Dimmer{ .id = ""LivingRoom_MasterDimmer"", .brightness = 42 }'
.\bin\examples\object-reader\object-reader.exe 'StatelessSwitch{ .id = ""Stairwell_LowerSwitch"" }'
.\bin\examples\object-reader\object-reader.exe 'StatelessSwitch{ .id = ""Stairwell_UpperSwitch"" }'
.\bin\examples\object-reader\object-reader.exe 'Switch{ .id = ""Basement_MotionSwitch"", .enabled = true }'
...
```

Note that when using PowerShell on Windows, quotes must be specified redundantly.

### Reading multiple objects

When reading of multiple objects is required, it might be easier to use a file and pipe it line-by-line through the object-reader.

#### dots-objects.txt:
```cpp
Dimmer{ .id = "LivingRoom_MasterDimmer", .brightness = 42 }
StatelessSwitch{ .id = "Stairwell_LowerSwitch" }
Switch{ .id = "Basement_MotionSwitch", .enabled = true }
```

#### Unix shell:

```sh
xargs -n 1 -d \\n ./bin/examples/object-reader/object-reader < dots-objects.txt
```

#### PowerShell:

```sh
Get-Content -Path dots-objects.txt | % { .\bin\examples\object-reader\object-reader.exe $_ }
```
