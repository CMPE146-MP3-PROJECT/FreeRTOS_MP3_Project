"""
SCons tools - extendedcmd

References:
    - https://bitbucket.org/scons/scons/wiki/LongCmdLinesOnWin32
"""

from SCons.Script import *


"""
SCons tools functions
"""


def generate(env):
    env.AddMethod(enable_extended_cmd_method, "EnableExtendedCmd")


def exists():
    return True


"""
Environment functions
"""


def enable_extended_cmd_method(self):
    if self["PLATFORM"] == "win32":
        import win32file
        import win32event
        import win32process
        import win32security

        def my_spawn(sh, escape, cmd, args, spawnenv):
            for var in spawnenv:
                spawnenv[var] = spawnenv[var].encode("ascii", "replace")

            sAttrs = win32security.SECURITY_ATTRIBUTES()
            StartupInfo = win32process.STARTUPINFO()
            newargs = " ".join(map(escape, args[1:]))
            cmdline = cmd + " " + newargs

            # check for any special operating system commands
            if cmd == "del":
                for arg in args[1:]:
                    win32file.DeleteFile(arg)
                exit_code = 0
            else:
                # otherwise execute the command.
                hProcess, hThread, dwPid, dwTid = win32process.CreateProcess(
                    None, cmdline, None, None, 1, 0, spawnenv, None, StartupInfo
                )
                win32event.WaitForSingleObject(hProcess, win32event.INFINITE)
                exit_code = win32process.GetExitCodeProcess(hProcess)
                win32file.CloseHandle(hProcess)
                win32file.CloseHandle(hThread)
            return exit_code

        self["SPAWN"] = my_spawn
