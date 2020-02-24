## General Information

### Floating point

`float` is an IEEE single precision 32-bit floating point number. The Cortex-M4 has hardware support for this data type, and math operations involving this data type will be very fast.

Example of `float f = random() * 1.1;`
```c
    16ac:	ed9f 7a30 	vldr	s14, [pc, #192]	;
    16b0:	eddd 7a03 	vldr	s15, [sp, #12]
    16b4:	482f      	ldr	r0, [pc, #188]	;
    16b6:	ee67 7a87 	vmul.f32	s15, s15, s14
```

`double` is a 64-bit float, and there is no hardware support for this. This means that math operations performed using this data type will be VERY slow.

Same example with double that uses software routine `__aeabi_dmul()` to multiply:
```
    16ce:	e9dd 0104 	ldrd	r0, r1, [sp, #16]
    16d2:	a321      	add	r3, pc, #132	;
    16d4:	e9d3 2300 	ldrd	r2, r3, [r3]
    16d8:	f002 ff60 	bl	459c <__aeabi_dmul>
    16dc:	e9cd 0104 	strd	r0, r1, [sp, #16]
```

----

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
