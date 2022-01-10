from os import scandir
import os.path
import os
import subprocess
import time
import random
import string

import argparse
from enum import Enum


class ExecutionMode(Enum):
    input = 'input'
    argument = 'argument'
    script = 'script'

    def __str__(self):
        return self.value


parser = argparse.ArgumentParser(
    description='Our shell End2End testsuite.')
parser.add_argument('shell', metavar='program', type=str, nargs=1,
                    help='The shell to test')
parser.add_argument('-v', '--verbose',
                    action="store_true",
                    help='Enable verbose mode (show successful tests)')
parser.add_argument('--mode',
                    type=ExecutionMode,
                    choices=list(ExecutionMode),
                    help="The way commands will be passed to shell",
                    nargs=1)

args = parser.parse_args()

err = 0
successes = 0
test_dir = "/tmp/test_42sh"

showed_curr_section_label = False
curr_section_id = None
curr_section_label = None


def print_info(str):
    print("[\033[94m*\033[0m] {}".format(str))


def test_mode() -> BaseException:
    """
    Return way commands must be passed to shell
    """
    return args.mode[0] if args.mode is not None else ExecutionMode.argument


def new_section(id, label):
    """
    Indicates that a new sections of test has started.
    Set id to null to explicitly specify that this is
    a global section that can not be skipped
    """
    global showed_curr_section_label
    global curr_section_id
    global curr_section_label
    showed_curr_section_label = False
    curr_section_id = id
    curr_section_label = label


def print_section_if_required():
    """
    Print current section name, if required
    """
    global showed_curr_section_label
    global curr_section_id
    global curr_section_label
    if not showed_curr_section_label:
        showed_curr_section_label = True
        print_info("[{}] [{}] {}".format(
            test_mode(), curr_section_id, curr_section_label))


def shell_to_use():
    """
    Return the name of the binary to use for tests
    """
    return args.shell[0]


def verbose_mode():
    """
    Returns True if verbose mode is enabled (if passing
    tests should be also printed)
    """
    return args.verbose


def test_failed():
    global err
    err += 1


def test_passed():
    global successes
    successes += 1


def number_failed_tests():
    global err
    return err


def total_number_of_tests():
    global err
    global successes
    return err + successes


def gen_tmp_file_name() -> str:
    return "/tmp/" + ''.join(random.choices(string.ascii_uppercase + string.digits, k=10))


def write_to_file(filename: str, content: str):
    f = open(filename, "w")
    f.write(content)
    f.flush()
    f.close()


def test_simple_cmd(cmd, stdout=None,
                    stderr=None,
                    status=None,
                    validate_status=None,
                    validate_stderr=None,
                    empty_stdout=None,
                    empty_stderr=None,
                    env=None,
                    max_exec_time=None,
                    working_directory=None,
                    additional_checks=None,
                    check_for_leak=True):
    """
    Run a simple command, and check its output
    Some options can be used to customize test
    """
    global test_dir
    t = time.time()
    exec_time = 0
    errors = []
    tmp_file_name = None
    try:
        args = [shell_to_use()]
        stdin = None

        # Pass arguments to shell
        if test_mode() == ExecutionMode.argument:
            args.append("-c")
            args.append(cmd)

        else:
            tmp_file_name = gen_tmp_file_name()
            write_to_file(tmp_file_name, cmd)

        if test_mode() == ExecutionMode.input:
            stdin = open(tmp_file_name, "r")

        elif test_mode() == ExecutionMode.script:
            args.append(tmp_file_name)

        res = subprocess.run(
            args,
            capture_output=True,
            shell=False,
            check=False,
            timeout=1,
            env=env,
            stdin=stdin,
            # cwd=test_dir if working_directory is None else working_directory
        )

        if status is not None and res.returncode != status:
            errors.append("* Exit code\texpected\t: {}\n\t\tgot instead\t: {}\n".format(
                status,
                res.returncode
            ))

        if stdout is not None and res.stdout != stdout:
            errors.append(
                "* stdout\texpected\t: {}\n\t\tgot instead\t: {}\n".format(
                    stdout,
                    res.stdout,
                )
            )

        if empty_stdout == True and res.stdout != b"":
            errors.append(
                "* stdout\texpected empty\n\t\tgot instead\t: {}\n".format(
                    res.stdout
                )
            )

        if empty_stdout == False and res.stdout == b"":
            errors.append(
                "* stdout\texpected not empty stdout, but it is !\n"
            )

        if stderr is not None and res.stderr != stderr:
            errors.append(
                "* stderr\texpected\t: {}\n\t\tgot instead\t: {}\n".format(
                    stderr,
                    res.stderr,
                )
            )

        if empty_stderr == True and res.stderr != b"":
            errors.append(
                "* stderr\texpected empty\n\t\tgot instead\t: {}\n".format(
                    res.stderr,
                )
            )
        if empty_stderr == False and res.stderr == b"":
            errors.append(
                "* stderr\texpected not empty, but it was!\n"
            )

        if validate_status is not None and not validate_status(res.returncode):
            errors.append(
                "* Exit code failed to validate (got {})!".format(res.returncode))

        if validate_stderr is not None and validate_stderr(res.stderr) is not None:
            errors.append(
                "* stderr failed to validate ({}) !".format(validate_stderr(res.stderr)))

        if additional_checks is not None:
            for err in additional_checks():
                errors.append(err)

        exec_time = time.time() - t
        if max_exec_time is not None:
            if exec_time > max_exec_time:
                errors.append("* Exec time ({} secs) higher than allowed maximum ({} secs)".format(
                    exec_time,
                    max_exec_time
                ))

        if check_for_leak and "LeakSanitizer: detected memory leaks" in res.stderr.decode("utf-8"):
            errors.append(
                "* Memory leak: {}".format(res.stderr.decode("utf-8")))

        if check_for_leak and "AddressSanitizer: heap-use-after-free " in res.stderr.decode("utf-8"):
            errors.append(
                "* Use after free: {}".format(res.stderr.decode("utf-8")))

    except subprocess.TimeoutExpired:
        test_failed()
        errors.append("* Test did timeout")

    # Delete script file (if any)
    if tmp_file_name is not None and os.path.isfile(tmp_file_name):
        os.remove(tmp_file_name)

    if len(errors) == 0:
        test_passed()
        if verbose_mode():
            print_section_if_required()
            print("[\033[92m+\033[0m] {}s ".format(round(exec_time, 2)) +
                  cmd.replace("\n", "<NEWLINE>"))

    else:
        test_failed()
        print_section_if_required()
        print("[\033[91m-\033[0m] {}s ".format(round(exec_time, 2)) +
              cmd.replace("\n", "<NEWLINE>"))
        print("\n".join(errors))
        print()


