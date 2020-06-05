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

#include "build-pre.h"
#include "eb.h"
#include "error.h"
#ifdef ENABLE_EBNET
#include "ebnet.h"
#endif
#include "build-post.h"

/*
 * Initial value of `max_entry_count' in `EB_BookList'.
 */
#define EB_INITIAL_BOOKLIST_MAX_ENTRY_COUNT	16

/*
 * BookList ID counter.
 */
static EB_Book_Code booklist_counter = 0;

/*
 * Mutex for `booklist_counter'.
 */
#ifdef ENABLE_PTHREAD
static pthread_mutex_t booklist_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif


/*
 * Initialize a book list.
 */
void
eb_initialize_booklist(EB_BookList *booklist)
{
    LOG(("in: eb_initialize_booklist()"));

    booklist->entry_count = 0;
    booklist->max_entry_count = 0;
    booklist->entries = NULL;
    eb_initialize_lock(&booklist->lock);

    LOG(("out: eb_initialize_booklist()"));
}


/*
 * Finalize a book list.
 */
void
eb_finalize_booklist(EB_BookList *booklist)
{
    int i;

    LOG(("in: eb_finalize_booklist()"));

    if (booklist->entries != NULL) {
	for (i = 0; i < booklist->entry_count; i++) {
	    free(booklist->entries[i].name);
	    free(booklist->entries[i].title);
	}
	free(booklist->entries);
	booklist->entries = NULL;
    }
    booklist->entry_count = 0;
    booklist->max_entry_count = 0;

    LOG(("out: eb_finalize_booklist()"));
}


/*
 * Get a list of books from an EBNET server.
 */
EB_Error_Code
eb_bind_booklist(EB_BookList *booklist, const char *path)
{
    EB_Error_Code error_code;
    int i;

    eb_lock(&booklist->lock);
    LOG(("in: eb_bind_booklist(path=%s)", path));

    pthread_mutex_lock(&booklist_counter_mutex);
    booklist->code = booklist_counter++;
    pthread_mutex_unlock(&booklist_counter_mutex);

#ifndef ENABLE_EBNET
    error_code = EB_ERR_EBNET_UNSUPPORTED;
    goto failed;
#endif
    if (!is_ebnet_url(path)) {
	error_code = EB_ERR_BAD_FILE_NAME;
	goto failed;
    }
    for (i = 0; i < booklist->entry_count; i++) {
	free(booklist->entries[i].name);
	free(booklist->entries[i].title);
    }

    error_code = ebnet_bind_booklist(booklist, path);
    if (error_code != EB_SUCCESS)
	goto failed;

    LOG(("out: eb_bind_booklist(book=%d) = %s", (int)booklist->code,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&booklist->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_finalize_booklist(booklist);
    LOG(("out: eb_bind_booklist() = %s", eb_error_string(error_code)));
    eb_unlock(&booklist->lock);
    return error_code;
}


/*
 * Add a book entry to `booklist'.
 */
EB_Error_Code
eb_booklist_add_book(EB_BookList *booklist, const char *name,
    const char *title)
{
    int new_max_entry_count;
    EB_BookList_Entry *new_entries;
    char *new_name = NULL;
    char *new_title = NULL;
    EB_Error_Code error_code;

    LOG(("in: eb_booklist_add_book(name=%s, title=%s)", name, title));

    if (booklist->entry_count == booklist->max_entry_count) {
	if (booklist->max_entry_count == 0) {
	    new_max_entry_count = EB_INITIAL_BOOKLIST_MAX_ENTRY_COUNT;
	    new_entries = (EB_BookList_Entry *)
		malloc(sizeof(EB_BookList_Entry) * new_max_entry_count);
	} else {
	    new_max_entry_count = booklist->max_entry_count * 2;
	    new_entries = (EB_BookList_Entry *)realloc(booklist->entries,
		sizeof(EB_BookList_Entry) * new_max_entry_count);
	}
	if (new_entries == NULL) {
	    error_code = EB_ERR_MEMORY_EXHAUSTED;
	    goto failed;
	}
	booklist->max_entry_count = new_max_entry_count;
	booklist->entries = new_entries;
    }

    new_name = (char *)malloc(strlen(name) + 1);
    if (new_name == NULL) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }
    strcpy(new_name, name);

    new_title = (char *)malloc(strlen(title) + 1);
    if (new_title == NULL) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }
    strcpy(new_title, title);

    booklist->entries[booklist->entry_count].name = new_name;
    booklist->entries[booklist->entry_count].title = new_title;
    booklist->entry_count++;

    LOG(("out: eb_booklist_add_book() = %s", eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    if (new_name != NULL)
	free(new_name);
    if (new_title != NULL)
	free(new_title);

    LOG(("out: eb_booklist_book_add() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return the number of books in `booklist'.
 */
EB_Error_Code
eb_booklist_book_count(EB_BookList *booklist, int *book_count)
{
    EB_Error_Code error_code;

    eb_lock(&booklist->lock);
    LOG(("in: eb_booklist_book_count(booklist=%d)", (int)booklist->code));

    if (booklist->entries == NULL) {
	error_code = EB_ERR_UNBOUND_BOOKLIST;
	goto failed;
    }
    *book_count = booklist->entry_count;

    LOG(("out: eb_booklist_book_count(count=%d) = %s", *book_count,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&booklist->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_booklist_book_count() = %s", eb_error_string(error_code)));
    eb_unlock(&booklist->lock);
    return error_code;
}


/*
 * Return title of a book entry in `booklist'.
 */
EB_Error_Code
eb_booklist_book_name(EB_BookList *booklist, int book_index, char **book_name)
{
    EB_Error_Code error_code;

    eb_lock(&booklist->lock);
    LOG(("in: eb_booklist_book_name(booklist=%d,index=%d)",
	(int)booklist->code, book_index));

    if (booklist->entries == NULL) {
	error_code = EB_ERR_UNBOUND_BOOKLIST;
	goto failed;
    }
    if (book_index < 0 || booklist->entry_count <= book_index) {
	error_code = EB_ERR_NO_SUCH_BOOK;
	goto failed;
    }

    *book_name = booklist->entries[book_index].name;

    LOG(("out: eb_booklist_book_name(*book_name=%s) = %s",
	(*book_name == NULL) ? "NULL" : *book_name,
	eb_error_string(EB_SUCCESS)));

    eb_unlock(&booklist->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_booklist_book_name() = %s", eb_error_string(error_code)));
    eb_unlock(&booklist->lock);
    return error_code;
}


/*
 * Return name of a book entry in `booklist'.
 */
EB_Error_Code
eb_booklist_book_title(EB_BookList *booklist, int book_index,
    char **book_title)
{
    EB_Error_Code error_code;

    eb_lock(&booklist->lock);
    LOG(("in: eb_booklist_book_title(booklist=%d,index=%d)",
	(int)booklist->code, book_index));

    if (booklist->entries == NULL) {
	error_code = EB_ERR_UNBOUND_BOOKLIST;
	goto failed;
    }
    if (book_index < 0 || booklist->entry_count <= book_index) {
	error_code = EB_ERR_NO_SUCH_BOOK;
	goto failed;
    }
    *book_title = booklist->entries[book_index].title;

    LOG(("out: eb_booklist_book_title(*book_title=%s) = %s",
	(*book_title == NULL) ? "NULL" : *book_title,
	eb_error_string(EB_SUCCESS)));

    eb_unlock(&booklist->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_booklist_book_title() = %s", eb_error_string(error_code)));
    eb_unlock(&booklist->lock);
    return error_code;
}


