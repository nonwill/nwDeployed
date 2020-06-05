/*
 * Copyright (c) 1997-2006  Motoyuki Kasahara
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "build-pre.h"
#include "eb.h"
#include "error.h"
#include "build-post.h"

/*
 * Get a BCD (binary coded decimal) packed integer with 2 bytes
 * from an octet stream.
 */
unsigned
eb_bcd2(const char *stream)
{
    unsigned value;
    const unsigned char *s = (const unsigned char *)stream;

    value  = ((*(s    ) >> 4) & 0x0f) * 1000;
    value += ((*(s    )     ) & 0x0f) * 100;
    value += ((*(s + 1) >> 4) & 0x0f) * 10;
    value += ((*(s + 1)     ) & 0x0f);

    return value;
}


/*
 * Get a BCD (binary coded decimal) packed integer with 4 bytes
 * from an octet stream.
 */
unsigned
eb_bcd4(const char *stream)
{
    unsigned value;
    const unsigned char *s = (const unsigned char *)stream;

    value  = ((*(s    ) >> 4) & 0x0f) * 10000000;
    value += ((*(s    )     ) & 0x0f) * 1000000;
    value += ((*(s + 1) >> 4) & 0x0f) * 100000;
    value += ((*(s + 1)     ) & 0x0f) * 10000;
    value += ((*(s + 2) >> 4) & 0x0f) * 1000;
    value += ((*(s + 2)     ) & 0x0f) * 100;
    value += ((*(s + 3) >> 4) & 0x0f) * 10;
    value += ((*(s + 3)     ) & 0x0f);

    return value;
}


/*
 * Get a BCD (binary coded decimal) packed integer with 6 bytes
 * from an octet stream.
 */
unsigned
eb_bcd6(const char *stream)
{
    unsigned value;
    const unsigned char *s = (const unsigned char *)stream;

    value  = ((*(s + 1)     ) & 0x0f);
    value += ((*(s + 2) >> 4) & 0x0f) * 10;
    value += ((*(s + 2)     ) & 0x0f) * 100;
    value += ((*(s + 3) >> 4) & 0x0f) * 1000;
    value += ((*(s + 3)     ) & 0x0f) * 10000;
    value += ((*(s + 4) >> 4) & 0x0f) * 100000;
    value += ((*(s + 4)     ) & 0x0f) * 1000000;
    value += ((*(s + 5) >> 4) & 0x0f) * 10000000;
    value += ((*(s + 5)     ) & 0x0f) * 100000000;

    return value;
}


