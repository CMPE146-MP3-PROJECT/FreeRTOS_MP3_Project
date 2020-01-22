"""
This module is responsible for finding C/C++ preprocessor #include directives
Notes:
- Ignores standard headers (e.g. <stdio.h>)
- Captures supported file extensions only: *.h, *.hh, *.hpp
"""

import copy
import os
import re


class IncludeParser:
    supported_file_exts = [
        "h",
        "hh",
        "hpp",
    ]

    # #include "([_a-zA-Z][_a-zA-Z0-9]*).(h|hh|hpp)"
    # Example: #include "test.h"
    include_pattern = "#include \"([_a-zA-Z][_a-zA-Z0-9]*).({})\"".format("|".join(supported_file_exts))

    def __init__(self, filepath):
        self._filepath = filepath
        self._filenames = []

        with open(filepath) as file_obj:
            for line in file_obj.readlines():
                match = re.match(pattern=self.include_pattern, string=line)
                if match is not None:
                    self._filenames.append("{}.{}".format(match.group(1), match.group(2)))

    @property
    def basenames(self):
        basenames = []
        for filename in self._filenames:
            basenames.append(os.path.splitext(filename)[0])
        return copy.deepcopy(basenames)
    
    @property
    def filenames(self):
        return self._filenames
    
    @property
    def filepath(self):
        return self._filepath
