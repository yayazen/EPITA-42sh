/**
 * \file symtab.h
 * \brief Symbols table. Contains functions, alias and words
 */

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

/** \brief a single word information */
struct symword
{
    /** \brief The word itself, as OWNING pointer */
    char *word;

    /** \brief Only for word tokens. Specify whether these tokens
        should be applied as child environment variable */
    bool exported;
};

/** \brief unions of different symbols that can be stored in the symtable */
union symval
{
    /** \brief word for alias and word (variables) */
    struct symword word;

    /** \brief execution tree in case of function */
    struct rl_exectree *func;
};

/** \brief hold a key-value pair */
struct kvpair
{
    /** \brief type of value */
    int type;

    /** \brief hashed key */
    uint32_t hkey;

    /** \brief the key itself */
    char *key;

    /** \brief the associated value */
    union symval value;

    /** \brief a pointer on the next key-value pair with the same hash, or NULL
     * if there are none */
    struct kvpair *next;
};

/** \brief hash table structure */
struct symtab
{
    /** \brief The capacity of this symbols table */
    size_t capacity;
    /** \brief Space actually used in this symbols table */
    size_t size;

    /** \brief pointer on key-value pairs storage location */
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
