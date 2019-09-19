## Introduction

This tool should be used to flash firmware onto an LPC target.

`flash.py` is the main interface for NXP programming and is a wrapper of `nxpprog`.

References:
- https://github.com/kammce/nxpprog

## Prerequisites

- Python 2.7.x or 3.x.x
    - https://www.python.org/downloads
- CP210x USB to UART driver
    - https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers

## Usage

```bash
> python flash.py -p/--port <Device ID> -i/--input <.bin file path>
```

### Examples

You would have to know the `COM` port, which is named differently for each OS.

Windows:

```bash
> python flash.py -p COM6 -i ..\_build_lpc40xx_freertos\lpc40xx_freertos.bin
```

Linux:
```bash
> python flash.py -p /dev/ttyUSB0 -i ../_build_lpc40xx_freertos/lpc40xx_freertos.bin
```

Mac:
```bash
> python flash.py -p /dev/cu.SLAB_USBtoUART -i ../_build_lpc40xx_freertos/lpc40xx_freertos.bin
```
