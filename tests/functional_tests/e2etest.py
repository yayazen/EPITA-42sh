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
        print("[\033[92m+\033[0m] " + cmd)
        return
    
    else:
        test_failed()
        print("[\033[91m-\033[0m] " + cmd)
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


if err:
    sys.exit(-1)
