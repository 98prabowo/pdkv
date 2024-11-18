#ifndef __PD_HASH_H
#define __PD_HASH_H

#include <stddef.h>

#ifndef PD_HASHVAL_INIT
#define PD_HASHVAL_INIT 5381
#endif

size_t pd_hash(const char *buf);

#endif /* __PD_HASH_H */
