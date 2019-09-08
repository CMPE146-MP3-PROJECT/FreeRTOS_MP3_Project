"""
SCons site init - executes before SConstruct and SConscripts
"""

from cli import cli_init
from SCons.Script import *


cli_init()
