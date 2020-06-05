/*
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

#include "build-pre.h"

#ifdef ENABLE_WINSOCK2
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif
#include <sys/time.h>

#include "eb.h"
#include "error.h"
#include "build-post.h"
#include "dummyin6.h"

#if !defined(HAVE_GETADDRINFO) || !defined(HAVE_GETNAMEINFO)
#include "getaddrinfo.h"
#endif

#include "ebnet.h"
#include "linebuf.h"
#include "urlparts.h"

#ifndef IF_NAMESIZE
#ifdef IFNAMSIZ
#define IF_NAMESIZE             IFNAMSIZ
#else
#define IF_NAMESIZE             16
#endif
#endif

#ifndef NI_MAXHOST
#define NI_MAXHOST		1025
#endif

#ifndef SHUT_RD
#define SHUT_RD 0
#endif
#ifndef SHUT_WR
#define SHUT_WR 1
#endif
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

/*
 * Max retry count for establishing a new connection with the server.
 */
#define EBNET_MAX_RETRY_COUNT	1

/*
 * Unexported functions.
 */
static int ebnet_parse_booklist_entry(const char *line, char *book_name,
    char *book_title);
static int ebnet_send_quit(int file);
static int ebnet_parse_url(const char *url, char *host, in_port_t *port,
    char *book_name, char *file_path);
static int is_integer(const char *string);
static int write_string_all(int file, int timeout, const char *string);


/*
 * Initialize ebnet.
 */
void
ebnet_initialize(void)
{
#ifdef ENABLE_WINSOCK2
    WSADATA wsa_data;

    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    ebnet_initialize_multiplex();
    ebnet_set_bye_hook(ebnet_send_quit);
}


/*
 * Get a book list from a server.
 */
