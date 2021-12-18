#!/bin/python3

import subprocess
import sys
import time

err = 0
successes = 0
test_dir = "/tmp/test_42sh"

#
#
# Functions part
#
#


def print_info(str):
    print("[\033[94m*\033[0m] {}".format(str))


def shell_to_use():
    return "dash" if len(sys.argv) == 1 else sys.argv[1]


def verbose_mode():
    return len(sys.argv) > 2


def test_failed():
    global err
    err += 1


def test_passed():
    global successes
    successes += 1


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
    with open(path) as f:
        return len(f.readlines())


#
#
# Tests part
#
# These tests where meant to be temporary, but it seems
# that it won't be the case. I am sorry for the ugly code,
# but fortunately, it is working as expected
#
#
print_info("Using shell {}".format(shell_to_use()))


print()

print_info("Setup environment...")
test_simple_cmd("rm -rf "+test_dir+";", b"", b"", 0, working_directory="/tmp")
test_simple_cmd("mkdir -p "+test_dir+"/repa "+test_dir+"/forbidden "+test_dir+"/repb " +
                test_dir+"/toat; chmod a-r "+test_dir+"/forbidden", b"", b"", 0, working_directory="/tmp")

# Test simple commands
print_info("Simple commands...")
test_simple_cmd("uname", b"Linux\n", b"", 0)
test_simple_cmd(
    "ln -h",
    b"",
    b"ln: invalid option -- 'h'\nTry 'ln --help' for more information.\n",
    1
)
test_simple_cmd("/bin/sh -c \"echo 'toto'\"", b"toto\n", b"", 0)
test_simple_cmd("nonexisting_command", b"", None, 127)


print_info("If else...")
test_simple_cmd("if true; then uname; fi", b"Linux\n", b"", 0)
test_simple_cmd("if false; then uname; fi", b"", b"", 0)
test_simple_cmd("if false\n then uname; fi", b"", b"", 0)
test_simple_cmd("if false\n true;true; then uname; fi", b"Linux\n", b"", 0)
test_simple_cmd("if false; then ls; else uname; fi", b"Linux\n", b"", 0)
test_simple_cmd(
    "if false; then ls; elif true; then uname; else ls; fi", b"Linux\n", b"", 0)
test_simple_cmd("if false; then ls; elif false; then uname; fi", b"", b"", 0)
test_simple_cmd("if true; then nonexisting_command; fi", b"", None, 127)

print_info("Single quote expansion...")
test_simple_cmd("echo 'yes'72", b"yes72\n", b"", 0)
test_simple_cmd("echo 'yes'72'non'", b"yes72non\n", b"", 0)
test_simple_cmd("echo '$PATH'72", b"$PATH72\n", b"", 0)
test_simple_cmd("echo ''", b"\n", b"", 0)
test_simple_cmd("echo 'a\nb\nc'", b"a\nb\nc\n", b"", 0)
test_simple_cmd("echo '$HAPPY_42SH'", b"$HAPPY_42SH\n", b"",
                0, env={"HAPPY_42SH": "I love 42sh"})

print_info("Double quotes expansion...")
test_simple_cmd("echo $HAPPY_42SH", b"\n", b"", 0)
test_simple_cmd("echo $HAPPY_42SH", b"I love 42sh\n", b"",
                0, env={"HAPPY_42SH": "I love 42sh"})
test_simple_cmd("echo \"$HAPPY_42SH\"", b"I love 42sh\n", b"",
                0, env={"HAPPY_42SH": "I love 42sh"})
test_simple_cmd("echo \"$HAPPY_42SH$NONEXISTING_VAR\"", b"I love 42sh\n", b"",
                0, env={"HAPPY_42SH": "I love 42sh"})
test_simple_cmd("echo \"hop hop hop\"", b"hop hop hop\n", b"", 0)


print_info("Variables assignments...")
test_simple_cmd("X=ABC; echo $X", b"ABC\n", b"", 0)
test_simple_cmd("X=TOTO;X=ABC;Y=TOT; echo $X", b"ABC\n", b"", 0)
test_simple_cmd("HAPPY_42SH=\"I love 42sh\";echo $HAPPY_42SH", b"I love 42sh\n", b"",
                0, env={"HAPPY_42SH": "I hate 42sh"})


print_info("echo builtin...")
if shell_to_use() == "dash":
    print_info("Skipping non POSIX tests for dash")
else:
    test_simple_cmd("echo hello", b"hello\n", b"", 0)
    test_simple_cmd("echo hello yes", b"hello yes\n", b"", 0)
    test_simple_cmd("echo -n hello", b"hello", b"", 0)
    test_simple_cmd("echo -n h\\tello", b"h\\tello", b"", 0)
    test_simple_cmd("echo -ne h\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -en h\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -e -n h\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -n -e h\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -n -n -e h\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -e h\\tello", b"h\tello\n", b"", 0)
    test_simple_cmd("echo -e h\\nello", b"h\nello\n", b"", 0)
    test_simple_cmd("echo -e h\\nel\\\\lo", b"h\nel\\lo\n", b"", 0)

print_info("cd builtin...")
test_simple_cmd("cd /tmp", b"", b"", 0)
test_simple_cmd("cd /tmp; pwd", b"/tmp\n", b"", 0)
test_simple_cmd("cd /nonexisting", b"", validate_status=lambda s: s > 0)
test_simple_cmd("cd /tmp; cd /bin; cd -; pwd",
                b'/tmp\n/tmp\n', b"", 0)
