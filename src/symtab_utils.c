#include <assert.h>
#include <string.h>

#include "symtab.h"

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
                   kv->type == KV_FUNC ? "function" : kv->value.word.word);
            kv = kv->next;
        }
        printf("\n");
    }
}

int symtab_fill_with_env_vars(struct symtab *st, char **envp)
{
    assert(st);

    for (char **env = envp; *env != 0; env++)
    {
        char *key = strdup(*env);

        char *val = strchr(key, '=');
        *val = '\0';
        val++;
        symtab_add(st, key, KV_WORD, strdup(val))->value.word.exported = 1;

        free(key);
    }

    return 0;
}

// I built it the easy and un-efficient way... Maybe Yanis will kill me...
struct symtab *symtab_clone(const struct symtab *st)
{
    struct symtab *clone = symtab_new();

    for (size_t i = 0; i < st->capacity; i++)
    {
        struct kvpair *kv = st->data[i];
        while (kv)
        {
            void *data;
            bool exported = false;
            if (kv->type == KV_FUNC)
                data = rl_exectree_clone(kv->value.func);
            else
            {
                exported = kv->value.word.exported;
                data = strdup(kv->value.word.word);
            }

            struct kvpair *kvclone = symtab_add(clone, kv->key, kv->type, data);
            if (exported)
                kvclone->value.word.exported = true;

            kv = kv->next;
        }
    }

    return clone;
}