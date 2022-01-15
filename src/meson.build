include_dirs += include_directories('include')

subdir('utils')
subdir('io')
bin_sources += files('42sh.c')

all_sources += files(
    'token.c',

    'lexer_dfa.c',
    'lexer.c',

    'parser.c',

    'symtab_alloc.c',
    'symtab_ops.c',
    'symtab_utils.c',
    'symexp.c',

    'list.c',

    'rl_accept.c',
    'rl_expect.c',
    'rl_prefix.c',
    'rl_element.c',
    'rl_redirection.c',
    'rl_input.c',
    'rl_list.c',
    'rl_and_or.c',
    'rl_pipeline.c',
    'rl_cmd.c',
    'rl_simple_cmd.c',
    'rl_shell_cmd.c',
    'rl_compound_list.c',
    'rl_if_clause.c',
    'rl_else_clause.c',
    'rl_while.c',
    'rl_until.c',
    'rl_for.c',
    'rl_do_group.c',
    'rl_fundec.c',
    'rl_case.c',
    'rl_case_clause.c',
    'rl_case_item.c',
    'rl_subshell.c',

    'builtins.c',
    'bi_echo.c',
    'bi_exit.c',
    'bi_cd.c',
    'bi_export.c',
    'bi_continue.c',
    'bi_source.c',
    'bi_unset.c',
    'bi_alias.c',
    'bi_unalias.c',

    'ast_dot.c',
)
