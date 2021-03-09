from SCons.Script import *

import os


SELF_DIR = Dir(os.path.dirname(__file__))
GENERATE_SYMBOL_TABLE_PY = SELF_DIR.File("generate_symbol_table.py")


"""
SCons tools functions
"""


def generate(env):
    env.AddMethod(symbol_table_generator_method, "SymbolTableGenerator")


def exists(env):
    return True


def symbol_table_generator_method(env, source, target):
    target = Dir(target)
    elf_filenode = File(source[0])

    basename, _ = os.path.splitext(elf_filenode.name)
    output_filenodes = []
    output_filenodes.append(target.File("{}_symbol_table.{}".format(basename, "json")))
    output_filenodes.append(target.File("{}_symbol_table.{}".format(basename, "log")))

    command = [
        "python",
        GENERATE_SYMBOL_TABLE_PY.abspath,
        "$SOURCE",
        "--output $TARGETS",
    ]

    command = " ".join(command)

    result = env.Command(action=command, source=elf_filenode.abspath, target=output_filenodes)

    return result
