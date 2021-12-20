#include "symtab.h"

#include <criterion/criterion.h>

Test(symtab, insert)
{
    struct symtab *st = symtab_new();

    cr_assert_eq(NULL, symtab_lookup(st, "NAME", KV_WORD));
    symtab_add(st, "NAME", KV_WORD, strdup("HELLO WORLD"));
    cr_assert_neq(NULL, symtab_lookup(st, "NAME", KV_WORD));

    struct kvpair *pair = symtab_lookup(st, "NAME", KV_WORD);
    cr_assert_eq(pair->type, KV_WORD);
    cr_assert_str_eq(pair->value.word, "HELLO WORLD");

    cr_assert_eq(NULL, symtab_lookup(st, "NAME1", KV_WORD));

    symtab_free(st);
}

Test(symtab, use_different_types)
{
    struct symtab *st = symtab_new();

    cr_assert_eq(NULL, symtab_lookup(st, "NAME", KV_WORD));
    symtab_add(st, "NAME", KV_WORD, strdup("HELLO WORLD"));
    cr_assert_neq(NULL, symtab_lookup(st, "NAME", KV_WORD));
    cr_assert_eq(NULL, symtab_lookup(st, "NAME", KV_ALIAS));

    symtab_free(st);
}

Test(symtab, delete)
{
    struct symtab *st = symtab_new();

    cr_assert_eq(NULL, symtab_lookup(st, "NAME", KV_WORD));
    symtab_add(st, "NAME", KV_WORD, strdup("HELLO WORLD"));
    symtab_add(st, "NAME_OF", KV_WORD, strdup("NAME_OF_L"));
    cr_assert_neq(NULL, symtab_lookup(st, "NAME", KV_WORD));

    for (int i = 0; i < 250; i++)
    {
        char key[100];
        char val[100];
        sprintf(key, "name %d", i);
        sprintf(val, "value %d", i);
        symtab_add(st, key, KV_WORD, strdup(val));
    }

    struct kvpair *pair = symtab_lookup(st, "NAME", KV_WORD);
    cr_assert_eq(pair->type, KV_WORD);
    cr_assert_str_eq(pair->value.word, "HELLO WORLD");

    cr_assert_eq(0, symtab_del(st, pair));

    for (int i = 50; i < 200; i++)
    {
        char key[100];
        sprintf(key, "name %d", i);
        symtab_del(st, symtab_lookup(st, key, KV_WORD));
    }

    cr_assert_eq(NULL, symtab_lookup(st, "NAME", KV_WORD));

    symtab_free(st);
}