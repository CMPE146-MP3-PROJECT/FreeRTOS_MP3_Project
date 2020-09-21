from SCons.Script import *

import fsops
from sources import Sources


class Artifacts(object):
    def __init__(self, object_filenodes, elf_filenode, hex_filenode, bin_filenode, lst_filenode, map_filenode):
        self.object_filenodes = object_filenodes
        self.elf_filenode = elf_filenode
        self.hex_filenode = hex_filenode
        self.bin_filenode = bin_filenode
        self.lst_filenode = lst_filenode
        self.map_filenode = map_filenode


class EnvironmentBuilder(object):
    def __init__(self, environment):
        self._environment = environment

    #
    # Public methods
    #

    def build(self, base_filename, output_dirnode, sources):
        self._environment.VariantDir(variant_dir=output_dirnode, src_dir=Dir("."), duplicate=0)

        target_elf_filenode = output_dirnode.File("{}.elf".format(base_filename))
        target_hex_filenode = output_dirnode.File("{}.hex".format(base_filename))
        target_bin_filenode = output_dirnode.File("{}.bin".format(base_filename))
        target_lst_filenode = output_dirnode.File("{}.lst".format(base_filename))
        target_map_filenode = output_dirnode.File("{}.map".format(base_filename))
        target_size_filenode = output_dirnode.File("{}.size".format(base_filename))

        self._configure_include_paths(sources.include_dirnodes)
        self._configure_map(target_map_filenode)
        self._configure_linker_script(sources.linker_filenodes)

        object_filenodes = self._build_objects(filenodes=sources.compileable_filenodes, output_dirnode=output_dirnode.Dir("objects"))
        elf_filenode = self._environment.Program(target=target_elf_filenode, source=object_filenodes)
        hex_filenode = self._environment.Objcopy(target=target_hex_filenode, source=elf_filenode)
        bin_filenode = self._environment.Objcopy(target=target_bin_filenode, source=elf_filenode)
        lst_filenode = self._environment.Objdump(target=target_lst_filenode, source=elf_filenode)
        size = self._environment.Size(target=target_size_filenode, source=elf_filenode)

        Depends(elf_filenode, sources.linker_filenodes)

        artifacts = Artifacts(
            object_filenodes=object_filenodes,
            elf_filenode=elf_filenode,
            hex_filenode=hex_filenode,
            bin_filenode=bin_filenode,
            lst_filenode=lst_filenode,
            map_filenode=target_map_filenode,
        )

        return artifacts

    #
    # Private methods
    #

    def _build_objects(self, filenodes, output_dirnode):
        object_filenodes = []
        for filenode in filenodes:
            dest_filenode = fsops.ch_target_filenode(filenode, output_dirnode, ext="o")
            new_object_filenodes = self._environment.Object(target=dest_filenode, source=filenode)
            object_filenodes.extend(new_object_filenodes)
        return object_filenodes

    def _configure_include_paths(self, dirnodes):
        for dirnode in dirnodes:
            if dirnode not in self._environment["CPPPATH"]:
                self._environment["CPPPATH"].append(dirnode)

    def _configure_linker_script(self, linker_filenodes):
        for linker_filenode in linker_filenodes:
            option = "-T{}".format(File(linker_filenode).abspath)
            if option not in self._environment["LINKFLAGS"]:
                self._environment["LINKFLAGS"].append(option)

    def _configure_map(self, map_filenode):
        option = "-Wl,-Map,{}".format(map_filenode.abspath)
        if option not in self._environment["LINKFLAGS"]:
            self._environment["LINKFLAGS"].append(option)
