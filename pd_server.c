#include "pd_event.h"
#include "pd_hdlr.h"
#include "pd_net.h"
#include "pd_server.h"
#include "pd_tree.h"

int pd_run_server(void)
{
    pd_hdlr_t *hdlr;
    pd_net_t *net;
    pd_tree_t *tree;

    tree = NULL;
    net  = pd_net_alloc();

    if (!net)
        goto __fallback;

    hdlr = pd_hdlr_alloc();

    if (!hdlr)
        goto __release_net;

    pd_hdlr_build(&hdlr);
    pd_net_open(&net);

    if (pd_net_bind(&net, "0.0.0.0", 31337) < 0)
        goto __release_net;

    if (pd_net_listen(&net, PD_MAX_BACKLOG) < 0)
        goto __release_net;

    pd_event_loop(net, tree, hdlr);

__release_net:
    pd_net_release(net);

__fallback:
    return -1;
}
