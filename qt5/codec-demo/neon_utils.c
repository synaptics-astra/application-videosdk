// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "neon_utils.h"

void memcpy_neon(void* dst, const void* src, size_t len)
{
    asm volatile (
                    "1:\n\t"
                    "MOV x0,%0\n\t"
                    "MOV x1,%1\n\t"
                    "2:\n\t"
                    "LDP q0,q1,[x0],#0x20\n\t"
                    "STP q0,q1,[x1],#0x20\n\t"
                    "SUBS %2,%2,#0x20\n\t"
                    "BGT 2b\n\t"
                    : "=&r" (src), "=&r" (dst), "=&r" (len)
                    : "0" (src), "1" (dst), "2" (len)
                    : "q0", "q1", "x0", "x1"
    );
}

