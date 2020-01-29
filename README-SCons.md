# SCons

SCons is a lightweight build system implemented in Python. **It is not a compiler**, but in layman terms, it is a tool that uses the compiler to generate a compiled executable. Its minimalistic paradigm and full support of native Python enables fast prototyping of both simple and complex projects that may depend on factors ranging from cross-compiler support to supplementary software tools.

## Prerequisites

- [Python](https://www.python.org/) 3.x.x (Python 2.x is now obsolete)
- [SCons](https://pypi.org/project/scons/)

Please reference this [README](README-GETTING-STARTED.md) that talks about Python & SCons installation. **Install SCons only after you have already installed Python.** SCons is sort of a Python library and thus fundamentally relies on your machine's ability to run python scripts.

----

## Build Projects using SCons

### Build default project

By default, the `scons` command will build the `lpc40xx_freertos` project:
```bash
cd sjtwo-c

# Invoke scons from the root sjtwo-c folder
scons
```

### Build a project explicitly

This explicitly indicates which project to build inside of the `projects` directory
```bash
scons --project=<project name>

# Examples:
scons --project=lpc40xx_freertos
scons --project=x86_sandbox
```

### Build and run unit tests

Unit tests are ran by default before compilation of the project.

```bash
# By default, this will run unit tests and build the `lpc40xx_freertos` project
scons

# Or you can be more explicit
scons --project=lpc40xx_freertos

# Run unit tests and build the `x86_sandbox` project
scons --project=x86_sandbox
```

To disable unit tests, add the option `--no-unit-test`

```bash
scons --no-unit-test

# OR
scons --no-unit-test --project=x86_sandbox
```

----

## SCons Files' Documentation

This repository's SCons infrastructure consists of multiple files:

- `SConstruct` - SCons entry point
- `site_scons` - A directory which contains supplementary artifacts (i.e. modules, SCons construction environments, tools/builders, etc.)
- `site_scons/site_init.py` - First file to be executed on SCons entry
- `site_scons/cli.py` - Custom command line arguments
- `site_scons/fsops.py` - Custom supplementary functions for file system operations
- `site_scons/osops.py` - Custom supplementary functions for OS operations
- `site_scons/site_tools` - A directory which contains custom tools for construction environments
- `site_scons/environments/env_arm` - Definition of ARM construction environment and supplementary builders
- `site_scons/environments/env_x86` - Definition of Intel x86 construction environment and supplementary builders

SCons works by performing the following high-level steps:

1. Execute scripts and assemble dependency trees
2. Detect changes in file content then determine which build steps need to be executed
3. Finally, execute all required build steps

---
