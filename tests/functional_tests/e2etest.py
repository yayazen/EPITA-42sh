#!/bin/python3
from os import write
import sys
from lib import *

#
#
# Tests part
#
# These tests where meant to be temporary, but it seems
# that it won't be the case. I am sorry for the ugly code,
# but fortunately, it is working as expected
#
#
print_info("Using {} passing commands in {}".format(
    shell_to_use(), test_mode()))


new_section(None, "Setup environment...")
test_simple_cmd("rm -rf "+test_dir+";", b"", b"", 0, working_directory="/tmp")
test_simple_cmd("mkdir -p "+test_dir+"/repa "+test_dir+"/forbidden "+test_dir+"/repb " +
                test_dir+"/toat; chmod a-r "+test_dir+"/forbidden", b"", b"", 0, working_directory="/tmp")

# Test simple commands
new_section("simplecmd", "Simple commands...")
test_simple_cmd("uname", b"Linux\n", b"", 0)
test_simple_cmd(
    "ln -h",
    b"",
    b"ln: invalid option -- 'h'\nTry 'ln --help' for more information.\n",
    1
)
test_simple_cmd("/bin/sh -c \"echo 'toto'\"", b"toto\n", b"", 0)
test_simple_cmd("nonexisting_command", b"", None, 127)
# very_long_command
test_simple_cmd("echo {}".format(LOREM), stdout="{}\n".format(
    LOREM).encode("ascii"), empty_stderr=True, status=0)
# exit_code
test_simple_cmd("nonexisting_command;;", b"", None, 2)

new_section("if", "If else...")
test_simple_cmd("if true; then uname; fi", b"Linux\n", b"", 0)
test_simple_cmd("if false; then uname; fi", b"", b"", 0)
test_simple_cmd("if false\n then uname; fi", b"", b"", 0)
test_simple_cmd("if false\n true;true; then uname; fi", b"Linux\n", b"", 0)
test_simple_cmd("if false; then ls; else uname; fi", b"Linux\n", b"", 0)
test_simple_cmd(
    "if false; then ls; elif true; then uname; else ls; fi", b"Linux\n", b"", 0)
test_simple_cmd("if false; then ls; elif false; then uname; fi", b"", b"", 0)
test_simple_cmd("if true; then nonexisting_command; fi", b"", None, 127)
# bad_if_no_separator
test_simple_cmd("if true then true; fi", b"", None, 2)
# bad_if_no_condition
test_simple_cmd("if ; then true; fi", b"", None, 2)
# bad_if_no_body
test_simple_cmd("if true; then ; fi", b"", None, 2)
# bad_if_elif_without_then
test_simple_cmd("if true; then true; elif ; fi", b"", None, 2)
test_simple_cmd("if true; then true; elif true; echo yes; fi", b"", None, 2)
# bad_if_no_then
test_simple_cmd("if true; fi", b"", None, 2)

new_section("singlequotes", "Single quote expansion...")
test_simple_cmd("echo 'yes'72", b"yes72\n", b"", 0)
test_simple_cmd("echo 'yes'72'non'", b"yes72non\n", b"", 0)
test_simple_cmd("echo '$PATH'72", b"$PATH72\n", b"", 0)
test_simple_cmd("echo ''", b"\n", b"", 0)
test_simple_cmd("echo 'a\nb\nc'", b"a\nb\nc\n", b"", 0)
test_simple_cmd("echo '$HAPPY_42SH'", b"$HAPPY_42SH\n", b"",
                0, env={"HAPPY_42SH": "I love 42sh"})
# weak_quoting3

test_simple_cmd(
    cmd="echo 'toto''",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)

test_simple_cmd("echo '\\''", empty_stdout=True, empty_stderr=False, status=2)
test_simple_cmd("echo '\\'", stdout=b"\\\n", empty_stderr=True, status=0)
test_simple_cmd("echo '\\`'", stdout=b"\\`\n", empty_stderr=True, status=0)
test_simple_cmd("echo '\\\\'", stdout=b"\\\n", empty_stderr=True, status=0)
test_simple_cmd("echo '\\\n'", stdout=b"\\\n\n", empty_stderr=True, status=0)

new_section("escape_outside_quote", "Escape outside quotes")
test_simple_cmd("echo \\a", stdout=b"a\n", empty_stderr=True, status=0)
test_simple_cmd("echo \\\"", stdout=b"\"\n", empty_stderr=True, status=0)
test_simple_cmd("echo \\\'", stdout=b"\'\n", empty_stderr=True, status=0)
test_simple_cmd("echo \\t", stdout=b"t\n", empty_stderr=True, status=0)
test_simple_cmd("echo \\\\", stdout=b"\\\n", empty_stderr=True, status=0)

