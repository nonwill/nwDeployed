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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <fcntl.h>

#ifdef ENABLE_PTHREAD
#include <pthread.h>
#endif

#ifdef ENABLE_LIBDEFLATE
#include <libdeflate.h>
#else
#include <zlib.h>
#endif

#include "zio.h"
#ifdef ENABLE_EBNET
#include "ebnet.h"
#endif

/*
 * Flags for open().
 */
#ifndef O_BINARY
#define O_BINARY 0
#endif

/*
 * The maximum length of path name.
 */
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX        MAXPATHLEN
#else /* not MAXPATHLEN */
#define PATH_MAX        1024
#endif /* not MAXPATHLEN */
#endif /* not PATH_MAX */

/*
 * Mutual exclusion lock of Pthreads.
 */
#ifndef ENABLE_PTHREAD
#define pthread_mutex_lock(m)
#define pthread_mutex_unlock(m)
#endif

/*
 * Debug message handler.
 */
#ifdef EB_BUILD_LIBRARY
extern int eb_log_flag;
extern void eb_log(const char *, ...);
#define LOG(x) do {if (eb_log_flag) eb_log x;} while (0)
#else
#define LOG(x)
#endif

/*
 * Get an unsigned value from an octet stream buffer.
 */
#define zio_uint1(p) (*(const unsigned char *)(p))

#define zio_uint2(p) ((*(const unsigned char *)(p) << 8) \
        + (*(const unsigned char *)((p) + 1)))

#define zio_uint3(p) ((*(const unsigned char *)(p) << 16) \
        + (*(const unsigned char *)((p) + 1) << 8) \
        + (*(const unsigned char *)((p) + 2)))

#define zio_uint4(p) (((off_t) *(const unsigned char *)(p) << 24) \
        + (*(const unsigned char *)((p) + 1) << 16) \
        + (*(const unsigned char *)((p) + 2) << 8) \
        + (*(const unsigned char *)((p) + 3)))

#define zio_uint5(p) (((off_t) (*(const unsigned char *)(p)) << 32) \
	+ ((off_t) (*(const unsigned char *)((p) + 1)) << 24) \
	+ (*(const unsigned char *)((p) + 2) << 16) \
	+ (*(const unsigned char *)((p) + 3) << 8) \
	+ (*(const unsigned char *)((p) + 4)))

/*
 * Test whether the path is URL with the `ebnet' scheme.
 */
#define is_ebnet_url(p) \
	(   ((p)[0] == 'E' || (p)[0] == 'e') \
	 && ((p)[1] == 'B' || (p)[1] == 'b') \
	 && ((p)[2] == 'N' || (p)[2] == 'n') \
	 && ((p)[3] == 'E' || (p)[3] == 'e') \
	 && ((p)[4] == 'T' || (p)[4] == 't') \
	 && (p)[5] == ':' && (p)[6] == '/' && (p)[7] == '/')

/*
 * Size of a page (The term `page' means `block' in JIS X 4081).
 */
#define ZIO_SIZE_PAGE			2048

/*
 * Size of a cache buffer.
 * It must be large enough to memory an uncompressed slice.
 *
 * (In EBZIP and EPWING compressions, the size of uncompressed slice
 * is 2048.  In S-EBXA compression, the size is 4096.)
 */
#define ZIO_CACHE_BUFFER_SIZE (ZIO_SIZE_PAGE << ZIO_MAX_EBZIP_LEVEL)

/*
 * NULL Zio ID.
 */
#define ZIO_ID_NONE			-1

/*
 * Buffer for caching uncompressed data.
 */
static char *cache_buffer = NULL;

/*
 * Zio ID which caches data in `cache_buffer'.
 */
static int cache_zio_id = ZIO_ID_NONE;

/*
 * Offset of the beginning of the cached data `cache_buffer'.
 */
static off_t cache_location;

/*
 * Zio object counter.
 */
static int zio_counter = 0;

/*
 * Mutex for cache variables.
 */
#ifdef ENABLE_PTHREAD
static pthread_mutex_t zio_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * Test whether `off_t' represents a large integer.
 */
#define off_t_is_large \
	((((off_t) 1 << 41) + ((off_t) 1 << 40) + 1) % 9999991 == 7852006)

/*
 * Unexported function.
 */
static int zio_reopen(Zio *zio, const char *file_name);
static int zio_open_plain(Zio *zio, const char *file_name);
static int zio_open_ebzip(Zio *zio, const char *file_name);
static int zio_open_epwing(Zio *zio, const char *file_name);
static int zio_open_epwing6(Zio *zio, const char *file_name);
static int zio_make_epwing_huffman_tree(Zio *zio, int leaf_count);
static ssize_t zio_read_ebzip(Zio *zio, char *buffer, size_t length);
static ssize_t zio_read_epwing(Zio *zio, char *buffer, size_t length);
static ssize_t zio_read_sebxa(Zio *zio, char *buffer, size_t length);
static int zio_unzip_slice_ebzip1(Zio *zio, char *out_buffer,
    size_t zipped_slice_size);
inline static int zio_unzip_slice_ebzip1_internal(Zio *zio,
    char *out_buffer, size_t zipped_slice_size);
static int zio_unzip_slice_epwing(Zio *zio, char *out_buffer);
static int zio_unzip_slice_epwing6(Zio *zio, char *out_buffer);
static int zio_unzip_slice_sebxa(Zio *zio, char *out_buffer);
static int zio_open_raw(Zio *zio, const char *file_name);
static void zio_close_raw(Zio *zio);
static off_t zio_lseek_raw(Zio *zio, off_t offset, int whence);
static ssize_t zio_read_raw(Zio *zio, void *buffer, size_t length);


/*
 * Initialize cache buffer.
 */
int
zio_initialize_library(void)
{
    pthread_mutex_lock(&zio_mutex);
    LOG(("in: zio_initialize_library()"));

    /*
     * Allocate memory for cache buffer.
     */
    if (cache_buffer == NULL) {
	cache_buffer = (char *) malloc(ZIO_CACHE_BUFFER_SIZE);
	if (cache_buffer == NULL)
	    goto failed;
    }

    LOG(("out: zio_initialize_library() = %d", 0));
    pthread_mutex_unlock(&zio_mutex);
    return 0;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_initialize_library() = %d", -1));
    pthread_mutex_unlock(&zio_mutex);
    return -1;
}


/*
 * Clear cache buffer.
 */
void
zio_finalize_library(void)
{
    pthread_mutex_lock(&zio_mutex);
    LOG(("in: zio_finalize_library()"));

    if (cache_buffer != NULL)
	free(cache_buffer);
    cache_buffer = NULL;
    cache_zio_id = ZIO_ID_NONE;

    LOG(("out: zio_finalize_library()"));
    pthread_mutex_unlock(&zio_mutex);
}


/*
 * Initialize `zio'.
 */
void
zio_initialize(Zio *zio)
{
    LOG(("in: zio_initialize()"));

    zio->id = -1;
    zio->file = -1;
    zio->huffman_nodes = NULL;
    zio->huffman_root = NULL;
    zio->code = ZIO_INVALID;
    zio->file_size = 0;
    zio->is_ebnet = 0;

    LOG(("out: zio_initialize()"));
}


/*
 * Finalize `zio'.
 */
void
zio_finalize(Zio *zio)
{
    LOG(("in: zio_finalize(zio=%d)", (int)zio->id));

    zio_close(zio);
    if (zio->huffman_nodes != NULL)
	free(zio->huffman_nodes);

    zio->id = -1;
    zio->huffman_nodes = NULL;
    zio->huffman_root = NULL;
    zio->code = ZIO_INVALID;

    LOG(("out: zio_finalize()"));
}


/*
 * Set S-EBXA compression mode.
 */
