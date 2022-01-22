/**
 * \file symtab_alloc.c
 * \brief Symbols table allocation functions implementation
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/alloc.h>

#include "symtab.h"

#define DEFAULT_CAPACITY 16

struct symtab *symtab_realloc(struct symtab *st, size_t capacity)
{
    if (capacity <= st->capacity)
        return st;

    st->data = xrealloc(st->data, sizeof(struct kvpair *) * capacity);
    memset(st->data + st->capacity, 0,
           sizeof(struct kvpair *) * (capacity - st->capacity));

    for (size_t i = 0; i < st->capacity; i++)
    {
        struct kvpair **kvp = &st->data[i];

        while (*kvp)
        {
            size_t j = (*kvp)->hkey % capacity;

            if (i != j)
            {
                struct kvpair *kv = *kvp;
                *kvp = (*kvp)->next;

                if (!st->data[i] && st->data[j])
                    st->size--;

                kv->next = st->data[j];
                st->data[j] = kv;
            }
            else
                kvp = &(*kvp)->next;
        }
    }

    st->capacity = capacity;
    return st;
}

struct symtab *symtab_new(void)
{
    struct symtab *st = xmalloc(sizeof(struct symtab));
    st->data = zalloc(sizeof(struct kvpair *) * DEFAULT_CAPACITY),
    st->capacity = DEFAULT_CAPACITY;
    st->size = 0;
    return st;
}

void symtab_clear(struct symtab *st)
{
    for (size_t i = 0; i < st->capacity; i++)
    {
        struct kvpair *kv = st->data[i];
        while (kv)
        {
            struct kvpair *tmp = kv->next;
            kv_free(kv);
            kv = tmp;
        }
    }
}

void kv_free(struct kvpair *kv)
{
    if (kv->type == KV_WORD || kv->type == KV_ALIAS)
        free(kv->value.word.word);
    else if (kv->type == KV_FUNC)
        rl_exectree_free(kv->value.func);
    free(kv->key);
    free(kv);
}

void symtab_free(struct symtab *st)
{
    if (!st)
        return;
    symtab_clear(st);
    free(st->data);
    free(st);
}
