/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2024, Paulus Gandung Prakosa <gandung@infradead.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pd_cmd.h>
#include <pd_core.h>
#include <pd_hdlr.h>
#include <pd_smbuf.h>
#include <pd_tree.h>

#define PD_RESP_MAXLEN 1024

static void pd_hdlr_get(pd_tree_t ***root, pd_smbuf_t **cmds, pd_net_t *net,
                        int fd, pd_smbuf_t *key, pd_smbuf_t *value)
{
    int rsize, tfd;
    char resp[PD_RESP_MAXLEN];
    pd_tree_t *ret, *tmp;

    tfd = net == NULL ? fd : net->accept;
    tmp = **root;
    ret = pd_tree_search_key(tmp, key->buf);

    if (!ret) {
        memset(resp, 0, PD_RESP_MAXLEN);

        rsize = snprintf(
            resp,
            PD_RESP_MAXLEN,
            "(-1) Entry with key '%s' not found.\n",
            key->buf
        );

        write(tfd, resp, rsize);
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

    write(tfd, resp, rsize);
    return;
}

static void pd_hdlr_set(pd_tree_t ***root, pd_smbuf_t **cmds, pd_net_t *net,
                        int fd, pd_smbuf_t *key, pd_smbuf_t *value)
{
    int tfd;
    pd_smbuf_t *tkey, *tval;
    pd_tree_t *tree;
    pd_tree_data_t *data;

    tkey = pd_smbuf_copy(key);

    if (!tkey)
        return;

    tval = pd_smbuf_copy(value);

    if (!tval)
        return;

    tfd  = net == NULL ? fd : net->accept;
    data = pd_tree_data_init(tkey, tval);

    if (!data)
        return;

    tree = pd_tree_init(data);

    if (!tree)
        return;

    pd_tree_insert(*root, tree);
    write(tfd, "+OK\n", 4);
}

static void pd_hdlr_delete(pd_tree_t ***root, pd_smbuf_t **cmds, pd_net_t *net,
                           int fd, pd_smbuf_t *key, pd_smbuf_t *value)
{
    int tfd;
    pd_tree_t *ret, *tmp;

    tfd = !net ? fd : net->accept;
    tmp = **root;
    ret = pd_tree_search_key(tmp, key->buf);

    if (!ret) {
        write(tfd, ":0\n", 3);
        return;
    }

    pd_tree_delete(*root, ret);
    write(tfd, ":1\n", 3);
    return;
}

static void pd_hdlr_invalid(pd_tree_t ***root, pd_smbuf_t **cmds, pd_net_t *net,
                            int fd, pd_smbuf_t *key, pd_smbuf_t *value)
{
    int n, rsize, tfd;
    char resp[PD_RESP_MAXLEN];
    pd_cmd_args_t *pargs;

    tfd   = !net ? fd : net->accept;
    rsize = snprintf(
        resp,
        PD_RESP_MAXLEN,
        "-ERR unknown command `%s`, with args beginning with: ",
        (*cmds)->buf
    );

    write(tfd, resp, rsize);

    pargs = pd_container_of(cmds, pd_cmd_args_t, pcmd);

    for (n = 0; n < pargs->plen; n++) {
        rsize = snprintf(
            resp,
            PD_RESP_MAXLEN,
            n == (pargs->plen - 1) ? "`%s`" : "`%s`, ",
            pargs->paux[n]->buf
        );

        write(tfd, resp, rsize);
    }

    write(tfd, "\n", 1);
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
                      void (*fn)(pd_tree_t ***, pd_smbuf_t **,
                                 pd_net_t *, int, pd_smbuf_t *, pd_smbuf_t *))
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