int
zio_set_sebxa_mode(Zio *zio, off_t index_location, off_t index_base,
    off_t zio_start_location, off_t zio_end_location)
{
    LOG(("in: zio_set_sebxa_mode(zio=%d, index_location=%ld, index_base=%ld, \
zio_start_location=%ld, zio_end_location=%ld)",
	(int)zio->id, (long)index_location, (long)index_base,
	(long)zio_start_location, (long)zio_end_location));

    if (zio->code != ZIO_PLAIN)
	goto failed;

    zio->code = ZIO_SEBXA;
    zio->index_location = index_location;
    zio->index_base = index_base;
    zio->zio_start_location = zio_start_location;
    zio->zio_end_location = zio_end_location;
    zio->file_size = zio_end_location;

    LOG(("out: zio_set_sebxa_mode() = %d", 0));
    return 0;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_set_sebxa_mode() = %d", -1));
    return -1;
}

/*
 * Open `file'.
 */
int
zio_open(Zio *zio, const char *file_name, Zio_Code zio_code)
{
    int result;

    LOG(("in: zio_open(zio=%d, file_name=%s, zio_code=%d)",
	(int)zio->id, file_name, zio_code));

    if (0 <= zio->file) {
	if (zio_code == ZIO_REOPEN) {
	    result = 0;
	    goto succeeded;
	}
	zio_finalize(zio);
	zio_initialize(zio);
    }

    switch (zio_code) {
    case ZIO_REOPEN:
	result = zio_reopen(zio, file_name);
	break;
    case ZIO_PLAIN:
	result = zio_open_plain(zio, file_name);
	break;
    case ZIO_EBZIP1:
	result = zio_open_ebzip(zio, file_name);
	break;
    case ZIO_EPWING:
	result = zio_open_epwing(zio, file_name);
	break;
    case ZIO_EPWING6:
	result = zio_open_epwing6(zio, file_name);
	break;
    case ZIO_SEBXA:
	result = zio_open_plain(zio, file_name);
	break;
    default:
	result = -1;
    }

  succeeded:
    LOG(("out: zio_open() = %d", result));
    return result;
}


/*
 * Reopen a file.
 */
static int
zio_reopen(Zio *zio, const char *file_name)
{
    LOG(("in: zio_reopen(zio=%d, file_name=%s)", (int)zio->id, file_name));

    if (zio->code == ZIO_INVALID)
	goto failed;

    if (zio_open_raw(zio, file_name) < 0) {
	zio->code = ZIO_INVALID;
	goto failed;
    }
    zio->location = 0;

    LOG(("out: zio_reopen() = %d", zio->file));
    return zio->file;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_reopen() = %d", -1));
    return -1;
}


/*
 * Open an non-compressed file.
 */
static int
zio_open_plain(Zio *zio, const char *file_name)
{
    LOG(("in: zio_open_plain(zio=%d, file_name=%s)", (int)zio->id, file_name));

    if (zio_open_raw(zio, file_name) < 0)
	goto failed;

    zio->code = ZIO_PLAIN;
    zio->slice_size = ZIO_SIZE_PAGE;
    zio->file_size = zio_lseek_raw(zio, 0, SEEK_END);
    if (zio->file_size < 0 || zio_lseek_raw(zio, 0, SEEK_SET) < 0)
	goto failed;

    /*
     * Assign ID.
     */
    pthread_mutex_lock(&zio_mutex);
    zio->id = zio_counter++;
    pthread_mutex_unlock(&zio_mutex);

    LOG(("out: zio_open_plain(zio=%d) = %d", (int)zio->id, zio->file));
    return zio->file;

    /*
     * An error occurs...
     */
  failed:
    if (0 <= zio->file)
	zio_close_raw(zio);
    zio->file = -1;
    zio->code = ZIO_INVALID;

    LOG(("out: zio_open_plain() = %d", -1));
    return -1;
}


/*
 * Open an EBZIP compression file.
 */
static int
zio_open_ebzip(Zio *zio, const char *file_name)
{
    char header[ZIO_SIZE_EBZIP_HEADER];
    int ebzip_mode;

    LOG(("in: zio_open_ebzip(zio=%d, file_name=%s)", (int)zio->id, file_name));

    /*
     * Try to open a `*.ebz' file.
     */
    if (zio_open_raw(zio, file_name) < 0)
	goto failed;

    /*
     * Read header part of the ebzip'ped file.
     */
    if (zio_read_raw(zio, header, ZIO_SIZE_EBZIP_HEADER)
	!= ZIO_SIZE_EBZIP_HEADER)
	goto failed;
    ebzip_mode = zio_uint1(header + 5) >> 4;
    zio->code = ZIO_EBZIP1;
    zio->zip_level = zio_uint1(header + 5) & 0x0f;
    zio->slice_size = ZIO_SIZE_PAGE << zio->zip_level;
    zio->file_size = zio_uint5(header +  9);
    zio->crc = zio_uint4(header + 14);
    zio->mtime = zio_uint4(header + 18);
    zio->location = 0;

    if (zio->file_size      < (off_t) 1 << 16)
	zio->index_width = 2;
    else if (zio->file_size < (off_t) 1 << 24)
	zio->index_width = 3;
    else if (zio->file_size < (off_t) 1 << 32 || !off_t_is_large)
	zio->index_width = 4;
    else
	zio->index_width = 5;

    /*
     * Check zio header information.
     */
    if (memcmp(header, "EBZip", 5) != 0
	|| ZIO_SIZE_PAGE << ZIO_MAX_EBZIP_LEVEL < zio->slice_size)
	goto failed;

    if (off_t_is_large) {
	if (ebzip_mode != 1 && ebzip_mode != 2)
	    goto failed;
    } else {
	if (ebzip_mode != 1)
	    goto failed;
    }

    /*
     * Assign ID.
     */
    pthread_mutex_lock(&zio_mutex);
    zio->id = zio_counter++;
    pthread_mutex_unlock(&zio_mutex);

    LOG(("out: zio_open_ebzip(zio=%d) = %d", (int)zio->id, zio->file));
    return zio->file;

    /*
     * An error occurs...
     */
  failed:
    if (0 <= zio->file)
	zio_close_raw(zio);
    zio->file = -1;
    zio->code = ZIO_INVALID;
    LOG(("out: zio_open_ebzip() = %d", -1));
    return -1;
}


/*
 * The buffer size must be 512 bytes, the number of 8 bit nodes.
 */
#define ZIO_EPWING_BUFFER_SIZE 512

/*
 * Open an EPWING compression file.
 */
