"""
SCons command line interface
"""

from SCons.Script import *


def cli_init():
    AddOption("--no-format", dest="no_format", action="store_true", default=False)
    AddOption("--project", metavar="<project directory name>", default="lpc40xx_freertos", help="Specify a target project directory to build")
    AddOption("--unit-test", dest="unit_test", action="store_true", default=False)
    AddOption("--verbose", dest="verbose", action="store_true", default=False)
