#include "symtab.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

int symtab_add(struct symtab *st, char *kvs)
{
    char *p;
    for (p = kvs; *p != '\0' && *p != '='; p++)
        ;
    if (*p == '\0')
        return 1;
    *p = '\0';
    char *key = kvs;
    char *value = p + 1;
    uint32_t hkey = __hash(key);
    size_t i = hkey % st->capacity;

    struct kvpair *kv = zalloc(sizeof(struct kvpair));
    kv->hkey = hkey;
    kv->key = key;
    kv->value = value;
    kv->next = st->data[i];

    if (!st->data[i])
        st->size++;
    st->data[i] = kv;

    if (100 * st->size / st->capacity > 75)
        __symtab_realloc(st, st->capacity * 2);

    return 0;
}

struct kvpair *symtab_lookup(struct symtab *st, const char *key)
{
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

void symtab_print(struct symtab *st)
{
    if (!st)
        return;

    for (size_t i = 0; i < st->capacity; i++)
    {
        struct kvpair *kv = st->data[i];
        printf("[%zu]", i);
        while (kv)
        {
            printf(" --> %s=%s", kv->key, kv->value);
            kv = kv->next;
        }
        printf("\n");
    }
}