static int
zio_open_epwing(Zio *zio, const char *file_name)
{
    int leaf16_count;
    int leaf_count;
    char buffer[ZIO_EPWING_BUFFER_SIZE];
    char *buffer_p;
    ssize_t read_length;
    Zio_Huffman_Node *tail_node_p;
    int i;

    LOG(("in: zio_open_epwing(zio=%d, file_name=%s)", (int)zio->id,
	file_name));

    zio->code = ZIO_EPWING;
    zio->huffman_nodes = NULL;

    /*
     * Open `HONMON2'.
     */
    if (zio_open_raw(zio, file_name) < 0)
	goto failed;

    /*
     * Read a header of `HONMON2' (32 bytes).
     * When `frequencies_length' is shorter than 512, we assumes the
     * file is broken.
     */
    if (zio_read_raw(zio, buffer, 32) != 32)
	goto failed;
    zio->location = 0;
    zio->slice_size = ZIO_SIZE_PAGE;
    zio->index_location = zio_uint4(buffer);
    zio->index_length = zio_uint4(buffer + 4);
    zio->frequencies_location = zio_uint4(buffer + 8);
    zio->frequencies_length = zio_uint4(buffer + 12);
    leaf16_count = (zio->frequencies_length - (256 * 2)) / 4;
    leaf_count = leaf16_count + 256 + 1;
    if (zio->index_length < 36 || zio->frequencies_length < 512)
	goto failed;

    /*
     * Check for the length of an uncompressed file.
     *
     * If the index of the non-first page in the last index group
     * is 0x0000, we assumes the data corresponding with the index
     * doesn't exist.
     */
    if (zio_lseek_raw(zio, zio->index_location
	    + ((off_t) zio->index_length - 36) / 36 * 36, SEEK_SET) < 0)
	goto failed;
    if (zio_read_raw(zio, buffer, 36) != 36)
	goto failed;
    zio->file_size = ((off_t) zio->index_length / 36) * (ZIO_SIZE_PAGE * 16);
    for (i = 1, buffer_p = buffer + 4 + 2; i < 16; i++, buffer_p += 2) {
	if (zio_uint2(buffer_p) == 0)
	    break;
    }
    zio->file_size -= ZIO_SIZE_PAGE * (16 - i);

    /*
     * Allocate memory for huffman nodes.
     */
    zio->huffman_nodes = (Zio_Huffman_Node *) malloc(sizeof(Zio_Huffman_Node)
	* leaf_count * 2);
    if (zio->huffman_nodes == NULL)
	goto failed;
    tail_node_p = zio->huffman_nodes;

    /*
     * Make leafs for 16bit character.
     */
    read_length = ZIO_EPWING_BUFFER_SIZE - (ZIO_EPWING_BUFFER_SIZE % 4);
    if (zio_lseek_raw(zio, zio->frequencies_location, SEEK_SET) < 0)
	goto failed;
    if (zio_read_raw(zio, buffer, read_length) != read_length)
	goto failed;

    buffer_p = buffer;
    for (i = 0; i < leaf16_count; i++) {
	if (buffer + read_length <= buffer_p) {
	    if (zio_read_raw(zio, buffer, read_length) != read_length)
		goto failed;
	    buffer_p = buffer;
	}
	tail_node_p->type = ZIO_HUFFMAN_NODE_LEAF16;
	tail_node_p->value = zio_uint2(buffer_p);
	tail_node_p->frequency = zio_uint2(buffer_p + 2);
	tail_node_p->left = NULL;
	tail_node_p->right = NULL;
	buffer_p += 4;
	tail_node_p++;
    }

    /*
     * Make leafs for 8bit character.
     */
    if (zio_lseek_raw(zio, zio->frequencies_location + leaf16_count * 4,
	SEEK_SET) < 0)
	goto failed;
    if (zio_read_raw(zio, buffer, 512) != 512)
	goto failed;

    buffer_p = buffer;
    for (i = 0; i < 256; i++) {
	tail_node_p->type = ZIO_HUFFMAN_NODE_LEAF8;
	tail_node_p->value = i;
	tail_node_p->frequency = zio_uint2(buffer_p);
	tail_node_p->left = NULL;
	tail_node_p->right = NULL;
	buffer_p += 2;
	tail_node_p++;
    }

    /*
     * Make a leaf for the end-of-page character.
     */
    tail_node_p->type = ZIO_HUFFMAN_NODE_EOF;
    tail_node_p->value = 256;
    tail_node_p->frequency = 1;
    tail_node_p++;

    /*
     * Make a huffman tree.
     */
    if (zio_make_epwing_huffman_tree(zio, leaf_count) < 0)
	goto failed;

    /*
     * Assign ID.
     */
    pthread_mutex_lock(&zio_mutex);
    zio->id = zio_counter++;
    pthread_mutex_unlock(&zio_mutex);

    LOG(("out: zio_open_epwing(zio=%d) = %d", (int)zio->id, zio->file));
    return zio->file;

    /*
     * An error occurs...
     */
  failed:
    if (0 <= zio->file)
	zio_close_raw(zio);
    if (zio->huffman_nodes != NULL)
	free(zio->huffman_nodes);
    zio->file = -1;
    zio->huffman_nodes = NULL;
    zio->huffman_root = NULL;
    zio->code = ZIO_INVALID;

    LOG(("out: zio_open_epwing() = %d", -1));
    return -1;
}


/*
 * Open an EPWING compression file.
 */
static int
zio_open_epwing6(Zio *zio, const char *file_name)
{
    int leaf32_count;
    int leaf16_count;
    int leaf_count;
    char buffer[ZIO_EPWING_BUFFER_SIZE];
    char *buffer_p;
    ssize_t read_length;
    Zio_Huffman_Node *tail_node_p;
    int i;

    LOG(("in: zio_open_epwing6(zio=%d, file_name=%s)", (int)zio->id,
	file_name));

    zio->code = ZIO_EPWING6;
    zio->huffman_nodes = NULL;

    /*
     * Open `HONMON2'.
     */
    if (zio_open_raw(zio, file_name) < 0)
	goto failed;

    /*
     * Read a header of `HONMON2' (48 bytes).
     * When `frequencies_length' is shorter than 512, we assumes the
     * file is broken.
     */
    if (zio_read_raw(zio, buffer, 48) != 48)
	goto failed;
    zio->location = 0;
    zio->slice_size = ZIO_SIZE_PAGE;
    zio->index_location = zio_uint4(buffer);
    zio->index_length = zio_uint4(buffer + 4);
    zio->frequencies_location = zio_uint4(buffer + 8);
    zio->frequencies_length = zio_uint4(buffer + 12);
    leaf16_count = 0x400;
    leaf32_count = (zio->frequencies_length - (leaf16_count * 4) - (256 * 2))
	/ 6;
    leaf_count = leaf32_count + leaf16_count + 256 + 1;
    if (zio->index_length < 36 || zio->frequencies_length < 512)
	goto failed;

    /*
     * Check for the length of an uncompressed file.
     *
     * If the index of the non-first page in the last index group
     * is 0x0000, we assumes the data corresponding with the index
     * doesn't exist.
     */
    if (zio_lseek_raw(zio, zio->index_location
	+ ((off_t) zio->index_length - 36) / 36 * 36, SEEK_SET) < 0)
	goto failed;
    if (zio_read_raw(zio, buffer, 36) != 36)
	goto failed;
    zio->file_size = ((off_t) zio->index_length / 36) * (ZIO_SIZE_PAGE * 16);
    for (i = 1, buffer_p = buffer + 4 + 2; i < 16; i++, buffer_p += 2) {
	if (zio_uint2(buffer_p) == 0)
	    break;
    }
    zio->file_size -= ZIO_SIZE_PAGE * (16 - i);

    /*
     * Allocate memory for huffman nodes.
     */
    zio->huffman_nodes = (Zio_Huffman_Node *) malloc(sizeof(Zio_Huffman_Node)
	* leaf_count * 2);
    if (zio->huffman_nodes == NULL)
	goto failed;
    tail_node_p = zio->huffman_nodes;

    /*
     * Make leafs for 32bit character.
     */
    read_length = ZIO_EPWING_BUFFER_SIZE - (ZIO_EPWING_BUFFER_SIZE % 6);
    if (zio_lseek_raw(zio, zio->frequencies_location, SEEK_SET) < 0)
	goto failed;
    if (zio_read_raw(zio, buffer, read_length) != read_length)
	goto failed;

    buffer_p = buffer;
    for (i = 0; i < leaf32_count; i++) {
	if (buffer + read_length <= buffer_p) {
	    if (zio_read_raw(zio, buffer, read_length) != read_length)
		goto failed;
	    buffer_p = buffer;
	}
	tail_node_p->type = ZIO_HUFFMAN_NODE_LEAF32;
	tail_node_p->value = zio_uint4(buffer_p);
	tail_node_p->frequency = zio_uint2(buffer_p + 4);
	tail_node_p->left = NULL;
	tail_node_p->right = NULL;
	buffer_p += 6;
	tail_node_p++;
    }

    /*
     * Make leafs for 16bit character.
     */
    read_length = ZIO_EPWING_BUFFER_SIZE - (ZIO_EPWING_BUFFER_SIZE % 4);
    if (zio_lseek_raw(zio, zio->frequencies_location + leaf32_count * 6,
	SEEK_SET) < 0)
	goto failed;
    if (zio_read_raw(zio, buffer, read_length) != read_length)
	goto failed;

    buffer_p = buffer;
    for (i = 0; i < leaf16_count; i++) {
	if (buffer + read_length <= buffer_p) {
	    if (zio_read_raw(zio, buffer, read_length) != read_length)
		goto failed;
	    buffer_p = buffer;
	}
	tail_node_p->type = ZIO_HUFFMAN_NODE_LEAF16;
	tail_node_p->value = zio_uint2(buffer_p);
	tail_node_p->frequency = zio_uint2(buffer_p + 2);
	tail_node_p->left = NULL;
	tail_node_p->right = NULL;
	buffer_p += 4;
	tail_node_p++;
    }

    /*
     * Make leafs for 8bit character.
     */
    if (zio_lseek_raw(zio, zio->frequencies_location + leaf32_count * 6
	+ leaf16_count * 4, SEEK_SET) < 0)
	goto failed;
    if (zio_read_raw(zio, buffer, 512) != 512)
	goto failed;

    buffer_p = buffer;
    for (i = 0; i < 256; i++) {
	tail_node_p->type = ZIO_HUFFMAN_NODE_LEAF8;
	tail_node_p->value = i;
	tail_node_p->frequency = zio_uint2(buffer_p);
	tail_node_p->left = NULL;
	tail_node_p->right = NULL;
	buffer_p += 2;
	tail_node_p++;
    }

    /*
     * Make a leaf for the end-of-page character.
     */
    tail_node_p->type = ZIO_HUFFMAN_NODE_EOF;
    tail_node_p->value = 256;
    tail_node_p->frequency = 1;
    tail_node_p++;

    /*
     * Make a huffman tree.
     */
    if (zio_make_epwing_huffman_tree(zio, leaf_count) < 0)
	goto failed;

    /*
     * Assign ID.
     */
    pthread_mutex_lock(&zio_mutex);
    zio->id = zio_counter++;
    pthread_mutex_unlock(&zio_mutex);

    LOG(("out: zio_open_epwing6(zio=%d) = %d", (int)zio->id, zio->file));
    return zio->file;

    /*
     * An error occurs...
     */
  failed:
    if (0 <= zio->file)
	zio_close_raw(zio);
    if (zio->huffman_nodes != NULL)
	free(zio->huffman_nodes);
    zio->file = -1;
    zio->huffman_nodes = NULL;
    zio->huffman_root = NULL;
    zio->code = ZIO_INVALID;

    LOG(("out: zio_open_epwing6() = %d", -1));
    return -1;
}


