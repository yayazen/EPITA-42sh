#!/bin/python3

import subprocess
import sys
import time

err = False


def print_info(str):
    print("[\033[94m*\033[0m] {}".format(str))


def shell_to_use():
    return "dash" if len(sys.argv) == 1 else sys.argv[1]


def test_failed():
    global err
    err = True


def test_simple_cmd(cmd, stdout=None,
                    stderr=None,
                    status=None,
                    validate_status=None,
                    env=None,
                    max_exec_time=None):
    """
    Run a simple command, and check its output
    """
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

        if stderr is not None and res.stderr != stderr:
            errors.append(
                "* stderr\texpected\t: {}\n\t\tgot instead\t: {}\n".format(
                    stderr,
                    res.stderr,
                )
            )

        if validate_status is not None and not validate_status(res.returncode):
            errors.append("* Exit code failed to validate!")

        if max_exec_time is not None:
            exec_time = time.time() - t
            if exec_time > max_exec_time:
                errors.append("* Exec time ({} secs) higher than allowed maximum ({} secs)".format(
                    exec_time,
                    max_exec_time
                ))

    except subprocess.TimeoutExpired:
        test_failed()
        errors.append("Test did timeout")

    if len(errors) == 0:
        # print("[\033[92m+\033[0m] " + cmd.replace("\n", "<NEWLINE>"))
        return

    else:
        test_failed()
        print("[\033[91m-\033[0m] " + cmd.replace("\n", "<NEWLINE>"))
        print("\n".join(errors))
        print()


print_info("Using shell {}".format(shell_to_use()))


# Test simple commands
print()
print_info("Simple commands...")
test_simple_cmd("uname", b"Linux\n", b"", 0)
test_simple_cmd(
    "ln -h",
    b"",
    b"ln: invalid option -- 'h'\nTry 'ln --help' for more information.\n",
    1
)
# Quotes are not supported yet
#test_simple_cmd("/bin/sh -c \"echo 'toto'\"", b"toto\n", b"", 0)
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

#print_info("Single quote expansion...")
#test_simple_cmd("echo 'yes'72", b"yes72\n", b"", 0)
#test_simple_cmd("echo 'yes'72'non'", b"yes72non\n", b"", 0)
#test_simple_cmd("echo '$PATH'72", b"$PATH72\n", b"", 0)
#test_simple_cmd("echo ''", b"\n", b"", 0)
#test_simple_cmd("echo 'a\nb\nc'", b"a\nb\nc\n", b"", 0)


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
test_simple_cmd("cd /tmp; /usr/bin/pwd", b"/tmp\n", b"", 0)
test_simple_cmd("cd /nonexisting", b"", validate_status=lambda s: s > 0)
test_simple_cmd("cd /tmp; cd /bin; cd -; /usr/bin/pwd",
                b'/tmp\n/tmp\n', b"", 0)
# test_simple_cmd("cd /tmp; cd /bin; cd -; /usr/bin/pwd; cd -; /usr/bin/pwd",
#                b'/tmp\n/tmp\n/bin\n/usr/bin\n', b"", 0)
test_simple_cmd("cd /tmp;cd;/usr/bin/pwd", b"/var\n",
                b"", 0, env={"HOME": "/var"})
test_simple_cmd("cd /tmp;cd;/usr/bin/pwd", b"/tmp\n", b"", 0, env={"HOME": ""})


print_info("Pipelines")
test_simple_cmd("uname -a | cut -f 1 -d x", b"Linu\n", b"", 0)
test_simple_cmd(
    " | ".join(["sleep 0.001"] * 100),
    stdout=b"",
    stderr=b"",
    status=0,
    max_exec_time=0.09
)

print_info("Compound lists")
test_simple_cmd("{ uname; uname; }", b"Linux\nLinux\n", b"", 0)
test_simple_cmd("{ echo yes; }\n{ uname; }", b"yes\nLinux\n", b"", 0)
#test_simple_cmd("{ echo a; echo b; } | cat -e", b"a$\nb$\n", b"", 0)
#test_simple_cmd("{ uname; uname; } | cat -e", b"Linux$\nLinux$\n", b"", 0)

if err:
    sys.exit(-1)
