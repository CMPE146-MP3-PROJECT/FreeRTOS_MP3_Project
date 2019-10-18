
## Folder Structure

The folder structure proposed below not only organizes the source code, but also promotes good practices indicating which code should invoke (or not invoke) code at other folders.

The folder structure takes inspiration from code development at Lockheed Martin. Then, it was tried and tested further and many controllers at Tesla use similar directory structure as the one proposed below.

- L0 - Low level
- L1 - FreeRTOS
- L2 - Standalone utilities
- L3 - Drivers
- L4 - I/O
- L5 - Application

The function invocations shall go from a layer **downwards**, but never upwards; so `L2 Standalone utilities` should never make a function call into the `L3 - drivers` or the Application functions.

### `l0_low_level`

Normally, you will not modify anything at this folder.

`l0_low_level` is responsible to do the following:

- Initialize RAM
- Initialize the clock system
- Jump to `main()`

### `l1_rtos`

Do not modify anything at this is the folder; it is for FreeRTOS source code only.

### `l2_utils`

Typical sources here that can be compiled **without** any other dependency on a library other than standard C library.

This folder contains utility libraries, such as `buffer.h` that the drivers or above layers can utilize. Include code here that does not depend on a lot of other code modules. **Ensure that the code that goes here does not make function calls to higher layers**.

Another common use case is for instance a driver utilizing a buffer from this folder, or an `l4_io` utilizing an `sl_string` from the code modules at this folder.

### `l3_drivers`

We can state the obvious here: All drivers should be placed at this directory and it is allowed to use the RTOS and standalone utilities.

### `l4_io`

This is for sources that perform input-output, such as a CLI handler. You should not put tasks' code here, but rather helper code for the `l5`

### `l5_application`

This is mainly for task level code.
