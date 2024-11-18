#ifndef __PD_CMD_H
#define __PD_CMD_H

#include "pd_smbuf.h"

typedef struct pd_cmd {
    char *cmd;
    size_t arity;
} pd_cmd_t;

pd_smbuf_t *pd_cmd_parse_cmd(const char *cmd);
int pd_cmd_validate_cmd(pd_smbuf_t *cmds);

#endif /* __PD_CMD_H */
