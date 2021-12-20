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

struct vword
{
    char *word;
    int exported;
};

union symval
{
    struct vword word;
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
/* fill the symtable with environment variables */
int symtab_fill_with_env_vars(struct symtab *st, char **envp);
/* resize allocated memory for symtab */
struct symtab *symtab_realloc(struct symtab *st, size_t capacity);
/* clear hash table and free kvpairs */
void symtab_clear(struct symtab *st);
/* free memory used by a key-value pair */
void kv_free(struct kvpair *pair);
/* free the memory used by st */
void symtab_free(struct symtab *st);
/* add a new kv of the form K=V to a hash table */
struct kvpair *symtab_add(struct symtab *st, const char *key, int type,
                          void *value);
/* remove a k from the hashtable */
int symtab_del(struct symtab *st, struct kvpair *pair);
/* search for a key in the symbol table */
struct kvpair *symtab_lookup(struct symtab *st, const char *key, int type);

/* print current symbols table */
void symtab_print(struct symtab *st);
