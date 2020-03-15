"""
SCons site init - executes before SConstruct and SConscripts
"""

import os

from SCons.Script import *

from cli import cli_init

#
# CLI initialization
#

cli_init()

#
# Environment variable initialization
#

environment_variables = {
    "PYTHONPATH": [
        Dir("#/site_scons"),
    ]
}

for env_var, dirnodes in environment_variables.items():
    if env_var not in os.environ:
        os.environ[env_var] = ""

    dirpaths = list(map(lambda dirnode: Dir(dirnode).abspath, dirnodes))
    os.environ[env_var] = os.pathsep.join(dirpaths + os.environ[env_var].split(os.pathsep))
