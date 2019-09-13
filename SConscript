import sys

import fsops
import osops

"""
Imports
"""
Import("project_dirnode")


"""
Define file nodes and directory nodes
"""
REPO_ROOT_DIR = Dir("#")
PROJECT_DIR = project_dirnode

""" Build artifact nodes """
VARIANT_DIR = REPO_ROOT_DIR.Dir("_build_{}".format(PROJECT_DIR.name))
OBJ_DIR = VARIANT_DIR.Dir("obj")
MAP_FILE = VARIANT_DIR.File("{}.map".format(PROJECT_DIR.name))
SIZE_FILE = VARIANT_DIR.File("{}.size".format(PROJECT_DIR.name))

""" Source and include nodes """
INCLUDE_DIRS = []

INCLUDE_DIRS_ROOT = [
    PROJECT_DIR,
]

SRC_DIRS_ROOT = [
    PROJECT_DIR,
]

LINKER_FILES = [
    PROJECT_DIR.File("l0_lowlevel/linker/layout_lpc4078.ld"),
]

EXCLUDED_SRC_FILES = []


"""
Import build environment
"""
SConscript(REPO_ROOT_DIR.File("env_arm"))
Import("env_arm")

if osops.is_windows():
  print("-- Using ARM compiler on WINDOWS")
  osops.prepend_env_var(env_arm, REPO_ROOT_DIR.Dir("compiler/windows/gcc-arm-none-eabi-8-2019-q3-update/bin"))
elif osops.is_linux():
  print("-- Using ARM compiler on LINUX")
  osops.prepend_env_var(env_arm, REPO_ROOT_DIR.Dir("compiler/linux/gcc-arm-none-eabi-8-2019-q3-update/bin"))
elif osops.is_macos():
  print("-- Using ARM compiler on MAC")
  osops.prepend_env_var(env_arm, REPO_ROOT_DIR.Dir("compiler/mac/gcc-arm-none-eabi-8-2019-q3-update/bin"))
else:
  print("[{}] is an unsupported OS!".format(sys.platform))
  exit(-1)

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

elf_filenodes = env_arm.Program(target=VARIANT_DIR.File("{}.elf".format(PROJECT_DIR.name)), source=obj_filenodes)
hex_filenodes = env_arm.Objcopy(target=VARIANT_DIR.File("{}.hex".format(PROJECT_DIR.name)), source=elf_filenodes)
bin_filenodes = env_arm.Objcopy(target=VARIANT_DIR.File("{}.bin".format(PROJECT_DIR.name)), source=elf_filenodes)
lst_filenodes = env_arm.Objdump(target=VARIANT_DIR.File("{}.lst".format(PROJECT_DIR.name)), source=elf_filenodes)
size_filenodes = env_arm.Size(target=VARIANT_DIR.File("{}.size".format(PROJECT_DIR.name)), source=elf_filenodes)

Depends(elf_filenodes, LINKER_FILES)
