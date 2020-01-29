# SJ2-C Getting Started

## Setup and Install

Setup and install should be super simple unless you have Windows, which is not suited for ideal software development, but you can still use it. These steps should still get you setup regardless of your OS.

1. Install Python on Windows (Mac and Linux should already have that)
    * Skip this for Mac or Linux
    * Follow [this guide](installs/README.md) for Windows install guide
2. Open up a terminal window or command prompt, and install `scons`:
    * Type `pip install scons`
    * If any issues on Ubuntu(Linux), try `sudo apt install scons`
3. Install the Board driver from the `installs/drivers` directory

That is it, you should now be ready to build software for your board.

### Additional dependencies for Mac and Linux

In order to run the unit-tests, `Ruby` and a `GCC compiler` is required for Mac and Linux. This is because the unit-tests compile an executable you run on your host machine, and it is not compiled for running on the embedded ARM target.

* For *Windows*, we have checked in Ruby and MinGW (Minimal GCC for Windows) already
* In *Linux*, the GCC (development tools) should already be installed as part of default installation of Ubuntu
* For *Mac*, you may have to install GCC equivalent which should be as easy as typing `xcode-select --install`

----

## Compile & Flash

1. Use any IDE and open up the `lpc40xx_freertos` folder
    * We recommend `Visual Studio Code`
    * You can work on your code in an IDE and use command line to compile
2. Build the project:
    * **From the root directory** of this `sjtwo-c` folder, type: `scons`
3. Invoke the python script to flash your new program
    * From the root of `sjtwo-c` folder, type: `python nxp-programmer/flash.py` and it might just work :)
    * The `flash.py` defaults to `lpc40xx_freertos.bin` file and auto detects your SJ2 serial port
    * See [nxp-programmer README](nxp-programmer/README.md) and more examples in the following *Examples* section
4. After flashing your new program, use your favorite serial terminal to watch the output from your board.

### SCons 

Full documentation of the `SCons` command [is listed at this README](README-SCons.md). This should be read so you fully understand how to build various different projects and run the unit-tests.

### Typical Workflow

This describes typical commands you will use to compile and flash the project:

```bash
# 1. Edit your code and save it in Visual Studio Code

# 2. This will run unit tests and compile the `lpc40xx_freertos` project
scons

# 3. Finally, flash the project
python nxp-programmer/flash.py
```

### More advanced stuff
```bash
# Optionally, you can clean and compile the LPC project

# To clean compiled artifacts for the default project `lpc40xx_freertos`
scons -c

# You can compile the `lpc40xx_freertos` project without running unit-tests
# Warning: If unit-tests fail, you will waste a lot of time debugging it on 
# the controller, so do not skip them
scons --no-unit-test

# Compile with multiple threads (use as many threads as your machine has
# Since I have 12, I will use -j12
scons -j12
```

----

## How `flash.py` works

This script takes a COM port and your firmware file to program, however:
*  COM port can be automatically detected if `--port` argument is not provided
*  Firmware file is defaulted to `_build_lpc40xx_freertos/lpc40xx_freertos.bin` if `--input` argument is not provided

Example:
```bash
python nxp-programmer/flash.py --port <Device Port> --input <.bin file path>`
# <Device Port>    is your serial port
# <.bin file path> is the path to your firmware you want to load to the board

# The script can auto-detects your `--port`, so you should be able to flash using:
python nxp-programmer/flash.py --input _build_lpc40xx_freertos/lpc40xx_freertos.bin`
```

### More `flash.py` Examples

Providing an explicit `--port` may be faster to program, but initially you would need to know what `--port` your SJ board is at. Try using `python nxp-programmer/flash.py` which will use the default binary file, and automatically find the port for you, otherwise follow the examples below:

```bash
# All these examples will default to use "_build_lpc40xx_freertos/lpc40xx_freertos.bin"

# Example on Windows:
python nxp-programmer/flash.py --port COM6

# Example on Linux:
python nxp-programmer/flash.py --port /dev/ttyUSB

# Example on Mac:
python nxp-programmer/flash.py --port /dev/tty.SLAB_USBtoUART

# ##################################
# Fully explicit command on windows:
python nxp-programmer/flash.py --port COM6 --input _build_lpc40xx_freertos/lpc40xx_freertos.bin
```

### Advanced Tips

* You can use `-i` (single dash) in place of `--input`
* You can use `-p` (single dash) in place of `--port`
* If `-i` is not provided, then the tool will default to `_build_lpc40xx_freertos/lpc40xx_freertos.bin`
* So, you could use: `python nxp-programmer/flash.py --device /dev/ttyUSB`
