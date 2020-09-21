import fsops

Import("project_dirnode")

sources = SConscript("subsidiary_scons")
sources.source_filenodes.append(File("l5_application/main.c"))

Export("sources")

SConscript("#/projects/lpc40xx_shared_sconscript")
