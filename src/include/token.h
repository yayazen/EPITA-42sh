#ifndef SH42_TOKEN_H
#define SH42_TOKEN_H

#include <stddef.h>

/* Tokens
 *
 * @brief   token(KEY, STR, TYPE)
 *
 * KEY: enum value of `token`
 *
 * STR: string representation of `token`
 *
 * TYPE: type of `token`
 *      BLANK : non-visible token
 *          * '\0' -> EOF
 *          * '\r', '\n' -> separate commands
 *          * '\t'  '\s' -> discard except if in ("" | '' | HEREDOC | ...)
 *      DEFAULT : misc token subject to exp and glob
 *          * WORD | ASSIGN | HEREDOC
 *      SPECIAL : special meaning token
 *          * hold only if not in ("" | '') else OTHER
 *      KEYWORD : reserved token
 *          * hold only if KEYWORD is the first token of a command else OTHER
 *      BUILTIN : shell builtins
 *
 */

#define TOKEN_STR(T) __token_desc[T].str
#define TOKEN_TYPE(T) __token_desc[T].type
#define TOKEN_LEN(T) __token_desc[T].len

#define TOKEN_LIST_SIZE sizeof(__token_desc) / sizeof(struct token)
#define TOKEN_LIST                                                             \
    TOKEN(AND, "&", SPECIAL)                                                   \
    TOKEN(AND_IF, "&&", SPECIAL)                                               \
    TOKEN(BANG, "!", KEYWORD)                                                  \
    TOKEN(CLOBBER, ">|", SPECIAL)                                              \
    TOKEN(CR, "\r", BLANK)                                                     \
    TOKEN(DBANG, "!!", SPECIAL)                                                \
    TOKEN(DGREAT, ">>", SPECIAL)                                               \
    TOKEN(DLESS, "<<", SPECIAL)                                                \
    TOKEN(DLESSDASH, "<<-", SPECIAL)                                           \
    TOKEN(DPIPE, "||", SPECIAL)                                                \
    TOKEN(DSEMI, ";;", SPECIAL)                                                \
    TOKEN(GREAT, ">", SPECIAL)                                                 \
    TOKEN(GREATAND, ">&", SPECIAL)                                             \
    TOKEN(LBRACE, "{", KEYWORD)                                                \
    TOKEN(LESS, "<", SPECIAL)                                                  \
    TOKEN(LESSAND, "<&", SPECIAL)                                              \
    TOKEN(LESSGREAT, "<>", SPECIAL)                                            \
    TOKEN(LF, "\n", BLANK)                                                     \
    TOKEN(PIPE, "|", SPECIAL)                                                  \
    TOKEN(RBRACE, "}", KEYWORD)                                                \
    TOKEN(SEMICOL, ";", SPECIAL)                                               \
    TOKEN(CASE, "case", KEYWORD)                                               \
    TOKEN(ESAC, "esac", KEYWORD)                                               \
    TOKEN(DO, "do", KEYWORD)                                                   \
    TOKEN(DONE, "done", KEYWORD)                                               \
    TOKEN(ELSE, "else", KEYWORD)                                               \
    TOKEN(ELIF, "elif", KEYWORD)                                               \
    TOKEN(FI, "fi", KEYWORD)                                                   \
    TOKEN(FUNC, "function", KEYWORD)                                           \
    TOKEN(IF, "if", KEYWORD)                                                   \
    TOKEN(IN, "in", KEYWORD)                                                   \
    TOKEN(IONUMBER, "", DEFAULT)                                               \
    TOKEN(UNTIL, "until", KEYWORD)                                             \
    TOKEN(THEN, "then", KEYWORD)                                               \
    TOKEN(WHILE, "while", KEYWORD)                                             \
    TOKEN(ALIAS, "alias", BUILTIN)                                             \
    TOKEN(CD, "cd", BUILTIN)                                                   \
    TOKEN(ECHO, "echo", BUILTIN)                                               \
    TOKEN(EVAL, "eval", BUILTIN)                                               \
    TOKEN(WORD, "", DEFAULT)                                                   \
    TOKEN(ASSIGN_WORD, "", DEFAULT)                                            \
    TOKEN(HEREDOC, "", DEFAULT)

/**
 * @brief   struct of token as described above
 */
struct token
{
    enum
    {
#define TOKEN(K, S, T) K,
        TOKEN_LIST
#undef TOKEN
    } const key;

    enum
    {
        DEFAULT = 0,
        BLANK,
        SPECIAL,
        KEYWORD,
        BUILTIN
    } const type;

    const char *str;
    size_t len;
};

/*
 * @brief   internal token description table
 * @comment should not be used! use TOKEN_[MEMBER](TOKEN) macro instead.
 *          (e.g TOKEN_STR(AND_IF))
 */
static const struct token __token_desc[] = {
#define TOKEN(K, S, T)                                                         \
    [K] = { .key = K, .type = T, .str = S, .len = sizeof(S) - 1 },
    TOKEN_LIST
#undef TOKEN
};

#endif /* ! SH42_TOKEN_H */
