#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pd_hdlr.h"
#include "pd_smbuf.h"
#include "pd_tree.h"

#define PD_RESP_MAXLEN 1024

static void pd_hdlr_get(pd_tree_t **root, pd_smbuf_t *cmds,
                        pd_net_t *net, pd_smbuf_t *key, pd_smbuf_t *value)
{
	int rsize;
    char resp[PD_RESP_MAXLEN];
    pd_tree_t *ret;

    ret = pd_tree_search_key(*root, key->buf);

    if (!ret) {
        memset(resp, 0, PD_RESP_MAXLEN);

        rsize = snprintf(
            resp,
            PD_RESP_MAXLEN,
            "(-1) Entry with key '%s' not found.\n",
            key->buf
        );

        write(net->accept, resp, rsize);
        return;
    }

    memset(resp, 0, PD_RESP_MAXLEN);

    rsize = snprintf(
        resp,
        PD_RESP_MAXLEN,
        "(%ld) %s\n",
        strlen(ret->data->value->buf),
        ret->data->value->buf
    );

    write(net->accept, resp, rsize);
    return;
}

static void pd_hdlr_set(pd_tree_t **root, pd_smbuf_t *cmds,
                        pd_net_t *net, pd_smbuf_t *key, pd_smbuf_t *value)
{
    pd_tree_t *tree;
    pd_tree_data_t *data;

    data = pd_tree_data_init(key, value);

    if (!data)
        return;

    tree = pd_tree_init(data);

    if (!tree)
        return;

    pd_tree_insert(root, tree);

    write(net->accept, "+OK\n", 4);
}

static void pd_hdlr_delete(pd_tree_t **root, pd_smbuf_t *cmds,
                           pd_net_t *net, pd_smbuf_t *key, pd_smbuf_t *value)
{
    pd_tree_t *ret;

    ret = pd_tree_search_key(*root, key->buf);

    if (!ret) {
        write(net->accept, ":0\n", 3);
        return;
    }

    pd_tree_delete(root, ret);
    write(net->accept, ":1\n", 3);
    return;
}

static void pd_hdlr_invalid(pd_tree_t **root, pd_smbuf_t *cmds,
                            pd_net_t *net, pd_smbuf_t *key, pd_smbuf_t *value)
{
    int rsize;
    char resp[PD_RESP_MAXLEN];
    pd_smbuf_t *cmd;

    rsize = snprintf(
        resp,
        PD_RESP_MAXLEN,
        "-ERR unknown command `%s`, with args beginning with: ",
        cmds->buf
    );

    write(net->accept, resp, rsize);

    pd_smbuf_for_each(cmd, cmds->next) {
        rsize = snprintf(
            resp,
            PD_RESP_MAXLEN,
            !cmd->next ? "`%s`" : "`%s`, ",
            cmd->buf
        );

        write(net->accept, resp, rsize);
    }

    write(net->accept, "\n", 1);
    return;
}

pd_hdlr_t *pd_hdlr_alloc(void)
{
    pd_hdlr_t *ret;

    ret = calloc(1, sizeof(pd_hdlr_t));

    if (!ret)
        return NULL;

    return ret;
}

void pd_hdlr_register(pd_hdlr_t **hdlr, pd_hdlr_type_t type,
                      void (*fn)(pd_tree_t **, pd_smbuf_t *,
                                 pd_net_t *, pd_smbuf_t *, pd_smbuf_t *))
{
    switch (type) {
        case GET:
            (*hdlr)->get = fn;
            break;
        case SET:
            (*hdlr)->set = fn;
            break;
        case DELETE:
            (*hdlr)->delete = fn;
            break;
        case INVALID:
            (*hdlr)->invalid = fn;
            break;
    }
}

void pd_hdlr_build(pd_hdlr_t **hdlr)
{
    pd_hdlr_register(hdlr, GET, pd_hdlr_get);
    pd_hdlr_register(hdlr, SET, pd_hdlr_set);
    pd_hdlr_register(hdlr, DELETE, pd_hdlr_delete);
    pd_hdlr_register(hdlr, INVALID, pd_hdlr_invalid);
}

void pd_hdlr_release(pd_hdlr_t *hdlr)
{
    free(hdlr);
}
