#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "pd_cmd.h"
#include "pd_event.h"
#include "pd_net.h"
#include "pd_smbuf.h"

#define PD_NETEV_BUFLEN 4096

int pd_event_loop(pd_net_t *net,
                  pd_tree_t *tree, pd_hdlr_t *hdlr)
{
    char buf[PD_NETEV_BUFLEN + 1];
    int n, efd, nfds;
    struct epoll_event ev;
    struct epoll_event events[PD_MAX_BACKLOG];
    pd_smbuf_t *sbuf;

    efd = epoll_create1(0);

    if (efd < 0)
        goto __fallback;

    ev.events = EPOLLIN;
    ev.data.fd = net->fd;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, net->fd, &ev) < 0)
        goto __fallback;

    while (1) {
        nfds = epoll_wait(efd, events, PD_MAX_BACKLOG, -1);

        if (nfds < 0)
            continue;

        for (n = 0; n < nfds; n++) {
            if (events[n].data.fd == net->fd) {
                pd_net_accept(&net);

                if (net->accept < 0)
                    continue;

                if (pd_net_accept_set_nb(&net) < 0)
                    continue;

                ev.events = EPOLLIN;
                ev.data.fd = net->accept;

                if (epoll_ctl(efd, EPOLL_CTL_ADD, net->accept, &ev) < 0)
                    continue;

                continue;
            }

            if (events[n].events & EPOLLIN) {
                memset(buf, '\0', PD_NETEV_BUFLEN + 1);

                if (read(net->accept, buf, PD_NETEV_BUFLEN) <= 0)
                    continue;

                if (!strcmp(buf, "\n"))
                    continue;

                sbuf = pd_cmd_parse_cmd(buf);

                if (!sbuf)
                    continue;

                if (pd_cmd_validate_cmd(sbuf) < 0) {
                    hdlr->invalid(&tree, sbuf, net, NULL, NULL);
                    continue;
                }

                ev.events = EPOLLOUT;
                ev.data.fd = events[n].data.fd;

                if (epoll_ctl(efd, EPOLL_CTL_MOD, events[n].data.fd, &ev) < 0)
                    continue;
            }

            if (events[n].events & EPOLLOUT) {
                if (!strncmp("GET", sbuf->buf, 3))
                    hdlr->get(&tree, sbuf, net, sbuf->next, NULL);

                if (!strncmp("SET", sbuf->buf, 3))
                    hdlr->set(&tree, sbuf, net,
                              sbuf->next, sbuf->next->next);

                if (!strncmp("DEL", sbuf->buf, 3) ||
                    !strncmp("DELETE", sbuf->buf, 6))
                    hdlr->delete(&tree, sbuf, net, sbuf->next, NULL);

                ev.events = EPOLLIN;
                ev.data.fd = events[n].data.fd;

                if (epoll_ctl(efd, EPOLL_CTL_MOD, events[n].data.fd, &ev) < 0)
                    continue;
            }
        }
    }

__fallback:
    return -1;
}