EB_Error_Code
ebnet_bind_booklist(EB_BookList *booklist, const char *url)
{
    EB_Error_Code error_code;
    char host[NI_MAXHOST];
    in_port_t port;
    char book_name[EBNET_MAX_BOOK_NAME_LENGTH + 1];
    char book_title[EBNET_MAX_BOOK_TITLE_LENGTH + 1];
    char url_path[EB_MAX_RELATIVE_PATH_LENGTH + 1];
    Line_Buffer line_buffer;
    char line[EBNET_MAX_LINE_LENGTH + 1];
    int ebnet_file = -1;
    ssize_t read_result;
    int lost_sync;
    int retry_count = 0;

    LOG(("in: ebnet_bind_booklist(url=%s)", url));

  retry:
    lost_sync = 0;
    initialize_line_buffer(&line_buffer);
    set_line_buffer_timeout(&line_buffer, EBNET_TIMEOUT_SECONDS);

    /*
     * Parse URL.
     */
    if (ebnet_parse_url(url, host, &port, book_name, url_path) < 0
	|| *book_name != '\0') {
	error_code = EB_ERR_BAD_FILE_NAME;
	goto failed;
    }

    /*
     * Establish a connection.
     */
    ebnet_file = ebnet_connect_socket(host, port, PF_UNSPEC);
    if (ebnet_file < 0) {
	error_code = EB_ERR_EBNET_FAIL_CONNECT;
	goto failed;
    }

    /*
     * Request BOOKLIST.
     */
    bind_file_to_line_buffer(&line_buffer, ebnet_file);
    sprintf(line, "BOOKLIST %s\r\n", book_name);
    if (write_string_all(ebnet_file, EBNET_TIMEOUT_SECONDS, line) <= 0) {
	error_code = EB_ERR_EBNET_FAIL_CONNECT;
	lost_sync = 1;
	goto failed;
    }
    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line) || *line != '!') {
	lost_sync = 1;
	error_code = EB_ERR_EBNET_FAIL_CONNECT;
	goto failed;
    }
    if (strncasecmp(line, "!OK;", 4) != 0) {
	error_code = EB_ERR_EBNET_FAIL_CONNECT;
	goto failed;
    }

    /*
     * Get a list.
     */
    for (;;) {
	read_result = read_line_buffer(&line_buffer, line, sizeof(line));
	if (read_result < 0 || read_result == sizeof(line)) {
	    lost_sync = 1;
	    error_code = EB_ERR_EBNET_FAIL_CONNECT;
	    goto failed;
	}
	if (*line == '\0')
	    break;
	if (ebnet_parse_booklist_entry(line, book_name, book_title) < 0) {
	    lost_sync = 1;
	    error_code = EB_ERR_EBNET_FAIL_CONNECT;
	    goto failed;
	}
	error_code = eb_booklist_add_book(booklist, book_name, book_title);
	if (error_code != EB_SUCCESS)
	    goto failed;
    }

    ebnet_disconnect_socket(ebnet_file);
    finalize_line_buffer(&line_buffer);
    LOG(("out: ebnet_bind_booklist() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    finalize_line_buffer(&line_buffer);
    if (0 <= ebnet_file) {
	if (lost_sync) {
	    shutdown(ebnet_file, SHUT_RDWR);
	    ebnet_set_lost_sync(ebnet_file);
	}
	ebnet_disconnect_socket(ebnet_file);
	ebnet_file = -1;
	if (lost_sync && retry_count < EBNET_MAX_RETRY_COUNT) {
	    retry_count++;
	    goto retry;
	}
    }
    LOG(("out: ebnet_bind_booklist() = %s", eb_error_string(error_code)));
    return error_code;
}


static int
ebnet_parse_booklist_entry(const char *line, char *book_name, char *book_title)
{
    const char *space;
    size_t book_name_length;
    size_t book_title_length;
    char *p;

    space = strchr(line, ' ');
    if (space == NULL)
	return -1;
    book_name_length = space - line;
    book_title_length = strlen(space + 1);

    if (book_name_length == 0
	|| EBNET_MAX_BOOK_NAME_LENGTH < book_name_length)
	return -1;
    if (book_title_length == 0
	|| EBNET_MAX_BOOK_TITLE_LENGTH < book_title_length)
	return -1;

    memcpy(book_name, line, book_name_length);
    *(book_name + book_name_length) = '\0';
    memcpy(book_title, space + 1, book_title_length);
    *(book_title + book_title_length) = '\0';

    for (p = book_name; *p != '\0'; p++) {
	if (!ASCII_ISLOWER(*p) && !ASCII_ISDIGIT(*p)
	    && *p != '_' && *p != '-' && *p != '.')
	    return -1;
    }

    return 0;
}


/*
 * Extension code for eb_bind() to support ebnet.
 */
EB_Error_Code
ebnet_bind(EB_Book *book, const char *url)
{
    EB_Error_Code error_code;
    char host[NI_MAXHOST];
    in_port_t port;
    char book_name[EBNET_MAX_BOOK_NAME_LENGTH + 1];
    char url_path[EB_MAX_RELATIVE_PATH_LENGTH + 1];
    Line_Buffer line_buffer;
    char line[EBNET_MAX_LINE_LENGTH + 1];
    ssize_t read_result;
    int lost_sync;
    int retry_count = 0;

    LOG(("in: ebnet_bind(url=%s)", url));

  retry:
    lost_sync = 0;
    initialize_line_buffer(&line_buffer);
    set_line_buffer_timeout(&line_buffer, EBNET_TIMEOUT_SECONDS);

    /*
     * Parse URL.
     */
    if (ebnet_parse_url(url, host, &port, book_name, url_path) < 0
	|| *book_name == '\0') {
	error_code = EB_ERR_BAD_FILE_NAME;
	goto failed;
    }

    /*
     * Establish a connection.
     */
    book->ebnet_file = ebnet_connect_socket(host, port, PF_UNSPEC);
    if (book->ebnet_file < 0) {
	error_code = EB_ERR_EBNET_FAIL_CONNECT;
	goto failed;
    }

    ebnet_set_book_name(book->ebnet_file, book_name);

    /*
     * Request BOOK.
     */
    bind_file_to_line_buffer(&line_buffer, book->ebnet_file);
    sprintf(line, "BOOK %s\r\n", book_name);
    if (write_string_all(book->ebnet_file, EBNET_TIMEOUT_SECONDS, line) <= 0) {
	error_code = EB_ERR_FAIL_OPEN_CAT;
	lost_sync = 1;
	goto failed;
    }
    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line) || *line != '!') {
	lost_sync = 1;
	error_code = EB_ERR_EBNET_FAIL_CONNECT;
	goto failed;
    }
    if (strncasecmp(line, "!OK;", 4) != 0) {
	if (strncasecmp(line, "!BUSY;", 6) == 0)
	    error_code = EB_ERR_EBNET_SERVER_BUSY;
	else if (strncasecmp(line, "!PERM;", 6) == 0)
	    error_code = EB_ERR_EBNET_NO_PERMISSION;
	else
	    error_code = EB_ERR_FAIL_OPEN_CAT;
	goto failed;
    }

    finalize_line_buffer(&line_buffer);
    LOG(("out: ebnet_bind() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    finalize_line_buffer(&line_buffer);
    if (0 <= book->ebnet_file) {
	if (lost_sync) {
	    shutdown(book->ebnet_file, SHUT_RDWR);
	    ebnet_set_lost_sync(book->ebnet_file);
	}
	ebnet_disconnect_socket(book->ebnet_file);
	book->ebnet_file = -1;
	if (lost_sync && retry_count < EBNET_MAX_RETRY_COUNT) {
	    retry_count++;
	    goto retry;
	}
    }
    LOG(("out: ebnet_bind() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Extension code for eb_bind_appendix() to support ebnet.
 */
EB_Error_Code
ebnet_bind_appendix(EB_Appendix *appendix, const char *url)
{
    EB_Error_Code error_code;
    char host[NI_MAXHOST];
    in_port_t port;
    char appendix_name[EBNET_MAX_BOOK_NAME_LENGTH + 1];
    char url_path[EB_MAX_RELATIVE_PATH_LENGTH + 1];
    Line_Buffer line_buffer;
    char line[EBNET_MAX_LINE_LENGTH + 1];
    ssize_t read_result;
    int lost_sync;
    int retry_count = 0;

    LOG(("in: ebnet_bind(url=%s)", url));

  retry:
    lost_sync = 0;
    initialize_line_buffer(&line_buffer);
    set_line_buffer_timeout(&line_buffer, EBNET_TIMEOUT_SECONDS);

    /*
     * Parse URL.
     */
    if (ebnet_parse_url(url, host, &port, appendix_name, url_path) < 0
	|| *appendix_name == '\0') {
	error_code = EB_ERR_BAD_FILE_NAME;
	goto failed;
    }

    /*
     * Establish a connection.
     */
    appendix->ebnet_file = ebnet_connect_socket(host, port, PF_UNSPEC);
    if (appendix->ebnet_file < 0) {
	error_code = EB_ERR_EBNET_FAIL_CONNECT;
	goto failed;
    }

    ebnet_set_book_name(appendix->ebnet_file, appendix_name);

    /*
     * Request BOOK.
     */
    bind_file_to_line_buffer(&line_buffer, appendix->ebnet_file);
    sprintf(line, "BOOK %s\r\n", appendix_name);
    if (write_string_all(appendix->ebnet_file, EBNET_TIMEOUT_SECONDS, line)
	<= 0) {
	error_code = EB_ERR_FAIL_OPEN_CAT;
	lost_sync = 1;
	goto failed;
    }
    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line) || *line != '!') {
	lost_sync = 1;
	error_code = EB_ERR_EBNET_FAIL_CONNECT;
	goto failed;
    }
    if (strncasecmp(line, "!OK;", 4) != 0) {
	if (strncasecmp(line, "!BUSY;", 6) == 0)
	    error_code = EB_ERR_EBNET_SERVER_BUSY;
	else if (strncasecmp(line, "!PERM;", 6) == 0)
	    error_code = EB_ERR_EBNET_NO_PERMISSION;
	else
	    error_code = EB_ERR_FAIL_OPEN_CAT;
	goto failed;
    }

    finalize_line_buffer(&line_buffer);
    LOG(("out: ebnet_bind() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    finalize_line_buffer(&line_buffer);
    if (0 <= appendix->ebnet_file) {
	if (lost_sync) {
	    shutdown(appendix->ebnet_file, SHUT_RDWR);
	    ebnet_set_lost_sync(appendix->ebnet_file);
	}
	ebnet_disconnect_socket(appendix->ebnet_file);
	appendix->ebnet_file = -1;
	if (lost_sync && retry_count < EBNET_MAX_RETRY_COUNT) {
	    retry_count++;
	    goto retry;
	}
    }
    LOG(("out: ebnet_bind() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Extension code for eb_finalize_book() to support ebnet.
 */
void
ebnet_finalize_book(EB_Book *book)
{
    LOG(("in+out: ebnet_finalize_book(book=%d)", (int)book->code));

    if (0 <= book->ebnet_file) {
	ebnet_disconnect_socket(book->ebnet_file);
	book->ebnet_file = -1;
    }
}


/*
 * Extension code for eb_finalize_appendix() to support ebnet.
 */
void
ebnet_finalize_appendix(EB_Appendix *appendix)
{
    LOG(("in+out: ebnet_finalize_appendix(appendix=%d)", (int)appendix->code));

    if (0 <= appendix->ebnet_file) {
	ebnet_disconnect_socket(appendix->ebnet_file);
	appendix->ebnet_file = -1;
    }
}


/*
 * Extension code for zio_open_raw() to support ebnet.
 */
int
ebnet_open(const char *url)
{
    char host[NI_MAXHOST];
    in_port_t port;
    char book_name[EBNET_MAX_BOOK_NAME_LENGTH + 1];
    char url_path[EB_MAX_RELATIVE_PATH_LENGTH + 1];
    Line_Buffer line_buffer;
    char line[EBNET_MAX_LINE_LENGTH + 1];
    ssize_t read_result;
    off_t file_size;
    int new_file;
    int lost_sync;
    int retry_count = 0;

    LOG(("in: ebnet_open(url=%s)", url));

  retry:
    new_file = -1;
    lost_sync = 0;
    initialize_line_buffer(&line_buffer);
    set_line_buffer_timeout(&line_buffer, EBNET_TIMEOUT_SECONDS);

    /*
     * Parse URL.
     */
    if (ebnet_parse_url(url, host, &port, book_name, url_path) < 0
	|| *book_name == '\0') {
	goto failed;
    }

    /*
     * Connect with a server.
     */
    new_file = ebnet_connect_socket(host, port, PF_UNSPEC);
    if (new_file < 0)
	goto failed;

    ebnet_set_book_name(new_file, book_name);
    ebnet_set_file_path(new_file, url_path);

    /*
     * Request FILESIZE.
     */
    bind_file_to_line_buffer(&line_buffer, new_file);
    sprintf(line, "FILESIZE %s /%s\r\n", book_name, url_path);
    if (write_string_all(new_file, EBNET_TIMEOUT_SECONDS, line) <= 0) {
	lost_sync = 1;
	goto failed;
    }
    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line) || *line != '!') {
	lost_sync = 1;
	goto failed;
    }
    if (strncasecmp(line, "!OK;", 4) != 0)
	goto failed;

    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line) || !is_integer(line)) {
	lost_sync = 1;
	goto failed;
    }

#if defined(HAVE_ATOLL)
     file_size = atoll(line);
#elif defined(HAVE__ATOI64)
    file_size = _atoi64(line);
#else
    file_size = atol(line);
#endif

    if (file_size < 0)
	goto failed;
    ebnet_set_file_size(new_file, file_size);

    finalize_line_buffer(&line_buffer);
    LOG(("out: ebnet_open() = %d", new_file));
    return new_file;

    /*
     * An error occurs...
     */
  failed:
    finalize_line_buffer(&line_buffer);
    if (0 <= new_file) {
	if (lost_sync) {
	    shutdown(new_file, SHUT_RDWR);
	    ebnet_set_lost_sync(new_file);
	}
	ebnet_disconnect_socket(new_file);
	if (lost_sync && retry_count < EBNET_MAX_RETRY_COUNT) {
	    retry_count++;
	    goto retry;
	}
    }
    LOG(("out: ebnet_open() = %d", -1));
    return -1;
}


/*
 * Extension code for zio_close_raw() to support ebnet.
 */
int
ebnet_close(int file)
{
    LOG(("in+out: ebnet_close(file=%d)", file));
    ebnet_disconnect_socket(file);
    return 0;
}


/*
 * Extension code for zio_lseek_raw() to support ebnet.
 */
off_t
ebnet_lseek(int file, off_t offset, int whence)
{
    off_t file_size;
    off_t new_offset = 0;

    LOG(("in: ebnet_lseek(file=%d, location=%ld, whence=%d)", file,
	(long)offset, whence));

    file_size = ebnet_get_file_size(file);
    if (file_size < 0)
	goto failed;

    switch (whence) {
    case SEEK_SET:
	new_offset = offset;
	break;
    case SEEK_CUR:
	new_offset = new_offset + offset;
	break;
    case SEEK_END:
	if (offset <= file_size)
	    new_offset = file_size - offset;
	else
	    new_offset = 0;
	break;
    default:
	goto failed;
    }

    ebnet_set_offset(file, new_offset);
    LOG(("out: ebnet_lseek() = %ld", (long)new_offset));
    return new_offset;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: ebnet_lseek() = %ld", (long)-1));
    return -1;
}


/*
 * Extension code for zio_read_raw() to support ebnet.
 */
ssize_t
ebnet_read(int *file, char *buffer, size_t length)
{
    Line_Buffer line_buffer;
    char line[EBNET_MAX_LINE_LENGTH + 1];
    const char *book_name;
    const char *url_path;
    off_t offset;
    size_t received_length;
    ssize_t read_result;
    ssize_t chunk_length;
    int lost_sync;
    int retry_count = 0;

    LOG(("in: ebnet_read(*file=%d, length=%ld)", *file, (long)length));

    if (length == 0) {
	LOG(("out: ebnet_read() = %ld", (long)0));
	return 0;
    }

  retry:
    lost_sync = 0;
    initialize_line_buffer(&line_buffer);

    /*
     * Request READ.
     */
    book_name = ebnet_get_book_name(*file);
    url_path = ebnet_get_file_path(*file);
    offset = ebnet_get_offset(*file);
    if (book_name == NULL || url_path == NULL || offset < 0)
	goto failed;

    bind_file_to_line_buffer(&line_buffer, *file);
#if defined(PRINTF_LL_MODIFIER)
    sprintf(line, "READ %s /%s %llu %ld\r\n", book_name, url_path,
	(unsigned long long) offset, (long) length);
#elif defined(PRINTF_I64_MODIFIER)
    sprintf(line, "READ %s /%s %I64u %ld\r\n", book_name, url_path,
	(unsigned __int64) offset, (long) length);
#else
    sprintf(line, "READ %s /%s %lu %ld\r\n", book_name, url_path,
	(unsigned long) offset, (long) length);
#endif
    if (write_string_all(*file, EBNET_TIMEOUT_SECONDS, line) <= 0) {
	lost_sync = 1;
	goto failed;
    }
    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line) || *line != '!') {
	lost_sync = 1;
	goto failed;
    }
    if (strncasecmp(line, "!OK;", 4) != 0)
	goto failed;

    received_length = 0;
    while (received_length < length) {
	read_result = read_line_buffer(&line_buffer, line, sizeof(line));
	if (read_result < 0 || read_result == sizeof(line) || *line != '*') {
	    lost_sync = 1;
	    goto failed;
	}

	if (!is_integer(line + 1)) {
	    lost_sync = 1;
	    goto failed;
	} else if (strcmp(line + 1, "-1") == 0) {
	  ebnet_set_offset(*file, (off_t) offset + received_length);
	    goto failed;
	} else if (strcmp(line + 1, "0") == 0) {
	    break;
	}
	chunk_length = atoi(line + 1);
	if (chunk_length <= 0 || length < received_length + chunk_length) {
	    lost_sync = 1;
	    goto failed;
	}

	read_result = binary_read_line_buffer(&line_buffer,
	    buffer + received_length, chunk_length);
	if (read_result != chunk_length) {
	    lost_sync = 1;
	    goto failed;
	}
	received_length += chunk_length;
    }

    ebnet_set_offset(*file, (off_t) offset + received_length);
    finalize_line_buffer(&line_buffer);
    LOG(("out: ebnet_read(*file=%d) = %ld", *file, (long)received_length));
    return received_length;

    /*
     * An error occurs...
     */
  failed:
    finalize_line_buffer(&line_buffer);
    if (lost_sync) {
	shutdown(*file, SHUT_RDWR);
	ebnet_set_lost_sync(*file);
	if (retry_count < EBNET_MAX_RETRY_COUNT) {
	    int new_file = ebnet_reconnect_socket(*file);
	    if (0 <= new_file) {
		*file = new_file;
		retry_count++;
		goto retry;
	    }
	}
    }
    LOG(("out: ebnet_read(*file=%d) = %ld", *file, (long)-1));
    return -1;
}


/*
 * Extension code for eb_fix_directory_name() to support ebnet.
 */
EB_Error_Code
ebnet_fix_directory_name(const char *url, char *directory_name)
{
    char host[NI_MAXHOST];
    in_port_t port;
    char book_name[EBNET_MAX_BOOK_NAME_LENGTH + 1];
    char url_path[EB_MAX_RELATIVE_PATH_LENGTH + 1];
    Line_Buffer line_buffer;
    char line[EBNET_MAX_LINE_LENGTH + 1];
    ssize_t read_result;
    int new_file;
    int lost_sync;
    int retry_count = 0;

  retry:
    new_file = -1;
    lost_sync = 0;
    initialize_line_buffer(&line_buffer);
    set_line_buffer_timeout(&line_buffer, EBNET_TIMEOUT_SECONDS);

    /*
     * Parse URL.
     */
    if (ebnet_parse_url(url, host, &port, book_name, url_path) < 0
	|| *book_name == '\0') {
	goto failed;
    }

    /*
     * Connect with a server.
     */
    new_file = ebnet_connect_socket(host, port, PF_UNSPEC);
    if (new_file < 0)
	goto failed;

    /*
     * Request DIR.
     */
    bind_file_to_line_buffer(&line_buffer, new_file);
    sprintf(line, "DIR %s /%s %s\r\n", book_name, url_path, directory_name);
    if (write_string_all(new_file, EBNET_TIMEOUT_SECONDS, line) <= 0) {
	lost_sync = 1;
	goto failed;
    }
    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line) || *line != '!') {
	lost_sync = 1;
	goto failed;
    }
    if (strncasecmp(line, "!OK;", 4) != 0)
	goto failed;

    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line)) {
	lost_sync = 1;
	goto failed;
    }

    if (*line == '\0' || EB_MAX_DIRECTORY_NAME_LENGTH < strlen(line))
	goto failed;
    strcpy(directory_name, line);

    finalize_line_buffer(&line_buffer);
    ebnet_disconnect_socket(new_file);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    finalize_line_buffer(&line_buffer);
    if (0 <= new_file) {
	if (lost_sync) {
	    shutdown(new_file, SHUT_RDWR);
	    ebnet_set_lost_sync(new_file);
	}
	ebnet_disconnect_socket(new_file);
	if (lost_sync && retry_count < EBNET_MAX_RETRY_COUNT) {
	    retry_count++;
	    goto retry;
	}
    }
    return EB_ERR_BAD_DIR_NAME;
}


