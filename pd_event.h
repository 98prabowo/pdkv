#ifndef __PD_EVENT_H
#define __PD_EVENT_H

#include "pd_hdlr.h"
#include "pd_net.h"
#include "pd_tree.h"

#define PD_MAX_BACKLOG 4096

int pd_event_loop(pd_net_t *net,
                  pd_tree_t *tree, pd_hdlr_t *hdlr);

#endif /* __PD_EVENT_H */
