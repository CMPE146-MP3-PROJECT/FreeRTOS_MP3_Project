from argparse import ArgumentParser
from collections import namedtuple
import os
import signal
import subprocess
import sys

from color import ColorString
from unit_test_summary import UnitTestSummary
from windows import NtStatusException

Signal = namedtuple("Signal", ["name", "code"])


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

    if error:
        signal_exception = get_signal_exception(error)
        if signal_exception is not None:
            print_signal_exception(signal_exception)
            print("")

    print("=" * 120)
    sys.stdout.flush()


def get_signal_exception(error):
    found_signal = None

    try:
        if "win32" == sys.platform:
            exception = NtStatusException(error)
            found_signal = Signal(name=exception.name, code=exception.code)
        else:
            raise KeyError
    except KeyError:
        # Unable to map error code with an NTSTATUS
        # Assume error does not represent a NTSTATUS; proceed to check if error is a POSIX signal

        for attr_name, attr_value in signal.__dict__.items():
            if attr_name.startswith("SIG") and error == -attr_value:
                found_signal = Signal(name=attr_name, code=attr_value)
                break

    return found_signal


def print_signal_exception(signal):
    message = "{} ({})".format(signal.name, "0x" + hex(signal.code)[2:].upper())
    print("Unit test executable crashed! {}".format(message))



if __name__ == "__main__":
    sys.exit(main())
