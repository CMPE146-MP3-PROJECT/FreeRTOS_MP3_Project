from collections import OrderedDict
from enum import Enum


class DataType(str, Enum):
    UINTPTR_T   = "uintptr_t"
    BOOL        = "bool"
    CHAR        = "char"
    FLOAT       = "float"
    DOUBLE      = "double"
    INT8_T      = "int8_t"
    INT16_T     = "int16_t"
    INT32_T     = "int32_t"
    INT64_T     = "int64_t"
    UINT8_T     = "uint8_t"
    UINT16_T    = "uint16_t"
    UINT32_T    = "uint32_t"
    UINT64_T    = "uint64_t"


class Symbol:
    def __init__(self, name, address, size, data_type="", bit_size=None, bit_offset=None):
        self.name = name
        self.address = address
        self.size = size
        self.data_type = data_type
        self.bit_size = bit_size
        self.bit_offset = bit_offset

    def serialize(self):
        symbol_dict = vars(self)
        symbol_dict = {key: val for key, val in symbol_dict.items() if val != None}
        return symbol_dict

    @classmethod
    def deserialize(cls, serialized_data):
        serialized_data["data_type"] = DataType(serialized_data["data_type"])
        if not "bit_size" in serialized_data:
            serialized_data["bit_size"] = None
        if not "bit_offset" in serialized_data:
            serialized_data["bit_offset"] = None
        return cls(**serialized_data)


class SymbolTable:
    def __init__(self, symbol_table=None):
        self._table = symbol_table if symbol_table else []

    def __iter__(self):
        for symbol in self._table:
            yield symbol

    def __len__(self):
        return len(self._table)

    def add_symbol(self, symbol):
        self._table.append(symbol)

    def serialize(self):
        return [symbol.serialize() for symbol in self._table]

    @classmethod
    def deserialize(cls, serialized_data):
        symbol_table = []
        for symbol in serialized_data["symbol_table"]:
            symbol_table.append(Symbol.deserialize(symbol))

        table_dict = OrderedDict()
        table_dict["symbol_table"] = symbol_table

        return cls(**table_dict)


class SymbolTableContainer:
    def __init__(self, symbol_table):
        self.symbol_table = symbol_table
        self.count = len(symbol_table)
        self.size = self._get_approx_table_size()

    def serialize(self):
        symbol_table_dict = OrderedDict()
        symbol_table_dict["count"] = self.count
        symbol_table_dict["size"] = self.size
        symbol_table_dict["symbol_table"] = self.symbol_table.serialize()

        return symbol_table_dict

    @classmethod
    def deserialize(cls, serialized_data):
        symbol_table = OrderedDict()
        symbol_table["symbol_table"] = SymbolTable.deserialize(serialized_data)

        return cls(**symbol_table)

    def _get_approx_table_size(self):
        NAME_SIZE = 65 # Assuming 64 bytes + 1 NULL
        ADDRESS_SIZE = 4
        DATA_ENUM_SIZE = 4
        DATA_SIZE = 4

        return (NAME_SIZE + ADDRESS_SIZE + DATA_ENUM_SIZE + DATA_SIZE) * len(self.symbol_table)
