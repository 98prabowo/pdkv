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

#ifndef __PD_EVENT_H
#define __PD_EVENT_H

#include <pthread.h>
#include <sys/epoll.h>

#include <pd_cmd.h>
#include <pd_hdlr.h>
#include <pd_net.h>
#include <pd_tree.h>

#define PD_NETEV_POLLFD_SIZE 1024

typedef struct pd_event {
    int ev_fd;
    pd_tree_t *ev_tree;
    pd_cmd_args_t *ev_cmds;
    struct epoll_event ev_curr;
    struct epoll_event ev_list[PD_NETEV_POLLFD_SIZE];

    int (*ev_rfn)(int, pd_tree_t **, pd_hdlr_t *, pd_cmd_args_t **, int *);
    int (*ev_wfn)(int, pd_tree_t **, pd_hdlr_t *, pd_cmd_args_t **, int *);
    int (*ev_efn)(int, pd_tree_t **, pd_hdlr_t *, pd_cmd_args_t **, int *);
} pd_event_t;

pd_event_t *pd_event_alloc(void);
pd_event_t *pd_event_init(pd_net_t *net);

void pd_event_release(pd_event_t *ev);

int pd_event_dispatch(pd_event_t *ev, pd_net_t *net, pd_hdlr_t *hdlr);

#endif /* __PD_EVENT_H */