test_simple_cmd("cd /tmp; cd "+test_dir+"; cd -; pwd; cd -; env -i pwd",
                b'/tmp\n/tmp\n'+(test_dir.encode('ascii'))+b'\n'+(test_dir.encode('ascii'))+b'\n', b"", 0)
test_simple_cmd("cd /tmp;cd;pwd", test_dir.encode('ascii')+b"\n",
                b"", 0, env={"HOME": test_dir})
test_simple_cmd("cd /tmp;cd;pwd", b"/tmp\n", b"", 0, env={"HOME": ""})


print_info("Pipelines")
test_simple_cmd("uname -a | cut -f 1 -d x", b"Linu\n", b"", 0)
test_simple_cmd(
    " | ".join(["sleep 0.001"] * 100),
    stdout=b"",
    stderr=b"",
    status=0,
    max_exec_time=0.2
)
test_simple_cmd("! uname", b"Linux\n", empty_stderr=True,
                validate_status=lambda s: s != 0)
test_simple_cmd("! { uname; }", b"Linux\n",
                empty_stderr=True, validate_status=lambda s: s != 0)
test_simple_cmd("ls /proc/self/fd | wc -l", b"4\n", b"", 0)
test_simple_cmd("ls /proc/self/fd | cat | cat | wc -l", b"4\n", b"", 0)
test_simple_cmd(
    "uname | cat | ls /proc/self/fd | cat | cat | wc -l", b"4\n", b"", 0)

print_info("Compound lists")
test_simple_cmd("{ uname; uname; }", b"Linux\nLinux\n", b"", 0)
test_simple_cmd("{ echo yes; }\n{ uname; }", b"yes\nLinux\n", b"", 0)
test_simple_cmd("{ echo a; echo b; } | cat -e", b"a$\nb$\n", b"", 0)
test_simple_cmd("{ uname; uname; } | cat -e", b"Linux$\nLinux$\n", b"", 0)


print_info("Redirections...")
test_simple_cmd("uname > /tmp/lolo;cat /tmp/lolo; rm /tmp/lolo",
                b"Linux\n", b"", 0)
test_simple_cmd("ls "+test_dir+" | grep to > /tmp/lolo; cat /tmp/lolo; rm /tmp/lolo",
                b"toat\n", b"", 0)
test_simple_cmd("uname > /tmp/lolo;cat /tmp/lolo; uname >> /tmp/lolo;cat -e /tmp/lolo; rm /tmp/lolo",
                b"Linux\nLinux$\nLinux$\n", b"", 0)
test_simple_cmd("uname > /tmp/lolo;cat /tmp/lolo; uname > /tmp/lolo;cat -e /tmp/lolo; rm /tmp/lolo",
                b"Linux\nLinux$\n", b"", 0)
test_simple_cmd("uname 1>&2", b"", b"Linux\n", 0)
test_simple_cmd("find /qsdfqsd 2>&1", empty_stdout=False,
                stderr=b"", validate_status=lambda s: s != 0)
test_simple_cmd(
    "uname > /dev/null | cat 2>&1 | ls /proc/self/fd | wc -l", b"4\n", b"", 0)


def postcheck():
    if count_lines_in_file(test_dir + "/out") == 0:
        yield "* stdout  is empty! (it should not be)"
    if count_lines_in_file(test_dir + "/err") == 0:
        yield "* stderr  is empty! (it should not be)"


test_simple_cmd("echo yes > "+test_dir+"/afile; cat "+test_dir + "/afile /nonexisting/file > "+test_dir +
                "/out 2> "+test_dir+"/err",
                empty_stderr=True,
                empty_stdout=True,
                validate_status=lambda s: s != 0,
                additional_checks=postcheck)


print_info("Until loops")
test_simple_cmd(
    "until cat /tmp/titi; do touch /tmp/titi; echo a; done; rm /tmp/titi",
    b"a\n",
    stderr=None,
    status=0
)
test_simple_cmd(
    "touch /tmp/titi;until cat /tmp/titi; do touch /tmp/titi; echo a; done; rm /tmp/titi",
    stdout=b"",
    stderr=b"",
    status=0
)

print_info("While loop")
test_simple_cmd(
    "touch /tmp/titi;while ! cat /tmp/titi; do touch /tmp/titi; echo a; done; rm /tmp/titi",
    stdout=b"",
    stderr=b"",
    status=0
)
test_simple_cmd(
    "touch /tmp/tatu;while cat /tmp/tatu; do rm /tmp/tatu; echo hihi; done;",
    stdout=b"hihi\n",
    stderr=None,
    status=0
)


print_info("Invalid commands")
test_simple_cmd("if", b"", empty_stderr=False,
                validate_status=lambda x: x != 0)
test_simple_cmd("if true; then ", b"", empty_stderr=False,
                validate_status=lambda x: x != 0)
test_simple_cmd("{ { { { { ls; } } } }", b"", empty_stderr=False,
                validate_status=lambda x: x != 0)

# Clean environment
print_info("Clean test environment")
test_simple_cmd("rm -rf "+test_dir+"", b"", b"", 0, working_directory="/tmp")

# . end of tests
print("{} / {} tests failed".format(err, successes + err))
if err:
    sys.exit(-1)
