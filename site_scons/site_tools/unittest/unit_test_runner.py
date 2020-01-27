from argparse import ArgumentParser
import os
import subprocess
import sys

from color import ColorString
from unit_test_summary import UnitTestSummary


def get_args():
    arg_parser = ArgumentParser()
    arg_parser.add_argument(
        "-i", "--input",
        metavar="<EXE>",
        action="append",
        default=None,
    )

    return arg_parser.parse_args()


def main():
    args = get_args()
    exe_filepaths = args.input if (args.input is not None) else []

    unit_test_summary = UnitTestSummary()

    for exe_filepath in exe_filepaths:
        filename = os.path.basename(exe_filepath)
        basename, _ = os.path.splitext(filename)
        print_execution_header(exe_filepath)
        error = subprocess.call(exe_filepath)
        print_execution_footer(exe_filepath, error)
        unit_test_summary.add_result(basename, not error)

    if len(unit_test_summary) > 0:
        print(unit_test_summary)
    else:
        print("No unit tests defined.")

    return unit_test_summary.has_failure


def print_execution_header(exe_filepath):
    print("")
    print("--- {} [{}] ---".format(ColorString("Executing").yellow, ColorString(os.path.basename(exe_filepath)).yellow))
    print("")
    sys.stdout.flush()


def print_execution_footer(exe_filepath, error):
    print("")
    if not error:
        result = ColorString("SUCCESS").green
        filename = ColorString(os.path.basename(exe_filepath)).green
    else:
        result = ColorString("FAILED").red
        filename = ColorString(os.path.basename(exe_filepath)).red
    print("--- {} - Completed [{}] with error code [{}] ---".format(result, filename, error))
    print("")
    print("=" * 120)
    sys.stdout.flush()


if __name__ == "__main__":
    sys.exit(main())
