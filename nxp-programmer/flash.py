from __future__ import print_function
from argparse import ArgumentParser
import os
import subprocess
import sys

from nxpprog import NXPPROG_PY

SELF_DIRPATH = os.path.dirname(__file__)
REPO_ROOT_DIRPATH = os.path.join(SELF_DIRPATH, "..")
DEFAULT_BIN_FILEPATH = os.path.join(REPO_ROOT_DIRPATH, "_build_lpc40xx_freertos", "lpc40xx_freertos.bin")

FAILURE = r"""
_____ _    ___ _    _   _ ____  _____
|  ___/ \  |_ _| |  | | | |  _ \| ____|
| |_ / _ \  | || |  | | | | |_) |  _|
|  _/ ___ \ | || |__| |_| |  _ <| |___
|_|/_/   \_\___|_____\___/|_| \_\_____|
"""

SUCCESS = r"""
 ____  _   _  ____ ____ _____ ____ ____
/ ___|| | | |/ ___/ ___| ____/ ___/ ___|
\___ \| | | | |  | |   |  _| \___ \___ \
 ___) | |_| | |__| |___| |___ ___) |__) |
|____/ \___/ \____\____|_____|____/____/
"""


def get_args():
    arg_parser = ArgumentParser(description=r"Example usage: python flash.py -d COM6 -i ..\_build\sjtwo-c.bin")
    arg_parser.add_argument(
        "-i", "--input",
        metavar="<FILE>",
        type=str,
        help="File path of firmware (.bin) to flash onto target",
        default=None,
    )

    # --device left here for backward compatibility, use -p or --port
    arg_parser.add_argument(
        "-p", "-d", "--port", "--device",
        metavar="<STRING or FILE>",
        type=str,
        help="USB to Serial device ID (i.e. COM6 or /dev/ttyUSB0)",
        default="",
    )
    arg_parser.add_argument(
        "-v", "--verbose",
        action="store_true"
    )

    args = arg_parser.parse_args()

    """
    Perform sanity checks against arguments
    """
    if args.input is None:
        args.input = DEFAULT_BIN_FILEPATH
        if not os.path.isfile(args.input):
            print("Default firmware file [{}] does not exist! Specify firmware file path using `-i` option.".format(os.path.abspath(args.input)))
            sys.exit(1)  # Exit early

    if not args.input.endswith(".bin"):
        print("The input firmware file [{}] must be in .bin format!".format(os.path.abspath(args.input)))
        sys.exit(1)  # Exit early

    return args


def main():
    args = get_args()
    input_filepath = os.path.abspath(args.input)
    device_id = args.port
    verbose = args.verbose

    if device_id:
        print("Flashing file [{}] using device ID [{}]".format(input_filepath, device_id))
    else:
        print("Flashing file [{}] using automatic port selection".format(input_filepath))
    sys.stdout.flush()

    cmd = [
        "python",
        NXPPROG_PY,
        "--binary={}".format(input_filepath),
        "--device={}".format(device_id),
    ]
    if verbose:
        print("Using command:\n{}".format(" ".join(cmd)))
        sys.stdout.flush()

    error = subprocess.call(cmd)
    return error


if __name__ == "__main__":
    _error = main()
    if not _error:
        print(SUCCESS)
    else:
        print(FAILURE)
    sys.exit(_error)
