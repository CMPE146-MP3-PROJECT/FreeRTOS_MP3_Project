This folder contains sample projects that you can build

## lpc40xx_freertos

* This the primary LPC40xx project with all of the sources
* This is the default project that gets compiled when you simply type `scons` without any arguments

Build the project by using any of these commands:
```
scons

scons --project=lpc40xx_freertos

# Build without any printf/scanf of float (saves program flash space, and is faster to program)
scons --project=lpc40xx_freertos --no-float-format

# Build without unit tests
scons --project=lpc40xx_freertos --no-float-format --no-unit-test

# Build with multiple threads on your machine
scons --project=lpc40xx_freertos --no-float-format --no-unit-test -j32

```

## lpc40xx_cmpe30

This is the "simple" main control loop project setup just for CmpE30 which is a freshmen level class at San Jose State University. It re-uses all of the code from `lpc40xx_freertos` except that it has its own main function override.


Build the project by using any of these commands:
```
scons

scons --project=lpc40xx_cmpe30
```

## x86_freertos

This is the FreeRTOS "simulator" on your host machine. For example, you can run a program with multiple FreeRTOS tasks on your Mac or Linux machine (or maybe even windows?)

Use this project to:

* Learn FreeRTOS API
* Experiment with multiple FreeRTOS tasks or queues


```
# Compile
scons --project=x86_freertos

# Run the executable
_build_x86_freertos/./x86_freertos.exe
```

## x86_sandbox

This is to compile a program on your host machine. For example, you can compile an executable that runs on your Mac or Linux machine (or maybe even windows?)

```
# Compile
scons --project=x86_sandbox

# Run the executable
_build_x86_sandbox/./x86_sandbox.exe
```

Use this project to:

* Compile a program for your host machine
* Run unit-tests for code modules
