#include "symtab.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/alloc.h>

#define DEFAULT_CAPACITY 16

/* Jenkins's one_at_a_time hash */
static inline uint32_t __hash(const char *key)
{
    uint32_t hkey = 0;

    for (size_t i = 0; key[i]; i++)
    {
        hkey += key[i];
        hkey += (hkey << 10);
        hkey ^= (hkey >> 6);
    }

    hkey += (hkey << 3);
    hkey ^= (hkey >> 11);
    hkey += (hkey << 15);

    return hkey;
}

static struct symtab *__symtab_realloc(struct symtab *st, size_t capacity)
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

static inline void __setkv(struct kvpair *kv, int type, void *value)
{
    // Clean previous content
    if (kv->type == KV_WORD || kv->type == KV_ALIAS)
    {
        if (kv->value.word)
            free(kv->value.word);
    }

    else if (kv->type == KV_FUNC)
    {
        if (kv->value.func)
            rl_exectree_free(kv->value.func);
    }

    // Set new content
    kv->type = type;
    if (kv->type == KV_WORD || kv->type == KV_ALIAS)
    {
        kv->value.word = value;
    }

    else if (kv->type == KV_FUNC)
    {
        kv->value.func = value;
    }
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
            if (kv->type == KV_WORD || kv->type == KV_ALIAS)
                free(kv->value.word);
            else if (kv->type == KV_FUNC)
                rl_exectree_free(kv->value.func);
            free(kv->key);
            free(kv);
            kv = tmp;
        }
    }
}

void symtab_free(struct symtab *st)
{
    if (!st)
        return;
    symtab_clear(st);
    free(st->data);
    free(st);
}

int symtab_add(struct symtab *st, const char *key, int type, void *value)
{
    struct kvpair *kv = symtab_lookup(st, key);

    if (!kv)
    {
        kv = zalloc(sizeof(struct kvpair));
        kv->key = strdup(key);

        uint32_t hkey = __hash(key);
        kv->hkey = hkey;
        size_t i = hkey % st->capacity;
        kv->next = st->data[i];

        if (!st->data[i])
            st->size++;
        st->data[i] = kv;

        if (100 * st->size / st->capacity > 75)
            __symtab_realloc(st, st->capacity * 2);
    }

    __setkv(kv, type, value);

    return 0;
}

struct kvpair *symtab_lookup(struct symtab *st, const char *key)
{
    if (!st)
        return NULL;
    uint32_t hkey = __hash(key);
    struct kvpair *kv = st->data[hkey % st->capacity];

    while (kv)
    {
        if (kv->hkey == hkey && !strcmp(kv->key, key))
            break;
        kv = kv->next;
    }

    return kv;
}