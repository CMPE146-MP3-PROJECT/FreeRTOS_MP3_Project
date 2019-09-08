"""
SCons command line interface
"""

from SCons.Script import *


def cli_init():
    AddOption("--verbose", dest="verbose", action="store_true", default=False)
