"""
SCons command line interface
"""

from SCons.Script import *


def cli_init():
    AddOption("--dbc-node-name", default=None)
    AddOption("--no-clang-format", action="store_true", default=False)
    AddOption("--no-float-format", action="store_true", default=False)
    AddOption("--no-unit-test", action="store_true", default=False)
    AddOption("--project", metavar="<project directory name>", default="lpc40xx_freertos", help="Specify a target project directory to build")
    AddOption("--test-output", action="store_true", default=False)
    AddOption("--timeout", type=int, default=5, help="Unit test timeout in seconds")
    AddOption("--verbose", action="store_true", default=False)
