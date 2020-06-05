/*                                                            -*- C -*-
 * Copyright (c) 2004-2006  Motoyuki Kasahara
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

#define zio_uint1(p) (*(const unsigned char *)(p))

#define zio_uint4(p) ((*(const unsigned char *)(p) << 24) \
        + (*(const unsigned char *)((p) + 1) << 16) \
        + (*(const unsigned char *)((p) + 2) << 8) \
        + (*(const unsigned char *)((p) + 3)))

/*
 * Initialize `speeedup'.
 */
void
ebzip_initialize_zip_speedup(Zip_Speedup *speedup)
{
    speedup->region_count = 0;
}


/*
 * Finalize `speeedup'.
 */
void
ebzip_finalize_zip_speedup(Zip_Speedup *speedup)
{
    speedup->region_count = 0;
}


/*
 * Read HONMON/START file to set `speedup'.
 */
int
ebzip_set_zip_speedup(Zip_Speedup *speedup, const char *file_name,
    Zio_Code zio_code, int index_page)
{
    char buffer[EB_SIZE_PAGE];
    char *buffer_p;
    int start_page;
    Zio zio;
    int index_count;
    int index_code;
    int i;

    zio_initialize(&zio);

    /*
     * Open the file and read it's index page.
     */
    if (zio_open(&zio, file_name, zio_code) < 0) {
	fprintf(stderr, _("%s: failed to open the file: %s\n"),
	    invoked_name, file_name);
	goto failed;
    }
    if (zio_lseek(&zio, ((off_t) index_page - 1) * EB_SIZE_PAGE, SEEK_SET)
	< 0) {
	fprintf(stderr, _("%s: failed to read the file: %s\n"),
	    invoked_name, file_name);
	goto failed;
    }
    if (zio_read(&zio, buffer, EB_SIZE_PAGE) != EB_SIZE_PAGE) {
	fprintf(stderr, _("%s: failed to read the file: %s\n"),
	    invoked_name, file_name);
	goto failed;
    }

    /*
     * Read the index page to get `start_page' location.
     */
    index_count = zio_uint1(buffer + 1);
    buffer_p = buffer + 16;
    for (i = 0; i < index_count; i++) {
	index_code = zio_uint1(buffer_p);
	if ((0x90 <= index_code && index_code <= 0x92)
	    || index_code == 0xc6) {
	    if (EBZIP_MAX_SPEEDUP_REGION_COUNT <= speedup->region_count)
		break;
	    speedup->regions[speedup->region_count].start_page
		= zio_uint4(buffer_p + 2);
	    speedup->region_count++;
	}
	buffer_p += 16;
    }

    /*
     * Read `start_page' to get `end_page' location.
     */
    for (i = 0; i < speedup->region_count; i++) {
	start_page = speedup->regions[i].start_page;
	if (zio_lseek(&zio, ((off_t) start_page - 1) * EB_SIZE_PAGE, SEEK_SET)
	    < 0) {
	    fprintf(stderr, _("%s: failed to read the file: %s\n"),
		invoked_name, file_name);
	    goto failed;
	}
	if (zio_read(&zio, buffer, EB_SIZE_PAGE) != EB_SIZE_PAGE) {
	    fprintf(stderr, _("%s: failed to read the file: %s\n"),
		invoked_name, file_name);
	    goto failed;
	}

	speedup->regions[i].end_page = start_page + zio_uint1(buffer + 3) - 1;
    }

    zio_close(&zio);
    zio_finalize(&zio);
    return 0;

    /*
     * An error occurs...
     */
  failed:
    zio_close(&zio);
    zio_finalize(&zio);
    speedup->region_count = 0;

    return -1;
}


/*
 * Check if `slice' is lain in/over/across a speedup region.
 */
int
ebzip_is_speedup_slice(Zip_Speedup *speedup, int slice, int ebzip_level)
{
    Zip_Speedup_Region *p;
    int start_page;
    int end_page;
    int i;

    start_page = slice * (1 << ebzip_level) + 1;
    end_page = (slice + 1) * (1 << ebzip_level);

    for (i = 0, p = speedup->regions; i < speedup->region_count; i++, p++) {
	/*
	 *          speedup region
	 *        +================+
	 *      o------o
	 *  or  o-----------------------o
	 *     
	 */
	if (start_page <= speedup->regions[i].start_page
	    && speedup->regions[i].start_page <= end_page)
	    return 1;

	/*
	 *          speedup region
	 *        +================+
	 *                    o------o
	 *  or  o--------------------o
	 */
	if (start_page <= speedup->regions[i].end_page
	    && speedup->regions[i].end_page <= end_page)
	    return 1;

	/*
	 *          speedup region
	 *        +================+
	 *            o--------o
	 */
	if (speedup->regions[i].start_page <= start_page
	    && end_page <= speedup->regions[i].end_page)
	    return 1;
    }

    return 0;
}