new_section("dlquotes", "Double quotes expansion...")
test_simple_cmd("echo $HAPPY_42SH", b"\n", b"", 0)
test_simple_cmd("echo $HAPPY_42SH", b"I love 42sh\n", b"",
                0, env={"HAPPY_42SH": "I love 42sh"})
test_simple_cmd("echo \"$HAPPY_42SH\"", b"I love 42sh\n", b"",
                0, env={"HAPPY_42SH": "I love 42sh"})
test_simple_cmd("echo \"$HAPPY_42SH$NONEXISTING_VAR\"", b"I love 42sh\n", b"",
                0, env={"HAPPY_42SH": "I love 42sh"})
test_simple_cmd("echo \"hop hop hop\"", b"hop hop hop\n", b"", 0)
test_simple_cmd("XXXX=ABCD; echo \"$XXXX\"", b"ABCD\n", b"", 0)
# strong_quoting3
test_simple_cmd("echo \"toto\"\"", empty_stdout=True,
                empty_stderr=False, status=2)
# backquote_command_substitution
test_simple_cmd("echo ` echo toto", empty_stdout=True,
                empty_stderr=False, status=2)
# backquote_command_substitution2
test_simple_cmd("echo `", empty_stdout=True, empty_stderr=False, status=2)
# parenthesis_command_substitution
test_simple_cmd("echo $(uname", empty_stdout=True,
                empty_stderr=False, status=2)

test_simple_cmd("echo \"\\'\"", stdout=b"\\'\n", empty_stderr=True, status=0)
test_simple_cmd("echo \"\\\"\"", stdout=b"\"\n", empty_stderr=True, status=0)
test_simple_cmd("echo \"\\`\"", stdout=b"`\n", empty_stderr=True, status=0)
test_simple_cmd("echo \"\\\\\"", stdout=b"\\\n", empty_stderr=True, status=0)
test_simple_cmd("echo \"\\\n\"", stdout=b"\n", empty_stderr=True, status=0)


new_section("symexp", "Symbols expansion")
test_simple_cmd("A=b;AA=c; echo $AA", stdout=b"c\n",
                empty_stderr=True, status=0)
test_simple_cmd("A=b; echo $AA", stdout=b"\n",
                empty_stderr=True, status=0)
test_simple_cmd(
    cmd="cat $NONEXISTING $BADAGAIN /nonexisting",
    empty_stdout=True,
    empty_stderr=False,
    validate_stderr=lambda stderr: None if len(stderr.decode(
        "utf-8").split("\n")) == 2 else "Expected only 2 lines in stderr!",
    status=1
)
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo $1 $2",
        additional_args=["ta", "ti", "to"],
        stdout=b"ti to\n" if test_mode() == ExecutionMode.argument else b"ta ti\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo $@",
        additional_args=["ta", "ti", "to"],
        stdout=b"ti to\n" if test_mode() == ExecutionMode.argument else b"ta ti to\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo a$@b",
        additional_args=["ta", "ti", "tu", "to"],
        stdout=b"ati tu tob\n" if test_mode() == ExecutionMode.argument else b"ata ti tu tob\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo a$@b",
        additional_args=["ta", "da", "li"],
        stdout=b"ada lib\n" if test_mode() == ExecutionMode.argument else b"ata da lib\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo a$@b",
        additional_args=["ta", "da"],
        stdout=b"adab\n" if test_mode() == ExecutionMode.argument else b"ata dab\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="for i in $@; do echo $i; done",
        additional_args=["a", "b", "c", "d"],
        stdout=b"b\nc\nd\n" if test_mode() == ExecutionMode.argument else b"a\nb\nc\nd\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="for i in \"$@\"; do echo $i; done",
        additional_args=["a", "b", "c", "d"],
        stdout=b"b\nc\nd\n" if test_mode() == ExecutionMode.argument else b"a\nb\nc\nd\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo $*",
        additional_args=["ta", "ti", "to"],
        stdout=b"ti to\n" if test_mode() == ExecutionMode.argument else b"ta ti to\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo \"a$*b\"",
        additional_args=["ta", "ti", "tu", "to"],
        stdout=b"ati tu tob\n" if test_mode() == ExecutionMode.argument else b"ata ti tu tob\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo \"a$*b\"",
        additional_args=["ta", "da", "li"],
        stdout=b"ada lib\n" if test_mode() == ExecutionMode.argument else b"ata da lib\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo \"a$*b\"",
        additional_args=["ta", "da"],
        stdout=b"adab\n" if test_mode() == ExecutionMode.argument else b"ata dab\n",
        empty_stderr=True,
        status=0
    )
