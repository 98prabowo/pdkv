#ifndef __PD_TREE_H
#define __PD_TREE_H

#include "pd_smbuf.h"

typedef struct pd_tree_data {
    pd_smbuf_t *key;
    pd_smbuf_t *value;
} pd_tree_data_t;

typedef struct pd_tree {
    size_t hash;
    pd_tree_data_t *data;
    struct pd_tree *parent;
    struct pd_tree *left;
    struct pd_tree *right;
} pd_tree_t;

pd_tree_data_t *pd_tree_data_alloc(void);
pd_tree_data_t *pd_tree_data_init(pd_smbuf_t *key, pd_smbuf_t *value);
void pd_tree_data_release(pd_tree_data_t *data);

pd_tree_t *pd_tree_alloc(void);
pd_tree_t *pd_tree_init(pd_tree_data_t *data);
pd_tree_t *pd_tree_search(pd_tree_t *root, pd_tree_t *node);
pd_tree_t *pd_tree_search_fn(pd_tree_t *root, pd_tree_t *node,
                           int (*compare)(pd_tree_t *, pd_tree_t *));
pd_tree_t *pd_tree_search_key(pd_tree_t *root, const char *buf);
pd_tree_t *pd_tree_min(pd_tree_t *root);
pd_tree_t *pd_tree_max(pd_tree_t *root);

void pd_tree_insert(pd_tree_t **root, pd_tree_t *node);
void pd_tree_delete(pd_tree_t **root, pd_tree_t *node);

#endif /* __PD_TREE_H */
