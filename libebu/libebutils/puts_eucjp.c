/*                                                            -*- C -*-
 * Copyright (c) 2003-2006  Motoyuki Kasahara
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif

#if defined(HAVE_LIBCHARSET_H)
#include <libcharset.h>
#elif defined(HAVE_LANGINFO_H)
#include <langinfo.h>
#endif

/*
 * Convert `string' from EUC-JP to the current locale encoding, and
 * then write it to `stream'.
 */
int
fputs_eucjp_to_locale(const char *string, FILE *stream)
{
#if defined(HAVE_ICONV_OPEN) || defined(HAVE_LIBICONV_OPEN)
    size_t string_length;
    const char *locale_encoding;
    char *buffer = NULL;
    size_t buffer_size;
    iconv_t cd = (iconv_t)-1;
    const char *in_p;
    char *out_p;
    size_t in_left;
    size_t out_left;
    int fputs_result;

    string_length = strlen(string);

#if defined(HAVE_LOCALE_CHARSET)
    locale_encoding = locale_charset();
#elif defined(HAVE_NL_LANGINFO) && defined(CODESET)
    locale_encoding = nl_langinfo(CODESET);
#else
    locale_encoding = NULL;
#endif
    if (locale_encoding == NULL)
	goto failed;
    cd = iconv_open(locale_encoding, "eucJP");
    if (cd == (iconv_t)-1)
	cd = iconv_open(locale_encoding, "EUC-JP");
    if (cd == (iconv_t)-1)
	goto failed;

    buffer_size = (string_length + 1) * 2;

    for (;;) {
	buffer = malloc(buffer_size);
	if (buffer == NULL)
	    goto failed;

	in_p = string;
	in_left = string_length + 1;
	out_p = buffer;
	out_left = buffer_size;

	if (iconv(cd, (char **)&in_p, &in_left, &out_p, &out_left) != -1)
	    break;
	if (errno == E2BIG) {
	    /*
	     * Reset initial state.
	     * To avoid a bug of iconv() on Solaris 2.6, we set `in_left',
	     * `out_p' and `out_left' to non-NULL values.
	     */
	    in_left = 0;
	    out_p = buffer;
	    out_left = 0;
	    iconv(cd, NULL, &in_left, &out_p, &out_left);

	    free(buffer);
	    buffer = NULL;
	    buffer_size += string_length + 1;

	    continue;
	} else {
	    goto failed;
	}
    }

    iconv_close(cd);
    fputs_result = fputs(buffer, stream);
    free(buffer);

    return fputs_result;

    /*
     * An error occurs...
     */
  failed:
    if (cd != (iconv_t)-1)
	iconv_close(cd);
    if (buffer != NULL)
	free(buffer);
    return fputs(string, stream);

#else /* not (HAVE_ICONV_OPEN || HAVE_LIBICONV_OPEN) */
    return fputs(string, stream);
#endif /* HAVE_ICONV_OPEN || HAVE_LIBICONV_OPEN */
}


/*
 * Convert `string' from EUC-JP to the current locale encoding, and
 * then write it and a newline to `stdout'.
 */
int
puts_eucjp_to_locale(const char *string)
{
    if (fputs_eucjp_to_locale(string, stdout) == EOF)
	return EOF;
    if (fputs_eucjp_to_locale("\n", stdout) == EOF)
	return EOF;

    return 0;
}
