#include <assert.h>

#include "symtab.h"

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

/* change the value of a key-value pair */
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

int symtab_add(struct symtab *st, const char *key, int type, void *value)
{
    struct kvpair *kv = symtab_lookup(st, key, type);

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
            symtab_realloc(st, st->capacity * 2);
    }

    __setkv(kv, type, value);

    return 0;
}

int symtab_del(struct symtab *st, struct kvpair *pair)
{
    assert(st && pair);

    size_t index = pair->hkey % st->capacity;

    if (st->data[index] == pair)
        st->data[index] = pair->next;

    else
    {
        struct kvpair *prev = st->data[index];
        while (prev->next != pair)
            prev = prev->next;

        prev->next = pair->next;
    }

    st->size -= 1;

    kv_free(pair);
    return 0;
}

struct kvpair *symtab_lookup(struct symtab *st, const char *key, int type)
{
    if (!st)
        return NULL;
    uint32_t hkey = __hash(key);
    struct kvpair *kv = st->data[hkey % st->capacity];

    while (kv)
    {
        if (kv->hkey == hkey && !strcmp(kv->key, key) && kv->type == type)
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
            printf(" --> %s = %s", kv->key,
                   kv->type == KV_FUNC ? "function" : kv->value.word);
            kv = kv->next;
        }
        printf("\n");
    }
}