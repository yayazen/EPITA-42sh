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
