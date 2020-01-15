import os

from SCons.Script import *


ENVIRONMENTS_DIR = Dir("#/site_scons/environments")


def register_all_environments():
    for filename in os.listdir(ENVIRONMENTS_DIR.abspath):
        SConscript(env_dirnode.File(filename))


def register_environment(target_filename):
    for filename in os.listdir(ENVIRONMENTS_DIR.abspath):
        if target_filename == filename:
            env = SConscript(ENVIRONMENTS_DIR.File(filename))
            break
    else:
        raise KeyError("Unable to find environment file: [{}]".format(ENVIRONMENTS_DIR.File(target_filename)))
    return env
