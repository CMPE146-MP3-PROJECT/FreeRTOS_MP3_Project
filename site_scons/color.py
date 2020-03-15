import sys

import colorama


def _is_command_prompt():
    return sys.stdin.isatty() is True


if _is_command_prompt():
    colorama.init()


class ColorString(object):
    COLORS = {
        "red": colorama.Fore.RED,
        "green": colorama.Fore.GREEN,
        "yellow": colorama.Fore.YELLOW,
        "blue": colorama.Fore.BLUE,
        "neutral": colorama.Style.RESET_ALL,
    }

    def __init__(self, string):
        self._string = string

    def __getattr__(self, name):
        if name in self.COLORS:
            ret =  "{}{}{}".format(self.COLORS[name], self._string, self.COLORS["neutral"])
        else:
            raise AttributeError(name)
        return ret
