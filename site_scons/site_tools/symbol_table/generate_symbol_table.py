from argparse import ArgumentParser
import json
import logging
import os
import sys

from symbol_table_generator import SymbolTableGenerator
from symbol_table import SymbolTableContainer


def get_args():
    arg_parser = ArgumentParser()
    arg_parser.add_argument(
        "elf_filepath",
        nargs=1,
        metavar="ELF-FILEPATH"
    )
    arg_parser.add_argument(
        "-o", "--output",
        required=True,
        nargs="+",
        metavar=("JSON-FILEPATH", "LOG-FILEPATH"),
    )
    return arg_parser.parse_args()

def main():
    args = get_args()
    elf_filepath = "".join(args.elf_filepath)
    json_output, log_output = args.output if len(args.output) > 1 else ("".join(args.output), None)

    elf_filename = os.path.basename(elf_filepath)
    basename, ext = os.path.splitext(os.path.basename(elf_filepath))
    json_filename = "{}.json".format(basename)

    if os.path.isdir(json_output) or "." not in os.path.basename(json_output):
        json_filepath = os.path.join(json_output, json_filename)
    else:
        json_filepath = json_output

    if not os.path.isdir(os.path.dirname(json_filepath)):
        os.makedirs(os.path.dirname(json_filepath))

    if log_output is not None:
        log_filename = "{}.log".format(basename)

        if os.path.isdir(log_output) or "." not in os.path.basename(log_output):
            log_filepath = os.path.join(log_output, log_filename)
        else:
            log_filepath = log_output

        logging.basicConfig(filename=log_filepath, level=logging.DEBUG)

    if not os.path.isfile(elf_filepath):
        print("Unable to find ELF file: [{}]".format(elf_filepath))
        return 1  # Return early

    with open(elf_filepath,"rb") as file:
        symbol_table_generator = SymbolTableGenerator(file)

    message = "Generating Symbol Table [{}] -> [{}]".format(elf_filename, json_filename)
    print(message)

    symbol_table = symbol_table_generator.generate_symbol_table()
    symbol_table_container = SymbolTableContainer(symbol_table)
    serialized_data = symbol_table_container.serialize()

    with open(json_filepath, "w") as file:
        json.dump(serialized_data, file, indent=4)

    return 0

if __name__ == "__main__":
    sys.exit(main())
