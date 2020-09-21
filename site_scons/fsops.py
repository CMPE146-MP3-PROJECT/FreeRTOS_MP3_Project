"""
fsops - file system operations - SCons file nodes and directory nodes related operations
"""

import fnmatch
import glob
import os

from SCons.Script import *

from sources import Sources

DEFAULT_SOURCE_PATTERNS = ["*.c", "*.cpp"]
DEFAULT_INCLUDE_PATTERNS = ["*.h", "*.hpp"]
DEFAULT_ASSEMBLY_PATTERNS = ["*.s", "*.S"]
DEFAULT_LINKER_PATTERNS = ["*.ld"]
DEFAULT_UNIT_TEST_PATTERNS = ["test_*"]
DEFAULT_UNIT_TEST_HEADER_OVERRIDE = "unittest_header_overrides"
DEFAULT_IGNORE_DIRNAMES = []


def scan_tree(
    dirnode,
    source_patterns=DEFAULT_SOURCE_PATTERNS,
    include_patterns=DEFAULT_INCLUDE_PATTERNS,
    assembly_patterns=DEFAULT_ASSEMBLY_PATTERNS,
    linker_patterns=DEFAULT_LINKER_PATTERNS,
    unit_test_patterns=DEFAULT_UNIT_TEST_PATTERNS,
    subsidiary_scons_filename="subsidiary_scons",
    ignore_dirnames=DEFAULT_IGNORE_DIRNAMES,
    recursive=True):
    """
    Recursively search/glob source files, include files, etc.
    :param dirnode: A root directory node - root of search tree (Dir)
    :param source_patterns: A list of source file name patterns to search (list of str)
    :param include_patterns: A list of include file name patterns to search (list of str)
    :param assembly_patterns: A list of assembly file name patterns to search (list of str)
    :param linker_patterns: A list of linker script file name patterns to search (list of str)
    :param unit_test_patterns: A list of unit test file name patterns to search (list of str)
    :param subsidiary_scons_filename: Subsidiary SCons file name (str)
    :param ignore_dirnames: List of directory names to ignore (list of str)
    :param recursive: Flag to determine if file/directory search operation should be recursive (bool)
    :return: A sources object (Sources)

    Example usage:
        sources = scan_tree(Dir("RTOS"))
        sources.source_filenodes  # A list of all source file nodes
        sources.include_dirnodes  # A list of all include directory nodes
    """
    dirnode = Dir(dirnode)
    ignore_dirnames = ignore_dirnames or []
    sources = Sources()

    unit_test_source_patterns = []
    for unit_test_pattern in unit_test_patterns:
        for source_pattern in source_patterns:
            unit_test_source_patterns.append("{}{}".format(unit_test_pattern, source_pattern))

    root_dirpath = os.path.relpath(dirnode.abspath)
    for dirpath, dirnames, filenames in os.walk(root_dirpath):
        if os.path.basename(dirpath) in ignore_dirnames:
            continue

        if os.path.basename(dirpath) == DEFAULT_UNIT_TEST_HEADER_OVERRIDE:
            for include_pattern in include_patterns:
                matching_include_filenodes = Glob(os.path.join(dirpath, include_pattern))
                sources.unit_test_header_filenodes.extend(matching_include_filenodes)
            continue

        # Do not invoke subsidiary scons in root directory to avoid infinite recursion
        if (dirpath != root_dirpath) and (subsidiary_scons_filename in filenames):
                subsidary_sources = SConscript(Dir(dirpath).File(subsidiary_scons_filename))
                if isinstance(subsidary_sources, Sources):
                    sources += subsidary_sources
                    dirnames[:] = []  # End recursion for this directory tree
                    continue

        for source_pattern in source_patterns:
            matching_source_filenodes = Glob(os.path.join(dirpath, source_pattern))
            for filenode in matching_source_filenodes:
                for unit_test_pattern in unit_test_patterns:
                    if not fnmatch.fnmatch(filenode.name, unit_test_pattern):
                        sources.source_filenodes.append(filenode)
                        if Dir(dirpath) not in sources.source_dirnodes:
                            sources.source_dirnodes.append(Dir(dirpath))

        for include_pattern in include_patterns:
            matching_include_filenodes = Glob(os.path.join(dirpath, include_pattern))
            sources.include_filenodes.extend(matching_include_filenodes)
            if (len(fnmatch.filter(filenames, include_pattern)) > 0) and (Dir(dirpath) not in sources.include_dirnodes):
                sources.include_dirnodes.append(Dir(dirpath))

        for assembly_pattern in assembly_patterns:
            matching_assembly_filenodes = Glob(os.path.join(dirpath, assembly_pattern))
            sources.assembly_filenodes.extend(matching_assembly_filenodes)

        for linker_pattern in linker_patterns:
            matching_linker_filenodes = Glob(os.path.join(dirpath, linker_pattern))
            sources.linker_filenodes.extend(matching_linker_filenodes)

        for unit_test_source_pattern in unit_test_source_patterns:
            matching_unit_test_source_filenodes = Glob(os.path.join(dirpath, unit_test_source_pattern))
            sources.unit_test_filenodes.extend(matching_unit_test_source_filenodes)

        if not recursive:
            break

    return sources


