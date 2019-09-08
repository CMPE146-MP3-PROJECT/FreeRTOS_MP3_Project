"""
SCons entry point

Invoke the command "scons" with a shell while the current directory is this file's directory

References:
- https://scons.org/
- https://github.com/SCons/scons/wiki
"""

import os

import fsops

"""
Define naming conventions
"""
TARGET_NAME = Dir(".").name


"""
Define file nodes and directory nodes
"""
PROJECT_DIR = Dir(".")
SOURCE_DIR = PROJECT_DIR.Dir("source")

""" Build artifact nodes """
VARIANT_DIR = PROJECT_DIR.Dir("_build")
OBJ_DIR = VARIANT_DIR.Dir("obj")
MAP_FILE = VARIANT_DIR.File("{}.map".format(TARGET_NAME))
SIZE_FILE = VARIANT_DIR.File("{}.size".format(TARGET_NAME))

""" Source and include nodes """
INCLUDE_DIRS = []

INCLUDE_DIRS_ROOT = [
    SOURCE_DIR,
]

SRC_DIRS_ROOT = [
    SOURCE_DIR,
]

LINKER_FILES = [
    SOURCE_DIR.File("platform/linker/layout_lpc4078.ld"),
]

EXCLUDED_SRC_FILES = []


"""
Import build environment
"""
SConscript(PROJECT_DIR.File("env_arm"))
Import("env_arm")

env_arm.VariantDir(variant_dir=VARIANT_DIR, src_dir=Dir("."), duplicate=0)

env_arm["CPPDEFINES"] += []

env_arm["LINKFLAGS"] += [
    "-Wl,-Map,{}".format(MAP_FILE.abspath),
]


"""
Search and group files to build
"""

""" Search and group source files and source directories """
target_src_filenodes = []
target_src_dirnodes = []
for dir in SRC_DIRS_ROOT:
    src_filenodes, src_dirnodes, _, _ = fsops.scan_tree(dir)
    target_src_filenodes.extend(src_filenodes)
    target_src_dirnodes.extend(src_dirnodes)

""" Group linker scripts """
for linker_file in LINKER_FILES:
    env_arm["LINKFLAGS"].append("-T{}".format(File(linker_file).abspath))

""" Search and group include paths """
env_arm["CPPPATH"].extend(INCLUDE_DIRS)
for dir in INCLUDE_DIRS_ROOT:
    _, _, _, include_dirnodes = fsops.scan_tree(dir)
    env_arm["CPPPATH"].extend(include_dirnodes)

""" Filter build files """
target_src_filenodes = fsops.filter_files(target_src_filenodes, EXCLUDED_SRC_FILES)
target_src_filenodes = fsops.remove_duplicate_filenodes(target_src_filenodes)

"""
Perform builds
"""

""" Compile all sources -> objects """
obj_filenodes = []
for src_filenode in target_src_filenodes:
    dest_filepath = fsops.ch_target_filenode(src_filenode, OBJ_DIR, "o")
    new_obj_filenodes = env_arm.Object(target=dest_filepath, source=src_filenode)
    obj_filenodes.extend(new_obj_filenodes)

elf_filenodes = env_arm.Program(target=VARIANT_DIR.File("{}.elf".format(TARGET_NAME)), source=obj_filenodes)
hex_filenodes = env_arm.Objcopy(target=VARIANT_DIR.File("{}.hex".format(TARGET_NAME)), source=elf_filenodes)
hex_filenodes = env_arm.Objcopy(target=VARIANT_DIR.File("{}.bin".format(TARGET_NAME)), source=elf_filenodes)
lst_filenodes = env_arm.Objdump(target=VARIANT_DIR.File("{}.lst".format(TARGET_NAME)), source=elf_filenodes)
size_filenodes = env_arm.Size(target=VARIANT_DIR.File("{}.size".format(TARGET_NAME)), source=elf_filenodes)

Depends(elf_filenodes, LINKER_FILES)
