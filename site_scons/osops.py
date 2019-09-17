"""
osops - OS related operations
"""

import os
import sys

from SCons.Script import *


def prepend_env_var(env, dirnode, env_var="PATH"):
    """
    Prepend a directory to an Environment's PATH environment variable
    :param env: Environment object (Environment)
    :param dirnode: A directory node to prepend to the PATH environment variable (Dir)
    :param env_var: Environment variable (str)
    """
    env.PrependENVPath(env_var, Dir(dirnode).abspath)


def append_env_var(env, dirnode, env_var="PATH"):
    """
    Prepend a directory to an Environment's PATH environment variable
    :param env: Environment object (Environment)
    :param dirnode: A directory node to prepend to the PATH environment variable (Dir)
    :param env_var: Environment variable (str)
    """
    env.AppendENVPath(env_var, Dir(dirnode).abspath)


def is_windows():
    return "win32" == sys.platform


def is_linux():
    return sys.platform.startswith("linux")


def is_macos():
    return "darwin" == sys.platform
