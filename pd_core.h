/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2024, Paulus Gandung Prakosa <gandung@infradead.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PD_CORE_H
#define __PD_CORE_H

#include <stdio.h>
#include <stdlib.h>

#define pd_assert(c, m) do { \
    if (!(c)) { \
        fprintf(stderr, (m)); \
        abort(); \
    } \
} while (0)

#ifndef __builtin_types_compatible_p
#define __pd_same_type(a, b) ({ (1); })
#else
#define __pd_same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#endif

#ifndef __builtin_offsetof
#define pd_offset_of(type, member) ((unsigned long)&(((type *)0)->member))
#else
#define pd_offset_of(type, member) __builtin_offsetof(type, member)
#endif

#define pd_container_of(ptr, type, member) ({                                                         \
        void *__mptr = (void *)(ptr);                            \
        int __c0 = __pd_same_type(*(ptr), ((type *)0)->member);  \
        int __c1 = __pd_same_type(*(ptr), void);                 \
        pd_assert(__c0 || __c1,                                  \
                  "pointer type mismatch is pd_container_of()"); \
        ((type *)(__mptr - pd_offset_of(type, member)));         \
    })

#endif /* __PD_CORE_H */