test_simple_cmd(
    cmd="echo a$@b",
    additional_args=[],
    stdout=b"ab\n",
    empty_stderr=True,
    status=0
)
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo a$1b",
        additional_args=["ta", "da", "li"],
        stdout=b"adab\n" if test_mode() == ExecutionMode.argument else b"atab\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo a$9b",
        additional_args=["ta", "da", "li"],
        stdout=b"ab\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="for i in $*; do echo $i; done",
        additional_args=["a", "b", "c", "d"],
        stdout=b"b\nc\nd\n" if test_mode() == ExecutionMode.argument else b"a\nb\nc\nd\n",
        empty_stderr=True,
        status=0
    )
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="for i in \"$*\"; do echo $i; done",
        additional_args=["a", "b", "c", "d"],
        stdout=b"b c d\n" if test_mode() == ExecutionMode.argument else b"a b c d\n",
        empty_stderr=True,
        status=0
    )
test_simple_cmd("echo $?", stdout=b"0\n", empty_stderr=True, status=0)
test_simple_cmd("uname;echo $?", stdout=b"Linux\n0\n",
                empty_stderr=True, status=0)
test_simple_cmd("/nonexisting_cmd;echo $?", stdout=b"127\n",
                empty_stderr=False, status=0)
test_simple_cmd("echo $#", stdout=b"0\n", empty_stderr=True, status=0)
if not test_mode() == ExecutionMode.input:
    test_simple_cmd(
        cmd="echo $#",
        additional_args=["a", "b", "c", "d"],
        stdout=b"3\n" if test_mode() == ExecutionMode.argument else b"4\n",
        empty_stderr=True,
        status=0
    )
if shell_to_use() != "dash":
    test_simple_cmd("{ echo $RANDOM; echo $RANDOM; } | uniq | wc -l",
                    stdout=b"2\n", empty_stderr=True, status=0)
    test_simple_cmd("{ echo $UID; id -u; } | uniq | wc -l",
                    stdout=b"1\n", empty_stderr=True, status=0)
test_simple_cmd("echo $", stdout=b"$\n", empty_stderr=True, status=0)
test_simple_cmd("echo $'yes'", stdout=b"$yes\n", empty_stderr=True, status=0)


new_section("varassign", "Variables assignments...")
test_simple_cmd("X=ABC; echo $X", b"ABC\n", b"", 0)
test_simple_cmd("X=TOTO;X=ABC;Y=TOT; echo $X", b"ABC\n", b"", 0)
test_simple_cmd("HAPPY_42SH=\"I love 42sh\";echo $HAPPY_42SH", b"I love 42sh\n", b"",
                0, env={"HAPPY_42SH": "I hate 42sh"})


new_section("echo", "echo builtin...")
if shell_to_use() == "dash":
    print_info("Skipping non POSIX tests for dash")
else:
    test_simple_cmd("echo hello", b"hello\n", b"", 0)
    test_simple_cmd("echo hello yes", b"hello yes\n", b"", 0)
    test_simple_cmd("echo -n hello", b"hello", b"", 0)
    test_simple_cmd("echo '\\n'", b"\\n\n", b"", 0)
    test_simple_cmd("echo -n '\\n'", b"\\n", b"", 0)
    test_simple_cmd("echo -e '\\n'", b"\n\n", b"", 0)
    test_simple_cmd("echo -b bibi", b"-b bibi\n", b"", 0)
    test_simple_cmd("echo -E \\\\\\\\", b"\\\\\n")

    # These tests are problematic
    test_simple_cmd("echo -n h\\\\tello", b"h\\tello", b"", 0)
    test_simple_cmd("echo -ne h\\\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -en h\\\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -e -n h\\\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -n -e h\\\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -n -n -e h\\\\tello", b"h\tello", b"", 0)
    test_simple_cmd("echo -e h\\\\tello", b"h\tello\n", b"", 0)
    test_simple_cmd("echo -e h\\\\nello", b"h\nello\n", b"", 0)
    test_simple_cmd("echo -e h\\\\nel\\\\\\lo", b"h\nel\\lo\n", b"", 0)
    test_simple_cmd("echo -e \\\\\\\\", b"\\\n", b"", 0)
test_simple_cmd("echo p t", b"p t\n", b"", 0)

new_section("exit", "exit builtin...")
test_simple_cmd("echo a;\nexit; echo yolo", b"a\n", b"", 0)
test_simple_cmd("echo a;\nexit 15; echo yolo", b"a\n", b"", 15)
test_simple_cmd("echo a;\nexit 15 a; echo yolo", b"a\n", b"", 15)
# exit_bad_arg
test_simple_cmd("exit bad_arg;", empty_stdout=True,
                empty_stderr=False, status=2)

new_section("cd", "cd builtin...")
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

new_section("export", "export builtin...")
test_simple_cmd(cmd="export A; A=10; printenv A",
                stdout=b"10\n", empty_stderr=True, status=0)
test_simple_cmd(cmd="A=10;export A; printenv A",
                stdout=b"10\n", empty_stderr=True, status=0)
