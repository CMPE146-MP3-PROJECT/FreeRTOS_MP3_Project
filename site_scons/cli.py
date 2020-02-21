"""
SCons command line interface
"""

from SCons.Script import *


def cli_init():
    AddOption("--no-format", action="store_true", default=False)
    AddOption("--no-unit-test", action="store_true", default=False)
    AddOption("--project", metavar="<project directory name>", default="lpc40xx_freertos", help="Specify a target project directory to build")
    AddOption("--test-output", action="store_true", default=False)
    AddOption("--verbose", action="store_true", default=False)