/*
 * Make a huffman tree for decompressing EPWING compression data.
 */
static int
zio_make_epwing_huffman_tree(Zio *zio, int leaf_count)
{
    Zio_Huffman_Node *target_node;
    Zio_Huffman_Node *most_node;
    Zio_Huffman_Node *node_p;
    Zio_Huffman_Node temporary_node;
    Zio_Huffman_Node *least_node_p;
    Zio_Huffman_Node *tail_node_p;
    int i;
    int j;

    LOG(("in: zio_make_epwing_huffman_tree(zio=%d, leaf_count=%d)",
	(int)zio->id, leaf_count));

    tail_node_p = zio->huffman_nodes + leaf_count;

    /*
     * Sort the leaf nodes in frequency order.
     */
    for (i = 0; i < leaf_count - 1; i++) {
        target_node = zio->huffman_nodes + i;
        most_node = target_node;
        node_p = zio->huffman_nodes + i + 1;

        for (j = i + 1; j < leaf_count; j++) {
            if (most_node->frequency < node_p->frequency)
                most_node = node_p;
            node_p++;
        }

        temporary_node.type = most_node->type;
        temporary_node.value = most_node->value;
        temporary_node.frequency = most_node->frequency;

        most_node->type = target_node->type;
        most_node->value = target_node->value;
        most_node->frequency = target_node->frequency;

        target_node->type = temporary_node.type;
        target_node->value = temporary_node.value;
        target_node->frequency = temporary_node.frequency;
    }

    /*
     * Make intermediate nodes of the huffman tree.
     * The number of intermediate nodes of the tree is <the number of
     * leaf nodes> - 1.
     */
    for (i = 1; i < leaf_count; i++) {
	/*
	 * Initialize a new intermediate node.
	 */
	tail_node_p->type = ZIO_HUFFMAN_NODE_INTERMEDIATE;
	tail_node_p->left = NULL;
	tail_node_p->right = NULL;

	/*
	 * Find for a least frequent node.
	 * That node becomes a left child of the new intermediate node.
	 */
	least_node_p = NULL;
	for (node_p = zio->huffman_nodes; node_p < tail_node_p; node_p++) {
	    if (node_p->frequency == 0)
		continue;
	    if (least_node_p == NULL
		|| node_p->frequency <= least_node_p->frequency)
		least_node_p = node_p;
	}
	if (least_node_p == NULL)
	    goto failed;
	tail_node_p->left = least_node_p;
	tail_node_p->frequency = least_node_p->frequency;
	least_node_p->frequency = 0;

	/*
	 * Find for a next least frequent node.
	 * That node becomes a right child of the new intermediate node.
	 */
	least_node_p = NULL;
	for (node_p = zio->huffman_nodes; node_p < tail_node_p; node_p++) {
	    if (node_p->frequency == 0)
		continue;
	    if (least_node_p == NULL
		|| node_p->frequency <= least_node_p->frequency)
		least_node_p = node_p;
	}
	if (least_node_p == NULL)
	    goto failed;
	tail_node_p->right = least_node_p;
	tail_node_p->frequency += least_node_p->frequency;
	least_node_p->frequency = 0;

	tail_node_p++;
    }

    /*
     * Set a root node of the huffman tree.
     */
    zio->huffman_root = tail_node_p - 1;

    LOG(("out: zio_make_epwing_huffman_tree() = %d", 0));
    return 0;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_make_epwing_huffman_tree() = %d", -1));
    return -1;
}


/*
 * Close `zio'.
 */
void
zio_close(Zio *zio)
{
    pthread_mutex_lock(&zio_mutex);
    LOG(("in: zio_close(zio=%d)", (int)zio->id));

    /*
     * If contents of the file is cached, clear the cache.
     */
    if (0 <= zio->file)
	zio_close_raw(zio);
    zio->file = -1;

    LOG(("out: zio_close()"));
    pthread_mutex_unlock(&zio_mutex);
}


/*
 * Return file descriptor of `zio'.
 */
int
zio_file(Zio *zio)
{
    LOG(("in+out: zio_file(zio=%d) = %d", (int)zio->id, zio->file));

    return zio->file;
}


/*
 * Return compression mode of `zio'.
 */
Zio_Code
zio_mode(Zio *zio)
{
    LOG(("in+out: zio_mode(zio=%d) = %d", (int)zio->id, zio->code));

    return zio->code;
}


/*
 * Seek `zio'.
 */
off_t
zio_lseek(Zio *zio, off_t location, int whence)
{
    off_t result;

    LOG(("in: zio_lseek(zio=%d, location=%ld, whence=%d)",
	(int)zio->id, (long)location, whence));

    if (zio->file < 0)
	goto failed;

    if (zio->code == ZIO_PLAIN) {
	/*
	 * If `zio' is not compressed, simply call lseek().
	 */
	result = zio_lseek_raw(zio, location, whence);
    } else {
	/*
	 * Calculate new location according with `whence'.
	 */
	switch (whence) {
	case SEEK_SET:
	    zio->location = location;
	    break;
	case SEEK_CUR:
	    zio->location = zio->location + location;
	    break;
	case SEEK_END:
	    zio->location = zio->file_size - location;
	    break;
	default:
#ifdef EINVAL
	    errno = EINVAL;
#endif
	    goto failed;
	}

	/*
	 * Adjust location.
	 */
	if (zio->location < 0)
	    zio->location = 0;
	if (zio->file_size < zio->location)
	    zio->location = zio->file_size;

	/*
	 * Update `zio->location'.
	 * (We don't actually seek the file.)
	 */
	result = zio->location;
    }

    LOG(("out: zio_lseek() = %ld", (long)result));
    return result;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_lseek() = %ld", (long)-1));
    return -1;
}


