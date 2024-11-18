#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pd_net.h"

pd_net_t *pd_net_alloc(void)
{
    pd_net_t *ret;

    ret = calloc(1, sizeof(pd_net_t));

    if (!ret)
        return NULL;

    return ret;
}

void pd_net_open(pd_net_t **net)
{
    int ret;
    pd_net_t *tmp;

    tmp = *net;
    ret = socket(AF_INET, SOCK_STREAM, 0);
    tmp->fd = ret < 0 ? -1 : ret;
}

int pd_net_bind(pd_net_t **net, const char *addr,
                 unsigned short port)
{
    int ret;
    struct sockaddr_in s;
    pd_net_t *tmp;

    tmp = *net;

    bzero(&s, sizeof(struct sockaddr_in));

    s.sin_family = PF_INET;
    s.sin_port = htons(port);
    s.sin_addr.s_addr = !strcmp(addr, "0.0.0.0")
        ? INADDR_ANY
        : inet_addr(addr);

    ret = bind(tmp->fd,
    	       (struct sockaddr *)&s,
    	       sizeof(struct sockaddr));

    return ret < 0 ? -1 : 0;
}

int pd_net_listen(pd_net_t **net, int backlog)
{
    int ret;
    pd_net_t *tmp;

    tmp = *net;
    ret = listen(tmp->fd, backlog);

    return ret < 0 ? -1 : 0;
}

void pd_net_accept(pd_net_t **net)
{
    int ret;
    pd_net_t *tmp;

    tmp = *net;
    ret = accept(tmp->fd, NULL, NULL);

    tmp->accept = ret < 0 ? -1 : ret;
}

int pd_net_close_socket(pd_net_t **net)
{
    int tfd;

    tfd = (*net)->fd;
    (*net)->fd = -1;

    return close(tfd);
}

int pd_net_close_accept(pd_net_t **net)
{
    int tfd;

    tfd = (*net)->accept;
    (*net)->accept = -1;

    return close(tfd);
}

void pd_net_release(pd_net_t *net)
{
    if (net->accept != -1)
        pd_net_close_accept(&net);

    if (net->fd != -1)
        pd_net_close_socket(&net);

    net->rbytes = 0;
    net->wbytes = 0;

    free(net);
}

int pd_net_socket_set_nb(pd_net_t **net)
{
    int ret;
    int flags;

    flags = fcntl((*net)->fd, F_GETFL, 0);

    if (flags < 0)
        return -1;

    ret = fcntl((*net)->fd, F_SETFL, flags | O_NONBLOCK);

    if (ret < 0)
        return -1;

    return 0;
}

int pd_net_accept_set_nb(pd_net_t **net)
{
    int ret;
    int flags;

    flags = fcntl((*net)->accept, F_GETFL, 0);

    if (flags < 0)
        return -1;

    ret = fcntl((*net)->accept, F_SETFL, flags | O_NONBLOCK);

    if (ret < 0)
        return -1;

    return 0;
}

int pd_net_socket_clear_nb(pd_net_t **net)
{
    int ret;
    int flags;

    flags = fcntl((*net)->fd, F_GETFL, 0);

    if (flags < 0)
        return -1;

    ret = fcntl((*net)->fd, F_SETFL, flags & ~O_NONBLOCK);

    if (ret < 0)
        return -1;

    return 0;
}

int pd_net_accept_clear_nb(pd_net_t **net)
{
    int ret;
    int flags;

    flags = fcntl((*net)->accept, F_GETFL, 0);

    if (flags < 0)
        return -1;

    ret = fcntl((*net)->accept, F_SETFL, flags & ~O_NONBLOCK);

    if (ret < 0)
        return -1;

    return 0;
}
