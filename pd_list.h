#ifndef __PD_LIST_H
#define __PD_LIST_H

#include <stddef.h>

typedef struct pd_list {
    struct pd_list *prev;
    struct pd_list *next;
} pd_list_t;

#define pd_list_init_val(a, b) { (a), (b) }

#define pd_list(name) \
    pd_list_t name = pd_list_init_val(NULL, NULL)

#define pd_list_initref_ext(name, a, b) do { \
    (name)->prev = (a);
    (name)->next = (b);
} while (0);

static inline void pd_list_add(pd_list_t *root, pd_list_t *node)
{
    node->prev = !root->prev ? root : root->prev;
    node->next = NULL;

    if (node->prev)
        node->prev->next = node;

    root->prev = node;
}

static inline int pd_list_empty(pd_list_t *list)
{
    return list == NULL;
}

#define pd_list_entry(ptr, type, member) \
    pd_container_of(ptr, type, member)

#define pd_list_for_each(elm, list) \
    for ((elm) = (list); (elm) != NULL; (elm) = (elm)->next)

static inline size_t pd_list_count_nodes(pd_list_t *list)
{
    size_t count;
    pd_list_t *elm;

    count = 0;

    pd_list_for_each(elm, list)
        count++;

    return count;
}

#endif /* __PD_LIST_H */