/*
 * Extension code for eb_find_file_name() to support ebnet.
 */
EB_Error_Code
ebnet_find_file_name(const char *url, const char *target_file_name,
    char *found_file_name)
{
    char host[NI_MAXHOST];
    in_port_t port;
    char book_name[EBNET_MAX_BOOK_NAME_LENGTH + 1];
    char url_path[EB_MAX_RELATIVE_PATH_LENGTH + 1];
    Line_Buffer line_buffer;
    char line[EBNET_MAX_LINE_LENGTH + 1];
    ssize_t read_result;
    int new_file;
    int lost_sync;
    int retry_count = 0;

  retry:
    new_file = -1;
    lost_sync = 0;
    initialize_line_buffer(&line_buffer);
    set_line_buffer_timeout(&line_buffer, EBNET_TIMEOUT_SECONDS);

    /*
     * Parse URL.
     */
    if (ebnet_parse_url(url, host, &port, book_name, url_path) < 0
	|| *book_name == '\0') {
	goto failed;
    }

    /*
     * Connect with a server.
     */
    new_file = ebnet_connect_socket(host, port, PF_UNSPEC);
    if (new_file < 0)
	goto failed;

    /*
     * Request FILE.
     */
    bind_file_to_line_buffer(&line_buffer, new_file);
    sprintf(line, "FILE %s /%s %s\r\n", book_name, url_path, target_file_name);
    if (write_string_all(new_file, EBNET_TIMEOUT_SECONDS, line) <= 0) {
	lost_sync = 1;
	goto failed;
    }
    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line) || *line != '!') {
	lost_sync = 1;
	goto failed;
    }
    if (strncasecmp(line, "!OK;", 4) != 0)
	goto failed;

    read_result = read_line_buffer(&line_buffer, line, sizeof(line));
    if (read_result < 0 || read_result == sizeof(line)) {
	lost_sync = 1;
	goto failed;
    }

    if (*line == '\0' || EB_MAX_FILE_NAME_LENGTH < strlen(line))
	goto failed;
    strcpy(found_file_name, line);

    finalize_line_buffer(&line_buffer);
    ebnet_disconnect_socket(new_file);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    finalize_line_buffer(&line_buffer);
    if (0 <= new_file) {
	if (lost_sync) {
	    shutdown(new_file, SHUT_RDWR);
	    ebnet_set_lost_sync(new_file);
	}
	ebnet_disconnect_socket(new_file);
	if (lost_sync && retry_count < EBNET_MAX_RETRY_COUNT) {
	    retry_count++;
	    goto retry;
	}
    }
    return EB_ERR_BAD_FILE_NAME;
}


