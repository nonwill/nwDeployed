/*
 * copyright (c) 1997-2005  Motoyuki Kasahara
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
#ifdef ENABLE_WINSOCK2
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "linebuf.h"

#ifdef USE_FAKELOG
#include "fakelog.h"
#endif


/*
 * Initialize `linebuffer'.
 */
void
initialize_line_buffer(Line_Buffer *line_buffer)
{
    line_buffer->buffer[0] = '\0';
    line_buffer->file = -1;
    line_buffer->cache_length = 0;
    line_buffer->timeout = 0;
}


/*
 * Finalize `line_buffer'.
 */
void
finalize_line_buffer(Line_Buffer *line_buffer)
{
    line_buffer->buffer[0] = '\0';
    line_buffer->file = -1;
    line_buffer->cache_length = 0;
    line_buffer->timeout = 0;
}


/*
 * Set timeout seconds.
 */
void
set_line_buffer_timeout(Line_Buffer *line_buffer, int timeout)
{
    line_buffer->timeout = timeout;
}


/*
 * Bind `file' to `line_buffer'.
 */
void
bind_file_to_line_buffer(Line_Buffer *line_buffer, int file)
{
    if (line_buffer->file < 0)
	initialize_line_buffer(line_buffer);
    line_buffer->file = file;
}


/*
 * Return file descriptor bound to `line_buffer'.
 * Return -1 when no file is bound.
 */
int
file_bound_to_line_buffer(Line_Buffer *line_buffer)
{
    return line_buffer->file;
}


/*
 * Discard cache data in `line_buffer'.
 */
void
discard_cache_in_line_buffer(Line_Buffer *line_buffer)
{
    line_buffer->cache_length = 0;
}


/*
 * Get the length of cache data in `line_buffer'.
 */
size_t
cache_length_in_line_buffer(Line_Buffer *line_buffer)
{
    return line_buffer->cache_length;
}


/*
 * Read a line from the file bound to `line_buffer', and copy the read
 * line to `line'.  It reads at most `max_line_length' bytes.
 *
 * The function recognizes both "\n" and "\r\n" as end of line.
 * "\n" or "\r\n" is not copied to `buffer', but "\0" is added, instead.
 *
 * The function returns the number of characters in the line, upon
 * successful.  It doesn't count "\n" or "\r\n" in the tail of the line,
 * so that 0 is returned for an empty line, and the line length doesn't
 * exceed one less than `max_line_length'.
 *
 * If EOF is received or an error occurs, -1 is returned.  If the
 * line is too long, `max_line_length' is returned.
 */
ssize_t
read_line_buffer(Line_Buffer *line_buffer, char *line, size_t max_line_length)
{
    char *line_p;
    char *newline;
    size_t search_length;
    size_t additional_length;
    size_t line_length;
    fd_set fdset;
    struct timeval timeval;
    int select_result;
    ssize_t read_result;

    /*
     * Return -1 if no file is bound, or if `max_line_length' is 0.
     */
    if (line_buffer->file < 0)
	return -1;
    if (max_line_length == 0)
	return -1;

    /*
     * Read a file until newline is appeared.
     */
    line_length = 0;
    line_p = line;

    for (;;) {
	if (0 < line_buffer->cache_length) {
	    /*
	     * Find a newline in the cache data.
	     */
	    if (max_line_length - line_length < line_buffer->cache_length)
		search_length = max_line_length - line_length;
	    else
		search_length = line_buffer->cache_length;

	    newline = (char *)memchr(line_buffer->buffer, '\n', search_length);

	    /*
	     * Append cache data in front of the newline to `line'.
	     */
	    if (newline != NULL)
		additional_length = newline - line_buffer->buffer + 1;
	    else
		additional_length = search_length;
	    memcpy(line_p, line_buffer->buffer, additional_length);
	    line_p += additional_length;
	    line_length += additional_length;
	    line_buffer->cache_length -= additional_length;

	    /*
	     * If cache data not copied to `line' are remained in the
	     * buffer, we move them to the beginning of the buffer.
	     */
	    memmove(line_buffer->buffer,
		line_buffer->buffer + additional_length,
		line_buffer->cache_length);

	    if (newline != NULL)
		break;
	}

	/*
	 * Check for the length of the current line.  Return if the
	 * line is too long.
	 *
	 * Note that the following conditional expression can be
	 * substituted to (line_buffer->cache_length != 0), because
	 * remained cache data mean that the line is too long.
	 */
	if (max_line_length <= line_length)
	    return line_length;

	/*
	 * Call select().
	 */
	errno = 0;
	FD_ZERO(&fdset);
	FD_SET(line_buffer->file, &fdset);

	if (line_buffer->timeout == 0) {
	    select_result = select(line_buffer->file + 1, &fdset, NULL, NULL,
		NULL);
	} else {
	    timeval.tv_sec = line_buffer->timeout;
	    timeval.tv_usec = 0;
	    select_result = select(line_buffer->file + 1, &fdset, NULL, NULL,
		&timeval);
	}
	if (select_result < 0) {
	    if (errno == EINTR)
		continue;
	    return -1;
	} else if (select_result == 0) {
	    return -1;
	}

	/*
	 * Read from a file.  (No cache data are remaind.)
	 */
	errno = 0;
	read_result = recv(line_buffer->file, line_buffer->buffer,
	    LINEBUF_BUFFER_SIZE, 0);
	if (read_result < 0) {
	    if (errno == EINTR)
		continue;
	    return -1;
	} else if (read_result == 0) {
	    if (line_length == 0) {
		return -1;
	    }
	    return line_length;
	}
	line_buffer->cache_length += read_result;
    }

    /*
     * Overwrite `\n' with `\0'.
     */
    line_p--;
    *line_p = '\0';
    line_length--;

    /*
     * If the line is end with `\r\n', remove not only `\n' but `\r'.
     */
    if (0 < line_length && *(line_p - 1) == '\r') {
	line_p--;
	*line_p = '\0';
	line_length--;
    }

    return line_length;
}


