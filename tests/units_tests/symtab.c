#include "symtab.h"

#include <criterion/criterion.h>

Test(symtab, insert)
{
    struct symtab *st = symtab_new();

    cr_assert_eq(NULL, symtab_lookup(st, "NAME"));
    symtab_add(st, "NAME", KV_WORD, strdup("HELLO WORLD"));
    cr_assert_neq(NULL, symtab_lookup(st, "NAME"));

    struct kvpair *pair = symtab_lookup(st, "NAME");
    cr_assert_eq(pair->type, KV_WORD);
    cr_assert_str_eq(pair->value.word, "HELLO WORLD");

    cr_assert_eq(NULL, symtab_lookup(st, "NAME1"));

    symtab_free(st);
}