/*
 * Bye hook.
 * We must send `QUIT' command before close a connection.
 */
static int
ebnet_send_quit(int file)
{
    if (write_string_all(file, EBNET_TIMEOUT_SECONDS, "QUIT\r\n") <= 0)
	return -1;
    return 0;
}


/*
 * URL version of eb_canonicalize_path_name().
 */
EB_Error_Code
ebnet_canonicalize_url(char *url)
{
    char host[NI_MAXHOST];
    in_port_t port;
    char book_name[EBNET_MAX_BOOK_NAME_LENGTH + 1];
    char url_path[EB_MAX_RELATIVE_PATH_LENGTH + 1];

    if (ebnet_parse_url(url, host, &port, book_name, url_path) < 0
	|| *book_name == '\0') {
	return EB_ERR_BAD_FILE_NAME;
    }

    /*
     * "ebnet://[<host>]:<port>/<book_name>" must not exceed
     * EB_MAX_PATH_LENGTH.
     *
     * Note:
     *   "ebnet://[" + "]:" + "/" = 12 characters.
     *    <port> is 5 characters maximum.
     */
    if (EB_MAX_PATH_LENGTH
	< strlen(host) + strlen(book_name) + strlen(url_path) + 17)
	return EB_ERR_TOO_LONG_FILE_NAME;

    if (strchr(host, ':') != NULL)
	sprintf(url, "ebnet://[%s]:%d/%s", host, (int)port, book_name);
    else
	sprintf(url, "ebnet://%s:%d/%s", host, (int)port, book_name);

    return EB_SUCCESS;
}


