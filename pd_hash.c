#include "pd_hash.h"

size_t pd_hash(const char *buf)
{
    size_t hash;
    size_t c;

    while ((c = *buf++) != 0)
        hash = ((hash << 5) + hash) + c;

    return hash;
}
