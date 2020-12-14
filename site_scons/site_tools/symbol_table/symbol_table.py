from enum import Enum
import json

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
    def __init__(self, name, address, size, data_type=""):
        self.name = name
        self.address = address
        self.size = size
        self.data_type = data_type

    def serialize(self):
        return vars(self)

    @classmethod
    def deserialize(cls, serialized_data):
        serialized_data["data_type"] = DataType(serialized_data["data_type"])
        return cls(**serialized_data)


class SymbolTable:
    def __init__(self):
        self._table = []

    def __iter__(self):
        for symbol in self._table:
            yield symbol

    @classmethod
    def deserialize(cls, serialized_data):
        symbol_table = []
        for data in serialized_data:
            symbol_table.append(Symbol.deserialize(data))

        return symbol_table

    def serialize(self):
        return [symbol.serialize() for symbol in self._table]

    def to_json(self, json_filepath):
        with open(json_filepath, "w") as file:
            json.dump(self.serialize(), file, indent=4)

    def add_symbol(self, symbol):
        self._table.append(symbol)