/*
 * Parse URL.
 *
 * As the result, host, port, book_name and file-path are put into
 * the corresponding arguments.
 */
static int
ebnet_parse_url(const char *url, char *host, in_port_t *port,
    char *book_name, char *file_path)
{
    URL_Parts parts;
    const char *scheme_part;
    const char *host_part;
    const char *port_part;
    const char *port_part_p;
    const char *path_part;
    const char *slash;
    size_t book_name_length;

    *host = '\0';
    *port = 0;
    *book_name = '\0';
    *file_path = '\0';

    url_parts_initialize(&parts);
    if (url_parts_parse(&parts, url) < 0)
	goto failed;

    /*
     * Check scheme.
     */
    scheme_part = url_parts_scheme(&parts);
    if (scheme_part == NULL || strcmp(scheme_part, EBNET_SERVICE_NAME) != 0)
	goto failed;

    /*
     * Check host.
     */
    host_part = url_parts_host(&parts);
    if (host_part == NULL || *host_part == '\0'
	|| NI_MAXHOST < strlen(host_part) + 1)
	goto failed;
    strcpy(host, host_part);

    /*
     * Check port.
     */
    port_part = url_parts_port(&parts);
    if (port_part == NULL || *port_part == '\0')
	port_part = EBNET_DEFAULT_PORT;

    for (port_part_p = port_part; *port_part_p != '\0'; port_part_p++) {
	if (!ASCII_ISDIGIT(*port_part_p))
	    goto failed;
    }
    *port = atoi(port_part);

    /*
     * Check path.
     */
    path_part = url_parts_path(&parts);
    if (path_part == NULL || *path_part == '\0') {
	*book_name = '\0';
	*file_path = '\0';
    } else {
	slash = strchr(path_part + 1, '/');
	if (slash == NULL) {
	    book_name_length = strlen(path_part + 1);
	    if (EBNET_MAX_BOOK_NAME_LENGTH < book_name_length)
		goto failed;
	    strcpy(book_name, path_part + 1);
	    *file_path = '\0';
	} else {
	    book_name_length = slash - (path_part + 1);
	    if (book_name_length == 0
		|| EBNET_MAX_BOOK_NAME_LENGTH < book_name_length)
		goto failed;
	    memcpy(book_name, path_part + 1, book_name_length);
	    *(book_name + book_name_length) = '\0';

	    if (EB_MAX_RELATIVE_PATH_LENGTH < strlen(slash + 1))
		goto failed;
	    strcpy(file_path, slash + 1);
	}
    }

    url_parts_finalize(&parts);
    return 0;

    /*
     * An error occurs...
     */
  failed:
    *host = '\0';
    *port = 0;
    *book_name = '\0';
    *file_path = '\0';
    url_parts_finalize(&parts);
    return -1;
}


