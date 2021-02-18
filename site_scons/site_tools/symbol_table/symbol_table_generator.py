from itertools import dropwhile

from elf_parser import ElfParser
from symbol_table import DataType, Symbol, SymbolTable


class SymbolTableGenerator(object):
    def __init__(self, elf_file):
        self._elf_parser = ElfParser(elf_file)

        self._symbol_table = self._elf_parser.parse_symbol_table()
        self._dwarf_info = self._elf_parser.parse_dwarf_info()
        self.valid_symbol_table = None

    """
    Public methods
    """
    def generate_symbol_table(self):
        """ build valid symbol table data structure
            compare symbol name and address from symbol table with dwarf info attributes
            valid symbol if address match and symbol name within dwarf attribute
            get the base type for all valid symbols
        """
        if self.valid_symbol_table is None:
            self.valid_symbol_table = SymbolTable()

            if self._symbol_table is None or self._dwarf_info is None:
                raise ValueError("Either Symbol Table or DWARF Info not populated")

            duplicate_list = []
            for entry in self._symbol_table:
                for abbrev_offset in self._dwarf_info:
                    symbol_found = False
                    addr_found = False
                    symbol_name = ""
                    for attr in self._dwarf_info[abbrev_offset]["attr"]:
                        # There may be multiple symbol names so we do not match exact string
                        if isinstance(attr["desc"], str):
                            if attr["desc"] in entry.name:
                                if not symbol_found:
                                    symbol_name = attr["desc"]
                                    symbol_found = True
                        # We do however, match address since its definitive
                        if isinstance(attr["desc"], int):
                            if attr["desc"] == entry.address:
                                if not addr_found:
                                    addr_found = True

                    # We only want symbols that have address in its DWARF attributes
                    # get symbol type also
                    if symbol_found and addr_found:
                        if "." in entry.name:
                            # There may be duplicate symbol names: symbol_name.####
                            duplicate_symbol = entry.name.split(".")[0]
                            if duplicate_symbol in duplicate_list:
                                print("WARNING: Duplicate Symbol: {0}".format(duplicate_symbol))
                                continue
                            else:
                                duplicate_list.append(duplicate_symbol)
                        # time to go down rabbit holes...
                        base_offset, base_type , base_size = self._get_type(abbrev_offset)
                        if base_type == "struct":
                            # collect member info from struct
                            members = self._get_struct_or_union_members(base_offset)
                            decoded_struct = self._decode_struct(members)
                            for member in decoded_struct:
                                # append struct info to its member
                                if member.data_type not in ["void", "ptr"]:
                                    member_name = "{0}.{1}".format(symbol_name, member.name)
                                    member_address = entry.address + member.address
                                    data_type = self._decode_data_type(member.data_type, member.size)
                                    symbol_entry = Symbol(member_name, hex(member_address), member.size, data_type, member.bit_size, member.bit_offset)
                                    self.valid_symbol_table.add_symbol(symbol_entry)
                        elif base_type in ["void", "ptr"]:
                            # do not include these in symbol table
                            pass
                        else:
                            # base types or pointers
                            entry.data_type = base_type
                            data_type = self._decode_data_type(base_type, base_size)
                            symbol_entry = Symbol(symbol_name, hex(entry.address), base_size, data_type)
                            self.valid_symbol_table.add_symbol(symbol_entry)
                        break

        return self.valid_symbol_table

    """
    Private methods
    """
    def _decode_struct(self, struct):
        """ decode all members in struct
            recursive call itself to decode structs within structs, if any exists

            struct:
                a list of Symbol objects or Symbol object

            return:
                a list of all members in struct
        """
        struct_list = []
        if isinstance(struct, list):
            for struct_member in struct:
                if isinstance(struct_member.data_type, list):
                    for member_list in struct_member.data_type:
                        members =  self._decode_struct(member_list)
                        for member in members:
                            member_name = "{0}.{1}".format(struct_member.name, member.name)
                            member_address = struct_member.address + member.address
                            member_symbol = Symbol(member_name, member_address, member.size, member.data_type, member.bit_size, member.bit_offset)
                            struct_list.append(member_symbol)
                else:
                    struct_list.append(struct_member)
        elif isinstance(struct, Symbol):
            members = self._decode_struct(struct.data_type)
            for member in members:
                member_name = "{0}.{1}".format(struct.name, member.name)
                member_address = struct.address + member.address
                member_symbol = Symbol(member_name, member_address, member.size, member.data_type, member.bit_size, member.bit_offset)
                struct_list.append(member_symbol)
            if not isinstance(struct.data_type, list):
                struct_list.append(struct)

        return struct_list

    @staticmethod
    def _decode_data_type(data_type, data_size):
        """ Given base type and size, translate to a generic set of
            data type enum
        """
        enum_type = ""
        sign = ""
        if "char" in data_type:
            # all data_type containing char normalized to char
            if 'enum' in data_type and data_size == 1:
                if "unsigned" in data_type:
                    sign = "u"
                data_type = "int8_t"
            else:
                data_type = "char"
        elif "ptr" == data_type:
            # data size may vary
            data_type = "uintptr_t"
        elif data_type == "_Bool":
            data_type = "bool"
        elif "int" in data_type:
            size_to_data_type_map = {
                1: "int8_t",
                2: "int16_t",
                4: "int32_t",
                8: "int64_t"
            }
            if "unsigned" in data_type:
                sign = "u"
            if "uint8_t" != data_type:
                data_type = size_to_data_type_map[data_size]
        elif data_type in ["float", "double"]:
            # IEEE 754: float: 4 bytes
            # IEEE 754: double: 8 bytes
            # data type unchanged
            pass
        else:
            raise ValueError("Unable to decode type: {0}".format(data_type))

        enum_type = "{0}{1}".format(sign, data_type)

        return DataType(enum_type)

    def _get_type(self, offset):
        """ get base symbol information
            recursive call to find base type, if needed

            offset:
                offset number in dwarf info to start search

            return:
                return tuple of base symbol information
        """
        symbol_offset_list = self._get_type_offset_tree_list(offset)
        symbol_offset_list.reverse()

        if symbol_offset_list:
            # SPECIAL CASE: uint8_t
            symbol_type = None
            for symbol_offset in symbol_offset_list:
                special_type = self._get_description(symbol_offset, "DW_AT_name")
                if "uint8_t" == special_type:
                    # iterate through offset tree to see if there exist "uint8_t"
                    # use "uint8_t" instead of base type
                    symbol_type = special_type
                    break

            if symbol_type is None:
                # default get base type
                symbol_type = self._get_description(symbol_offset_list[0], "DW_AT_name")

            symbol_size = self._get_description(symbol_offset_list[0], "DW_AT_byte_size")

            if self._is_tag_type(symbol_offset_list, self._is_struct_or_union):
                # name the type struct for later processing
                symbol_type = "struct"
            elif self._is_tag_type(symbol_offset_list, self._is_pointer):
                symbol_type = "ptr"
            elif self._is_tag_type(symbol_offset_list, self._is_enum):
                symbol_type = "enum {0}".format(symbol_type)
            elif not symbol_type:
                # name the type void for later processing
                symbol_type = "void"
            else:
                # base types
                pass
            # return tuple after we find base type
            return (symbol_offset_list[0], symbol_type, symbol_size)

    def _get_type_offset_tree_list(self, offset):
        """ get descripton at offset for type

            offset:
                offset number in dwarf info

            return:
                list of offset locations: first index: root, last index: base
        """
        offset_list = []
        offset_list.append(offset)

        if self._is_pointer(offset):
            # return early if pointer type
            return offset_list

        symbol_offset = self._get_description(offset, "DW_AT_type")
        if symbol_offset:
            offset_list += self._get_type_offset_tree_list(symbol_offset)

        return offset_list

    def _get_struct_or_union_members(self, offset):
        """ get structure or union members starting at offset

            offset:
                offset number in dwarf info

            return:
                a list of Symbol object which contain struct member information
                it is possible for Symbol["type"] to be either a string or a list
                Symbol["type"] is a list if that member is a struct and ["type"] contains the struct member
        """
        struct_members = []

        for key, val in dropwhile(lambda x: x[0] != offset, self._dwarf_info.items()):
            # we start iterating from offset
            if val["offset"] == offset:
                # we skip it, we are interested in the struct members
                base_die_depth = val["depth"]
                continue

            if val["tag"] == "DW_TAG_member" and val["depth"] == base_die_depth+1:
                member_name = self._get_description(key, "DW_AT_name")
                member_location_offset = self._get_description(key, "DW_AT_data_member_location")
                member_bit_size = self._get_description(key, "DW_AT_bit_size")
                member_bit_offset = self._get_description(key, "DW_AT_bit_offset")
                if not member_location_offset:
                    # for union, since they do not have "DW_AT_data_member_location" type
                    member_location_offset = 0
                else:
                    member_location_offset = int(member_location_offset)
                base_member_offset, base_member_type, base_member_size = self._get_type(key)
                if base_member_type == "struct":
                    # struct inside struct
                    base_member_type = self._get_struct_or_union_members(base_member_offset)
                    # embed struct members in "type" key to be decoded

                new_struct_member = Symbol(member_name, member_location_offset, base_member_size,  base_member_type, member_bit_size, member_bit_offset)
                struct_members.append(new_struct_member)
            else:
                return struct_members

    def _is_tag_type(self, offset_list, tag_func):
        for offset in offset_list:
            if tag_func(offset):
                return True

        return False

    def _is_struct_or_union(self, offset):
        """ check if struct or union tag exists at offset

            offset:
                offset number in dwarf info

            return:
                True if struct or union tag, else False
        """
        dwarf_tag = self._get_dwarf_tag(offset)
        return dwarf_tag in ["DW_TAG_structure_type", "DW_TAG_union_type"]

    def _is_pointer(self, offset):
        """ check if pointer tag exists at offset

            offset:
                offset number in dwarf info

            return:
                True if pointer tag, else False
        """
        dwarf_tag = self._get_dwarf_tag(offset)
        return  (dwarf_tag == "DW_TAG_pointer_type")

    def _is_enum(self, offset):
        dwarf_tag = self._get_dwarf_tag(offset)
        return (dwarf_tag == "DW_TAG_enumeration_type")

    def _get_dwarf_tag(self, offset):
        """ check if abbrev tag exists at offset

            offset:
                offset number in dwarf info

            return:
                boolean value if tag matches
        """
        if self._dwarf_info is None:
            return

        tag = ""
        if offset in self._dwarf_info:
            tag = self._dwarf_info[offset]["tag"]

            if not tag:
                raise ValueError("Abbreviation tag not found")

        return tag

    def _get_description(self, offset, attr_type):
        """ get description for attribute type at offset

            offset:
                offset number in dwarf info

            attr_type:
                attribute type of interest

            return:
                return description for attribute type at offset if it exist
        """
        if self._dwarf_info is None:
            return

        if offset in self._dwarf_info:
            for attr in self._dwarf_info[offset]["attr"]:
                if attr["name"] == attr_type:
                    return attr["desc"]

        return None