test_simple_cmd(cmd="export A=10; printenv A",
                stdout=b"10\n", empty_stderr=True, status=0)
test_simple_cmd(cmd="B=10;export A=$B; printenv A",
                stdout=b"10\n", empty_stderr=True, status=0)
test_simple_cmd(cmd="B=10;export A=10 B=120 C; C=42; printenv A; printenv B; printenv C;",
                stdout=b"10\n120\n42\n", empty_stderr=True, status=0)

new_section("continuebreak", "continue & break builtins...")
test_simple_cmd("while true; do echo a; break; echo b; done", b"a\n", b"", 0)
test_simple_cmd(
    "while true; do while true; do while true; do break 3; echo a; done; echo b; done; echo c; done", b"", b"", 0)
test_simple_cmd(
    "while true; do while true; do while true; do break 2; echo a; done; echo b; done; echo c; break; done", b"c\n", b"", 0)
test_simple_cmd("touch "+test_dir +
                "/l1; while true; echo u; do if rm "+test_dir+"/l1 2> /dev/null; then echo a; else break; fi; echo b; done", b"u\na\nb\nu\n", b"", 0)
test_simple_cmd("touch "+test_dir +
                "/l1; while true; do echo u; if rm "+test_dir+"/l1 2> /dev/null; then continue 100; else break; fi; echo b; done", b"u\nu\n", b"", 0)
test_simple_cmd("touch "+test_dir +
                "/l1; until false; do echo u; if rm "+test_dir+"/l1 2> /dev/null; then echo a; else break; fi; echo b; done", b"u\na\nb\nu\n", b"", 0)
test_simple_cmd("touch "+test_dir +
                "/l1; until false; do echo u;if rm "+test_dir+"/l1 2> /dev/null; then continue 100; else break; fi; echo b; done", b"u\nu\n", b"", 0)
test_simple_cmd("break;", b"", b"", 0)
test_simple_cmd("break 0;", empty_stdout=True,
                empty_stderr=False, validate_status=lambda s: s != 0)
test_simple_cmd("break abc;", empty_stdout=True,
                empty_stderr=False, validate_status=lambda s: s != 0)

new_section("unset", "unset builtin...")
test_simple_cmd("unset nonexsiting", b"", b"", 0)
test_simple_cmd("unset -badopt nonexitsing", b"",
                empty_stderr=False, validate_status=lambda x: x != 0)
test_simple_cmd("XX=tt;unset XX;echo $XX", b"\n", b"", 0)
test_simple_cmd("XX=tt;unset -f XX;echo $XX", b"tt\n", b"", 0)
test_simple_cmd("XX=tt;unset -v XX;echo $XX", b"\n", b"", 0)
test_simple_cmd("XX=tt;unset -fv XX;echo $XX", b"\n", b"", 0)
test_simple_cmd("unset -fv PWD;echo $PWD", b"\n", b"", 0)
test_simple_cmd("unset AAAA; printenv AAAA", b"", b"", 1, env={"AAAA": "bbb"})
test_simple_cmd("cd /bin; unset PWD; cd ..; pwd", b"/\n", b"", 0)


new_section("andor", "And or")
test_simple_cmd("false && echo yes", b"", b"", 1)
test_simple_cmd("false || echo yes", b"yes\n", b"", 0)
test_simple_cmd("! false && echo yes", b"yes\n", b"", 0)
test_simple_cmd("true || false && echo yes", b"yes\n", b"", 0)
test_simple_cmd("true || (false && echo yes)", b"", b"", 0)
test_simple_cmd("false || false || echo yes", b"yes\n", b"", 0)
test_simple_cmd("false && false || echo yes", b"yes\n", b"", 0)
test_simple_cmd("echo yes && false && echo ya", b"yes\n", b"", 1)
test_simple_cmd("echo yes && true || echo ya", b"yes\n", b"", 0)
test_simple_cmd("echo yes && ! true || echo ya", b"yes\nya\n", b"", 0)
# bad_beginning_and
test_simple_cmd("; && echo yes", b"", empty_stderr=False, status=2)
# bad_beginning_or
test_simple_cmd("; || echo yes", b"", empty_stderr=False, status=2)
# bad_end_or
test_simple_cmd("echo yes || ;", empty_stdout=True,
                empty_stderr=False, status=2)
# bad_ending_and
test_simple_cmd("echo yes && ;", empty_stdout=True,
                empty_stderr=False, status=2)
test_simple_cmd("echo yes &&", empty_stdout=True,
                empty_stderr=False, status=2)
# bad_or_or
test_simple_cmd("echo yes || || echo yes", empty_stdout=True,
                empty_stderr=False, status=2)
# bad_and_and
test_simple_cmd("echo yes && && echo yes", empty_stdout=True,
                empty_stderr=False, status=2)

