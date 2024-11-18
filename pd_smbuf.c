#include <stdlib.h>
#include <string.h>

#include "pd_smbuf.h"

pd_smbuf_t *pd_smbuf_alloc(void)
{
    pd_smbuf_t *ret;

    ret = calloc(1, sizeof(pd_smbuf_t));

    if (!ret)
        return NULL;

    return ret;
}

static void __pd_smbuf_insert_buf(pd_smbuf_t **root, const char *buf)
{
    pd_smbuf_t *tmp;

    tmp = *root;

    while (1) {
        if (*buf == '\0')
            break;

        if (tmp->buflen == PD_SMBUF_MAXLEN_PER_NODE) {
            pd_smbuf_append(&tmp, pd_smbuf_alloc());
            tmp = tmp->next;
        }

        tmp->buf[tmp->buflen++] = *buf++;
    }

	return;
}

pd_smbuf_t *pd_smbuf_init(const char *buf)
{
    pd_smbuf_t *ret;

    ret = pd_smbuf_alloc();

    if (!ret)
        return NULL;

    __pd_smbuf_insert_buf(&ret, buf);

    return ret;
}

void pd_smbuf_insert_buf(pd_smbuf_t **root, const char *buf)
{
    __pd_smbuf_insert_buf(root, buf);
}

void pd_smbuf_append(pd_smbuf_t **root, pd_smbuf_t *buf)
{
    pd_smbuf_t *tmp;

    tmp = *root;

    while (tmp->next != NULL)
        tmp = tmp->next;

    buf->next = NULL;
    buf->prev = tmp;
    tmp->next = buf;
}

void pd_smbuf_release(pd_smbuf_t *root)
{
    pd_smbuf_release(root->next);
    free(root);
}

size_t pd_smbuf_nodelen(pd_smbuf_t *buf)
{
    size_t ncnt;

    ncnt = 0;

    while (buf != NULL) {
        ncnt++;
        buf = buf->next;
    }

    return ncnt;
}