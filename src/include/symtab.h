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

struct symword
{
    char *word;

    /** \brief Only for word tokens. Specify whether these tokens
        should be applied as child environment variable */
    bool exported;
};

union symval
{
    struct symword word;
    struct rl_exectree *func;
};

/** \brief hold a key-value pair */
struct kvpair
{
    int type;

    uint32_t hkey;
    char *key;

    union symval value;

    struct kvpair *next;
};

/** \brief hash table structure */
struct symtab
{
    size_t capacity;
    size_t size;
    struct kvpair **data;
};

/** \brief alloc a new hash table */
struct symtab *symtab_new(void);
/** \brief fill the symtable with environment variables */
int symtab_fill_with_env_vars(struct symtab *st, char **envp);
/** \brief resize allocated memory for symtab */
struct symtab *symtab_realloc(struct symtab *st, size_t capacity);
/** \brief clear hash table and free kvpairs */
void symtab_clear(struct symtab *st);
/** \brief free memory used by a key-value pair */
void kv_free(struct kvpair *pair);
/** \brief free the memory used by st */
void symtab_free(struct symtab *st);
/** \brief add a new kv of the form K=V to a hash table */
struct kvpair *symtab_add(struct symtab *st, const char *key, int type,
                          void *value);
/** \brief remove a k from the hashtable */
int symtab_del(struct symtab *st, struct kvpair *pair);
/** \brief search for a key in the symbol table */
struct kvpair *symtab_lookup(struct symtab *st, const char *key, int type);

/** \brief clone a symtab */
struct symtab *symtab_clone(const struct symtab *st);

/** \brief print current symbols table */
void symtab_print(struct symtab *st);