/*
 * Read data from `zio' file.
 */
ssize_t
zio_read(Zio *zio, char *buffer, size_t length)
{
    ssize_t read_length;

    pthread_mutex_lock(&zio_mutex);
    LOG(("in: zio_read(zio=%d, length=%ld)", (int)zio->id, (long)length));

    /*
     * If the zio `file' is not compressed, call read() and return.
     */
    if (zio->file < 0)
	goto failed;

    switch (zio->code) {
    case ZIO_PLAIN:
	read_length = zio_read_raw(zio, buffer, length);
	break;
    case ZIO_EBZIP1:
	read_length = zio_read_ebzip(zio, buffer, length);
	break;
    case ZIO_EPWING:
    case ZIO_EPWING6:
	read_length = zio_read_epwing(zio, buffer, length);
	break;
    case ZIO_SEBXA:
	read_length = zio_read_sebxa(zio, buffer, length);
	break;
    default:
	goto failed;
    }

    LOG(("out: zio_read() = %ld", (long)read_length));
    pthread_mutex_unlock(&zio_mutex);

    return read_length;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_read() = %ld", (long)-1));
    return -1;
}


/*
 * Read data from the `zio' file compressed with the ebzip compression
 * format.
 */
static ssize_t
zio_read_ebzip(Zio *zio, char *buffer, size_t length)
{
    char temporary_buffer[8];
    ssize_t read_length = 0;
    size_t zipped_slice_size;
    off_t slice_location;
    off_t next_slice_location;
    int n;

    LOG(("in: zio_read_ebzip(zio=%d, length=%ld)", (int)zio->id,
	(long)length));

    /*
     * Read data.
     */
    while (read_length < length) {
	if (zio->file_size <= zio->location)
	    goto succeeded;

	/*
	 * If data in `cache_buffer' is out of range, read data from
	 * `zio->file'.
	 */
	if (cache_zio_id != zio->id
	    || zio->location < cache_location
	    || cache_location + zio->slice_size <= zio->location) {

	    cache_zio_id = ZIO_ID_NONE;
	    cache_location = zio->location - (zio->location % zio->slice_size);

	    /*
	     * Get buffer location and size from index table in `zio->file'.
	     */
	    if (zio_lseek_raw(zio, zio->location / zio->slice_size
		* zio->index_width + ZIO_SIZE_EBZIP_HEADER, SEEK_SET) < 0)
		goto failed;
	    if (zio_read_raw(zio, temporary_buffer, zio->index_width * 2)
		!= zio->index_width * 2)
		goto failed;

	    switch (zio->index_width) {
	    case 2:
		slice_location = zio_uint2(temporary_buffer);
		next_slice_location = zio_uint2(temporary_buffer + 2);
		break;
	    case 3:
		slice_location = zio_uint3(temporary_buffer);
		next_slice_location = zio_uint3(temporary_buffer + 3);
		break;
	    case 4:
		slice_location = zio_uint4(temporary_buffer);
		next_slice_location = zio_uint4(temporary_buffer + 4);
		break;
	    case 5:
		slice_location = zio_uint5(temporary_buffer);
		next_slice_location = zio_uint5(temporary_buffer + 5);
		break;
	    default:
		goto failed;
	    }
	    zipped_slice_size = next_slice_location - slice_location;

	    if (next_slice_location <= slice_location
		|| zio->slice_size < zipped_slice_size)
		goto failed;

	    /*
	     * Read a compressed slice from `zio->file' and uncompress it.
	     * The data is not compressed if its size is equals to
	     * slice size.
	     */
	    if (zio_lseek_raw(zio, slice_location, SEEK_SET) < 0)
		goto failed;
	    if (zio_unzip_slice_ebzip1(zio, cache_buffer, zipped_slice_size)
		< 0)
		goto failed;

	    cache_zio_id = zio->id;
	}

	/*
	 * Copy data from `cache_buffer' to `buffer'.
	 */
	n = zio->slice_size - (zio->location % zio->slice_size);
	if (length - read_length < n)
	    n = length - read_length;
	if (zio->file_size - zio->location < n)
	    n = zio->file_size - zio->location;
	memcpy(buffer + read_length,
	    cache_buffer + (zio->location % zio->slice_size), n);
	read_length += n;
	zio->location += n;
    }

  succeeded:
    LOG(("out: zio_read_ebzip() = %ld", (long)read_length));
    return read_length;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_read_ebzip() = %ld", (long)-1));
    return -1;
}


/*
 * Read data from the `zio' file compressed with the EPWING or EPWING V6
 * compression format.
 */
static ssize_t
zio_read_epwing(Zio *zio, char *buffer, size_t length)
{
    char temporary_buffer[36];
    ssize_t read_length = 0;
    off_t page_location;
    int n;

    LOG(("in: zio_read_epwing(zio=%d, length=%ld)", (int)zio->id,
	(long)length));

    /*
     * Read data.
     */
    while (read_length < length) {
	if (zio->file_size <= zio->location)
	    goto succeeded;

	/*
	 * If data in `cache_buffer' is out of range, read data from the zio
	 * file.
	 */
	if (cache_zio_id != zio->id
	    || zio->location < cache_location
	    || cache_location + zio->slice_size <= zio->location) {
	    cache_zio_id = ZIO_ID_NONE;
	    cache_location = zio->location - (zio->location % zio->slice_size);

	    /*
	     * Get page location from index table in `zio->file'.
	     */
	    if (zio_lseek_raw(zio, zio->index_location
		+ zio->location / (ZIO_SIZE_PAGE * 16) * 36, SEEK_SET) < 0)
		goto failed;
	    if (zio_read_raw(zio, temporary_buffer, 36) != 36)
		goto failed;
	    page_location = zio_uint4(temporary_buffer)
		+ zio_uint2(temporary_buffer + 4
		    + (zio->location / ZIO_SIZE_PAGE % 16) * 2);

	    /*
	     * Read a compressed page from `zio->file' and uncompress it.
	     */
	    if (zio_lseek_raw(zio, page_location, SEEK_SET) < 0)
		goto failed;
	    if (zio->code == ZIO_EPWING) {
		if (zio_unzip_slice_epwing(zio, cache_buffer) < 0)
		    goto failed;
	    } else {
		if (zio_unzip_slice_epwing6(zio, cache_buffer) < 0)
		    goto failed;
	    }

	    cache_zio_id = zio->id;
	}

	/*
	 * Copy data from `cache_buffer' to `buffer'.
	 */
	n = ZIO_SIZE_PAGE - (zio->location % ZIO_SIZE_PAGE);
	if (length - read_length < n)
	    n = length - read_length;
	if (zio->file_size - zio->location < n)
	    n = zio->file_size - zio->location;
	memcpy(buffer + read_length,
	    cache_buffer + (zio->location - cache_location), n);
	read_length += n;
	zio->location += n;
    }

  succeeded:
    LOG(("out: zio_read_epwing() = %ld", (long)read_length));
    return read_length;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_read_epwing() = %ld", (long)-1));
    return -1;
}


#define ZIO_SEBXA_SLICE_LENGTH	4096

/*
 * Read data from the zio `file' compressed with the S-EBXA compression
 * format.
 */
