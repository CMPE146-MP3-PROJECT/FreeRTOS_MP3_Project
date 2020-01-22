"""
Unit test using Unity + CMock
"""

import logging
import os
import subprocess

from SCons.Script import *

import fsops
from .include_parser import IncludeParser
import osops
from sources import Sources

SELF_DIR = Dir(os.path.dirname(__file__))
UNITY_DIR = SELF_DIR.Dir("throw_the_switch/unity/v2.5.0")

UNITTEST_UNITY_FILE = UNITY_DIR.File("src/unity.c")
UNITTEST_GENERATE_TEST_RUNNER_RB = UNITY_DIR.File("auto/generate_test_runner.rb")

UNITTEST_INCLUDE_DIRS = [
    UNITY_DIR.Dir("src"),
]

OBJ_DIRNAME = "obj"
EXE_DIRNAME = "exe"


"""
SCons tools functions
"""


def generate(env):
    env.AddMethod(unittest_method, "Test")


def exists():
    return True


"""
Environment functions
"""

def unittest_method(env, source, target, sources=None, verbose=False):
    all_exe_filenodes = []
    results = []
    dependent_srcpath_objpath_map = {}
    env_ut = get_unittest_env(env)
    unity_obj_filenodes = env_ut.Object(target=fsops.ch_target_filenode(UNITTEST_UNITY_FILE, target.Dir(OBJ_DIRNAME), "o"), source=UNITTEST_UNITY_FILE)

    for filenode_ut in search_for_tests(source):
        output_dirnode = target.Dir(fsops.basename(filenode_ut))

        env_ut.Append(CPPPATH=Dir("{}/..".format(filenode_ut.dir.abspath)))

        filenode_ut_main = generate_test_main(env, filenode_ut, target_dirnode=output_dirnode)

        obj_filenodes = []

        if sources is not None:
            dependent_source_filenodes = find_dependencies_from_sources(filenode_ut, sources, verbose)
            for filenode in dependent_source_filenodes:
                if filenode not in dependent_srcpath_objpath_map:
                    obj_filenodes = env_ut.Object(target=fsops.ch_target_filenode(filenode, target.Dir(OBJ_DIRNAME), "o"), source=filenode)
                    dependent_srcpath_objpath_map[filenode] = obj_filenodes[0]
                else:
                    obj_filenodes.append(dependent_srcpath_objpath_map[filenode])

        obj_filenodes += unity_obj_filenodes
        obj_filenodes += env_ut.Object(target=fsops.ch_target_filenode(filenode_ut_main, output_dirnode.Dir(OBJ_DIRNAME), "o"), source=filenode_ut_main)
        obj_filenodes += env_ut.Object(target=fsops.ch_target_filenode(filenode_ut, output_dirnode.Dir(OBJ_DIRNAME), "o"), source=filenode_ut)

        exe_filenodes = env_ut.Program(target=fsops.ch_target_filenode(filenode_ut, output_dirnode, "exe"), source=obj_filenodes)
        result = env_ut.Command(target=None, source=exe_filenodes, action=exe_filenodes[0].abspath)
        all_exe_filenodes += exe_filenodes
        results += result
        Depends(result, exe_filenodes)

    return results


"""
Helper methods
"""


def get_unittest_env(source_env):
    env_ut = source_env.Clone()
    env_ut.Append(CPPDEFINES=["UNIT_TEST=1"])
    env_ut.Append(CPPPATH=UNITTEST_INCLUDE_DIRS)
    env_ut.Append(
        CFLAGS=[
            "-g",
            "-O0",
        ],
    )

    return env_ut

def get_unittest_coverage_env(source_env):
    env_ut = get_unittest_env(source_env)
    env_ut.Append(
        CPPFLAGS=[
            "-fprofile-arcs",
            "-ftest-coverage"
        ],
        LIBS=["gcov"],
    )
    return env_ut


def search_for_tests(dirnode):
    sources = fsops.scan_tree(dirnode, ignore_dirnames=None)
    for filenode in sources.source_filenodes:
        if filenode.dir.name == "test":
            yield filenode


def generate_test_main(env, filenode, target_dirnode):
    output_filenode = target_dirnode.File(fsops.suffix_filenode_name(filenode, suffix="_runner").name)
    return env.Command(action="ruby {} $SOURCE $TARGET".format(UNITTEST_GENERATE_TEST_RUNNER_RB.abspath), source=filenode, target=output_filenode)[0]


def find_dependencies_from_sources(filenode, sources, verbose=False):
    include_parser = IncludeParser(filenode.abspath)
    dependent_source_filenodes = []
    missing_dependency_filename = []

    for filename in include_parser.filenames:
        basename = os.path.splitext(filename)[0]
        if basename in UNITTEST_UNITY_FILE.name:
            continue
        for source_filenode in sources.source_filenodes:
            source_basename = os.path.splitext(source_filenode.name)[0]
            if basename == source_basename:
                dependent_source_filenodes.append(source_filenode)
                break
        else:
            missing_dependency_filename.append(filename)

    if len(missing_dependency_filename) > 0 and verbose:
        print("WARNING: Missing dependent source files for [{}]".format(filenode.name))
        for filename in missing_dependency_filename:
            print(filename)
        print("")

    return dependent_source_filenodes
