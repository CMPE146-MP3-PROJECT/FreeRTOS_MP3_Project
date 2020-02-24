from argparse import ArgumentParser
from collections import namedtuple
import os
import signal
import subprocess
import sys
from threading import Thread
import time

from color import ColorString
from unit_test_summary import UnitTestSummary
from windows import NtStatusException

Signal = namedtuple("Signal", ["name", "code"])


class ProcessWatchdog(Thread):
    def __init__(self, process, timeout, **kwargs):
        super(ProcessWatchdog, self).__init__(**kwargs)
        self.setDaemon = True
        self._process = process
        self._timeout = timeout
        self._timeout_expired = False
        self.start()

    def run(self):
        start_time = time.time()
        while time.time() - start_time < self._timeout:
            if self._process.poll() is not None:
                break
            time.sleep(0.001)
        else:
            self._process.kill()
            self._timeout_expired = True

    @property
    def timeout_expired(self):
        return self._timeout_expired


def get_args():
    arg_parser = ArgumentParser()
    arg_parser.add_argument(
        "-i", "--input",
        metavar="<EXE>",
        action="append",
        default=None,
    )
    arg_parser.add_argument(
        "--summary-only",
        action="store_true",
    )
    arg_parser.add_argument(
        "--timeout",
        type=int,
        default=5,
    )
    return arg_parser.parse_args()


def main():
    args = get_args()
    exe_filepaths = args.input if (args.input is not None) else []
    summary_only = args.summary_only
    timeout = args.timeout

    unit_test_summary = UnitTestSummary()

    for exe_filepath in exe_filepaths:
        filename = os.path.basename(exe_filepath)
        basename, _ = os.path.splitext(filename)

        stdout, stderr, error, timeout_expired = execute_process_with_timeout(exe_filepath, timeout=timeout)
        if timeout_expired:
            timeout_expired = timeout

        if error or timeout_expired or not summary_only:
            print_execution_header(exe_filepath)
            if stdout:
                print(str(stdout) if not hasattr(stdout, "decode") else stdout.decode("ascii"))
            print_execution_footer(exe_filepath, error, timeout_expired)

        unit_test_summary.add_result(basename, not error)

    if len(unit_test_summary) > 0:
        print(unit_test_summary)
    else:
        print("No unit-tests defined.")

    return unit_test_summary.has_failure


def execute_process_with_timeout(command, timeout):
    process = subprocess.Popen(command, stdout=subprocess.PIPE)
    stdout = ""
    stderr = ""
    error = 1
    timeout_expired = False

    if hasattr(subprocess, "TimeoutExpired"):
        try:
            stdout, stderr = process.communicate(timeout=timeout)
            error = process.returncode
        except subprocess.TimeoutExpired:
            process.kill()
            error = 1
            timeout_expired = True
    else:
        process_watchdog = ProcessWatchdog(process=process, timeout=timeout)
        stdout, stderr = process.communicate()
        error = process.returncode
        process_watchdog.join(1)
        if process_watchdog.timeout_expired:
            stdout = ""
            stderr = ""
            error = 1
            timeout_expired = True

    return stdout, stderr, error, timeout_expired


def print_execution_header(exe_filepath):
    print("")
    print("--- {} [{}] ---".format(ColorString("Executing").yellow, ColorString(os.path.basename(exe_filepath)).yellow))
    print("")
    sys.stdout.flush()


def print_execution_footer(exe_filepath, error, timeout_expired=None):
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
        if timeout_expired:
            print_timeout_expired(timeout_expired)
        else:
            signal_exception = get_signal_exception(error)
            if signal_exception is not None:
                print_signal_exception(signal_exception)

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


def print_timeout_expired(timeout):
    message = ColorString("Timeout expired after {} seconds!".format(timeout)).red
    print(message)
    print("")

    recommendation = (
        "Unit-tests should normally be designed to quickly execute and finish.\n"
        "It is likely that you have an infinite loop and need to revise the logic.\n"
        "If you really believe your unit-test is meant to take a long time, invoke scons with an extended timeout.\n"
        "Example timeout of 10 seconds:\n"
        "> scons --timeout=10"
    ).format(timeout)
    print(ColorString(recommendation).red)
    print("")


def print_signal_exception(signal):
    error_string = "{} ({})".format(signal.name, "0x" + hex(signal.code)[2:].upper())
    message = ColorString("Unit-test executable crashed! {}".format(error_string)).red
    print(message)
    print("")

    recommendation = "Your code logic is likely doing something malicious and needs to be fixed."
    print(ColorString(recommendation).red)
    print("")


if __name__ == "__main__":
    sys.exit(main())
