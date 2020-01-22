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
DEFAULT_IGNORE_DIRNAMES = ["test"]


def scan_tree(
    dirnode,
    source_patterns=DEFAULT_SOURCE_PATTERNS,
    include_patterns=DEFAULT_INCLUDE_PATTERNS,
    assembly_patterns=DEFAULT_ASSEMBLY_PATTERNS,
    subsidiary_scons_filename="subsidiary-scons",
    ignore_dirnames=DEFAULT_IGNORE_DIRNAMES,
    recursive=True):
    """
    Recursively search/glob source files, include files, etc.
    :param dirnode: A root directory node - root of search tree (Dir)
    :param source_patterns: A list of source file name patterns to search (list of str)
    :param include_patterns: A list of include file name patterns to search (list of str)
    :param assembly_patterns: A list of assembly file name patterns to search (list of str)
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
    ignore_dirnames = ignore_dirnames if ignore_dirnames is not None else []
    sources = Sources()

    for dirpath, dirnames, filenames in os.walk(os.path.relpath(dirnode.abspath)):
        if os.path.basename(dirpath) in ignore_dirnames:
            continue

        if subsidiary_scons_filename in filenames:
            subsidary_sources = SConscript(Dir(dirpath).File(subsidiary_scons_filename))
            if isinstance(subsidary_sources, Sources):
                sources += subsidary_sources
                dirnames[:] = []  # End recursion for this directory tree
                continue

        for source_pattern in source_patterns:
            matching_source_filenodes = Glob(os.path.join(dirpath, source_pattern))
            sources.source_filenodes.extend(matching_source_filenodes)
            if Dir(dirpath) not in sources.source_dirnodes:
                sources.source_dirnodes.append(Dir(dirpath))

        for assembly_pattern in assembly_patterns:
            matching_assembly_filenodes = Glob(os.path.join(dirpath, assembly_pattern))
            sources.assembly_filenodes.extend(matching_assembly_filenodes)

        for include_pattern in include_patterns:
            matching_include_filenodes = Glob(os.path.join(dirpath, include_pattern))
            sources.include_filenodes.extend(matching_include_filenodes)
            if (len(fnmatch.filter(filenames, include_pattern)) > 0) and (Dir(dirpath) not in sources.include_dirnodes):
                sources.include_dirnodes.append(Dir(dirpath))

        if not recursive:
            break

    return sources


def filter_files(filenodes, exclude_filenodes=None, exclude_dirnodes=None, exclude_filename_pattern=None):
    """
    Filter file nodes
    :param filenodes: A list of file nodes (list of File)
    :param exclude_filenodes: A list of file nodes to filter out (list of File)
    :param exclude_dirnodes: A list of directory nodes to filter out (list of Dir)
    :param exclude_filename_pattern: A file name pattern to filter out files with a matching file name pattern (str)
    :return: A list of filtered file nodes (list of File)
    """
    exclude_filenodes = exclude_filenodes if (exclude_filenodes is not None) else []
    exclude_dirnodes = exclude_dirnodes if (exclude_dirnodes is not None) else []

    filenodes = list(map(File, filenodes))
    exclude_filenodes = list(map(File, exclude_filenodes))
    exclude_dirnodes = list(map(Dir, exclude_dirnodes))

    filtered_filenodes = []
    filtered_filenodes.extend(list(filter(lambda filenode: filenode not in exclude_filenodes, filenodes)))

    if exclude_filename_pattern is not None:
        filtered_filenodes.extend(list(filter(lambda filenode: not fnmatch(filenode.name, exclude_filename_pattern), filtered_filenodes)))

    new_filtered_filenodes = []
    if exclude_dirnodes is not None:
        for filenode in filtered_filenodes:
            for dirnode in exclude_dirnodes:
                if dirnode.abspath in filenode.abspath:
                    break
            else:
                new_filtered_filenodes.append(filenode)
        filtered_filenodes = new_filtered_filenodes

    return filtered_filenodes


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
