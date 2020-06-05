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

#ifndef LINEBUF_H
#define LINEBUF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

/*
 * Buffer size of `Line_Buffer' struct.
 */
#define LINEBUF_BUFFER_SIZE	256

/*
 * Line buffer manager.
 */
typedef struct {
    int file;				/* file descriptor */
    int timeout;			/* idle timeout interval */
    size_t cache_length;		/* length of cache data */
    char buffer[LINEBUF_BUFFER_SIZE];	/* buffer */
} Line_Buffer;
 

/*
 * Function declarations.
 */
void initialize_line_buffer(Line_Buffer *line_buffer);
void finalize_line_buffer(Line_Buffer *line_buffer);
void set_line_buffer_timeout(Line_Buffer *line_buffer, int timeout);
void bind_file_to_line_buffer(Line_Buffer *line_buffer, int file);
int file_bound_to_line_buffer(Line_Buffer *line_buffer);
void discard_cache_in_line_buffer(Line_Buffer *line_buffer);
size_t cache_length_in_line_buffer(Line_Buffer *line_buffer);
ssize_t read_line_buffer(Line_Buffer *line_buffer, char *line,
    size_t max_line_length);
ssize_t binary_read_line_buffer(Line_Buffer *line_buffer, char *stream,
    size_t stream_length);
int skip_line_buffer(Line_Buffer *line_buffer);

#endif /* not LINEBUF_H */