/*
 * Read just `stream_length' bytes from the file bound to `line_buffer',
 * and copy the read bytes to `stream'.
 *
 * Unlike read_line_buffer(), it doesn't append `\0' to the read data,
 * nor remove newline character in the read data.
 *
 * If it succeeds, the number of bytes actually read is returned.
 * If EOF is received or an error occurs, -1 is returned.
 */
ssize_t
binary_read_line_buffer(Line_Buffer *line_buffer, char *stream,
    size_t stream_length)
{
    char *stream_p;
    size_t done_length;
    fd_set fdset;
    struct timeval timeval;
    int select_result;
    ssize_t read_result;

    /*
     * Return -1 if no file is bound.
     */
    if (line_buffer->file < 0)
	return -1;

    /*
     * Return 0 if `stream_length' is 0.
     */
    if (stream_length == 0)
	return 0;

    /*
     * Test whether cache data are left in `line_buffer->buffer'.
     * If they are, copy them to `stream'.
     */
    stream_p = stream;
    done_length = 0;

    if (0 < line_buffer->cache_length) {
	if (stream_length <= line_buffer->cache_length)
	    done_length = stream_length;
	else
	    done_length = line_buffer->cache_length;

	memcpy(stream_p, line_buffer->buffer, done_length);
	stream_p += done_length;
	line_buffer->cache_length -= done_length;
	memmove(line_buffer->buffer,
	    line_buffer->buffer + done_length,
	    line_buffer->cache_length);
    }

    /*
     * Read the file until the number of read bytes (`done_length') is
     * reached to `stream_length'.
     */
    while (done_length < stream_length) {
	/*
	 * Call select().
	 */
	errno = 0;
	FD_ZERO(&fdset);
	FD_SET(line_buffer->file, &fdset);

	if (line_buffer->timeout == 0) {
	    select_result = select(line_buffer->file + 1, NULL, &fdset, NULL,
		NULL);
	} else {
	    timeval.tv_sec = line_buffer->timeout;
	    timeval.tv_usec = 0;
	    select_result = select(line_buffer->file + 1, NULL, &fdset, NULL,
		&timeval);
	}
	if (select_result < 0) {
	    if (errno == EINTR)
		continue;
	    return -1;
	} else if (select_result == 0) {
	    return -1;
	}

	/*
	 * Read from a file.
	 */
	errno = 0;
	read_result = recv(line_buffer->file, stream_p,
	    stream_length - done_length, 0);
	if (read_result < 0) {
	    if (errno == EINTR)
		continue;
	    return read_result;
	} else if (read_result == 0) {
	    if (done_length == 0) {
		return -1;
	    }
	    return done_length;
	}
	stream_p += read_result;
	done_length += read_result;
    }

    return stream_length;
}


/*
 * Skip too long line read by read_line_buffer().
 *
 * If a line read by read_line_buffer() doesn't contain a newline
 * character, the line is too long.  This function reads and discards
 * the rest of the line.
 *
 * If EOF is received or an error occurs, -1 is returned.
 * Otherwise, 0 is returned.
 */
int
skip_line_buffer(Line_Buffer *line_buffer)
{
    ssize_t line_length;

    /*
     * Read data until the end of the line is found.
     */
    for (;;) {
	line_length = read_line_buffer(line_buffer, line_buffer->buffer,
	    LINEBUF_BUFFER_SIZE);
	if (line_length < 0)
	    return -1;
	if (line_length < LINEBUF_BUFFER_SIZE)
	    break;
    }

    return 0;
}


