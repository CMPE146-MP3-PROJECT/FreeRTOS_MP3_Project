import inspect

from SCons.Script import *


class Sources(object):
    def __init__(self,
        source_filenodes=None,
        source_dirnodes=None,
        include_filenodes=None,
        include_dirnodes=None,
        assembly_filenodes=None):

        self.source_filenodes = [] if (source_filenodes is None) else list(map(File, source_filenodes))
        self.source_dirnodes = [] if (source_dirnodes is None) else list(map(Dir, source_dirnodes))
        self.include_filenodes = [] if (include_filenodes is None) else list(map(File, include_filenodes))
        self.include_dirnodes = [] if (include_dirnodes is None) else list(map(Dir, include_dirnodes))
        self.assembly_filenodes = [] if (assembly_filenodes is None) else list(map(File, assembly_filenodes))

    def __iter__(self):
        for attr_name, attr_value in inspect.getmembers(self, lambda member: not inspect.isroutine(member)):
            if not attr_name.startswith("_"):
                yield attr_name, attr_value

    def __str__(self):
        lines = []
        for attr_name, attr_value in self:
            lines.append(attr_name)
            for node in attr_value:
                lines.append(node.abspath)
            lines.append("")
        return "\n".join(lines)

    def __add__(self, obj):
        assert isinstance(obj, Sources)
        for attr_name, attr_value in obj:
            getattr(self, attr_name).extend(attr_value)
        return self