static ssize_t
zio_read_sebxa(Zio *zio, char *buffer, size_t length)
{
    char temporary_buffer[4];
    ssize_t read_length = 0;
    off_t slice_location;
    ssize_t n;
    int slice_index;

    LOG(("in: zio_read_sebxa(zio=%d, length=%ld)", (int)zio->id,
	(long)length));

    /*
     * Read data.
     */
    while (read_length < length) {
	if (zio->file_size <= zio->location)
	    goto succeeded;

	if (zio->location < zio->zio_start_location) {
	    /*
	     * Data is located in front of compressed text.
	     */
	    if (zio->zio_start_location - zio->location < length - read_length)
		n = zio->zio_start_location - zio->location;
	    else
		n = length - read_length;
	    if (zio_lseek_raw(zio, zio->location, SEEK_SET) < 0)
		goto failed;
	    if (zio_read_raw(zio, buffer, n) != n)
		goto failed;
	    read_length += n;

	} else if (zio->zio_end_location <= zio->location) {
	    /*
	     * Data is located behind compressed text.
	     */
	    if (zio_lseek_raw(zio, zio->location, SEEK_SET) < 0)
		goto failed;
	    if (zio_read_raw(zio, buffer, length - read_length)
		!= length - read_length)
		goto failed;
	    read_length = length;

	} else {
	    /*
	     * Data is located in compressed text.
	     *
	     * If data in `cache_buffer' is out of range, read data from
	     * `file'.
	     */
	    if (cache_zio_id != zio->id
		|| zio->location < cache_location
		|| cache_location + ZIO_SEBXA_SLICE_LENGTH <= zio->location) {

		cache_zio_id = ZIO_ID_NONE;
		cache_location = zio->location
		    - (zio->location % ZIO_SEBXA_SLICE_LENGTH);

		/*
		 * Get buffer location and size.
		 */
		slice_index = (zio->location - zio->zio_start_location)
		    / ZIO_SEBXA_SLICE_LENGTH;
		if (slice_index == 0)
		    slice_location = zio->index_base;
		else {
		    if (zio_lseek_raw(zio, ((off_t) slice_index - 1) * 4
			+ zio->index_location, SEEK_SET) < 0)
			goto failed;
		    if (zio_read_raw(zio, temporary_buffer, 4) != 4)
			goto failed;
		    slice_location = zio->index_base
			+ zio_uint4(temporary_buffer);
		}

		/*
		 * Read a compressed slice from `zio->file' and uncompress it.
		 */
		if (zio_lseek_raw(zio, slice_location, SEEK_SET) < 0)
		    goto failed;
		if (zio_unzip_slice_sebxa(zio, cache_buffer) < 0)
		    goto failed;

		cache_zio_id = zio->id;
	    }

	    /*
	     * Copy data from `cache_buffer' to `buffer'.
	     */
	    n = ZIO_SEBXA_SLICE_LENGTH
		- (zio->location % ZIO_SEBXA_SLICE_LENGTH);
	    if (length - read_length < n)
		n = length - read_length;
	    if (zio->file_size - zio->location < n)
		n = zio->file_size - zio->location;
	    memcpy(buffer + read_length,
		cache_buffer + (zio->location - cache_location), n);
	    read_length += n;
	    zio->location += n;
	}
    }

  succeeded:
    LOG(("out: zio_read_sebxa() = %ld", (long)read_length));
    return read_length;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_read_sebxa() = %ld", (long)-1));
    return -1;
}


/*
 * Uncompress an ebzip'ped slice.
 *
 * If it succeeds, 0 is returned.  Otherwise, -1 is returned.
 */
static int
zio_unzip_slice_ebzip1(Zio *zio, char *out_buffer, size_t zipped_slice_size)
{
    LOG(("in: zio_unzip_slice_ebzip1(zio=%d, zipped_slice_size=%ld)",
	 (int)zio->id, (long)zipped_slice_size));

    if (zio->slice_size == zipped_slice_size) {
	/*
	 * The input slice is not compressed.
	 * Read the target page in the slice.
	 */
	if (zio_read_raw(zio, out_buffer, zipped_slice_size) !=
	    zipped_slice_size)
	    goto failed;

    } else {
	/*
	 * The input slice is compressed.
	 * Read and uncompress the target page in the slice.
	 */
	if (zio_unzip_slice_ebzip1_internal
	    (zio, out_buffer, zipped_slice_size) != 0)
	    goto failed;
    }

    LOG(("out: zio_unzip_slice_ebzip1() = %d", 0));
    return 0;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_unzip_slice_ebzip1() = %d", -1));
    return -1;
}

inline static int
zio_unzip_slice_ebzip1_internal
(Zio *zio, char *out_buffer, size_t zipped_slice_size)
{
#ifdef ENABLE_LIBDEFLATE
    char *in_buffer = NULL;
    struct libdeflate_decompressor *decompressor = NULL;
    enum libdeflate_result result;

    LOG(("in: zio_unzip_slice_ebzip1_internal(zio=%d, zipped_slice_size=%ld)",
	 (int)zio->id, (long)zipped_slice_size));

    decompressor = libdeflate_alloc_decompressor();
    if (!decompressor) goto failed;

    in_buffer = malloc(zipped_slice_size);
    if (!in_buffer) goto failed;

    if (zio_read_raw(zio, in_buffer, zipped_slice_size) !=
	zipped_slice_size)
	goto failed;

    result = libdeflate_zlib_decompress
	(decompressor, in_buffer, zipped_slice_size, out_buffer,
	 zio->slice_size, NULL);

    if (result != LIBDEFLATE_SUCCESS) goto failed;

    libdeflate_free_decompressor(decompressor);
    free (in_buffer);

    LOG(("out: zio_unzip_slice_ebzip1_internal() = %d", 0));
    return 0;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_unzip_slice_ebzip1_internal() = %d", -1));
    if (!decompressor) libdeflate_free_decompressor(decompressor);
    if (!in_buffer) free (in_buffer);
    return -1;
#else
    char in_buffer[ZIO_SIZE_PAGE];
    z_stream stream;
    size_t read_length;
    int z_result;

    LOG(("in: zio_unzip_slice_ebzip1_internal(zio=%d, zipped_slice_size=%ld)",
	(int)zio->id, (long)zipped_slice_size));

    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;

    if (inflateInit(&stream) != Z_OK)
	goto failed;

    stream.next_in = (Bytef *) in_buffer;
    stream.avail_in = 0;
    stream.next_out = (Bytef *) out_buffer;
    stream.avail_out = zio->slice_size;

    while (stream.total_out < zio->slice_size) {
	if (0 < stream.avail_in)
	    memmove(in_buffer, stream.next_in, stream.avail_in);

	if (zipped_slice_size - stream.total_in < ZIO_SIZE_PAGE) {
	    read_length = zipped_slice_size - stream.total_in
		- stream.avail_in;
	} else {
	    read_length = ZIO_SIZE_PAGE - stream.avail_in;
	}

	if (zio_read_raw(zio, in_buffer + stream.avail_in,
	    read_length) != read_length)
	    goto failed;

	stream.next_in = (Bytef *) in_buffer;
	stream.avail_in += read_length;
	stream.avail_out = zio->slice_size - stream.total_out;

	z_result = inflate(&stream, Z_SYNC_FLUSH);
	if (z_result == Z_STREAM_END) {
	    break;
	} else if (z_result != Z_OK && z_result != Z_BUF_ERROR) {
	    goto failed;
	}
    }

    inflateEnd(&stream);

    LOG(("out: zio_unzip_slice_ebzip1_internal() = %d", 0));
    return 0;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_unzip_slice_ebzip1_internal() = %d", -1));
    inflateEnd(&stream);
    return -1;
#endif
}


/*
 * Uncompress an EPWING compressed slice.
 * The offset of `zio->file' must points to the beginning of the compressed
 * slice.  Uncompressed data are put into `out_buffer'.
 *
 * If it succeeds, 0 is returned.  Otherwise, -1 is returned.
 */
