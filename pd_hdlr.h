#ifndef __PD_HDLR_H
#define __PD_HDLR_H

#include "pd_smbuf.h"
#include "pd_net.h"
#include "pd_tree.h"

typedef enum pd_hdlr_type {
    GET,
    SET,
    DELETE,
    INVALID
} pd_hdlr_type_t;

typedef struct pd_hdlr {
    void (*get)(pd_tree_t **,
                pd_smbuf_t *, pd_net_t *, pd_smbuf_t *, pd_smbuf_t *);
    void (*set)(pd_tree_t **,
                pd_smbuf_t *, pd_net_t *, pd_smbuf_t *, pd_smbuf_t *);
    void (*delete)(pd_tree_t **,
                   pd_smbuf_t *, pd_net_t *, pd_smbuf_t *, pd_smbuf_t *);
    void (*invalid)(pd_tree_t **,
                    pd_smbuf_t *, pd_net_t *, pd_smbuf_t *, pd_smbuf_t *);
} pd_hdlr_t;

pd_hdlr_t *pd_hdlr_alloc(void);

void pd_hdlr_register(pd_hdlr_t **hdlr, pd_hdlr_type_t type,
                      void (*fn)(pd_tree_t **, pd_smbuf_t *,
                                 pd_net_t *, pd_smbuf_t *, pd_smbuf_t *));
void pd_hdlr_build(pd_hdlr_t **hdlr);
void pd_hdlr_release(pd_hdlr_t *hdlr);

#endif /* __PD_HDLR_H */
