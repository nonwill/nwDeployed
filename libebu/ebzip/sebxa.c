/*                                                            -*- C -*-
 * Copyright (c) 2001-2006  Motoyuki Kasahara
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

#define zio_uint4(p) ((*(const unsigned char *)(p) << 24) \
        + (*(const unsigned char *)((p) + 1) << 16) \
        + (*(const unsigned char *)((p) + 2) << 8) \
        + (*(const unsigned char *)((p) + 3)))

/*
 * Delete compression information in START file of S-EBXA.
 * Return 0 if succeeds, -1 otherwise.
 */
int
rewrite_sebxa_start(const char *file_name, int index_page)
{
    char buffer[EB_SIZE_PAGE];
    int file = -1;
    int index_count;
    int removed_index_count;
    char *index_in_p;
    char *index_out_p;
    ssize_t done_length;
    ssize_t n;
    int i;

    if (index_page == 0)
	index_page = 1;

    /*
     * Output information.
     */
    if (!ebzip_quiet_flag) {
        fprintf(stderr, _("==> rewrite %s <==\n"), file_name);
        fflush(stderr);
    }

    /*
     * Open the file.
     */
    file = open(file_name, O_RDWR | O_BINARY);
    if (file < 0) {
        fprintf(stderr, _("%s: failed to open the file, %s: %s\n"),
            invoked_name, strerror(errno), file_name);
        goto failed;
    }

    /*
     * Read index page.
     */
    if (lseek(file, ((off_t) index_page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
	fprintf(stderr, _("%s: failed to seek the file, %s: %s\n"),
	    invoked_name, strerror(errno), file_name);
	goto failed;
    }

    done_length = 0;
    while (done_length < EB_SIZE_PAGE) {
	errno = 0;
	n =  read(file, buffer + done_length, EB_SIZE_PAGE - done_length);
	if (n < 0) {
            if (errno == EINTR)
                continue;
	    fprintf(stderr, _("%s: failed to read the file, %s: %s\n"),
		invoked_name, strerror(errno), file_name);
	    goto failed;
	} else if (n == 0) {
	    fprintf(stderr, _("%s: unexpected EOF: %s\n"),
		invoked_name, file_name);
	    goto failed;
	} else {
	    done_length += n;
	}
    }

    /*
     * Remove index 0x21 and 0x22.
     * They has comression information.
     */
    index_count = *(unsigned char *)(buffer + 1);
    removed_index_count = 0;

    index_in_p = buffer + 16;
    index_out_p = buffer + 16;
    for (i = 0; i < index_count; i++, index_in_p += 16) {
	if (*index_in_p == 0x21 || *index_in_p == 0x22) {
	    removed_index_count++;
	} else {
	    if (index_in_p != index_out_p)
		memcpy(index_out_p, index_in_p, 16);
	    index_out_p += 16;
	}
    }
    for (i = 0; i < removed_index_count; i++, index_out_p += 16) {
	memset(index_out_p, 0, 16);
    }

    *(unsigned char *)(buffer + 1) = index_count - removed_index_count;

    /*
     * Write back the index page.
     */
    if (lseek(file, ((off_t) index_page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
	fprintf(stderr, _("%s: failed to seek the file, %s: %s\n"),
	    invoked_name, strerror(errno), file_name);
	goto failed;
    }

    done_length = 0;
    while (done_length < EB_SIZE_PAGE) {
	errno = 0;
	n =  write(file, buffer + done_length, EB_SIZE_PAGE - done_length);
	if (n < 0) {
            if (errno == EINTR)
                continue;
	    fprintf(stderr, _("%s: failed to write the file, %s: %s\n"),
		invoked_name, strerror(errno), file_name);
	    goto failed;
	} else {
	    done_length += n;
	}
    }

    if (!ebzip_quiet_flag) {
#if defined(PRINTF_LL_MODIFIER)
	fprintf(stderr, _("completed (%llu / %llu bytes)\n"),
	    (unsigned long long) done_length,
	    (unsigned long long) done_length);
#elif defined(PRINTF_I64_MODIFIER)
	fprintf(stderr, _("completed (%I64u / %I64u bytes)\n"),
	    (unsigned __int64) done_length,
	    (unsigned __int64) done_length);
#else
	fprintf(stderr, _("completed (%lu / %lu bytes)\n"),
	    (unsigned long) done_length,
	    (unsigned long) done_length);
#endif
	fputc('\n', stderr);
	fflush(stderr);
    }

    close(file);
    return 0;

  failed:
    if (0 <= file)
	close(file);
    fputc('\n', stderr);
    return -1;
}


/*
 * Get compression information (`index_page', `index_location', `index_base',
 * `zio_start_location' and `zio_end_location') in START file.
 * Return 0 if succeeds, -1 otherwise.
 */
int
get_sebxa_indexes(const char *file_name, int index_page, off_t *index_location,
    off_t *index_base, off_t *zio_start_location, off_t *zio_end_location)
{
    char buffer[EB_SIZE_PAGE];
    int file = -1;
    int index_count;
    char *index_p;
    ssize_t done_length;
    ssize_t n;
    int page;
    int page_count;
    int i;

    *index_location     = 0;
    *index_base         = 0;
    *zio_start_location = 0;
    *zio_end_location   = 0;

    if (index_page == 0)
	index_page = 1;

    /*
     * Open the file.
     */
    file = open(file_name, O_RDONLY | O_BINARY);
    if (file < 0) {
        fprintf(stderr, _("%s: failed to open the file, %s: %s\n"),
            invoked_name, strerror(errno), file_name);
        goto failed;
    }

    /*
     * Read index page.
     */
    if (lseek(file, ((off_t) index_page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
	fprintf(stderr, _("%s: failed to seek the file, %s: %s\n"),
	    invoked_name, strerror(errno), file_name);
	goto failed;
    }

    done_length = 0;
    while (done_length < EB_SIZE_PAGE) {
	errno = 0;
	n =  read(file, buffer + done_length, EB_SIZE_PAGE - done_length);
	if (n < 0) {
            if (errno == EINTR)
                continue;
	    fprintf(stderr, _("%s: failed to read the file, %s: %s\n"),
		invoked_name, strerror(errno), file_name);
	    goto failed;
	} else if (n == 0) {
	    fprintf(stderr, _("%s: unexpected EOF: %s\n"),
		invoked_name, file_name);
	    goto failed;
	} else {
	    done_length += n;
	}
    }

    /*
     * Get information in index 0x21 and 0x22.
     * They has comression information.
     */
    index_count = *(unsigned char *)(buffer + 1);

    index_p = buffer + 16;
    for (i = 0; i < index_count; i++, index_p += 16) {
	page       = zio_uint4(index_p + 2);
	page_count = zio_uint4(index_p + 6);

	switch (*index_p) {
	case 0x00:
	  *zio_start_location
	      = ((off_t) page - 1) * EB_SIZE_PAGE;
	  *zio_end_location
	      = ((off_t) page + page_count - 1) * EB_SIZE_PAGE - 1;
	    break;
	case 0x21:
	  *index_base = ((off_t) page - 1) * EB_SIZE_PAGE;
	    break;
	case 0x22:
	  *index_location = ((off_t) page - 1) * EB_SIZE_PAGE;
	    break;
	}
    }

    close(file);
    return 0;

  failed:
    if (0 <= file)
	close(file);
    fputc('\n', stderr);
    return -1;
}
