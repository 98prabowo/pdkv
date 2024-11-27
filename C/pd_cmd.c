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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <pd_cmd.h>
#include <pd_smbuf.h>

static pd_cmd_t avail_cmds[] = {
    {"GET", 1},
    {"SET", 2},
    {"DELETE", 1},
    {"DEL", 1}
};

pd_cmd_args_t *pd_cmd_args_alloc(void)
{
    pd_cmd_args_t *ret;

    ret = calloc(1, sizeof(pd_cmd_args_t));

    if (!ret)
        return NULL;

    ret->plen = 0;
    ret->pcmd = NULL;

    return ret;
}

int pd_cmd_parse_cmd(pd_cmd_args_t **pargs, const char *cmd)
{
    pd_smbuf_t **cpbuf;
    pd_cmd_args_t *targs;
    char buf[129], *pbuf;

    pbuf  = buf;
    targs = *pargs;
    cpbuf = NULL;

    bzero(buf, 129);

    while (*cmd != '\0') {
        if (strlen(buf) == 128) {
            if (!cpbuf) {
                cpbuf = &targs->pcmd;
                targs->plen++;
            }

            if (!(*cpbuf))
                *cpbuf = pd_smbuf_init(buf);
            else
                pd_smbuf_insert_buf(cpbuf, buf);

            bzero(buf, 129);
            pbuf = buf;
        }

        if (*cmd == ' ') {
            if (!cpbuf) {
                cpbuf = &targs->pcmd;
                targs->plen++;
            } else {
                cpbuf = &targs->paux[targs->plen++ - 1];
            }

            if (!(*cpbuf))
                *cpbuf = pd_smbuf_init(buf);
            else
                pd_smbuf_insert_buf(cpbuf, buf);

            bzero(buf, 129);
            pbuf = buf;
            cmd++;
        }

        *pbuf++ = *cmd++;
    }

    pbuf = buf;

    while (1) {
        if (*pbuf == '\n') {
            *pbuf = '\0';
            break;
        }

        pbuf++;
    }

    if (!cpbuf) {
        cpbuf = &targs->pcmd;
        targs->plen++;
    } else {
        cpbuf = &targs->paux[targs->plen++ - 1];
    }

    if (!(*cpbuf))
        *cpbuf = pd_smbuf_init(buf);
    else
        pd_smbuf_insert_buf(cpbuf, buf);

    targs->plen--;
    return 0;
}

int pd_cmd_validate_cmd(pd_cmd_args_t *pargs)
{
    int i, found;
    size_t arity;

    found = 0;
    arity = pargs->plen;

    for (i = 0; i < sizeof(avail_cmds) / sizeof(pd_cmd_t); i++) {
        if (!strcmp(pargs->pcmd->buf, avail_cmds[i].cmd) &&
            arity == avail_cmds[i].arity) {
            found = 1;
            break;
        }
    }

    return !found ? -1 : 0;
}

void pd_cmd_args_release(pd_cmd_args_t **pargs)
{
    pd_cmd_args_release_smbuf(pargs);
    free(*pargs);
}

void pd_cmd_args_release_smbuf(pd_cmd_args_t **pargs)
{
    int i;

    pd_smbuf_release((*pargs)->pcmd);

    for (i = 0; i < (*pargs)->plen; i++) {
        pd_smbuf_release((*pargs)->paux[i]);
        (*pargs)->paux[i] = NULL;
    }

    (*pargs)->pcmd = NULL;
    (*pargs)->plen = 0;
}
