#ifndef __PD_SMBUF_H
#define __PD_SMBUF_H

#include <stddef.h>

#ifndef PD_SMBUF_MAXLEN_PER_NODE
#define PD_SMBUF_MAXLEN_PER_NODE 1024
#endif

typedef struct pd_smbuf {
    char buf[PD_SMBUF_MAXLEN_PER_NODE + 1];
    size_t buflen;
    struct pd_smbuf *prev;
    struct pd_smbuf *next;
} pd_smbuf_t;

#define pd_smbuf_for_each(el, root) \
    for ((el) = (root); (el) != NULL; (el) = (el)->next)

pd_smbuf_t *pd_smbuf_alloc(void);
pd_smbuf_t *pd_smbuf_init(const char *buf);

void pd_smbuf_insert_buf(pd_smbuf_t **root, const char *buf);
void pd_smbuf_append(pd_smbuf_t **root, pd_smbuf_t *buf);
void pd_smbuf_release(pd_smbuf_t *root);

size_t pd_smbuf_nodelen(pd_smbuf_t *buf);

#endif /* __PD_SMBUF_H */
