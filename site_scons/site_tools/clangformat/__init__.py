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


def clang_format_method(self, filenode, verbose=False):
    """
    Auto format an input file.
    Note, this operation will change the contents in the source file.

    :param filenode: Source or header file to be formatted (File)
    """
    filenode = File(filenode)
    if is_format_necessary(filenode=filenode):
        perform_format(filenode=filenode, verbose=verbose)


"""
Helper functions
"""


def get_clang_format_binary_filenode():
    if osops.is_windows():
        ret = SELF_DIRNODE.File("win32/clang-format.exe")
    elif osops.is_macos():
        ret = SELF_DIRNODE.File("mac/clang-format")
    else:  # osops.is_linux()
        ret = SELF_DIRNODE.File("linux/clang-format")
    return ret


def is_format_necessary(filenode):
    clang_format_binary_filenode = get_clang_format_binary_filenode()
    cmd = [
        clang_format_binary_filenode.abspath,
        "-i",
        filenode.abspath,
        "--output-replacements-xml",
    ]
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    lines = stdout.decode().split("\n")

    # If number of "<replacement>" node in XML string is greater than 1, then clang format is required
    return sum("<replacement" in line for line in lines) > 1


def perform_format(filenode, verbose=False):
    clang_format_binary_filenode = get_clang_format_binary_filenode()
    cmd = [
        clang_format_binary_filenode.abspath,
        "-i",
        filenode.abspath,
    ]
    print("Clang formatting: [{}]".format(os.path.relpath(filenode.abspath)))
    if verbose:
        print(" ".join(cmd))
    return subprocess.call(cmd)
