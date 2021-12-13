#include <criterion/criterion.h>

extern "C"
{
#include <io/cstream.h>

#include "constants.h"
#include "lexer.h"
#include "token.h"
}

#include <cassert>
#include <string>
#include <vector>

// Fix ISO C++17 issue
#define register

class lex_entry
{
public:
    lex_entry(int token, const char *word = nullptr)
        : token_(token)
        , word_(word)
    {}

    bool has_word() const
    {
        return word_ != NULL;
    }

    int token_;
    const char *word_;
};

/**
 * \brief Generate a new parser state to run a test unit
 */
static struct rl_state *get_new_state(const char *s)
{
    struct rl_state *state = (struct rl_state *)zalloc(sizeof(struct rl_state));
    state->cs = cstream_string_create(s);
    state->err = KEYBOARD_INTERRUPT;
    state->ast = NULL;
    state->flag = 1 | PARSER_LINE_START | LEX_CMDSTART;
    state->token = T_EOF;
    vec_init(&state->word);

    return state;
}

/**
 * \brief Free generated test unit
 */
static void free_state(struct rl_state *state)
{
    vec_destroy(&state->word);
    cstream_free(state->cs);
    free(state);
}

/**
 * \brief Check if a given lexer sequence is valid or not
 */
static void check_lexer_seq(const char *s,
                            std::vector<class lex_entry> expected_tokens)
{
    auto state = get_new_state(s);

    for (auto entry : expected_tokens)
    {
        cr_assert_eq(lexer(state), NO_ERROR);
        cr_assert_eq(state->token, entry.token_, "Expected token %s but got %s",
                     TOKEN_STR(entry.token_), TOKEN_STR(state->token));

        if (entry.has_word())
            cr_assert_str_eq(vec_cstring(&state->word), entry.word_);
    }

    free_state(state);
}

Test(lexer, empty)
{
    check_lexer_seq("", std::vector<class lex_entry>{ lex_entry(T_EOF) });
}

Test(lexer, simple_command)
{
    std::vector<class lex_entry> tokens{ lex_entry(T_WORD, "ls"),
                                         lex_entry(T_WORD, "-lah"),
                                         lex_entry(T_EOF) };
    check_lexer_seq("ls -lah", tokens);
}

Test(lexer, two_commands)
{
    std::vector<class lex_entry> tokens{
        lex_entry(T_WORD, "ls"), lex_entry(T_WORD, "-lah"), lex_entry(T_LF),
        lex_entry(T_WORD, "uname"), lex_entry(T_EOF)
    };
    check_lexer_seq("ls -lah\nuname", tokens);
}

Test(lexer, simple_if)
{
    std::vector<class lex_entry> tokens{
        lex_entry(T_IF),           lex_entry(T_WORD, "true"), lex_entry(T_THEN),
        lex_entry(T_WORD, "echo"), lex_entry(T_WORD, "yes"),  lex_entry(T_FI),
        lex_entry(T_EOF)
    };
    check_lexer_seq("if true then echo yes fi", tokens);
}

Test(lexer, if_in_string)
{
    auto state = get_new_state("echo if");

    cr_assert_eq(lexer(state), NO_ERROR);
    cr_assert_eq(state->token, T_WORD);

    state->flag &= ~LEX_CMDSTART;
    cr_assert_eq(lexer(state), NO_ERROR);
    cr_assert_eq(state->token, T_WORD);
    cr_assert_str_eq(vec_cstring(&state->word), "if");

    free_state(state);
}