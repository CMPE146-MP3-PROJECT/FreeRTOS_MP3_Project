"""
fsops - file system operations - SCons file nodes and directory nodes related operations
"""

import fnmatch
import glob
import os

from SCons.Script import *


DEFAULT_SRC_PATTERNS = ["*.c", "*.cpp", "*.s", "*.S"]
DEFAULT_INCLUDE_PATTERNS = ["*.h", "*.hpp"]


def scan_tree(dirnode, src_patterns=DEFAULT_SRC_PATTERNS, header_patterns=DEFAULT_INCLUDE_PATTERNS, recursive=True):
    """
    Recursively search/glob source files, header files, etc.
    :param dirnode: A root directory node - root of search tree (Dir)
    :param scr_patterns: A list of source file name patterns to search (list of str)
    :param header_patterns: A list of header file name patterns to search (list of str)
    :return: Tuple(
        A list of source file nodes (list of File),
        A list of header file nodes (list of File),
        A list of directory nodes that contain a source file (list of Dir),
        A list of directory nodes that contain a header file (list of Dir),
    )

    Example usage:
        src_filenodes, header_filenodes, src_dirnodes, include_dirnodes = scan_tree(Dir("RTOS"))
    """
    dirnode = Dir(dirnode)

    src_filenodes = []
    header_filenodes = []
    src_dirnodes = []
    include_dirnodes = []

    for dirpath, dirnames, filenames in os.walk(os.path.relpath(dirnode.abspath)):
        for src_pattern in src_patterns:
            matching_src_filenodes = Glob(os.path.join(dirpath, src_pattern))
            src_filenodes.extend(matching_src_filenodes)
            if Dir(dirpath) not in src_dirnodes:
                src_dirnodes.append(Dir(dirpath))

        for header_pattern in header_patterns:
            matching_header_filenodes = Glob(os.path.join(dirpath, header_pattern))
            header_filenodes.extend(matching_header_filenodes)
            if (len(fnmatch.filter(filenames, header_pattern)) > 0) and (
                Dir(dirpath) not in include_dirnodes
            ):
                include_dirnodes.append(Dir(dirpath))

        if not recursive:
            break

    return (src_filenodes, header_filenodes, src_dirnodes, include_dirnodes)


def filter_files(filenodes, exclude_filenodes=None, exclude_filename_pattern=None):
    """
    Filter file nodes
    :param filenodes: A list of file nodes (list of File)
    :param exclude_filenodes: A list of file nodes to filter out (list of File)
    :param exclude_filename_pattern: A file name pattern to filter out files with a matching file name pattern (str)
    :return: A list of filtered file nodes (list of File)
    """
    if exclude_filenodes is None:
        exclude_filenodes = []

    filenodes = list(map(File, filenodes))
    exclude_filenodes = list(map(File, exclude_filenodes))

    filtered_filenodes = []
    filtered_filenodes.extend(filter(lambda filenode: filenode not in exclude_filenodes, filenodes))

    if exclude_filename_pattern is not None:
        filtered_filenodes.extend(
            filter(
                lambda filenode: not fnmatch(filenode.name, exclude_filename_pattern),
                filtered_filenodes,
            )
        )

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
