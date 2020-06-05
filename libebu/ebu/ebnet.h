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

#ifndef EBNET_H
#define EBNET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include "eb.h"

/*
 * Service name.
 */
#define EBNET_SERVICE_NAME		"ebnet"

/*
 * Default port number.
 */
#define EBNET_DEFAULT_PORT		"22010"

/*
 * Maximum length of book name.
 * EBNETD defins 14, but we add +4 for the ".app" suffix here.
 */
#define EBNET_MAX_BOOK_NAME_LENGTH	18

/*
 * Maximum length of book title.
 * EBNETD defins 80, and EB Library uses the same value.
 */
#define EBNET_MAX_BOOK_TITLE_LENGTH	80

/*
 * Maximum length of an EBNET request or response line.
 */
#define EBNET_MAX_LINE_LENGTH		511

/*
 * Timeout period in seconds.
 */
#define EBNET_TIMEOUT_SECONDS		30

/*
 * Function declarations.
 */
/* multiplex.c */
void ebnet_initialize_multiplex(void);
void ebnet_finalize(void);
void ebnet_set_hello_hook(int (*hook)(int file));
void ebnet_set_bye_hook(int (*hook)(int file));
int ebnet_connect_socket(const char *host, int port, int family);
void ebnet_disconnect_socket(int file);
int ebnet_reconnect_socket(int file);
int ebnet_set_lost_sync(int file);
int ebnet_set_book_name(int file, const char *book_name);
const char *ebnet_get_book_name(int file);
int ebnet_set_file_path(int file, const char *file_path);
const char *ebnet_get_file_path(int file);
int ebnet_set_offset(int file, off_t offset);
off_t ebnet_get_offset(int file);
int ebnet_set_file_size(int file, off_t file_size);
off_t ebnet_get_file_size(int file);

/* ebnet.c */
void ebnet_initialize(void);
EB_Error_Code ebnet_bind_booklist(EB_BookList *booklist, const char *url);
EB_Error_Code ebnet_bind(EB_Book *book, const char *url);
EB_Error_Code ebnet_bind_appendix(EB_Appendix *appendix, const char *url);
void ebnet_finalize_book(EB_Book *book);
void ebnet_finalize_appendix(EB_Appendix *appendix);
int ebnet_open(const char *url);
int ebnet_close(int file);
off_t ebnet_lseek(int file, off_t offset, int whence);
ssize_t ebnet_read(int *file, char *buffer, size_t length);
EB_Error_Code ebnet_fix_directory_name(const char *url, char *directory_name);
EB_Error_Code ebnet_find_file_name(const char *url,
    const char *target_file_name, char *found_file_name);
EB_Error_Code ebnet_canonicalize_url(char *url);

#endif /* EBNET_H */
