# SJ2-C Getting Started


## Setup and Install

Setup and install should be super simple unless you have Windows, which is not suited for ideal software development. These steps should still get you setup regardless of your OS.

1. Install Python on windows (Mac and Linux should already have that)
    * Skip this for Mac or Linux
    * Follow [this guide](installs/README.md) for Windows install guide
2. Open up a terminal Windows, and type `pip install scons`
    * If any issues on Ubuntu(Linux), try `sudo apt install scons`
3. Install the Board driver from the `drivers` directory

That is it, you should now be ready to build software for your board.


## Compile & Flash

1. Use any IDE and open up the `lpc40xx_freertos` folder.
2. Build the project:
    * From the root directory of this `sjtwo-c` folder, type: `scons`
    * Once you get used to it, also try `scons -j4` to use more threads to build.
3. Invoke the python script to flash your new program
    * `python nxp-programmer/flash.py -d/--device <Device ID> -i/--input <.bin file path>`
    * If `-i` is not provided, then the tool will default to `_build_lpc40xx_freertos/lpc40xx_freertos.bin`
    * See [nxp-programmer README](nxp-programmer/README.md) for more information
    * Example commands:
        * `python nxp-programmer/flash.py -d COM6 -i _build_lpc40xx_freertos/lpc40xx_freertos.bin`
        * `python nxp-programmer/flash.py -d COM3`
        * `python nxp-programmer/flash.py -d /dev/ttyUSB -i _build_lpc40xx_freertos/lpc40xx_freertos.bin`
        * `python nxp-programmer/flash.py -d /dev/tty.SLAB_USBtoUART`
4. After flashing your new program, use your favorite serial terminal to watch the output from your board.
