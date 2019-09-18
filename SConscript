import sys

import fsops
import osops
from sources import Sources

"""
CLI
"""
verbose = GetOption("verbose")
no_format = GetOption("no_format")


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

""" Add/modify additional parameters """
env_arm = env_arm.Clone(
    tools=["clangformat"]
)

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
all_sources = Sources()

""" Search and group source files and source directories """
for dir in SRC_DIRS_ROOT:
    sources = fsops.scan_tree(dir)
    all_sources += sources

""" Group linker scripts """
for linker_file in LINKER_FILES:
    env_arm["LINKFLAGS"].append("-T{}".format(File(linker_file).abspath))

""" Search and group include paths """
for dir in INCLUDE_DIRS_ROOT:
    sources = fsops.scan_tree(dir)
    all_sources += sources
env_arm["CPPPATH"].extend(INCLUDE_DIRS)
env_arm["CPPPATH"].extend(all_sources.include_dirnodes)

""" Filter build files """
all_sources.source_filenodes = fsops.filter_files(all_sources.source_filenodes, EXCLUDED_SRC_FILES)
all_sources.source_filenodes = fsops.remove_duplicate_filenodes(all_sources.source_filenodes)


"""
Perform builds
"""

""" Compile all sources -> objects """
obj_filenodes = []
for source_filenode in all_sources.source_filenodes:
    dest_filepath = fsops.ch_target_filenode(source_filenode, OBJ_DIR, "o")
    new_obj_filenodes = env_arm.Object(target=dest_filepath, source=source_filenode)
    obj_filenodes.extend(new_obj_filenodes)

elf_filenodes = env_arm.Program(target=VARIANT_DIR.File("{}.elf".format(PROJECT_DIR.name)), source=obj_filenodes)
hex_filenodes = env_arm.Objcopy(target=VARIANT_DIR.File("{}.hex".format(PROJECT_DIR.name)), source=elf_filenodes)
bin_filenodes = env_arm.Objcopy(target=VARIANT_DIR.File("{}.bin".format(PROJECT_DIR.name)), source=elf_filenodes)
lst_filenodes = env_arm.Objdump(target=VARIANT_DIR.File("{}.lst".format(PROJECT_DIR.name)), source=elf_filenodes)
size_filenodes = env_arm.Size(target=VARIANT_DIR.File("{}.size".format(PROJECT_DIR.name)), source=elf_filenodes)

Depends(elf_filenodes, LINKER_FILES)


"""
Automatically format all files
"""
FORMAT_EXCLUDED_FILES = [
    PROJECT_DIR.File("lpc40xx.h"),
]

FORMAT_EXCLUDED_DIRS = [
    PROJECT_DIR.Dir("l0_lowlevel/arm-software"),
    PROJECT_DIR.Dir("l1_freertos"),
    PROJECT_DIR.Dir("l4_io/fatfs"),
]

format_filenodes = fsops.filter_files(
    filenodes=(all_sources.source_filenodes + all_sources.include_filenodes),
    exclude_filenodes=FORMAT_EXCLUDED_FILES,
    exclude_dirnodes=FORMAT_EXCLUDED_DIRS,
)

# If "--no-format" provided as command line argument, then do not run Clang Format builders
if not no_format:
    for filenode in format_filenodes:
        env_arm.ClangFormat(filenode=filenode, verbose=verbose)