def count_lines_in_file(path: str) -> int:
    """
    Returns the number of lines in a file
    """
    with open(path) as f:
        return len(f.readlines())


LOREM = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aliquam consequat massa sed urna venenatis bibendum. Praesent a varius enim, nec vehicula nulla. Ut at lectus aliquam, consectetur magna sed, suscipit ipsum. Mauris nec mauris ex. Ut sed nulla suscipit, ultricies eros sed, lacinia urna. Maecenas sagittis, metus non rhoncus placerat, nisi risus interdum mi, eget viverra urna erat sed velit. Ut pellentesque neque ex. Nulla ullamcorper vel purus vel sagittis. Vivamus at consequat sem. In lacinia sed neque quis rhoncus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nunc feugiat arcu nunc, ut efficitur augue vehicula sed. Morbi sodales, urna sit amet laoreet imperdiet, justo diam consequat mauris, nec suscipit enim tortor vitae sapien. Quisque ex mauris, tempus nec dolor quis, egestas ornare erat. Donec vitae ultrices eros. Donec ac lorem posuere, scelerisque est id, laoreet ligula. In imperdiet justo sed vehicula dignissim. Nunc ut ligula eu dolor bibendum consequat in eu tortor. Integer ornare suscipit nunc vel ullamcorper. Vestibulum dapibus nisl dapibus scelerisque convallis. Nullam feugiat libero non justo ornare, at venenatis lectus mollis. Maecenas efficitur enim in urna congue sodales. Nullam porttitor ut eros a sagittis. Fusce non orci vulputate, vulputate neque iaculis, vestibulum odio. Donec scelerisque, tellus sit amet pulvinar pretium, erat tellus cursus velit, ut semper dui erat vel justo. In at eros at arcu accumsan aliquet sit amet ut urna. Donec tellus ante, mollis sed ante tincidunt, semper vehicula est. Ut condimentum, nisi eu vestibulum ullamcorper, enim tortor mollis felis, a vulputate enim diam vitae tortor. Vivamus eu mi ac ligula condimentum aliquet. Ut eget hendrerit arcu. Vestibulum in dolor pellentesque, tincidunt magna at, bibendum dolor. In sagittis, justo et fringilla dapibus, nisl urna lacinia orci, quis malesuada libero lectus in dui. Aliquam vehicula libero in placerat semper. Etiam rhoncus, dui et elementum cursus, tellus nibh pellentesque nunc, in tristique nibh sapien nec lectus. Donec pulvinar aliquam convallis. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Nam ex dui, consequat id facilisis sit amet, tempor eu tellus. In hac habitasse platea dictumst. Donec vel eleifend felis. In consectetur blandit malesuada. Suspendisse eget sagittis urna. Vestibulum sollicitudin accumsan ligula, non bibendum diam commodo quis. Sed eget interdum nisi, sit amet volutpat leo. Quisque id porttitor sem. Proin id lacus nec tortor volutpat ultricies ut id eros. Etiam aliquet nisi ut neque blandit dapibus. Sed faucibus mollis luctus. In nibh massa, condimentum vitae nisi quis, venenatis faucibus justo. Duis faucibus ex odio, nec ultricies eros lacinia quis. Etiam a fermentum tellus. Quisque malesuada felis sed sapien porttitor luctus. Aliquam erat volutpat. Morbi porttitor nisl id ex gravida"
