import subprocess
import sys
import time


err = 0
successes = 0
test_dir = "/tmp/test_42sh"


def print_info(str):
    print("[\033[94m*\033[0m] {}".format(str))


def shell_to_use():
    """
    Return the name of the binary to use for tests
    """
    return "dash" if len(sys.argv) == 1 else sys.argv[1]


def verbose_mode():
    """
    Returns True if verbose mode is enabled (if passing
    tests should be also printed)
    """
    return len(sys.argv) > 2


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


def test_simple_cmd(cmd, stdout=None,
                    stderr=None,
                    status=None,
                    validate_status=None,
                    empty_stdout=None,
                    empty_stderr=None,
                    env=None,
                    max_exec_time=None,
                    working_directory=None,
                    additional_checks=None):
    """
    Run a simple command, and check its output
    Some options can be used to customize test
    """
    global test_dir
    t = time.time()
    exec_time = 0
    errors = []
    try:
        res = subprocess.run(
            [shell_to_use(), "-c", cmd],
            capture_output=True,
            shell=False,
            check=False,
            timeout=1,
            env=env,

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

    except subprocess.TimeoutExpired:
        test_failed()
        errors.append("Test did timeout")

    if len(errors) == 0:
        test_passed()
        if verbose_mode():
            print("[\033[92m+\033[0m] {}s ".format(round(exec_time, 2)) +
                  cmd.replace("\n", "<NEWLINE>"))
        return

    else:
        test_failed()
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
