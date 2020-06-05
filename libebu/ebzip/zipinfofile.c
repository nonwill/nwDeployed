/*                                                            -*- C -*-
 * Copyright (c) 1998-2006  Motoyuki Kasahara
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

#include "ebzip.h"

#include "samefile.h"
#include "yesno.h"

/*
 * Tricks for gettext.
 */
#ifdef ENABLE_NLS
#define _(string) gettext(string)
#ifdef gettext_noop
#define N_(string) gettext_noop(string)
#else
#define N_(string) (string)
#endif
#else
#define _(string) (string)
#define N_(string) (string)
#endif

/*
 * Unexported functions.
 */
static int ebzip_zipinfo_file_internal(const char *in_file_name,
    Zio_Code in_zio_code, int index_page);


/*
 * Output status of a file `in_file_name'.
 * For START file, use ebzip_zipinfo_start_file() instead.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
int
ebzip_zipinfo_file(const char *in_file_name, Zio_Code in_zio_code)
{
    return ebzip_zipinfo_file_internal(in_file_name, in_zio_code, 0);
}

/*
 * Output status of START file `in_file_name'.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
int
ebzip_zipinfo_start_file(const char *in_file_name, Zio_Code in_zio_code,
    int index_page)
{
    return ebzip_zipinfo_file_internal(in_file_name, in_zio_code, index_page);
}

/*
 * Output status of a file `file_name'.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
static int
ebzip_zipinfo_file_internal(const char *in_file_name, Zio_Code in_zio_code,
    int index_page)
{
    Zio in_zio;
    int in_file = -1;
    struct stat in_status;

    /*
     * Output file name information.
     */
    printf("==> %s <==\n", in_file_name);
    fflush(stdout);

    /*
     * Open the file.
     */
    zio_initialize(&in_zio);
    if (stat(in_file_name, &in_status) == 0 && S_ISREG(in_status.st_mode))
	in_file = zio_open(&in_zio, in_file_name, in_zio_code);

    if (in_file < 0) {
	fprintf(stderr, _("%s: failed to open the file: %s\n"),
	    invoked_name, in_file_name);
	goto failed;
    }
    if (in_zio_code == ZIO_SEBXA) {
	off_t index_location;
	off_t index_base;
	off_t zio_start_location;
	off_t zio_end_location;

	if (get_sebxa_indexes(in_file_name, index_page, &index_location,
	    &index_base, &zio_start_location, &zio_end_location) < 0) {
	    goto failed;
	}
	zio_set_sebxa_mode(&in_zio, index_location, index_base,
	    zio_start_location, zio_end_location);
    }

    /*
     * Close the file.
     */
    zio_close(&in_zio);

    /*
     * Output information.
     */
    if (in_zio.code == ZIO_PLAIN) {
#if defined(PRINTF_LL_MODIFIER)
	printf(_("%llu bytes (not compressed)\n"),
	    (unsigned long long) in_status.st_size);
#elif defined(PRINTF_I64_MODIFIER)
	printf(_("%I64u bytes (not compressed)\n"),
	    (unsigned __int64) in_status.st_size);
#else
	printf(_("%lu bytes (not compressed)\n"),
	    (unsigned long) in_status.st_size);
#endif
    } else {
#if defined(PRINTF_LL_MODIFIER)
	printf(_("%llu -> %llu bytes "),
	    (unsigned long long) in_zio.file_size,
	    (unsigned long long) in_status.st_size);
#elif defined(PRINTF_I64_MODIFIER)
	printf(_("%I64u -> %I64u bytes "),
	    (unsigned __int64) in_zio.file_size,
	    (unsigned __int64) in_status.st_size);
#else
	printf(_("%lu -> %lu bytes "),
	    (unsigned long) in_zio.file_size,
	    (unsigned long) in_status.st_size);
#endif
	if (in_zio.file_size == 0)
	    fputs(_("(empty original file, "), stdout);
	else {
	    printf("(%4.1f%%, ", (double)in_status.st_size * 100.0
		/ (double)in_zio.file_size);
	}
	if (in_zio.code == ZIO_EBZIP1)
	    printf(_("ebzip level %d compression)\n"), in_zio.zip_level);
	else if (in_zio.code == ZIO_SEBXA)
	    printf(_("S-EBXA compression)\n"));
	else
	    printf(_("EPWING compression)\n"));
    }

    fputc('\n', stdout);
    fflush(stdout);

    return 0;

    /*
     * An error occurs...
     */
  failed:
    if (0 <= in_file)
	close(in_file);

    fputc('\n', stderr);
    fflush(stderr);

    return -1;
}
