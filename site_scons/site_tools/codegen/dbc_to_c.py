from argparse import ArgumentParser
import os
import sys

sys.path.insert(0, "site_packages")

import cantools


def get_args():
    arg_parser = ArgumentParser()
    arg_parser.add_argument(
        "--dbc",
        required=True,
    )

    return arg_parser.parse_args()


def main():
    args = get_args()
    dbc_filepath = args.dbc

    if not os.path.isfile(dbc_filepath):
        print("Unable to find DBC file: [{}]".format(dbc_filepath))

    dbc = cantools.database.load_file(dbc_filepath)


if __name__ == "__main__":
    sys.exit(main())
