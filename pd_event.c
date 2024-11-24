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

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <pd_cmd.h>
#include <pd_event.h>
#include <pd_net.h>
#include <pd_smbuf.h>

#define PD_NETEV_BUFLEN 4096

static int pd_event_set_cloexec(int efd)
{
    int r;
    int flags;

    do {
        r = fcntl(efd, F_GETFD);
    } while (r == -1 && errno == EINTR);

    if (r == -1 || (r & FD_CLOEXEC))
        return r;

    flags = r | FD_CLOEXEC;

    do {
        r = fcntl(efd, F_SETFD, flags);
    } while (r == -1 && errno == EINTR);

    return r;
}

static int pd_event_read_fn(int fd, pd_tree_t **tree,
                            pd_hdlr_t *hdlr, pd_cmd_args_t **args, int *done)
{
    int ret;
    char buf[PD_NETEV_BUFLEN + 1];

    memset(buf, '\0', PD_NETEV_BUFLEN + 1);

    ret = read(fd, buf, PD_NETEV_BUFLEN);

    if (ret < 0)
        *done = errno != EAGAIN ? 1 : 0;
    else if (!ret)
        *done = 1;

    if (ret <= 0)
        return -1;

    if (!strcmp(buf, "\n"))
        return -1;

    if (pd_cmd_parse_cmd(args, buf) < 0)
        return -1;

    if (pd_cmd_validate_cmd(*args) < 0)
        return -1;

    return 0;
}

static int pd_event_write_fn(int fd, pd_tree_t **tree,
                             pd_hdlr_t *hdlr, pd_cmd_args_t **args, int *done)
{
    if (!strncmp("GET", (*args)->pcmd->buf, 3)) {
        hdlr->get(&tree, &(*args)->pcmd,
                  NULL, fd, (*args)->paux[0], NULL);
        goto __release_smbuf;
    }

    if (!strncmp("SET", (*args)->pcmd->buf, 3)) {
        hdlr->set(&tree, &(*args)->pcmd,
                  NULL, fd, (*args)->paux[0], (*args)->paux[1]);
        goto __release_smbuf;
    }

    if (!strncmp("DEL", (*args)->pcmd->buf, 3) ||
        !strncmp("DELETE", (*args)->pcmd->buf, 6)) {
        hdlr->delete(&tree, &(*args)->pcmd,
                     NULL, fd, (*args)->paux[0], NULL);
        goto __release_smbuf;
    }

__release_smbuf:
    pd_cmd_args_release_smbuf(&(*args));
    return 0;
}

static int pd_event_error_fn(int fd, pd_tree_t **tree,
                             pd_hdlr_t *hdlr, pd_cmd_args_t **args, int *done)
{
    if (!(*args)->pcmd)
        return -1;

    hdlr->invalid(&tree, &(*args)->pcmd, NULL, fd, NULL, NULL);
    pd_cmd_args_release_smbuf(&(*args));
    return 0;
}

pd_event_t *pd_event_alloc(void)
{
    pd_event_t *ret;

    ret = calloc(1, sizeof(pd_event_t));

    if (!ret)
        return NULL;

    return ret;
}

pd_event_t *pd_event_init(pd_net_t *net)
{
    int efd, res;
    pd_event_t *ret;
    pd_cmd_args_t *args;

    ret = pd_event_alloc();

    if (!ret)
        goto __fallback;

    args = pd_cmd_args_alloc();

    if (!args)
        goto __release_event;

    efd = epoll_create(PD_NETEV_POLLFD_SIZE);

    if (efd < 0)
        goto __release_event;

    res = pd_event_set_cloexec(efd);

    if (res < 0)
        goto __release_event;

    ret->ev_fd = efd;
    ret->ev_cmds = args;
    ret->ev_rfn = pd_event_read_fn;
    ret->ev_wfn = pd_event_write_fn;
    ret->ev_efn = pd_event_error_fn;

    ret->ev_curr.data.fd = net->fd;
    ret->ev_curr.events = EPOLLIN;

    res = epoll_ctl(ret->ev_fd,
                    EPOLL_CTL_ADD, net->fd, &ret->ev_curr);

    if (res < 0)
        goto __release_event;

    goto __fallback;

__release_event:
    { pd_event_release(ret); ret = NULL; }

__fallback:
    return ret;
}

void pd_event_release(pd_event_t *ev)
{
    if (ev->ev_fd > 0) {
        close(ev->ev_fd);
        ev->ev_fd = -1;
    }

    ev->ev_curr.events = 0;
    ev->ev_curr.data.ptr = NULL;
    ev->ev_curr.data.fd = -1;
    ev->ev_curr.data.u32 = 0;
    ev->ev_curr.data.u64 = 0;

    ev->ev_rfn = ev->ev_wfn = NULL;
    free(ev);
}

int pd_event_dispatch(pd_event_t *ev, pd_net_t *net, pd_hdlr_t *hdlr)
{
    int i, n;
    int ret, done;

    while (1) {
        n = epoll_wait(ev->ev_fd,
                       ev->ev_list, PD_NETEV_POLLFD_SIZE, -1);

        if (n < 0)
            continue;

        for (i = 0; i < n; i++) {
            if ((ev->ev_list[i].events & EPOLLERR) ||
                (ev->ev_list[i].events & EPOLLHUP) ||
                !(ev->ev_list[i].events & EPOLLIN)) {
                close(ev->ev_list[i].data.fd);
                continue;
            }

            if (ev->ev_list[i].data.fd == net->fd) {
                while (1) {
                    pd_net_accept(&net);

                    if (net->accept < 0)
                        break;

                    if (pd_net_accept_set_nb(&net) < 0)
                        goto __fallback;

                    ev->ev_curr.events = EPOLLIN;
                    ev->ev_curr.data.fd = net->accept;

                    ret = epoll_ctl(ev->ev_fd,
                                    EPOLL_CTL_ADD, net->accept, &ev->ev_curr);

                    if (ret < 0)
                        goto __fallback;
                }
            } else {
                done = 0;

                ret = ev->ev_rfn(ev->ev_list[i].data.fd,
                                 &ev->ev_tree, hdlr, &ev->ev_cmds, &done);

                if (ret < 0)
                    ev->ev_efn(ev->ev_list[i].data.fd,
                               &ev->ev_tree, hdlr, &ev->ev_cmds, &done);

                if (!ret)
                    ev->ev_wfn(ev->ev_list[i].data.fd,
                               &ev->ev_tree, hdlr, &ev->ev_cmds, &done);

                if (done)
                    close(ev->ev_list[i].data.fd);
            }
        }
    }

__fallback:
    return 0;
}
