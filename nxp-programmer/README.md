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
> python flash.py -d/--device <Device ID> -i/--input <.bin file path>
```

### Example Usage

```powershell
> python flash.py -d COM6 -i ..\_build_lpc40xx_freertos\lpc40xx_freertos.bin
```

```bash
> python flash.py -d /dev/ttyUSB0 -i ../_build_lpc40xx_freertos/lpc40xx_freertos.bin
```
