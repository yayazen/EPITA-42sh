/**
 * \file list.h
 * \brief Dynamically sized and expandable list of strings.
 */

#pragma once

/** \brief stores a list of strings */
struct list
{
    char **data;
    unsigned capacity;
    unsigned size;
};

/** \brief create a new list with a given capacity */
struct list *list_new(int capacity);

/** \brief append a new string at the end of the list */
void list_push(struct list *l, char *s);

/** \brief free the list */
void list_free(struct list *l);