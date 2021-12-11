#include "parser.h"

#include <io/cstream.h>
#include <utils/vec.h>

#include "ast_dot.h"
#include "constants.h"
#include "debug.h"
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

__attribute__((unused)) static void __dbg_ast_aux(struct rl_ast *ast)
{
    if (!ast)
        return;

    if (ast->type != RL_WORD)
    {
#define RULE(Rl, Str)                                                          \
    if (Rl == ast->type)                                                       \
        printf(#Str);
#include "rule.def"
#undef RULE
    }
    else
    {
        printf("%s", ast->word);
    }

    if (ast->child)
    {
        printf(": { ");
        __dbg_ast_aux(ast->child);
        printf(" }");
    }

    if (ast->sibling)
    {
        printf(" ");
        __dbg_ast_aux(ast->sibling);
    }
}

__attribute__((unused)) static void __dbg_ast(struct rl_ast *ast)
{
    printf("DBG << ");
    __dbg_ast_aux(ast);
    printf("\n");
}

/* Match anything you give it */
__attribute__((unused)) static int rl_all(struct rl_state *s)
{
    s->flag |= 1;

    while (rl_accept(s, T_EOF, RL_NORULE) != true)
    {
        if (s->err)
        {
            perror("rl_input");
            break;
        }

        __dbg_type(s);

        if (s->token == T_LF || s->token == T_SEMICOL)
            s->flag |= 1;
        else
            s->flag &= ~1;

        rl_accept(s, s->token, RL_NORULE);
    }

    return (s->err != NO_ERROR) ? -s->err : true;
}

int cs_parse(struct cstream *cs, int flag)
{
    struct rl_state s = { .err = KEYBOARD_INTERRUPT,
                          .cs = cs,
                          .ast = NULL,
                          .flag = flag | 1,
                          .token = T_EOF };

    vec_init(&s.word);

    // Run in lexer-only mode
    if (flag & OPT_DEBUG)
    {
        rl_all(&s);
    }

    // Run in parser mode
    else if (rl_input(&s) == true)
    {
        if (flag & OPT_PRINT_AST_DOT)
            ast_dot_print(s.ast);
        else
            rl_exec_input(s.ast);
    }

    // Print ast if requested
    if (flag & OPT_PRINT_AST)
        __dbg_ast(s.ast);

    rl_ast_free(s.ast);
    vec_destroy(&s.word);

    if (s.token == T_EOF)
        return REACHED_EOF;

    return (s.err != NO_ERROR) ? PARSER_ERROR : NO_ERROR;
}