/*
 * Check whether `string' is integer or not.
 * Return 1 if it is, 0 otherwise.
 */
static int
is_integer(const char *string)
{
    const char *s = string;

    if (*s == '-')
	s++;
    if (!ASCII_ISDIGIT(*s))
	return 0;
    s++;

    while (*s != '\0') {
	if (!ASCII_ISDIGIT(*s))
	    return 0;
	s++;
    }

    return 1;
}


/*
 * Write data to a file.
 * It repeats to call write() until all data will have written.
 * The function returns 1 upon success, 0 upon timeout, -1 upon error.
 */
static int
write_string_all(int file, int timeout, const char *string)
{
    const char *string_p = string;
    ssize_t rest_length = strlen(string);
    fd_set fdset;
    struct timeval timeval;
    int select_result;
    ssize_t write_result;

    while (0 < rest_length) {
	errno = 0;
	FD_ZERO(&fdset);
	FD_SET(file, &fdset);

	if (timeout == 0)
	    select_result = select(file + 1, NULL, &fdset, NULL, NULL);
	else {
	    timeval.tv_sec = timeout;
	    timeval.tv_usec = 0;
	    select_result = select(file + 1, NULL, &fdset, NULL, &timeval);
	}
	if (select_result < 0) {
	    if (errno == EINTR)
		continue;
	    return -1;
	} else if (select_result == 0) {
	    return 0;
	}

	errno = 0;
	write_result = send(file, string_p, rest_length, 0);
	if (write_result < 0) {
	    if (errno == EINTR)
		continue;
	    return -1;
	} else {
	    rest_length -= write_result;
	    string_p += write_result;
	}
    }

    return 1;
}


