#ifndef __PD_NET_H
#define __PD_NET_H

typedef struct pd_net {
    int fd;
    int accept;
    size_t rbytes;
    size_t wbytes;
} pd_net_t;

pd_net_t *pd_net_alloc(void);

void pd_net_open(pd_net_t **net);
void pd_net_accept(pd_net_t **net);
void pd_net_release(pd_net_t *net);

int pd_net_bind(pd_net_t **net, const char *addr,
                 unsigned short port);
int pd_net_listen(pd_net_t **net, int backlog);
int pd_net_close_socket(pd_net_t **net);
int pd_net_close_accept(pd_net_t **net);
int pd_net_socket_set_nb(pd_net_t **net);
int pd_net_accept_set_nb(pd_net_t **net);
int pd_net_socket_clear_nb(pd_net_t **net);
int pd_net_accept_clear_nb(pd_net_t **net);

#endif /* __PD_NET_H */
