"""
SCons command line interface
"""

from SCons.Script import *


def cli_init():
    AddOption("--verbose", dest="verbose", action="store_true", default=False)
    AddOption("--no-format", dest="no_format", action="store_true", default=False)
