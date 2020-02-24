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
CMOCK_DIR = SELF_DIR.Dir("throw_the_switch/cmock/v2.5.0")
CEXCEPTION_DIR = SELF_DIR.Dir("throw_the_switch/cexception/v1.3.1")

SOURCE_FILES = [
    UNITY_DIR.File("src/unity.c"),
    CMOCK_DIR.File("src/cmock.c"),
    CEXCEPTION_DIR.File("lib/cexception.c")
]

INCLUDE_DIRS = [
    UNITY_DIR.Dir("src"),
    CMOCK_DIR.Dir("src"),
    CEXCEPTION_DIR.Dir("lib"),
]

""" Unity variables """
GENERATE_TEST_RUNNER_RB = UNITY_DIR.File("auto/generate_test_runner.rb")

""" CMock variables """
MOCK_GENERATOR_RB = CMOCK_DIR.File("scripts/create_mock.rb")
MOCK_HEADER_PREFIX = "Mock"
MOCK_DIRNAME = "mock"
IGNORE_HEADER_FILENAME = [
    "unity.h"
]

""" Unit test runner variables """
UNIT_TEST_RUNNER_PY = SELF_DIR.File("unit_test_runner.py")

""" Common variables """
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

def unittest_method(env, source, target, sources=None, prepend_include_dirnodes=None, summary_only=False, timeout=None, verbose=False):
    if verbose:
        summary_only = False

    all_exe_filenodes = []
    dependent_srcpath_objpath_map = {}
    env_ut = get_unittest_env(env)

    prepend_include_dirnodes = [prepend_include_dirnodes] if (prepend_include_dirnodes is not None) else []
    env_ut.Prepend(CPPPATH=prepend_include_dirnodes)

    unittest_obj_filenodes = []
    for source_filenode in SOURCE_FILES:
        unittest_obj_filenodes += env_ut.Object(target=fsops.ch_target_filenode(source_filenode, target.Dir(OBJ_DIRNAME), "o"), source=source_filenode)

    for filenode_ut in search_for_tests(source):
        output_dirnode = target.Dir(fsops.basename(filenode_ut))
        mock_output_dirnode = output_dirnode.Dir(MOCK_DIRNAME)

        env_ut.Append(CPPPATH=Dir("{}/..".format(filenode_ut.dir.abspath)))
        env_ut.Append(CPPPATH=mock_output_dirnode)

        filenode_ut_main = generate_test_main(env, filenode_ut, target_dirnode=output_dirnode)

        obj_filenodes = []

        if sources is not None:
            header_filenodes_override = []
            if prepend_include_dirnodes is not None:
                for dirnode in prepend_include_dirnodes:
                    header_filenodes_override.extend(Glob(os.path.join(dirnode.abspath, "*.h*")))

            dependent_source_filenodes, mock_header_filenodes = find_dependencies_from_sources(filenode_ut, sources, header_filenodes_override, verbose)
            for filenode in dependent_source_filenodes:
                if filenode not in dependent_srcpath_objpath_map:
                    objs = env_ut.Object(target=fsops.ch_target_filenode(filenode, target.Dir(OBJ_DIRNAME), "o"), source=filenode)
                    dependent_srcpath_objpath_map[filenode] = objs[0]
                    obj_filenodes += objs
                else:
                    obj_filenodes.append(dependent_srcpath_objpath_map[filenode])

            _, mock_source_filenodes = generate_mocks(env_ut, mock_header_filenodes, mock_output_dirnode)
            for mock_source_filenode in mock_source_filenodes:
                obj_filenodes += env_ut.Object(target=fsops.ch_target_filenode(mock_source_filenode, output_dirnode.Dir(OBJ_DIRNAME), "o"), source=mock_source_filenode)

        obj_filenodes += unittest_obj_filenodes
        obj_filenodes += env_ut.Object(target=fsops.ch_target_filenode(filenode_ut_main, output_dirnode.Dir(OBJ_DIRNAME), "o"), source=filenode_ut_main)
        obj_filenodes += env_ut.Object(target=fsops.ch_target_filenode(filenode_ut, output_dirnode.Dir(OBJ_DIRNAME), "o"), source=filenode_ut)

        exe_filenodes = env_ut.Program(target=fsops.ch_target_filenode(filenode_ut, output_dirnode, "exe"), source=obj_filenodes)
        all_exe_filenodes += exe_filenodes

    result = execute_unit_tests(env_ut, all_exe_filenodes, summary_only=summary_only, timeout=timeout)

    return result