new_section("pipe", "Pipelines")
test_simple_cmd("uname -a | cut -f 1 -d x", b"Linu\n", b"", 0)
test_simple_cmd(
    " | ".join(["sleep 0.001"] * 100),
    stdout=b"",
    stderr=b"",
    status=0,
    max_exec_time=0.3
)
test_simple_cmd("! uname", b"Linux\n", empty_stderr=True,
                validate_status=lambda s: s != 0)
test_simple_cmd("! { uname; }", b"Linux\n",
                empty_stderr=True, validate_status=lambda s: s != 0)
test_simple_cmd("ls /proc/self/fd | wc -l", b"4\n", b"", 0)
test_simple_cmd("ls /proc/self/fd | cat | cat | wc -l", b"4\n", b"", 0)
test_simple_cmd(
    "uname | cat | ls /proc/self/fd | cat | cat | wc -l", b"4\n", b"", 0)
# bad_pipe
test_simple_cmd("; | cat -", empty_stdout=True,
                empty_stderr=False, status=2)
test_simple_cmd("; | cat -", empty_stdout=True,
                empty_stderr=False, status=2)
# bad_ending_pipe
test_simple_cmd("uname | cat - |", empty_stdout=True,
                empty_stderr=False, status=2)
test_simple_cmd("uname | cat - |;", empty_stdout=True,
                empty_stderr=False, status=2)
# bad_beginning_pipe
test_simple_cmd("| uname", empty_stdout=True,
                empty_stderr=False, status=2)


new_section("compound", "Compound lists")
test_simple_cmd("{ uname; uname; }", b"Linux\nLinux\n", b"", 0)
test_simple_cmd("{ echo yes; }\n{ uname; }", b"yes\nLinux\n", b"", 0)
test_simple_cmd("{ echo a; echo b; } | cat -e", b"a$\nb$\n", b"", 0)
test_simple_cmd("{ uname; uname; } | cat -e", b"Linux$\nLinux$\n", b"", 0)


new_section("redir", "Redirections...")
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
# lessgreat
test_simple_cmd("uname > {}/lolo;wc -l <> {}/lolo; rm {}/lolo".format(test_dir, test_dir, test_dir),
                b"1\n", b"", 0)
# bad_ending_append_redirection
test_simple_cmd("echo toto >>", empty_stdout=True,
                empty_stderr=False, status=2)
# bad_ending_input_redirection
test_simple_cmd("echo toto >", empty_stdout=True, empty_stderr=False, status=2)
# bad_redirections_pipes
test_simple_cmd("echo test > | cat - >", empty_stdout=True,
                empty_stderr=False, status=2)
# bad_ending_lessgreat
test_simple_cmd("echo toto <>", empty_stdout=True,
                empty_stderr=False, status=2)
# bad_ending_lessand
test_simple_cmd("echo toto 2>&", empty_stdout=True,
                empty_stderr=False, status=2)
# bad_ending_greatand
test_simple_cmd("echo toto <&")
test_simple_cmd(
    cmd="if true; then echo yesif; else echo nofi; fi > " +
        test_dir + "/yolo; cat " + test_dir+"/yolo",
    stdout=b"yesif\n",
    empty_stderr=True,
    status=0
)
# bad_redirections_pipes
test_simple_cmd(
    cmd="{ { echo yes; echo no} | echo } | > ",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)


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
test_simple_cmd(f"DEST={test_dir}/atestshi;echo yes > $DEST; echo a; cat {test_dir}/atestshi",
                stdout=b"a\nyes\n", empty_stderr=True, status=0)


