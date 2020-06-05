/*
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

#include "build-pre.h"
#include "eb.h"
#include "build-post.h"

#include <stdarg.h>


/*
 * Mutex.
 */
#ifdef ENABLE_PTHREAD
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * Initialization flag.
 */
int eb_log_initialized = 0;

/*
 * Debug log flag.
 */
int eb_log_flag = 0;

/*
 * Pointer to log function.
 */
static void (*eb_log_function)(const char *message, va_list) = eb_log_stderr;


/*
 * Initialize logging sub-system.
 */
void
eb_initialize_log(void)
{
    if (eb_log_initialized)
	return;

    eb_log_flag = (getenv(EB_DEBUG_ENVIRONMENT_VARIABLE) != NULL);
    eb_log_function = eb_log_stderr;
    eb_log_initialized = 1;
}

/*
 * Set log function.
 */
void
eb_set_log_function(void (*function)(const char *message, va_list ap))
{
    if (!eb_log_initialized)
	eb_initialize_log();
    eb_log_function = function;
}

/*
 * Enable logging.
 */
void
eb_enable_log(void)
{
    if (!eb_log_initialized)
	eb_initialize_log();
    eb_log_flag = 1;
}

/*
 * Disable logging.
 */
void
eb_disable_log(void)
{
    if (!eb_log_initialized)
	eb_initialize_log();
    eb_log_flag = 0;
}

/*
 * Log a message.
 */
void
eb_log(const char *message, ...)
{
    va_list ap;

    va_start(ap, message);

    if (eb_log_flag && eb_log_function != NULL)
	eb_log_function(message, ap);

    va_end(ap);
}

/*
 * Output a log message to standard error.
 * This is the default log handler.
 *
 * Currently, this function doesn't work if the system lacks vprintf()
 * and dopront().
 */
void
eb_log_stderr(const char *message, va_list ap)
{
    pthread_mutex_lock(&log_mutex);

    fputs("[EB] ", stderr);

    vfprintf(stderr, message, ap);
    fputc('\n', stderr);
    fflush(stderr);

    pthread_mutex_unlock(&log_mutex);
}

#define MAX_QUOTED_STREAM_LENGTH	100

/*
 * Return Quoted printable string of `stream'.
 */
const char *
eb_quoted_stream(const char *stream, size_t stream_length)
{
    static char quoted_streams[EB_MAX_KEYWORDS][MAX_QUOTED_STREAM_LENGTH + 3];
    static int current_index = 0;
    unsigned char *quoted_p;
    const unsigned char *stream_p;
    size_t quoted_length = 0;
    int i;

    current_index = (current_index + 1) % EB_MAX_KEYWORDS;
    quoted_p = (unsigned char *)quoted_streams[current_index];
    stream_p = (const unsigned char *)stream;

    if (stream == NULL)
	return "";

    for (i = 0; i < stream_length && *stream_p != '\0'; i++) {
	if (0x20 <= *stream_p && *stream_p <= 0x7f && *stream_p != '=') {
	    if (MAX_QUOTED_STREAM_LENGTH < quoted_length + 1) {
		*quoted_p++ = '.';
		*quoted_p++ = '.';
		break;
	    }
	    *quoted_p++ = *stream_p;
	    quoted_length++;
	} else {
	    if (MAX_QUOTED_STREAM_LENGTH < quoted_length + 3) {
		*quoted_p++ = '.';
		*quoted_p++ = '.';
		break;
	    }
	    *quoted_p++ = '=';
	    *quoted_p++ = "0123456789ABCDEF" [*stream_p / 0x10];
	    *quoted_p++ = "0123456789ABCDEF" [*stream_p % 0x10];
	    quoted_length += 3;
	}
	stream_p++;
    }

    *quoted_p = '\0';
    return quoted_streams[current_index];
}


/*
 * Return Quoted printable string.
 */
const char *
eb_quoted_string(const char *string)
{
    return eb_quoted_stream(string, strlen(string));
}
