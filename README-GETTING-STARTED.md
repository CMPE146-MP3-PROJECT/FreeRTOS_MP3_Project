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


## Compile & Flash

1. Use any IDE and open up the `lpc40xx_freertos` folder
    * You can work on your code in an IDE and use command line to compile
2. Build the project:
    * **From the root directory** of this `sjtwo-c` folder, type: `scons`
    * Once you get used to it, also try `scons -j4` to use more threads to build.
    * You can type `scons --clean` to clean the project
3. Invoke the python script to flash your new program
    * From the root of `sjtwo-c` folder, type: `python nxp-programmer/flash.py` and it might just work :)
    * The `flash.py` defaults to `lpc40xx_freertos.bin` file and auto detects your SJ2 serial port
    * See [nxp-programmer README](nxp-programmer/README.md) and more examples in the following `Examples` section
4. After flashing your new program, use your favorite serial terminal to watch the output from your board.

### How `flash.py` works

This script takes a COM port and your firmware file to program, however:
*  COM port can be automatically detected if `--port` argument is not provided
*  Firmware file is defaulted to `_build_lpc40xx_freertos/lpc40xx_freertos.bin` if `--input` argument is not provided

Example:

* `python nxp-programmer/flash.py --port <Device Port> --input <.bin file path>`
    * The `<Device Port>` is your serial port, and `<.bin file path>` is the path to your firmware you want to load to the board
* The script can auto-detect your `--port`, so you should be able to flash using:
    * `python nxp-programmer/flash.py --input _build_lpc40xx_freertos/lpc40xx_freertos.bin`

### More `flash.py` Examples

Providing an explicit `--port` may be faster to program, but initially you would need to know what `--port` your SJ board is at. Try using `python nxp-programmer/flash.py` which will use the default binary file, and automatically find the port for you, otherwise follow the examples below:

* Example on Windows:
    * `python nxp-programmer/flash.py --port COM6 --input _build_lpc40xx_freertos/lpc40xx_freertos.bin`
* Example on Linux:
    * `python nxp-programmer/flash.py --port /dev/ttyUSB --input _build_lpc40xx_freertos/lpc40xx_freertos.bin`
* Example on Mac:
    * `python nxp-programmer/flash.py --port /dev/tty.SLAB_USBtoUART --input _build_lpc40xx_freertos/lpc40xx_freertos.bin`

### Advanced Tips

* You can use `-i` (single dash) in place of `--input`
* You can use `-p` (single dash) in place of `--port`
* If `-i` is not provided, then the tool will default to `_build_lpc40xx_freertos/lpc40xx_freertos.bin`
* So, you could use: `python nxp-programmer/flash.py --device /dev/ttyUSB`
