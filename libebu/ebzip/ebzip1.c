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

#include "ebzip.h"

#include <stdio.h>
#include <sys/types.h>

#include "ebu/eb.h"

/*
 * Compress a slice with the ebzip compression format.
 *
 * If it succeeds, 0 is returned.  Otherwise, -1 is returned.
 */
int
ebzip1_slice(char *out_buffer, size_t *out_byte_length, char *in_buffer,
    size_t in_byte_length)
{
#ifdef ENABLE_LIBDEFLATE
    struct libdeflate_compressor *compressor;
    size_t result;

    if (!(compressor = libdeflate_alloc_compressor(12))) {
	return -1;
    }

    result = libdeflate_zlib_compress
	(compressor, in_buffer, in_byte_length, out_buffer, in_byte_length);

    *out_byte_length = result ? result : in_byte_length;
    libdeflate_free_compressor(compressor);
    return 0;
#else
    z_stream stream;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (deflateInit(&stream, ebzip_level > 3
		    ? Z_BEST_COMPRESSION : Z_DEFAULT_COMPRESSION) != Z_OK)
	return -1;

    stream.next_in = (Bytef *) in_buffer;
    stream.avail_in = in_byte_length;
    stream.next_out = (Bytef *) out_buffer;
    stream.avail_out = in_byte_length;

    if (deflate(&stream, Z_FINISH) != Z_STREAM_END) {
	*out_byte_length = in_byte_length;
	deflateEnd(&stream);
	return 0;
    }

    if (deflateEnd(&stream) != Z_OK)
	return -1;

    *out_byte_length = stream.total_out;
    return 0;
#endif
}


