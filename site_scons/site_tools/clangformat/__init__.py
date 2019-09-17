"""
LLVM Clang Format
"""

import os
import subprocess

from SCons.Script import *

import osops

SELF_DIRNODE = Dir(os.path.dirname(__file__))


"""
SCons tools functions
"""


def generate(env):
    env.AddMethod(clang_format_method, "ClangFormat")


def exists():
    return True


"""
Environment functions
"""


def clang_format_method(self, filenode):
    """
    Perform auto formatter against an input file.
    Note, this operation will change the contents in the source file.

    :param filenode: Source or header file to be formatted (File)
    :return: Results
    """
    filenode = File(filenode)
    clang_format_binary_filenode = get_clang_format_binary_filenode()
    # cmd = "{} -i={}".format(clang_format_binary_filenode.abspath, filenode.abspath)
    cmd = [
        clang_format_binary_filenode.abspath,
        "-i",
        filenode.abspath,
    ]
    print(" ".join(cmd))
    subprocess.call(cmd)


"""
Helper functions
"""


def get_clang_format_binary_filenode():
    clang_format_filename = "clang-format"
    if osops.is_windows():
        ret = SELF_DIRNODE.File("win64/clang-format.exe")
    elif osops.is_macos():
        ret = SELF_DIRNODE.File("mac/clang-format")
    else:  # osops.is_linux()
        ret = SELF_DIRNODE.File("linux/clang-format")
    return ret
