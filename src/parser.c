#include "parser.h"

#include <io/cstream.h>
#include <utils/vec.h>

#include "ast_dot.h"
#include "constants.h"
#include "rule.h"
#include "stdio.h"
#include "token.h"

__attribute__((unused)) static void __dbg_type(struct rl_state *s)
{
    if (!(s->flag & OPT_DEBUG))
        return;

    switch (TOKEN_TYPE(s->token))
    {
    case SPECIAL:
        printf("SPECIAL");
        break;
    case KEYWORD:
        printf("KEYWORD");
        break;
    default:
        printf("DEFAULT");
        break;
    }

#define TOKEN(Key, Str, Type)                                                  \
    if (Key == s->token)                                                       \
        printf(" | " #Key);
#include "token.def"
#undef TOKEN

    if (s->token == T_LF)
        printf(" | ['\\n']\n");
    else
        printf(" | ['%s']\n", vec_cstring(&s->word));
}

__attribute__((unused)) static void __dbg_ast_aux(struct rl_exectree *node)
{
    if (!node)
        return;

    if (node->type == RL_WORD)
    {
        printf("%s", node->attr.word);
    }
    else if (node->type == RL_ASSIGN_WORD)
    {
        printf("\"assign\"(%s)", node->attr.word);
    }
    else if (node->type == RL_REDIRECTION)
    {
        struct attr_redir *redir = &node->attr.redir;
        printf("\"redir\"(%d, %s, %s)", redir->ionumber,
               TOKEN_STR(redir->token), redir->file);
    }
    else
    {
#define RULE(Rl, Str)                                                          \
    if (Rl == node->type)                                                      \
        printf(#Str);
#include "rule.def"
#undef RULE
    }

    if (node->child)
    {
        printf(": { ");
        __dbg_ast_aux(node->child);
        printf(" }");
    }

    if (node->sibling)
    {
        printf(" ");
        __dbg_ast_aux(node->sibling);
    }
}

__attribute__((unused)) static void __dbg_ast(struct rl_exectree *node)
{
    if (!node)
        return;
    printf("DBG << ");
    __dbg_ast_aux(node);
    printf("\n");
}

/* Match anything you give it */
__attribute__((unused)) static int rl_all(struct rl_state *s)
{
    while (rl_accept(s, T_EOF) != true)
    {
        if (s->err)
        {
            perror("rl_input");
            break;
        }

        __dbg_type(s);
        rl_accept(s, s->token);
    }

    return (s->err != NO_ERROR) ? -s->err : true;
}

int parser(struct cstream *cs, int flag, int *exit_status)
{
    struct rl_state s = RL_DEFAULT_STATE;
    vec_init(&s.word);

    // Run in lexer-only mode
    if (flag & OPT_DEBUG)
    {
        s.flag |= flag;
        rl_all(&s);
    }

    else if (rl_input(&s) == true)
    {
        if (flag & OPT_PRINT_AST_DOT)
            ast_dot_print(s.node);
        if (flag & OPT_PRINT_AST)
            __dbg_ast(s.node);

        *exit_status = rl_exec_input(s.node);
    }
    else
    {
        fprintf(stderr, PACKAGE ": rule mismatch or unimplemented\n");
    }

    rl_exectree_free(s.node);
    vec_destroy(&s.word);

    if (s.token == T_EOF)
        return REACHED_EOF;

    return (s.err != NO_ERROR) ? PARSER_ERROR : NO_ERROR;
}