new_section("until", "Until loops")
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
# bad_until_no_separator
test_simple_cmd(
    "until cat /tmp/titi do touch /tmp/titi; echo a; done; exit 42",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_until_no_second_separator
test_simple_cmd(
    "until cat /tmp/titi; do touch /tmp/titi; echo a done; exit 42",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_until_no_body
test_simple_cmd(
    "until cat /tmp/titi; do ; done; exit 42",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_until_no_do
test_simple_cmd(
    "until cat /tmp/titi; echo yes; done; exit 42",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_until_no_condition
test_simple_cmd(
    "until ; echo yes; done; exit 42",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_until_no_done
test_simple_cmd(
    "until cat /tmp/titi; do echo yes;",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)

new_section("while", "While loop")
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
# bad_while_no_separator
test_simple_cmd(
    cmd="while true do echo yes; done",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_while_no_second_separator
test_simple_cmd(
    "while true; do echo yes done",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_while_no_condition
test_simple_cmd(
    "while ; do echo yes; done",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_while_no_do
test_simple_cmd(
    "while true; echo yes; done",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
test_simple_cmd(
    "while true; echo yes; done;\n\n",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_while_no_body
test_simple_cmd(
    "while true; do ; done",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
# bad_while_no_done
test_simple_cmd(
    "while true; do echo yes;",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)


new_section("for", "For loop")
test_simple_cmd("for i in a b c d; do echo $i; done", b"a\nb\nc\nd\n", b"", 0)
test_simple_cmd("for i in a b c d; do echo $i; continue; echo ah ah; done",
                b"a\nb\nc\nd\n", b"", 0)
test_simple_cmd("for i in a b c d; do echo $i; break; done",
                b"a\n", b"", 0)
test_simple_cmd("for i in a b c d; do echo $i; exit; done",
                b"a\n", b"", 0)
test_simple_cmd("for i in a b c d; do echo $i; exit 10; done",
                b"a\n", b"", 10)
# bad_for_no_body
test_simple_cmd("for i in a b c d; do; done",
                empty_stdout=True, empty_stderr=False, status=2)
# bad_for_no_separator
test_simple_cmd("for i in a b c d do echo $i; done",
                empty_stdout=True, empty_stderr=False, status=2)
# bad_for_no_do
test_simple_cmd(
    cmd="for i in a b c d; echo $i; done",
    empty_stdout=True,
    empty_stderr=False,
    status=2
)
test_simple_cmd("for i in $NONEXISTING; do echo $i; done",
                empty_stdout=True, empty_stderr=True, status=0)
test_simple_cmd("A=Me;for i in a $A; do echo $i; A=42sh; done",
                stdout=b"a\nMe\n", empty_stderr=True, status=0)
test_simple_cmd("A=John;for i in $A; do echo $A; done",
                stdout=b"John\n", empty_stderr=True, status=0)
# simple_for_no_list_no_semicolon
test_simple_cmd(cmd="for i do echo yes; done",
                empty_stdout=True, empty_stderr=True, status=0)


new_section("env", "Environment variables")
test_simple_cmd("AAAA=ccc; printenv AAAA", b"ccc\n",
                b"", 0, env={"AAAA": "bbb"})
test_simple_cmd("AAAA=ccc; printenv AAAA", b"", b"", 1)
test_simple_cmd("AAAA=ccc; AAAA=b printenv AAAA", b"b\n", b"", 0)
test_simple_cmd("AAAA=b printenv AAAA", b"b\n", b"", 0)
test_simple_cmd("AAAA=b HOME=t printenv AAAA", b"b\n", b"", 0)
test_simple_cmd("AAAA=b HOME=t printenv AAAA", b"b\n",
                b"", 0, env={"AAAA": "other value"})


new_section("alias", "Aliases definition / unalias")
test_simple_cmd("alias;", b"", b"", 0)
test_simple_cmd("alias invalid", b"", empty_stderr=False, status=1)
test_simple_cmd("alias; alias p=pierre", b"", b"", 0)
test_simple_cmd("alias p=pierre;echo; alias", b"\np='pierre'\n", b"", 0)
test_simple_cmd("alias p=pierre;echo; alias p t=toto; echo; alias p t;",
                b"\np='pierre'\n\np='pierre'\nt='toto'\n", b"", 0)
test_simple_cmd("unalias invalid;", b"", empty_stderr=False, status=1)
test_simple_cmd("alias a=1 b=2 c=3 d=4 e=5 f=6 g=7 h=8; unalias -a; alias;",
                b"", empty_stderr=True, status=0)
test_simple_cmd("alias a=1 b=2; unalias b; alias", b"a='1'\n", b"", 0)
test_simple_cmd("alias a=1 b=2; unalias a; alias", b"b='2'\n", b"", 0)
test_simple_cmd("alias a=1 b=2; unalias d a c; alias",
                b"b='2'\n", empty_stderr=False, status=0)
test_simple_cmd("alias a=1 b=2; unalias a b; alias", b"", b"", 0)
test_simple_cmd(
    "XX=abcd;alias;echo;alias XX=fromalias; alias; echo; echo $XX", b"\nXX='fromalias'\n\nabcd\n", b"", 0)
test_simple_cmd("XXX=abcdef;alias XXX=abcdef; unalias XXX; echo XXX",
                b"XXX\n", empty_stderr=True, status=0)


new_section("source", "source and dot builtins")
test_simple_cmd(
    ".",
    empty_stdout=True,
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    ".;uname",
    stdout=b"Linux\n",
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    ". /tmp/nononofile;uname",
    stdout=b"",
    empty_stderr=False,
    status=127,
)


def test_dot_cmd(dotfile: str, cmd: str, stdout, status, stderr=None, empty_stderr=None):
    dfile = gen_tmp_file_name()
    write_to_file(dfile, dotfile)
    test_simple_cmd(cmd.replace("[DFILE]", dfile),
                    stdout=stdout, stderr=stderr, status=status, empty_stderr=empty_stderr)
    os.remove(dfile)


test_dot_cmd(
    dotfile="MYBEST=42sh",
    cmd=". [DFILE];echo $MYBEST",
    stdout=b"42sh\n",
    stderr=b"",
    status=0
)
test_dot_cmd(
    dotfile="MYBEST=42sh\necho toto",
    cmd=". [DFILE];echo $MYBEST",
    stdout=b"toto\n42sh\n",
    stderr=b"",
    status=0
)
test_dot_cmd(
    dotfile="MYBEST=42sh\nexit 42",
    cmd=". [DFILE]",
    stdout=b"",
    stderr=b"",
    status=42
)
test_dot_cmd(
    dotfile="unset MYBEST",
    cmd="MYBEST=42sh;. [DFILE];echo $MYBEST",
    stdout=b"\n",
    empty_stderr=True,
    status=0
)
test_dot_cmd(
    dotfile="",
    cmd=". [DFILE]",
    stdout=b"",
    stderr=b"",
    status=0
)
test_dot_cmd(
    dotfile="if",
    cmd=". [DFILE];echo yes",
    stdout=b"",
    empty_stderr=False,
    status=2
)
test_simple_cmd(
    cmd="PATH={}:/tmp;echo MYBEST=42sh > {}/dummy_cmd; . dummy_cmd; echo $MYBEST".format(
        test_dir, test_dir),
    stdout=b"42sh\n",
    empty_stderr=True,
    status=0,
)

test_simple_cmd(
    cmd="unset PATH;echo MYBEST=42sh > {}/dummy_cmd; . dummy_cmd; echo $MYBEST".format(
        test_dir, test_dir),
    empty_stdout=True,
    empty_stderr=False,
    status=2,
)

new_section("funcdec", "Function declaration")
test_simple_cmd("foo()\n\n\n{\n\n\n\necho yes;\n\n\n\n\n}\n\n\n\n", empty_stdout=True,
                empty_stderr=True, status=0)
test_simple_cmd("foo() { echo yes; }", empty_stdout=True,
                empty_stderr=True, status=0)
test_simple_cmd("foo() { bar() { echo foobar; } echo yes; }", empty_stdout=True,
                empty_stderr=False, status=2)
test_simple_cmd("foo() { bar() { echo foobar; }; echo yes; }", empty_stdout=True,
                empty_stderr=True, status=0)
test_simple_cmd("foo() {\n\n\n\n\n\n\n\nbar() { echo foobar; }\n\n\n\necho yes; }", empty_stdout=True,
                empty_stderr=True, status=0)
test_simple_cmd("foo() { echo hello world; }; foo",
                stdout=b"hello world\n", empty_stderr=True, status=0)
test_simple_cmd("foo() { echo hello world; }; unset -f foo; foo",
                empty_stdout=True, empty_stderr=False, status=127)
test_simple_cmd("foo() { bar() { echo hello world; } }; bar",
                empty_stdout=True, empty_stderr=False, status=127)
test_simple_cmd("foo() { bar() { echo hello world; } }; foo",
                empty_stdout=True, empty_stderr=True, status=0)
test_simple_cmd("foo() { bar() { echo hello world; } }; foo;bar",
                stdout=b"hello world\n", empty_stderr=True, status=0)
test_simple_cmd("foo() { echo $1 $2; }; foo A B",
                stdout=b"A B\n", empty_stderr=True, status=0)
test_simple_cmd("foo() { unset -f foo; echo done; }; foo",
                stdout=b"done\n", empty_stderr=True, status=0)
test_simple_cmd("hon() { exit 42; }; hon",
                empty_stdout=True, empty_stderr=True, status=42)
test_simple_cmd("hon() { break; }; for i in a b c d; do  hon; echo yolo; done",
                stdout=b"yolo\nyolo\nyolo\nyolo\n", empty_stderr=True, status=0)
test_simple_cmd("hihi() { echo 42sh > " + test_dir + "/mytest; }; hihi; cat " + test_dir + "/mytest",
                stdout=b"42sh\n", empty_stderr=True, status=0)
test_simple_cmd("test() { echo $#; }; test; test a; test a b",
                stdout=b"0\n1\n2\n", empty_stderr=True, status=0)


new_section("case", "Case")
test_simple_cmd("case a in esac", empty_stdout=True,
                empty_stderr=True, status=0)
test_simple_cmd(
    cmd="""
case a in
b)
	echo really ???
	;;
a)
	echo well done !!!
	;;
e | f)
	echo again, really ???
	;;

g)
	;;

*)
	echo the ultimate case;
	echo you are a boss;
	;;

esac""",
    stdout=b"well done !!!\n",
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    cmd="""
case 42sh in
b)
	echo really ???
	;;
a)
	echo well done !!!
	;;
e | f)
	echo again, really ???
	;;

g)
	;;

*)
	echo not found
	;;

esac""",
    stdout=b"not found\n",
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    cmd="""
case 42sh in
b)
	echo really ???
	;;
a)
	echo well done !!!
	;;
e | f)
	echo again, really ???
	;;

g)
	;;

esac""",
    empty_stdout=True,
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    cmd="""
case f in
b)
	echo really ???
	;;
a)
	echo well done !!!
	;;
e | f)
	echo nice job!!!
	;;

g)
	;;

*)
	echo the ultimate case;
	;;

esac""",
    stdout=b"nice job!!!\n",
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    cmd="case a in a) echo yes ;; b) echo no ;; esac",
    stdout=b"yes\n",
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    cmd="case d in a) echo yes ;; b) echo no ;; esac",
    empty_stdout=True,
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    cmd="case $A in a) echo yes ;; b) echo no ;; esac",
    empty_stdout=True,
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    cmd="A=b;case $A in a) echo yes ;; b) echo no ;; esac",
    stdout=b"no\n",
    empty_stderr=True,
    status=0
)
test_simple_cmd(
    cmd="A=b;case $A in a) echo yes ;; b)  ;; esac",
    empty_stdout=True,
    empty_stderr=True,
    status=0
)
# bad_case_no_in
test_simple_cmd("case a  a) echo yes ;; b) echo no ;; esac",
                empty_stdout=True, empty_stderr=False, status=2)
# bad_case_no_esac
test_simple_cmd("case a in", empty_stdout=True, empty_stderr=False, status=2)
# bad_case_no_parenthesis
test_simple_cmd("case i in a echo yes ;; esac",
                empty_stdout=True, empty_stderr=False, status=2)


new_section("cmdsubstition", "Commands substitions")
test_simple_cmd("for i in $(seq 1 3); do echo $i; done",
                stdout=b"1\n2\n3\n", empty_stderr=True, status=0)
test_simple_cmd("for i in \"$(seq 1 3)\"; do echo $i; done",
                stdout=b"1 2 3\n", empty_stderr=True, status=0)
test_simple_cmd("echo a$(seq 1 3)b;",
                stdout=b"a1 2 3b\n", empty_stderr=True, status=0)
test_simple_cmd("echo $()", stdout=b"\n", empty_stderr=True, status=0)
test_simple_cmd("echo ``", stdout=b"\n", empty_stderr=True, status=0)
test_simple_cmd("echo a`uname`b", stdout=b"aLinuxb\n",
                empty_stderr=True, status=0)
test_simple_cmd("echo `echo `echo sakebon` `",
                stdout=b"echo sakebon\n", empty_stderr=True, status=0)

new_section("subshell", "Subshell")
test_simple_cmd("(exit 42;); echo $?", stdout=b"42\n",
                empty_stderr=True, status=0)
test_simple_cmd("A=b;(A=c; echo $A);echo $A;",
                stdout=b"c\nb\n", empty_stderr=True, status=0)
test_simple_cmd("( { foo() { echo yes; }; foo; })",
                stdout=b"yes\n", empty_stderr=True, status=0)
test_simple_cmd("( { foo() { echo yes; }; }); foo",
                empty_stdout=True, empty_stderr=False, status=127)
test_simple_cmd("for i in a b; do (break;); echo $i; done",
                stdout=b"a\nb\n", empty_stderr=True, status=0)
test_simple_cmd("a=b; (unset a); echo $a", stdout=b"b\n",
                empty_stderr=True, status=0)
test_simple_cmd("echo $({ echo yes; })",
                stdout=b"yes\n", empty_stderr=True, status=0)
test_simple_cmd("A=a;echo $(A=b;echo $A;)$A;",
                stdout=b"ba\n", empty_stderr=True, status=0)


new_section("invalid_cmd", "Invalid commands")
test_simple_cmd("if", b"", empty_stderr=False,
                validate_status=lambda x: x != 0)
test_simple_cmd("if true; then ", b"", empty_stderr=False,
                validate_status=lambda x: x != 0)
test_simple_cmd("{ { { { { ls; } } } }", b"", empty_stderr=False,
                validate_status=lambda x: x != 0)


new_section("given_tests", "Given tests")
test_simple_cmd(f"echo 'echo \"Hello World\"' > '{test_dir}/var=val'\nchmod +x {test_dir}/var=val\nexport PATH=$PATH:{test_dir}\nvar=val\necho hi${{var}}hi",
                stdout=b"hivalhi\n", empty_stderr=True, status=0)

# Clean environment
new_section(None, "Clean test environment")
test_simple_cmd("rm -rf "+test_dir+"", b"", b"", 0, working_directory="/tmp")

# . end of tests
print("{} / {} tests failed\n".format(number_failed_tests(), total_number_of_tests()))
if number_failed_tests() > 0:
    sys.exit(-1)


# to add
# { echo {; }
