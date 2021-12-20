#pragma once

#include <stddef.h>
#include <stdint.h>

#include "rule_exec.h"

enum
{
    KV_WORD,
    KV_FUNC,
    KV_ALIAS,
};

union symval
{
    char *word;
    struct rl_exectree *func;
};

/* hold a key-value pair */
struct kvpair
{
    int type;

    uint32_t hkey;
    char *key;

    union symval value;

    struct kvpair *next;
};

/* hash table structure */
struct symtab
{
    size_t capacity;
    size_t size;
    struct kvpair **data;
};

extern struct symtab *symtab;

/* alloc a new hash table */
struct symtab *symtab_new(void);
/* resize allocated memory for symtab */
struct symtab *symtab_realloc(struct symtab *st, size_t capacity);
/* clear hash table and free kvpairs */
void symtab_clear(struct symtab *st);
/* free the memory used by st */
void symtab_free(struct symtab *st);
/* add a new kv of the form K=V to a hash table */
int symtab_add(struct symtab *st, const char *key, int type, void *value);
/* search for a key in the symbol table */
struct kvpair *symtab_lookup(struct symtab *st, const char *key);

void symtab_print(struct symtab *st);
