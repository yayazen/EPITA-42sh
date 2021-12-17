#pragma once

#include <stddef.h>
#include <stdint.h>

/* hold a key-value pair */
struct kvpair
{
    uint32_t hkey;
    char *key;
    char *value;
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
/* clear hash table and free kvpairs */
void symtab_clear(struct symtab *st);
/* free the memory used by st */
void symtab_free(struct symtab *st);
/* add a new kv of the form K=V to a hash table */
int symtab_add(struct symtab *st, char *kvs);
/* search for a key in the symbol table */
struct kvpair *symtab_lookup(struct symtab *st, const char *key);

void symtab_print(struct symtab *st);
