#!/bin/python3

import subprocess
import sys

err = False

def print_info(str):
        print("[\033[94m*\033[0m] {}".format(str))

def shell_to_use() :
    return "dash" if len(sys.argv) == 1 else sys.argv[1]

def test_failed() :
    global err
    err = True

def test_simple_cmd(cmd, stdout, stderr, status):
    """
    Run a simple command, and check its ouput
    """
    res = subprocess.run(
        [shell_to_use(), "-c", cmd], 
        capture_output=True, 
        shell=False,
        check=False
    )

    errors = []

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
    
    if len(errors) == 0:
        print("[\033[92m+\033[0m] " + cmd.replace("\n", "<NEWLINE>"))
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
test_simple_cmd("if false; then ls; elif true; then uname; else ls; fi", b"Linux\n", b"", 0)
test_simple_cmd("if false; then ls; elif false; then uname; fi", b"", b"", 0)
test_simple_cmd("if true; then nonexisting_command; fi", b"", None, 127)

print_info("Single quote expansion...")
test_simple_cmd("echo 'yes'72", b"yes72\n", b"", 0)
test_simple_cmd("echo 'yes'72'non'", b"yes72non\n", b"", 0)
test_simple_cmd("echo '$PATH'72", b"$PATH72\n", b"", 0)
test_simple_cmd("echo ''", b"\n", b"", 0)
test_simple_cmd("echo 'a\nb\nc'", b"a\nb\nc\n", b"", 0)


print_info("echo builtin...")
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

if err:
    sys.exit(-1)
