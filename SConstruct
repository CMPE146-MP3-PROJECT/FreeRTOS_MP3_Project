"""
SCons entry point

Invoke the command "scons" with a shell while the current directory is this file's directory

References:
- https://scons.org/
- https://github.com/SCons/scons/wiki
"""

import os
import sys


"""
CLI arguments
"""
AddOption(
    "--project",
    metavar="<directory name>",
    default="lpc40xx_freertos",
    help="Specify a target project directory to build."
)


def main():
    project_dirpath = GetOption("project")

    """
    Jump to the target project subsidary SConscript
    """
    if not os.path.isdir(project_dirpath):
        print("Target project directory [{}] does not exist or is not a directory!".format(project_dirpath))
        sys.exit(-1)
    elif not has_subsidary_scons(Dir(project_dirpath)):
        print("Target project directory [{}] is invalid! Expecting SConscript inside!".format(project_dirpath))
        sys.exit(-1)
    else:
        project_dirnode = Dir(project_dirpath)

    Export("project_dirnode")

    SConscript("SConscript")


def has_subsidary_scons(dirnode):
    """
    Check if a directory node has a subsidiary SConscript file exactly named "SConscript"
    :param dirnode: A directory node (Dir)
    :return: Boolean indicating that the provided directory node contains an SConscript file (bool)
    """
    dirnode = Dir(dirnode)
    filenames = os.listdir(dirnode.abspath)
    return len(list(filter(lambda filename: filename == "SConscript", filenames))) > 0


main()
