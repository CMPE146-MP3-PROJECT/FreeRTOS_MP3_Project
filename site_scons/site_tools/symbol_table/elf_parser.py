from collections import defaultdict, OrderedDict
import logging
import os
import re
import sys


from symbol_table import Symbol, SymbolTable

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "site_packages"))

from elftools.dwarf.descriptions import describe_attr_value
from elftools.elf.descriptions import describe_symbol_type
from elftools.elf.elffile import ELFFile
from elftools.common.py3compat import itervalues
from elftools.elf.sections import SymbolTableSection

"""
References:
- https://sourceware.org/binutils/docs/binutils/readelf.html
- https://github.com/eliben/pyelftools
"""


class ElfParser(object):
    def __init__(self, elf_file):
        self._elf = ELFFile(elf_file)
        self.symbol_table = None
        self.dwarf_info = None

    """
    Public methods
    """
    def parse_symbol_table(self):
        """ build symbol table data structure

        :return: list of symbols
        """
        if self.symbol_table is None:
            self.symbol_table = SymbolTable()

            symbol_tables = [section for section in self._elf.iter_sections() if isinstance(section, SymbolTableSection)]
            for section in symbol_tables:
                for symbol in section.iter_symbols():
                    if ((int(symbol["st_size"]) > 0) and ("OBJECT" == describe_symbol_type(symbol["st_info"]["type"]))):
                        symbol_entry = Symbol(symbol.name, symbol["st_value"], symbol["st_size"])
                        self.symbol_table.add_symbol(symbol_entry)

        return self.symbol_table


    def parse_dwarf_info(self):
        """ build dwarf info data structure

        :return: OrderedDict
        """
        if self.dwarf_info is None:
            self.dwarf_info = OrderedDict()

            logging.debug('Parsing DWARF Info...')
            dwarf_info = self._elf.get_dwarf_info()
            if not dwarf_info.has_debug_info:
                raise ValueError("Debug information not available in ELF file. \
                                    Symbol table will be empty")

            for cu in dwarf_info.iter_CUs():
                die_depth = 0
                for die in cu.iter_DIEs():

                    if die.is_null():
                        die_depth -= 1
                        continue

                    # abbreviation property of interest
                    abbreviation = OrderedDict()
                    abbreviation["depth"] = die_depth
                    abbreviation["offset"] = die.offset
                    abbreviation["code"] = die.abbrev_code
                    abbreviation["tag"] = die.tag if not die.is_null() else ""
                    abbreviation["attr"] = []

                    abbreviation_log_string = " <{0}><{1}>: Abbrev Number: {2} ({3})".format(die_depth, hex(die.offset), die.abbrev_code, die.tag)
                    logging.debug(abbreviation_log_string)

                    for attr in itervalues(die.attributes):
                        description = self._get_attribute_description(attr, die)

                        if description is not None:
                            attr_dict = OrderedDict()
                            attr_dict["offset"] = attr.offset
                            attr_dict["name"] = attr.name
                            attr_dict["desc"] = description
                            abbreviation["attr"].append(attr_dict)

                            log_description = hex(description) if isinstance(description, int) else description
                            attribute_log_string = "    <{0}>   {1}: {2}".format(hex(attr.offset), attr.name, log_description)
                            logging.debug(attribute_log_string)

                    if abbreviation["attr"]:
                        self.dwarf_info[die.offset] = abbreviation

                    if die.has_children:
                        die_depth += 1

        return self.dwarf_info

    """
    Private methods
    """
    def _get_attribute_description(self, attr, die):
        """ Use regex to parse attribute description (value)
        """
        description = describe_attr_value(attr, die, 0)
        regex_pattern = ""
        if "DW_AT_name" == attr.name:
            regex_pattern = "^([\w ]+\t)|: ([\w ]+\t)$"
        elif "DW_AT_type" == attr.name:
            regex_pattern = "^<(0x[\da-fA-F]+)>\t$"
        elif "DW_AT_location" == attr.name:
            regex_pattern = ".*DW_OP_addr: ([\w]+)"
        elif attr.name in ["DW_AT_data_member_location", "DW_AT_byte_size", "DW_AT_bit_size", "DW_AT_bit_offset"]:
            regex_pattern = "^([\d]+\t)$"

        if "" != regex_pattern:
            match = re.compile(regex_pattern)
            match = match.search(description)
            if match:
                match_group = match.groups()

                if attr.name in ["DW_AT_type", "DW_AT_location"]:
                    description = match_group[0].rstrip()
                    description = int(description, 16)

                elif attr.name in ["DW_AT_data_member_location", "DW_AT_byte_size", "DW_AT_bit_size", "DW_AT_bit_offset"]:
                    description = match_group[0].rstrip()
                    description = int(description)

                elif attr.name in ["DW_AT_name"]:
                    index = [match for match in range(len(match_group)) if match_group[match] != None]
                    description = match_group[index[0]].rstrip()
                else:
                    pass
            else:
                description = description.rstrip()
        else:
            description = None

        return description