static int
zio_unzip_slice_epwing(Zio *zio, char *out_buffer)
{
    Zio_Huffman_Node *node_p;
    int bit;
    char in_buffer[ZIO_SIZE_PAGE];
    unsigned char *in_buffer_p;
    ssize_t in_read_length;
    int in_bit_index;
    unsigned char *out_buffer_p;
    size_t out_length;

    LOG(("in: zio_unzip_slice_epwing(zio=%d)", (int)zio->id));

    in_buffer_p = (unsigned char *)in_buffer;
    in_bit_index = 7;
    in_read_length = 0;
    out_buffer_p = (unsigned char *)out_buffer;
    out_length = 0;

    for (;;) {
	/*
	 * Descend the huffman tree until reached to the leaf node.
	 */
	node_p = zio->huffman_root;
	while (node_p->type == ZIO_HUFFMAN_NODE_INTERMEDIATE) {

	    /*
	     * If no data is left in the input buffer, read next chunk.
	     */
	    if ((unsigned char *)in_buffer + in_read_length <= in_buffer_p) {
		in_read_length = zio_read_raw(zio, in_buffer, ZIO_SIZE_PAGE);
		if (in_read_length <= 0)
		    goto failed;
		in_buffer_p = (unsigned char *)in_buffer;
	    }

	    /*
	     * Step to a child.
	     */
	    bit = (*in_buffer_p >> in_bit_index) & 0x01;

	    if (bit == 1)
		node_p = node_p->left;
	    else
		node_p = node_p->right;
	    if (node_p == NULL)
		goto failed;

	    if (0 < in_bit_index)
		in_bit_index--;
	    else {
		in_bit_index = 7;
		in_buffer_p++;
	    }
	}

	if (node_p->type == ZIO_HUFFMAN_NODE_EOF) {
	    /*
	     * Fill the rest of the output buffer with NUL,
             * when we meet an EOF mark before decode ZIO_SIZE_PAGE bytes.
	     */
	    if (out_length < ZIO_SIZE_PAGE) {
		memset(out_buffer_p, '\0', ZIO_SIZE_PAGE - out_length);
		out_length = ZIO_SIZE_PAGE;
	    }
	    break;

	} else if (node_p->type == ZIO_HUFFMAN_NODE_LEAF16) {
	    /*
	     * The leaf is leaf16, decode 2 bytes character.
	     */
	    if (ZIO_SIZE_PAGE <= out_length)
		goto failed;
	    else if (ZIO_SIZE_PAGE <= out_length + 1) {
		*out_buffer_p++ = (node_p->value >> 8) & 0xff;
		out_length++;
	    } else {
		*out_buffer_p++ = (node_p->value >> 8) & 0xff;
		*out_buffer_p++ = node_p->value & 0xff;
		out_length += 2;
	    }
	} else {
	    /*
	     * The leaf is leaf8, decode 1 byte character.
	     */
	    if (ZIO_SIZE_PAGE <= out_length)
		goto failed;
	    else {
		*out_buffer_p++ = node_p->value;
		out_length++;
	    }
	}
    }

    LOG(("out: zio_unzip_slice_epwing() = %d", 0));
    return 0;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_unzip_slice_epwing() = %d", -1));
    return -1;
}


/*
 * Uncompress an EPWING V6 compressed slice.
 * The offset of `zio->file' must points to the beginning of the compressed
 * slice.  Uncompressed data are put into `out_buffer'.
 *
 * If it succeeds, 0 is returned.  Otherwise, -1 is returned.
 */
static int
zio_unzip_slice_epwing6(Zio *zio, char *out_buffer)
{
    Zio_Huffman_Node *node_p;
    int bit;
    char in_buffer[ZIO_SIZE_PAGE];
    unsigned char *in_buffer_p;
    ssize_t in_read_length;
    int in_bit_index;
    unsigned char *out_buffer_p;
    size_t out_length;
    int compression_type;

    LOG(("in: zio_unzip_slice_epwing6(zio=%d)", (int)zio->id));

    in_buffer_p = (unsigned char *)in_buffer;
    in_bit_index = 7;
    in_read_length = 0;
    out_buffer_p = (unsigned char *)out_buffer;
    out_length = 0;

    /*
     * Get compression type.
     */
    if (zio_read_raw(zio, in_buffer, 1) != 1)
	goto failed;
    compression_type = zio_uint1(in_buffer);

    /*
     * If compression type is not 0, this page is not compressed.
     */
    if (compression_type != 0) {
	if (zio_read_raw(zio, out_buffer, ZIO_SIZE_PAGE) != ZIO_SIZE_PAGE)
	    goto failed;
	goto succeeded;
    }

    while (out_length < ZIO_SIZE_PAGE) {
	/*
	 * Descend the huffman tree until reached to the leaf node.
	 */
	node_p = zio->huffman_root;
	while (node_p->type == ZIO_HUFFMAN_NODE_INTERMEDIATE) {

	    /*
	     * If no data is left in the input buffer, read next chunk.
	     */
	    if ((unsigned char *)in_buffer + in_read_length <= in_buffer_p) {
		in_read_length = zio_read_raw(zio, in_buffer, ZIO_SIZE_PAGE);
		if (in_read_length <= 0)
		    goto failed;
		in_buffer_p = (unsigned char *)in_buffer;
	    }

	    /*
	     * Step to a child.
	     */
	    bit = (*in_buffer_p >> in_bit_index) & 0x01;

	    if (bit == 1)
		node_p = node_p->left;
	    else
		node_p = node_p->right;
	    if (node_p == NULL)
		goto failed;

	    if (0 < in_bit_index)
		in_bit_index--;
	    else {
		in_bit_index = 7;
		in_buffer_p++;
	    }
	}

	if (node_p->type == ZIO_HUFFMAN_NODE_EOF) {
	    /*
	     * Fill the rest of the output buffer with NUL,
             * when we meet an EOF mark before decode ZIO_SIZE_PAGE bytes.
	     */
	    if (out_length < ZIO_SIZE_PAGE) {
		memset(out_buffer_p, '\0', ZIO_SIZE_PAGE - out_length);
		out_length = ZIO_SIZE_PAGE;
	    }
	    break;

	} else if (node_p->type == ZIO_HUFFMAN_NODE_LEAF32) {
	    /*
	     * The leaf is leaf32, decode 4 bytes character.
	     */
	    if (ZIO_SIZE_PAGE <= out_length + 1) {
		*out_buffer_p++ = (node_p->value >> 24) & 0xff;
		out_length++;
	    } else if (ZIO_SIZE_PAGE <= out_length + 2) {
		*out_buffer_p++ = (node_p->value >> 24) & 0xff;
		*out_buffer_p++ = (node_p->value >> 16) & 0xff;
		out_length += 2;
	    } else if (ZIO_SIZE_PAGE <= out_length + 3) {
		*out_buffer_p++ = (node_p->value >> 24) & 0xff;
		*out_buffer_p++ = (node_p->value >> 16) & 0xff;
		*out_buffer_p++ = (node_p->value >> 8)  & 0xff;
		out_length += 3;
	    } else {
		*out_buffer_p++ = (node_p->value >> 24) & 0xff;
		*out_buffer_p++ = (node_p->value >> 16) & 0xff;
		*out_buffer_p++ = (node_p->value >> 8)  & 0xff;
		*out_buffer_p++ = node_p->value         & 0xff;
		out_length += 4;
	    }
	} else if (node_p->type == ZIO_HUFFMAN_NODE_LEAF16) {
	    /*
	     * The leaf is leaf16, decode 2 bytes character.
	     */
	    if (ZIO_SIZE_PAGE <= out_length + 1) {
		*out_buffer_p++ = (node_p->value >> 8)  & 0xff;
		out_length++;
	    } else {
		*out_buffer_p++ = (node_p->value >> 8)  & 0xff;
		*out_buffer_p++ = node_p->value & 0xff;
		out_length += 2;
	    }
	} else {
	    /*
	     * The leaf is leaf8, decode 1 byte character.
	     */
	    *out_buffer_p++ = node_p->value;
	    out_length++;
	}
    }

  succeeded:
    LOG(("out: zio_unzip_slice_epwing6() = %d", 0));
    return 0;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_unzip_slice_epwing6() = %d", -1));
    return -1;
}

