import inspect

from SCons.Script import *


class Sources(object):
    def __init__(self,
        source_filenodes=None,
        source_dirnodes=None,
        include_filenodes=None,
        include_dirnodes=None,
        assembly_filenodes=None,
        linker_filenodes=None,
        unit_test_filenodes=None,
        unit_test_header_filenodes=None):

        self.source_filenodes = list(map(File, source_filenodes or []))
        self.source_dirnodes = list(map(Dir, source_dirnodes or []))
        self.include_filenodes = list(map(File, include_filenodes or []))
        self.include_dirnodes = list(map(Dir, include_dirnodes or []))
        self.assembly_filenodes = list(map(File, assembly_filenodes or []))
        self.linker_filenodes = list(map(File, linker_filenodes or []))
        self.unit_test_filenodes = list(map(File, unit_test_filenodes or []))
        self.unit_test_header_filenodes = list(map(File, unit_test_filenodes or []))

    def __str__(self):
        lines = []
        for attr_name, attr_value in vars(self).items():
            lines.append(attr_name)
            if isinstance(attr_value, list):
                for node in attr_value:
                    lines.append(node.abspath)
                lines.append("")
            else:
                pass  # Do nothing
        return "\n".join(lines)

    def __add__(self, obj):
        for attr_name, attr_value in vars(obj).items():
            if isinstance(attr_value, list):
                for item in attr_value:
                    if item not in getattr(self, attr_name):
                        getattr(self, attr_name).append(item)
                    else:
                        print("repeated: {}".format(item.name))
            else:
                pass  # Do nothing
        return self

    #
    # Accessors
    #

    @property
    def compileable_filenodes(self):
        return self.source_filenodes + self.assembly_filenodes

    @property
    def formattable_filenodes(self):
        return self.source_filenodes + self.include_filenodes + self.unit_test_filenodes + self.unit_test_header_filenodes
