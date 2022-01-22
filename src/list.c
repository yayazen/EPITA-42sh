/**
 * \file list.c
 * \brief Strings list implementation
 */

#include "list.h"

#include <assert.h>

#include "utils/alloc.h"

struct list *list_new(int capacity)
{
    struct list *list = zalloc(sizeof(struct list));
    list->data = zalloc(capacity * sizeof(char *));
    list->capacity = capacity;
    return list;
}

void list_push(struct list *l, char *s)
{
    if (l->size == l->capacity)
    {
        l->capacity *= 2;
        l->data = xrealloc(l->data, l->capacity * sizeof(char *));
        assert(l->data != NULL);
    }

    l->data[l->size] = s;
    l->size++;
}

void list_free(struct list *l)
{
    if (!l)
        return;

    for (unsigned i = 0; i < l->size; i++)
    {
        if (l->data[i] != NULL)
            free(l->data[i]);
    }

    free(l->data);
    free(l);
}