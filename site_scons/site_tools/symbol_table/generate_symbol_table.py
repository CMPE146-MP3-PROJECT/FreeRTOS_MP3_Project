from argparse import ArgumentParser
import logging
import os
import sys

from symbol_table_generator import SymbolTableGenerator


def get_args():
    arg_parser = ArgumentParser()
    arg_parser.add_argument(
        "--elf",
        required=True,
    )
    arg_parser.add_argument(
        "-o", "--output",
        required=True,
    )
    arg_parser.add_argument(
        "-l", "--log-output",
        default=None,
        dest="log_output",
    )
    return arg_parser.parse_args()

def main():
    args = get_args()
    elf_filepath = args.elf
    output = args.output
    log_output = args.log_output

    elf_filename = os.path.basename(elf_filepath)
    basename, ext = os.path.splitext(os.path.basename(elf_filepath))
    output_filename = "{}.json".format(basename)

    if os.path.isdir(output) or "." not in os.path.basename(output):
        output_filepath = os.path.join(output, output_filename)
    else:
        output_filepath = output

    if not os.path.isdir(os.path.dirname(output_filepath)):
        os.makedirs(os.path.dirname(output_filepath))

    if log_output is not None:
        log_filename = "{}.log".format(basename)

        if os.path.isdir(log_output) or "." not in os.path.basename(log_output):
            log_filepath = os.path.join(log_output, log_filename)
        else:
            log_filepath = output

        logging.basicConfig(filename=log_filepath, level=logging.DEBUG)

    if not os.path.isfile(elf_filepath):
        print("Unable to find ELF file: [{}]".format(elf_filepath))
        return 1  # Return early

    with open(elf_filepath,"rb") as file:
        symbol_table_generator = SymbolTableGenerator(file)

    message = "Generating Symbol Table [{}] -> [{}]".format(elf_filename, output_filename)
    print(message)

    symbol_table = symbol_table_generator.generate_symbol_table()
    symbol_table.to_json(output_filepath)

    return 0

if __name__ == "__main__":
    sys.exit(main())
