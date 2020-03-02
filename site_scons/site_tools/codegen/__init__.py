from SCons.Script import *

import os
import subprocess
import sys

SELF_DIR = Dir(os.path.dirname(__file__))
DBC_TO_C_PY = SELF_DIR.File("dbc_to_c.py")

SUPPORTED_FORMATS = [
    "dbc",
]


"""
SCons tools functions
"""


def generate(env):
    env.AddMethod(database_code_generator_method, "DatabaseCodeGenerator")


def exists(env):
    return True


def database_code_generator_method(env, source, target, node_name=None):
    target = Dir(target)

    database_filenodes = []
    if source is Dir:
        for node in os.path.listdir(source.abspath):
            ext = os.path.splitext(node)[0][1:]
            if ext in SUPPORTED_FORMATS:
                database_filenodes.append(File(os.path.join(source.abspath, node)))
    elif isinstance(source, list):
        database_filenodes = list(map(File, source))
    else:
        database_filenodes.append(File(source))

    env.Append(CPPPATH=[target.abspath])

    error = 0
    for filenode in database_filenodes:
        basename, _ = os.path.splitext(filenode.name)
        output_filenode = target.File("{}.{}".format(basename, "h"))

        command = [
            "python",
            DBC_TO_C_PY.abspath,
            "--dbc=$SOURCE",
            "--output=$TARGET",
        ]

        if node_name is not None:
            command.append("--dbc-node-name={}".format(node_name))

        command = " ".join(command)

        result = env.Command(action=command, source=filenode.abspath, target=output_filenode)

    return result
