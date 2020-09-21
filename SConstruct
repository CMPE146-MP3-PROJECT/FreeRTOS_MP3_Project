"""
SCons entry point

Invoke the command "scons" with a shell while the current directory is this file's directory

References:
- https://scons.org/
- https://github.com/SCons/scons/wiki
"""

import os
import sys


def main():
    project_dirname = GetOption("project")
    project_dirnode = Dir("#/projects").Dir(project_dirname)

    if not os.path.isdir(project_dirnode.abspath):
        print("Unable to find project [{}]".format(project_dirnode.name))
        sys.exit(-1)

    if not has_subsidary_scons(project_dirnode):
        print("Target project directory [{}] is invalid! Expecting SConscript inside!".format(project_dirpath))
        sys.exit(-1)

    Export("project_dirnode")

    SConscript(project_dirnode.File("SConscript"))


def has_subsidary_scons(dirnode):
    """
    Check if a directory node has a subsidiary SConscript file exactly named "SConscript"
    :param dirnode: A directory node (Dir)
    :return: Boolean indicating that the provided directory node contains an SConscript file (bool)
    """
    filenames = os.listdir(dirnode.abspath)
    return len(list(filter(lambda filename: filename == "SConscript", filenames))) > 0


main()