def filter_files(filenodes, exclude_filenodes=None, exclude_dirnodes=None, exclude_filename_patterns=None, exclude_dirname_patterns=None):
    """
    Filter file nodes
    :param filenodes: A list of file nodes (list of File)
    :param exclude_filenodes: A list of file nodes to filter out (list of File)
    :param exclude_dirnodes: A list of directory nodes to filter out (list of Dir)
    :param exclude_filename_pattern: A file name pattern to filter out files with a matching file name (str)
    :param exclude_dirname_patterns: A directory name pattern to filter out files with a matching parent(s) directory name (str)
    :return: A list of filtered file nodes (list of File)
    """
    exclude_filenodes = exclude_filenodes or []
    exclude_dirnodes = exclude_dirnodes or []
    exclude_filename_patterns = exclude_filename_patterns or []
    exclude_dirname_patterns = exclude_dirname_patterns or []

    filenodes = list(map(File, filenodes))
    exclude_filenodes = list(map(File, exclude_filenodes))
    exclude_dirnodes = list(map(Dir, exclude_dirnodes))

    filtered_filenodes = filenodes
    excluded_filenodes = []

    if exclude_filenodes:
        new_filtered_filenodes = []
        for filenode in filenodes:
            if filenode not in exclude_filenodes:
                new_filtered_filenodes.append(filenode)
            else:
                excluded_filenodes.append(filenode)
        filtered_filenodes = new_filtered_filenodes or filtered_filenodes

    if exclude_filename_patterns:
        new_filtered_filenodes = []
        for filenode in filtered_filenodes:
            for exclude_filename_pattern in exclude_filename_patterns:
                if not fnmatch.fnmatch(filenode.name, exclude_filename_pattern):
                    new_filtered_filenodes.append(filenode)
                    break
                else:
                    excluded_filenodes.append(filenode)
        filtered_filenodes = new_filtered_filenodes or filtered_filenodes

    if exclude_dirname_patterns:
        new_filtered_filenodes = []
        for filenode in filtered_filenodes:
            parent_dirnames = filenode.abspath.split(os.path.sep)
            for parent_dirname in parent_dirnames:
                matched_parent_directory_name = False
                for exclude_dirname_pattern in exclude_dirname_patterns:
                    if fnmatch.fnmatch(parent_dirname, exclude_dirname_pattern):
                        excluded_filenodes.append(filenode)
                        matched_parent_directory_name = True
                        break
                if matched_parent_directory_name:
                    break
            else:
                new_filtered_filenodes.append(filenode)
        filtered_filenodes = new_filtered_filenodes or filtered_filenodes

    if exclude_dirnodes:
        new_filtered_filenodes = []
        for filenode in filtered_filenodes:
            for dirnode in exclude_dirnodes:
                if dirnode.abspath in filenode.abspath:
                    excluded_filenodes.append(filenode)
            else:
                new_filtered_filenodes.append(filenode)
        filtered_filenodes = new_filtered_filenodes or filtered_filenodes

    return filtered_filenodes, excluded_filenodes


def ch_filename_ext(filenode, ext=None):
    """
    Create a new File object with a modified file name extension
    :param filenode: A file node (File)
    :param ext: A new extension - i.e. "cpp" (str) or None (None)
    :return: A file node with a modified file name extension (File)
    """
    filenode = File(filenode)
    basename, curr_ext = os.path.splitext(filenode.name)
    new_filename = basename if (ext is None) else "{}.{}".format(basename, ext)
    return File(os.path.join(os.path.dirname(filenode.abspath), new_filename))


def ch_dirpath(filenode, target_dirnode):
    """
    Create a new File object with a modified directory path
    :param filenode: A file node (File)
    :param target_dirnode: A target or destination directory node (Dir)
    :return: A file node with a modified directory path (File)
    """
    filenode = File(filenode)
    target_dirnode = Dir(target_dirnode)
    return File(os.path.join(target_dirnode.abspath, filenode.name))


def prefix_filenode_name(filenode, prefix):
    """
    Add a prefix to a file node's name
    :param filenode: A file node (File)
    :param prefix: Prefix to add (str)
    :return: A file node with a modified file name (File)
    """
    filenode = File(filenode)
    new_filename = "{}{}".format(prefix, filenode.name)
    new_filenode = File(os.path.join(os.path.dirname(filenode.abspath), new_filename))
    return new_filenode


def suffix_filenode_name(filenode, suffix):
    """
    Add a suffix to a file node's name
    :param filenode: A file node (File)
    :param suffix: Suffix to add (str)
    :return: A file node with a modified file name (File)
    """
    filenode = File(filenode)
    basename, ext = os.path.splitext(filenode.name)
    new_filename = "{}{}{}".format(basename, suffix, ext)
    new_filenode = File(os.path.join(os.path.dirname(filenode.abspath), new_filename))
    return new_filenode


def ch_target_filenode(filenode, target_dirnode, ext=None):
    """
    Change the file node destination which includes the destination directory node and output file extension
    :param filenode: A file node (File)
    :param target_dirnode: A target or destination directory node (Dir)
    :param ext: A new extension - i.e. "cpp" (str) or None (None)
    :return: A file node with a modified destination directory path and a modified file name extension (File)
    """
    filenode = File(filenode)
    target_dirnode = Dir(target_dirnode)
    new_filenode = ch_filename_ext(filenode=filenode, ext=ext)
    new_filenode = ch_dirpath(filenode=new_filenode, target_dirnode=target_dirnode)
    return new_filenode


def remove_duplicate_filenodes(filenodes):
    """
    Remove duplicate file nodes from a given list of file nodes
    :param filenodes: A list of file nodes (list of File)
    :return: A list of filtered file nodes (list of File)
    """
    filtered_filenodes = []
    for filenode in filenodes:
        if filenode not in filtered_filenodes:
            filtered_filenodes.append(filenode)
    return filtered_filenodes


def basename(filenode):
    """
    Get a file node's basename (no file extension)
    :param filenode: A file node (File)
    :return: The base name of the file node (file name with no file extension) (str)
    """
    return os.path.splitext(File(filenode).name)[0]


def root_dirpath():
    """
    :return: Root directory path (str)
    """
    return os.path.abspath(os.sep)