"""
Helper methods
"""


def get_unittest_env(source_env):
    env_ut = source_env.Clone()
    env_ut.Append(
        CPPDEFINES=[
            "UNIT_TEST=1",
            "UNITY_OUTPUT_COLOR=1",
        ]
    )
    env_ut.Append(CPPPATH=INCLUDE_DIRS)
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
    return env.Command(action="ruby {} $SOURCE $TARGET".format(GENERATE_TEST_RUNNER_RB.abspath), source=filenode, target=output_filenode)[0]


def find_dependencies_from_sources(filenode, sources, header_filenodes_override=None, verbose=False):
    include_parser = IncludeParser(filenode.abspath)
    dependent_source_filenodes = []
    missing_dependency_filenames = []

    mock_header_filenodes = []
    missing_mock_header_filenames = []

    for filename in include_parser.filenames:
        basename, _ = os.path.splitext(filename)
        if filename in IGNORE_HEADER_FILENAME:
            continue

        if not filename.startswith(MOCK_HEADER_PREFIX):
            for source_filenode in sources.source_filenodes:
                source_basename, _ = os.path.splitext(source_filenode.name)
                if basename == source_basename:
                    dependent_source_filenodes.append(source_filenode)
                    break
            else:
                missing_dependency_filenames.append(filename)
        else:  # filename.startswith(MOCK_HEADER_PREFIX)
            mock_header_filename = filename.replace(MOCK_HEADER_PREFIX, "", 1)
            if header_filenodes_override is None:
                header_filenodes_override = []
            for header_filenode in header_filenodes_override + sources.include_filenodes:
                if mock_header_filename == header_filenode.name:
                    mock_header_filenodes.append(header_filenode)
                    break
            else:
                missing_mock_header_filenames.append(mock_header_filename)

    if (len(missing_dependency_filenames) > 0 or len(missing_mock_header_filenames) > 0) and verbose:
        print("WARNING: Missing dependencies for [{}]".format(filenode.name))
        for filename in missing_dependency_filenames:
            print("No matching source file for [{}]".format(filename))
        for filename in missing_mock_header_filenames:
            print("No header file [{}]".format(filename))
        print("")

    return dependent_source_filenodes, mock_header_filenodes


def generate_mocks(env, header_filenodes, target_dirnode):
    mock_header_filenodes = []
    mock_source_filenodes = []
    for header_filenode in header_filenodes:
        basename, ext = os.path.splitext(header_filenode.name)
        mock_header_filenode = target_dirnode.File("{}{}".format(MOCK_HEADER_PREFIX, header_filenode.name))
        mock_source_filenode = target_dirnode.File("{}{}{}".format(MOCK_HEADER_PREFIX, basename, ext.replace("h", "c")))
        results = env.Command(action="ruby {} $SOURCE {}".format(MOCK_GENERATOR_RB.abspath, target_dirnode.abspath), source=header_filenode, target=[mock_header_filenode, mock_source_filenode])
        mock_header_filenodes.append(mock_header_filenode)
        mock_source_filenodes.append(mock_source_filenode)
    return mock_header_filenodes, mock_source_filenodes


def execute_unit_tests(env, exe_filenodes, summary_only=False, timeout=None):
    # Example:
    # python <UNIT_TEST_RUNNER_PY> -i <exe> -i <exe> -i <exe>
    command = [
        "python",
        UNIT_TEST_RUNNER_PY.abspath,
    ]
    if summary_only:
        command.append("--summary-only")
    if timeout is not None:
        command.append("--timeout={}".format(str(timeout)))

    command.extend(map(lambda filenode: "-i {}".format(filenode.abspath), exe_filenodes))

    result = env.Command(target=None, source=exe_filenodes, action=" ".join(command))

    return result
