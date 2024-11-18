#include <stddef.h>
#include <string.h>
#include <strings.h>

#include "pd_cmd.h"
#include "pd_smbuf.h"

static pd_cmd_t avail_cmds[] = {
    {"GET", 1},
    {"SET", 2},
    {"DELETE", 1},
    {"DEL", 1}
};

pd_smbuf_t *pd_cmd_parse_cmd(const char *cmd)
{
	char buf[128], *pbuf;
    pd_smbuf_t *cmds;

    pbuf = buf;
    cmds = NULL;

    bzero(buf, 128);

    while (*cmd != '\0') {
        if (*cmd == ' ') {
            if (!cmds)
                cmds = pd_smbuf_init(buf);
            else
                pd_smbuf_append(&cmds, pd_smbuf_init(buf));

            bzero(buf, 128);
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

    if (!cmds)
        cmds = pd_smbuf_init(buf);
    else
        pd_smbuf_append(&cmds, pd_smbuf_init(buf));

    return cmds;
}

int pd_cmd_validate_cmd(pd_smbuf_t *cmds)
{
    int i, found;
    size_t arity;

    found = 0;
    arity = pd_smbuf_nodelen(cmds) - 1;

    for (i = 0; i < sizeof(avail_cmds) / sizeof(pd_cmd_t); i++) {
        if (!strcmp(cmds->buf, avail_cmds[i].cmd) &&
            arity == avail_cmds[i].arity) {
            found = 1;
            break;
        }
    }

    return !found ? -1 : 0;
}
