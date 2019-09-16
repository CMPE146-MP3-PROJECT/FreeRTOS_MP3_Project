# SJ2-C Getting Started


## Setup and Install

Setup and install should be super simple unless you have Windows, which is not suited for ideal software development, but you can still use it. These steps should still get you setup regardless of your OS.

1. Install Python on Windows (Mac and Linux should already have that)
    * Skip this for Mac or Linux
    * Follow [this guide](installs/README.md) for Windows install guide
2. Open up a terminal window or command prompt, and install `scons`:
    * Type `pip install scons`
    * If any issues on Ubuntu(Linux), try `sudo apt install scons`
3. Install the Board driver from the `drivers` directory

That is it, you should now be ready to build software for your board.


## Compile & Flash

1. Use any IDE and open up the `lpc40xx_freertos` folder
    * You can work on your code in an IDE and use command line to compile
2. Build the project:
    * **From the root directory** of this `sjtwo-c` folder, type: `scons`
    * Once you get used to it, also try `scons -j4` to use more threads to build.
    * You can type `scons -clean` to clean the project
3. Invoke the python script to flash your new program
    * `python nxp-programmer/flash.py --device <Device ID> --input <.bin file path>`
        * The `<Device ID>` is your serial port, and `<.bin file path>` is the path to your firmware you want to load to the board
    * Example on Windows:
        * `python nxp-programmer/flash.py --device COM6 --input _build_lpc40xx_freertos/lpc40xx_freertos.bin`
    * Example on Linux:
        * `python nxp-programmer/flash.py --device /dev/ttyUSB --input _build_lpc40xx_freertos/lpc40xx_freertos.bin`
    * Example on Mac:
        * `python nxp-programmer/flash.py --device /dev/tty.SLAB_USBtoUART --input _build_lpc40xx_freertos/lpc40xx_freertos.bin`
    * See [nxp-programmer README](nxp-programmer/README.md)
4. After flashing your new program, use your favorite serial terminal to watch the output from your board.


### Advanced Tips

* You can use `-i` (single dash) in place of `--input`
* You can use `-d` (single dash) in place of `--device`
* If `-i` is not provided, then the tool will default to `_build_lpc40xx_freertos/lpc40xx_freertos.bin`
* So, you could use: `python nxp-programmer/flash.py --device /dev/ttyUSB`
