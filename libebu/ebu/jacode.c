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

/*
 * Convert a string from JIS X 0208 to EUC JP.
 */
void
eb_jisx0208_to_euc(char *out_string, const char *in_string)
{
    unsigned char *out_p = (unsigned char *)out_string;
    const unsigned char *in_p = (unsigned char *)in_string;

    while (*in_p != '\0')
	*out_p++ = ((*in_p++) | 0x80);

    *out_p = '\0';
}


/*
 * Convert a string from shift-JIS to EUC JP.
 * (Shift-JIS is used only in the `LANGUAGE' file.)
 */
void
eb_sjis_to_euc(char *out_string, const char *in_string)
{
    unsigned char *out_p = (unsigned char *)out_string;
    const unsigned char *in_p = (unsigned char *)in_string;
    unsigned char c1, c2;

    for (;;) {
	/*
	 * Break at '\0'.
	 */
	c1 = *in_p++;
	if (c1 == '\0')
	    break;

	if (c1 <= 0x7f) {
	    /*
	     * JIS X 0201 Roman character.
	     */
	    *out_p++ = c1;
	} else if (0xa1 <= c1 && c1 <= 0xdf) {
	    /*
	     * JIS X 0201 Kana.
	     */
	    *out_p++ = ' ';
	} else {
	    /*
	     * JIS X 0208 character.
	     */
	    c2 = *in_p++;
	    if (c2 == 0x00)
		break;

	    if (c2 < 0x9f) {
		if (c1 < 0xdf)
		    c1 = ((c1 - 0x30) << 1) - 1;
		else
		    c1 = ((c1 - 0x70) << 1) - 1;

		if (c2 < 0x7f)
		    c2 += 0x61;
		else
		    c2 += 0x60;
	    } else {
		if (c1 < 0xdf)
		    c1 = (c1 - 0x30) << 1;
		else
		    c1 = (c1 - 0x70) << 1;
		c2 += 0x02;
	    }

	    *out_p++ = c1;
	    *out_p++ = c2;
	}
    }

    *out_p = '\0';
}