/*
 * Uncompress an S-EBXA compressed slice.
 * The offset of `zio->file' must points to the beginning of the compressed
 * slice.  Uncompressed data are put into `out_buffer'.
 *
 * If it succeeds, 0 is returned.  Otherwise, -1 is returned.
 */
static int
zio_unzip_slice_sebxa(Zio *zio, char *out_buffer)
{
    char in_buffer[ZIO_SEBXA_SLICE_LENGTH];
    unsigned char *in_buffer_p;
    ssize_t in_read_rest;
    unsigned char *out_buffer_p;
    size_t out_length;
    int compression_flags[8];
    int copy_offset;
    int copy_length;
    int i, j;

    LOG(("in: zio_unzip_slice_sebxa(zio=%d)", (int)zio->id));

    in_buffer_p = (unsigned char *)in_buffer;
    in_read_rest = 0;
    out_buffer_p = (unsigned char *)out_buffer;
    out_length = 0;

    for (;;) {
	/*
	 * If no data is left in the input buffer, read next chunk.
	 */
	if (in_read_rest <= 0) {
	    in_read_rest = zio_read_raw(zio, in_buffer,
		ZIO_SEBXA_SLICE_LENGTH);
	    if (in_read_rest <= 0)
		goto failed;
	    in_buffer_p = (unsigned char *)in_buffer;
	}

	/*
	 * The current input byte is recognized as compression flags
	 * for next 8 chunks.
	 */
	compression_flags[0] = !(*in_buffer_p & 0x01);
	compression_flags[1] = !(*in_buffer_p & 0x02);
	compression_flags[2] = !(*in_buffer_p & 0x04);
	compression_flags[3] = !(*in_buffer_p & 0x08);
	compression_flags[4] = !(*in_buffer_p & 0x10);
	compression_flags[5] = !(*in_buffer_p & 0x20);
	compression_flags[6] = !(*in_buffer_p & 0x40);
	compression_flags[7] = !(*in_buffer_p & 0x80);
	in_buffer_p++;
	in_read_rest--;

	/*
	 * Decode 8 chunks.
	 */
	for (i = 0; i < 8; i++) {
	    if (compression_flags[i]) {
		/*
		 * This chunk is compressed.
		 * Copy `copy_length' bytes from `copy_p' to the current
		 * point.
		 */
		unsigned char *copy_p;
		unsigned char c0, c1;

		if (in_read_rest <= 1)
		    goto failed;

		/*
		 * Get 2 bytes from the current input, and recognize
		 * them as following:
		 *
		 *              *in_buffer_p   *(in_bufer_p + 1)
		 *  bit pattern: [AAAA|BBBB]   [CCCC|DDDD]
		 *
		 *  copy_offset = ([CCCCAAAABBBB] + 18) % 4096
		 *  copy_length   = [DDDD] + 3
		 */
		c0 = *(unsigned char *)in_buffer_p;
		c1 = *((unsigned char *)in_buffer_p + 1);
		copy_offset = (((c1 & 0xf0) << 4) + c0 + 18)
		    % ZIO_SEBXA_SLICE_LENGTH;
		copy_length = (c1 & 0x0f) + 3;

		if (ZIO_SEBXA_SLICE_LENGTH < out_length + copy_length)
		    copy_length = ZIO_SEBXA_SLICE_LENGTH - out_length;

		copy_p = (unsigned char *)out_buffer + copy_offset;
		for (j = 0; j < copy_length; j++) {
		    if (copy_p < out_buffer_p)
			*out_buffer_p++ = *copy_p;
		    else
			*out_buffer_p++ = 0x00;
		    copy_p++;
		    if (ZIO_SEBXA_SLICE_LENGTH <=
			copy_p - (unsigned char *)out_buffer)
			copy_p -= ZIO_SEBXA_SLICE_LENGTH;
		}

		in_read_rest -= 2;
		in_buffer_p += 2;
		out_length += copy_length;

	    } else {
		/*
		 * This chunk is not compressed.
		 * Put the current input byte as a decoded value.
		 */
		if (in_read_rest <= 0)
		    goto failed;
		in_read_rest -= 1;
		*out_buffer_p++ = *in_buffer_p++;
		out_length += 1;
	    }

	    /*
	     * Return if the slice has been uncompressed.
	     */
	    if (ZIO_SEBXA_SLICE_LENGTH <= out_length)
		goto succeeded;
	}
    }

  succeeded:
    LOG(("out: zio_unzip_slice_sebxa() = %d", 0));
    return 0;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_unzip_slice_sebxa() = %d", -1));
    return -1;
}


/*
 * Low-level open function.
 *
 * If `file_name' is ebnet URL, it calls ebnet_open().  Otherwise it
 * calls the open() system call.
 *
 * Like open(), it returns file descrptor or -1.
 */
static int
zio_open_raw(Zio *zio, const char *file_name)
{
#ifdef ENABLE_EBNET
    if (is_ebnet_url(file_name)) {
	zio->is_ebnet = 1;
	zio->file = ebnet_open(file_name);
    } else {
	zio->is_ebnet = 0;
	zio->file = open(file_name, O_RDONLY | O_BINARY);
    }
#else
    zio->file = open(file_name, O_RDONLY | O_BINARY);
#endif

    return zio->file;
}


/*
 * Low-level close function.
 *
 * If `zio->file' is socket, it calls ebnet_close().  Otherwise it calls
 * the close() system call.
 */
static void
zio_close_raw(Zio *zio)
{
#ifdef ENABLE_EBNET
    if (zio->is_ebnet)
	ebnet_close(zio->file);
    else
	close(zio->file);
#else
	close(zio->file);
#endif
}


/*
 * Low-level seek function.
 *
 * If `zio->file' is socket, it calls ebnet_close().  Otherwise it calls
 * the close() system call.
 */
static off_t
zio_lseek_raw(Zio *zio, off_t offset, int whence)
{
    off_t result;

    if (zio->is_ebnet) {
#ifdef ENABLE_EBNET
	result = ebnet_lseek(zio->file, offset, whence);
#else
	result = -1;
#endif
    } else {
	result = lseek(zio->file, offset, whence);
    }

    return result;
}


/*
 * Low-level read function.
 *
 * If `zio->file' is socket, it calls ebnet_read().  Otherwise it calls
 * the read() system call.
 */
static ssize_t
zio_read_raw(Zio *zio, void *buffer, size_t length)
{
    char *buffer_p = buffer;
    ssize_t result;

    LOG(("in: zio_read_raw(file=%d, length=%ld)", zio->file, (long)length));

    if (zio->is_ebnet) {
	/*
	 * Read from a remote server.
	 */
#ifdef ENABLE_EBNET
	result = ebnet_read(&zio->file, buffer, length);
#else
	result = -1;
#endif
    } else {
	/*
	 * Read from a local file.
	 */
	ssize_t rest_length = length;
	ssize_t n;

	while (0 < rest_length) {
	    errno = 0;
	    n = read(zio->file, buffer_p, rest_length);
	    if (n < 0) {
		if (errno == EINTR)
		    continue;
		goto failed;
	    } else if (n == 0)
		break;
	    else {
		rest_length -= n;
		buffer_p += n;
	    }
	}

	result = length - rest_length;
    }

    LOG(("out: zio_read_raw() = %ld", (long)result));
    return result;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: zio_read_raw() = %ld", (long)-1));
    return -1;
}


