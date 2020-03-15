from argparse import ArgumentParser
import os
import sys

from code_writer import CodeWriter, GENERATE_ALL_NODE_NAME, InvalidDBCNodeError
from color import ColorString


def get_args():
    arg_parser = ArgumentParser()
    arg_parser.add_argument(
        "--dbc",
        required=True,
    )
    arg_parser.add_argument(
        "-o", "--output",
        default=None,
    )
    arg_parser.add_argument(
        "--print-only",
        action="store_true",
    )
    arg_parser.add_argument(
        "--dbc-node-name",
        default=GENERATE_ALL_NODE_NAME,
    )
    return arg_parser.parse_args()


def main():
    args = get_args()
    dbc_filepath = args.dbc
    output = args.output
    print_only = args.print_only
    dbc_node_name = args.dbc_node_name

    if not os.path.isfile(dbc_filepath):
        print("Unable to find DBC file: [{}]".format(dbc_filepath))
        return 1  # Return early

    try:
        code_writer = CodeWriter(dbc_filepath, dbc_node_name)
    except InvalidDBCNodeError as err:
        print(ColorString(str(err)).red)
        return 1  # Return early

    if not print_only:
        dbc_filename = os.path.basename(dbc_filepath)
        basename, ext = os.path.splitext(os.path.basename(dbc_filepath))
        output_filename = "{}.h".format(basename)

        if output is None:
            output_filepath = output_filename
        elif os.path.isdir(output) or "." not in os.path.basename(output):
            output_filepath = os.path.join(output, output_filename)
        else:
            output_filepath = output

        message = "Generating code [{}] -> [{}]".format(dbc_filename, output_filename)
        if dbc_node_name != GENERATE_ALL_NODE_NAME:
            message += " using node [{}]".format(dbc_node_name)
        print(ColorString(message).green)

        if not os.path.isdir(os.path.dirname(output_filepath)):
            os.makedirs(os.path.dirname(output_filepath))
        with open(output_filepath, "w") as file:
            file.write(str(code_writer))
    else:
        print(code_writer)

    return 0

if __name__ == "__main__":
    sys.exit(main())
