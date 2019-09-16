# SJ2-C SCons Documentation


## Introduction

SCons is a lightweight build system implemented in Python. Its minimalistic paradigm and full support of native Python enables fast prototyping of both simple and complex projects that may depend on factors ranging from cross-compiler support to supplementary software tools.

## Prerequisite

- Python 2.7.x or Python 3.x.x (https://www.python.org/)
- SCons (https://pypi.org/project/scons/)

## Setup

```bash
pip install scons
```

## Usage

```bash
scons
```

## Documentation

This repository's SCons infrastructure consists of multiple files:

- SConstruct - SCons entry point.
- SConscript - Supplementary files that extend the build system.
- env_arm - Definition of ARM construction environment and supplementary builders.
- site_scons - A directory which contains supplementary artifacts (i.e. modules, SCons construction environments, tools/builders, etc.).
- site_tools/site_init.py - First file to be executed on SCons entry.
- site_scons/cli.py - Custom command line arguments.
- site_scons/fsops.py - Custom supplementary functions for file system operations.
- site_scons/osops.py - Custom supplementary functions for OS operations.
- site_scons/site_tools - A directory which contains custom tools for construction environments.

SCons works by performing the following high-level steps:

1. Execute scripts and assemble dependency trees.
2. Detect changes in file content then determine which build steps need to be executed.
3. Finally, execute all required build steps.
