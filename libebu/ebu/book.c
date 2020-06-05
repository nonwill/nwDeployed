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

#include "build-pre.h"
#include "eb.h"
#include "error.h"
#include "font.h"
#ifdef ENABLE_EBNET
#include "ebnet.h"
#endif
#include "build-post.h"

/*
 * Book ID counter.
 */
static EB_Book_Code book_counter = 0;

/*
 * Mutex for `book_counter'.
 */
#ifdef ENABLE_PTHREAD
static pthread_mutex_t book_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * Unexported functions.
 */
static void eb_fix_misleaded_book(EB_Book *book);
static EB_Error_Code eb_load_catalog(EB_Book *book);
static EB_Error_Code eb_load_catalog_eb(EB_Book *book,
    const char *catalog_path);
static EB_Error_Code eb_load_catalog_epwing(EB_Book *book,
    const char *catalog_path);
static Zio_Code eb_get_hint_zio_code(int catalog_hint_value);
static void eb_load_language(EB_Book *book);


/*
 * Initialize `book'.
 */
void
eb_initialize_book(EB_Book *book)
{
    LOG(("in: eb_initialize_book()"));

    book->code = EB_BOOK_NONE;
    book->disc_code = EB_DISC_INVALID;
    book->character_code = EB_CHARCODE_INVALID;
    book->path = NULL;
    book->path_length = 0;
    book->subbooks = NULL;
    book->subbook_current = NULL;
#ifdef ENABLE_EBNET
    book->ebnet_file = -1;
#endif
    eb_initialize_text_context(book);
    eb_initialize_binary_context(book);
    eb_initialize_search_contexts(book);
    eb_initialize_binary_context(book);
    eb_initialize_lock(&book->lock);

    LOG(("out: eb_initialize_book()"));
}


/*
 * Bind `book' to `path'.
 */
EB_Error_Code
eb_bind(EB_Book *book, const char *path)
{
    EB_Error_Code error_code;
    char temporary_path[EB_MAX_PATH_LENGTH + 1];
    int is_ebnet;

    eb_lock(&book->lock);
    LOG(("in: eb_bind(path=%s)", path));

    /*
     * Clear the book if the book has already been bound.
     */
    if (book->path != NULL) {
	eb_finalize_book(book);
	eb_initialize_book(book);
    }

    /*
     * Assign a book code.
     */
    pthread_mutex_lock(&book_counter_mutex);
    book->code = book_counter++;
    pthread_mutex_unlock(&book_counter_mutex);

    /*
     * Check whether `path' is URL.
     */
    is_ebnet = is_ebnet_url(path);
#ifndef ENABLE_EBNET
    if (is_ebnet) {
	error_code = EB_ERR_EBNET_UNSUPPORTED;
	goto failed;
    }
#endif

    /*
     * Set the path of the book.
     * The length of the file name "<path>/subdir/subsubdir/file.ebz;1" must
     * be EB_MAX_PATH_LENGTH maximum.
     */
    if (EB_MAX_PATH_LENGTH < strlen(path)) {
	error_code = EB_ERR_TOO_LONG_FILE_NAME;
	goto failed;
    }
    strcpy(temporary_path, path);
#ifdef ENABLE_EBNET
    if (is_ebnet)
	error_code = ebnet_canonicalize_url(temporary_path);
    else
	error_code = eb_canonicalize_path_name(temporary_path);
#else
    error_code = eb_canonicalize_path_name(temporary_path);
#endif
    if (error_code != EB_SUCCESS)
	goto failed;

    book->path_length = strlen(temporary_path);
    if (EB_MAX_PATH_LENGTH
	< book->path_length + 1 + EB_MAX_RELATIVE_PATH_LENGTH) {
	error_code = EB_ERR_TOO_LONG_FILE_NAME;
	goto failed;
    }

    book->path = (char *)malloc(book->path_length + 1);
    if (book->path == NULL) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }
    strcpy(book->path, temporary_path);

    /*
     * Establish a connection with a ebnet server.
     */
#ifdef ENABLE_EBNET
    if (is_ebnet) {
	error_code = ebnet_bind(book, book->path);
	if (error_code != EB_SUCCESS)
	    goto failed;
    }
#endif

    /*
     * Read information from the `LANGUAGE' file.
     * If failed to initialize, JIS X 0208 is assumed.
     */
    eb_load_language(book);

    /*
     * Read information from the `CATALOG(S)' file.
     */
    error_code = eb_load_catalog(book);
    if (error_code != EB_SUCCESS)
	goto failed;

    LOG(("out: eb_bind(book=%d) = %s", (int)book->code,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_finalize_book(book);
    LOG(("out: eb_bind() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Finish using `book'.
 */
void
eb_finalize_book(EB_Book *book)
{
    LOG(("in: eb_finalize_book(book=%d)", (int)book->code));

    eb_unset_subbook(book);

    if (book->subbooks != NULL) {
	eb_finalize_subbooks(book);
	free(book->subbooks);
	book->subbooks = NULL;
    }

    book->subbook_current = NULL;
    eb_finalize_text_context(book);
    eb_finalize_binary_context(book);
    eb_finalize_search_contexts(book);
    eb_finalize_binary_context(book);
    eb_finalize_lock(&book->lock);

#ifdef ENABLE_EBNET
    ebnet_finalize_book(book);
#endif

    if (book->path != NULL)
	free(book->path);

    book->code = EB_BOOK_NONE;
    book->disc_code = EB_DISC_INVALID;
    book->character_code = EB_CHARCODE_INVALID;
    book->path = NULL;
    book->path_length = 0;

    LOG(("out: eb_finalize_book()"));
}


/*
 * There are some books that EB Library sets wrong character code of
 * the book.  They are written in JIS X 0208, but the library sets
 * ISO 8859-1.
 *
 * We fix the character of the books.  The following table lists
 * titles of the first subbook in those books.
 */
static const char * const misleaded_book_table[] = {
    /* SONY DataDiskMan (DD-DR1) accessories. */
    "%;%s%A%e%j!\\%S%8%M%9!\\%/%i%&%s",

    /* Shin Eiwa Waei Chujiten (earliest edition) */
    "8&5f<R!!?71QOBCf<-E5",

    /* EB Kagakugijutsu Yougo Daijiten (YRRS-048) */
    "#E#B2J3X5;=QMQ8lBg<-E5",

    /* Nichi-Ei-Futsu Jiten (YRRS-059) */
    "#E#N#G!?#J#A#N!J!\\#F#R#E!K",

    /* Japanese-English-Spanish Jiten (YRRS-060) */
    "#E#N#G!?#J#A#N!J!\\#S#P#A!K",

     /* Panasonic KX-EBP2 accessories. */
    "%W%m%7!<%I1QOB!&OB1Q<-E5",
    NULL
};

/*
 * Fix chachacter-code of the book if misleaded.
 */
static void
eb_fix_misleaded_book(EB_Book *book)
{
    const char * const * misleaded;
    EB_Subbook *subbook;
    int i;

    LOG(("in: eb_fix_misleaded_book(book=%d)", (int)book->code));

    for (misleaded = misleaded_book_table; *misleaded != NULL; misleaded++) {
	if (strcmp(book->subbooks[0].title, *misleaded) == 0) {
	    book->character_code = EB_CHARCODE_JISX0208;
	    for (i = 0, subbook = book->subbooks; i < book->subbook_count;
		 i++, subbook++) {
		eb_jisx0208_to_euc(subbook->title, subbook->title);
	    }
	    break;
	}
    }

    LOG(("out: eb_fix_misleaded_book()"));
}

/*
 * Read information from the `CATALOG(S)' file in 'book'.
 * Return EB_SUCCESS if it succeeds, error-code otherwise.
 */
static EB_Error_Code
eb_load_catalog(EB_Book *book)
{
    EB_Error_Code error_code;
    char catalog_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
    char catalog_path_name[EB_MAX_PATH_LENGTH + 1];

    LOG(("in: eb_load_catalog(book=%d)", (int)book->code));

    /*
     * Find a catalog file.
     */
    if (eb_find_file_name(book->path, "catalog", catalog_file_name)
	== EB_SUCCESS) {
	book->disc_code = EB_DISC_EB;
    } else if (eb_find_file_name(book->path, "catalogs", catalog_file_name)
	== EB_SUCCESS) {
	book->disc_code = EB_DISC_EPWING;
    } else {
	error_code = EB_ERR_FAIL_OPEN_CAT;
	goto failed;
    }

    eb_compose_path_name(book->path, catalog_file_name, catalog_path_name);

    /*
     * Load the catalog file.
     */
    if (book->disc_code == EB_DISC_EB)
	error_code = eb_load_catalog_eb(book, catalog_path_name);
    else
	error_code = eb_load_catalog_epwing(book, catalog_path_name);
    if (error_code != EB_SUCCESS)
	goto failed;

    /*
     * Fix chachacter-code of the book.
     */
    eb_fix_misleaded_book(book);
    LOG(("out: eb_load_catalog() = %s", eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    if (book->subbooks != NULL) {
	free(book->subbooks);
	book->subbooks = NULL;
    }
    LOG(("out: eb_load_catalog() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Read information from the `CATALOG' file in 'book'. (EB)
 */
static EB_Error_Code
eb_load_catalog_eb(EB_Book *book, const char *catalog_path)
{
    EB_Error_Code error_code;
    char buffer[EB_SIZE_PAGE];
    char *space;
    EB_Subbook *subbook;
    Zio zio;
    Zio_Code zio_code;
    int i;

    LOG(("in: eb_load_catalog_eb(book=%d, catalog=%s)",
	(int)book->code, catalog_path));

    zio_initialize(&zio);

    /*
     * Open a catalog file.
     */
    eb_path_name_zio_code(catalog_path, ZIO_PLAIN, &zio_code);
    if (zio_open(&zio, catalog_path, zio_code) < 0) {
	error_code = EB_ERR_FAIL_OPEN_CAT;
	goto failed;
    }

    /*
     * Get the number of subbooks in this book.
     */
    if (zio_read(&zio, buffer, 16) != 16) {
	error_code = EB_ERR_FAIL_READ_CAT;
	goto failed;
    }

    book->subbook_count = eb_uint2(buffer);
    LOG(("aux: eb_load_catalog_eb(): subbook_count=%d",
	book->subbook_count));
    if (EB_MAX_SUBBOOKS < book->subbook_count)
	book->subbook_count = EB_MAX_SUBBOOKS;
    if (book->subbook_count == 0) {
	error_code = EB_ERR_UNEXP_CAT;
	goto failed;
    }

    /*
     * Allocate memories for subbook entries.
     */
    book->subbooks = (EB_Subbook *) malloc(sizeof(EB_Subbook)
	* book->subbook_count);
    if (book->subbooks == NULL) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }
    eb_initialize_subbooks(book);

    /*
     * Read information about subbook.
     */
    for (i = 0, subbook = book->subbooks; i < book->subbook_count;
	 i++, subbook++) {
	/*
	 * Read data from the catalog file.
	 */
	if (zio_read(&zio, buffer, EB_SIZE_EB_CATALOG)
	    != EB_SIZE_EB_CATALOG) {
	    error_code = EB_ERR_FAIL_READ_CAT;
	    goto failed;
	}

	/*
	 * Set a directory name.
	 */
	strncpy(subbook->directory_name,
	    buffer + 2 + EB_MAX_EB_TITLE_LENGTH,
	    EB_MAX_DIRECTORY_NAME_LENGTH);
	subbook->directory_name[EB_MAX_DIRECTORY_NAME_LENGTH] = '\0';
	space = strchr(subbook->directory_name, ' ');
	if (space != NULL)
	    *space = '\0';
	eb_fix_directory_name(book->path, subbook->directory_name);

	/*
	 * Set an index page.
	 */
	subbook->index_page = 1;

	/*
	 * Set a title.  (Convert from JISX0208 to EUC JP)
	 */
	strncpy(subbook->title, buffer + 2, EB_MAX_EB_TITLE_LENGTH);
	subbook->title[EB_MAX_EB_TITLE_LENGTH] = '\0';
	if (book->character_code != EB_CHARCODE_ISO8859_1)
	    eb_jisx0208_to_euc(subbook->title, subbook->title);

	subbook->initialized = 0;
	subbook->code = i;
    }

    /*
     * Close the catalog file.
     */
    zio_close(&zio);
    zio_finalize(&zio);

    /*
     * Fix chachacter-code of the book.
     */
    eb_fix_misleaded_book(book);
    LOG(("out: eb_load_catalog_eb() = %s", eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    zio_close(&zio);
    zio_initialize(&zio);
    LOG(("out: eb_load_catalog() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Read information from the `CATALOGS' file in 'book'. (EPWING)
 */
static EB_Error_Code
eb_load_catalog_epwing(EB_Book *book, const char *catalog_path)
{
    EB_Error_Code error_code;
    char buffer[EB_SIZE_PAGE];
    char *buffer_p;
    char *space;
    EB_Subbook *subbook;
    EB_Font *font;
    Zio zio;
    Zio_Code zio_code;
    int epwing_version;
    int data_types;
    int i, j;

    LOG(("in: eb_load_catalog_epwing(book=%d, catalog=%s)",
	(int)book->code, catalog_path));

    zio_initialize(&zio);

    /*
     * Open a catalog file.
     */
    eb_path_name_zio_code(catalog_path, ZIO_PLAIN, &zio_code);
    if (zio_open(&zio, catalog_path, zio_code) < 0) {
	error_code = EB_ERR_FAIL_OPEN_CAT;
	goto failed;
    }

    /*
     * Get the number of subbooks in this book.
     */
    if (zio_read(&zio, buffer, 16) != 16) {
	error_code = EB_ERR_FAIL_READ_CAT;
	goto failed;
    }

    book->subbook_count = eb_uint2(buffer);
    LOG(("aux: eb_load_catalog_epwing(): subbook_count=%d",
	book->subbook_count));
    if (EB_MAX_SUBBOOKS < book->subbook_count)
	book->subbook_count = EB_MAX_SUBBOOKS;
    if (book->subbook_count == 0) {
	error_code = EB_ERR_UNEXP_CAT;
	goto failed;
    }

    epwing_version = eb_uint2(buffer + 2);
    LOG(("aux: eb_load_catalog_epwing(): epwing_version=%d", epwing_version));

    if (epwing_version >= 10)
      book->character_code = EB_CHARCODE_UTF8;

    /*
     * Allocate memories for subbook entries.
     */
    book->subbooks = (EB_Subbook *) malloc(sizeof(EB_Subbook)
	* book->subbook_count);
    if (book->subbooks == NULL) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }
    eb_initialize_subbooks(book);

    /*
     * Read information about subbook.
     */
    for (i = 0, subbook = book->subbooks; i < book->subbook_count;
	 i++, subbook++) {
	/*
	 * Read data from the catalog file.
	 */
	if (zio_read(&zio, buffer, EB_SIZE_EPWING_CATALOG)
	    != EB_SIZE_EPWING_CATALOG) {
	    error_code = EB_ERR_FAIL_READ_CAT;
	    goto failed;
	}

	/*
	 * Set a directory name.
	 */
	strncpy(subbook->directory_name,
	    buffer + 2 + EB_MAX_EPWING_TITLE_LENGTH,
	    EB_MAX_DIRECTORY_NAME_LENGTH);
	subbook->directory_name[EB_MAX_DIRECTORY_NAME_LENGTH] = '\0';
	space = strchr(subbook->directory_name, ' ');
	if (space != NULL)
	    *space = '\0';
	eb_fix_directory_name(book->path, subbook->directory_name);

	/*
	 * Set an index page.
	 */
	subbook->index_page = eb_uint2(buffer + 2 + EB_MAX_EPWING_TITLE_LENGTH
	    + EB_MAX_DIRECTORY_NAME_LENGTH + 4);

	/*
	 * Set a title.  (Convert from JISX0208 to EUC JP)
	 */
	strncpy(subbook->title, buffer + 2, EB_MAX_EPWING_TITLE_LENGTH);
	subbook->title[EB_MAX_EPWING_TITLE_LENGTH] = '\0';
	if (book->character_code != EB_CHARCODE_ISO8859_1)
	    eb_jisx0208_to_euc(subbook->title, subbook->title);

	/*
	 * Narrow font file names.
	 */
	buffer_p = buffer + 2 + EB_MAX_EPWING_TITLE_LENGTH + 50;
	for (font = subbook->narrow_fonts, j = 0; j < EB_MAX_FONTS;
	     j++, font++) {
	    /*
	     * Skip this entry if the first character of the file name
	     * is not valid.
	     */
	    if (*buffer_p == '\0' || 0x80 <= *((unsigned char *)buffer_p)) {
		buffer_p += EB_MAX_DIRECTORY_NAME_LENGTH;
		continue;
	    }
	    strncpy(font->file_name, buffer_p, EB_MAX_DIRECTORY_NAME_LENGTH);
	    font->file_name[EB_MAX_DIRECTORY_NAME_LENGTH] = '\0';
	    font->font_code = j;
	    font->page = 1;
	    space = strchr(font->file_name, ' ');
	    if (space != NULL)
		*space = '\0';
	    buffer_p += EB_MAX_DIRECTORY_NAME_LENGTH;
	}

	/*
	 * Wide font file names.
	 */
	buffer_p = buffer + 2 + EB_MAX_EPWING_TITLE_LENGTH + 18;
	for (font = subbook->wide_fonts, j = 0; j < EB_MAX_FONTS;
	     j++, font++) {
	    /*
	     * Skip this entry if the first character of the file name
	     * is not valid.
	     */
	    if (*buffer_p == '\0' || 0x80 <= *((unsigned char *)buffer_p)) {
		buffer_p += EB_MAX_DIRECTORY_NAME_LENGTH;
		continue;
	    }
	    strncpy(font->file_name, buffer_p, EB_MAX_DIRECTORY_NAME_LENGTH);
	    font->file_name[EB_MAX_DIRECTORY_NAME_LENGTH] = '\0';
	    font->font_code = j;
	    font->page = 1;
	    space = strchr(font->file_name, ' ');
	    if (space != NULL)
		*space = '\0';
	    buffer_p += EB_MAX_DIRECTORY_NAME_LENGTH;
	}

	subbook->initialized = 0;
	subbook->code = i;
    }

    /*
     * Set default file names and compression types.
     */
    for (i = 0, subbook = book->subbooks; i < book->subbook_count;
	 i++, subbook++) {
	strcpy(subbook->text_file_name, EB_FILE_NAME_HONMON);
	strcpy(subbook->graphic_file_name, EB_FILE_NAME_HONMON);
	strcpy(subbook->sound_file_name, EB_FILE_NAME_HONMON);
	subbook->text_hint_zio_code = ZIO_PLAIN;
	subbook->graphic_hint_zio_code = ZIO_PLAIN;
	subbook->sound_hint_zio_code = ZIO_PLAIN;
    }

    if (epwing_version == 1)
	goto succeeded;

    /*
     * Read extra information about subbook.
     */
    for (i = 0, subbook = book->subbooks; i < book->subbook_count;
	 i++, subbook++) {
	/*
	 * Read data from the catalog file.
	 *
	 * We don't complain about unexpected EOF.  In that case, we
	 * return EB_SUCCESS.
	 */
	ssize_t read_result = zio_read(&zio, buffer, EB_SIZE_EPWING_CATALOG);
	if (read_result < 0) {
	    error_code = EB_ERR_FAIL_READ_CAT;
	    goto failed;
	} else if (read_result != EB_SIZE_EPWING_CATALOG) {
	    break;
	}
	if (*(buffer + 4) == '\0')
	    continue;

	/*
	 * Set a text file name and its compression hint.
	 */
	*(subbook->text_file_name) = '\0';
	strncpy(subbook->text_file_name,
	    buffer + 4, EB_MAX_DIRECTORY_NAME_LENGTH);
	subbook->text_file_name[EB_MAX_DIRECTORY_NAME_LENGTH] = '\0';
	space = strchr(subbook->text_file_name, ' ');
	if (space != NULL)
	    *space = '\0';
	subbook->text_hint_zio_code
	    = eb_get_hint_zio_code(eb_uint1(buffer + 55));
	if (subbook->text_hint_zio_code == ZIO_INVALID) {
	    error_code = EB_ERR_UNEXP_CAT;
	    goto failed;
	}

	data_types = eb_uint2(buffer + 41);

	/*
	 * Set a graphic file name and its compression hint.
	 */
	*(subbook->graphic_file_name) = '\0';
	if ((data_types & 0x03) == 0x02) {
	    strncpy(subbook->graphic_file_name, buffer + 44,
		EB_MAX_DIRECTORY_NAME_LENGTH);
	    subbook->graphic_hint_zio_code
		= eb_get_hint_zio_code(eb_uint1(buffer + 54));
	} else if (((data_types >> 8) & 0x03) == 0x02) {
	    strncpy(subbook->graphic_file_name, buffer + 56,
		EB_MAX_DIRECTORY_NAME_LENGTH);
	    subbook->graphic_hint_zio_code
		= eb_get_hint_zio_code(eb_uint1(buffer + 53));
	}
	subbook->graphic_file_name[EB_MAX_DIRECTORY_NAME_LENGTH] = '\0';
	space = strchr(subbook->graphic_file_name, ' ');
	if (space != NULL)
	    *space = '\0';
	if (*(subbook->graphic_file_name) == '\0') {
	    strcpy(subbook->graphic_file_name, subbook->text_file_name);
	    subbook->graphic_hint_zio_code = subbook->text_hint_zio_code;
	}

	if (subbook->graphic_hint_zio_code == ZIO_INVALID) {
	    error_code = EB_ERR_UNEXP_CAT;
	    goto failed;
	}

	/*
	 * Set a sound file name and its compression hint.
	 */
	*(subbook->sound_file_name) = '\0';
	if ((data_types & 0x03) == 0x01) {
	    strncpy(subbook->sound_file_name, buffer + 44,
		EB_MAX_DIRECTORY_NAME_LENGTH);
	    subbook->sound_hint_zio_code
		= eb_get_hint_zio_code(eb_uint1(buffer + 54));
	} else if (((data_types >> 8) & 0x03) == 0x01) {
	    strncpy(subbook->sound_file_name, buffer + 56,
		EB_MAX_DIRECTORY_NAME_LENGTH);
	    subbook->sound_hint_zio_code
		= eb_get_hint_zio_code(eb_uint1(buffer + 53));
	}
	subbook->sound_file_name[EB_MAX_DIRECTORY_NAME_LENGTH] = '\0';
	space = strchr(subbook->sound_file_name, ' ');
	if (space != NULL)
	    *space = '\0';
	if (*(subbook->sound_file_name) == '\0') {
	    strcpy(subbook->sound_file_name, subbook->text_file_name);
	    subbook->sound_hint_zio_code = subbook->text_hint_zio_code;
	}

	if (subbook->sound_hint_zio_code == ZIO_INVALID) {
	    error_code = EB_ERR_UNEXP_CAT;
	    goto failed;
	}
    }

    /*
     * Close the catalog file.
     */
  succeeded:
    zio_close(&zio);
    zio_finalize(&zio);

    /*
     * Fix chachacter-code of the book.
     */
    eb_fix_misleaded_book(book);
    LOG(("out: eb_load_catalog_epwing() = %s", eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    zio_close(&zio);
    zio_initialize(&zio);
    LOG(("out: eb_load_catalog_epwing() = %s", eb_error_string(error_code)));
    return error_code;
}


static Zio_Code
eb_get_hint_zio_code(int catalog_hint_value)
{
    switch (catalog_hint_value) {
    case 0x00:
	return ZIO_PLAIN;
	break;
    case 0x11:
	return ZIO_EPWING;
	break;
    case 0x12:
	return ZIO_EPWING6;
	break;
    }

    return ZIO_INVALID;
}


/*
 * Read information from the `LANGUAGE' file in `book'.
 */
static void
eb_load_language(EB_Book *book)
{
    Zio zio;
    Zio_Code zio_code;
    char language_path_name[EB_MAX_PATH_LENGTH + 1];
    char language_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
    char buffer[16];

    LOG(("in: eb_load_language(book=%d)", (int)book->code));

    zio_initialize(&zio);
    book->character_code = EB_CHARCODE_JISX0208;

    /*
     * Open the language file.
     */
    if (eb_find_file_name(book->path, "language", language_file_name)
	!= EB_SUCCESS)
	goto failed;

    eb_compose_path_name(book->path, language_file_name, language_path_name);
    eb_path_name_zio_code(language_path_name, ZIO_PLAIN, &zio_code);

    if (zio_open(&zio, language_path_name, zio_code) < 0)
	goto failed;

    /*
     * Get a character code of the book, and get the number of langueages
     * in the file.
     */
    if (zio_read(&zio, buffer, 16) != 16)
	goto failed;

    book->character_code = eb_uint2(buffer);
    if (book->character_code != EB_CHARCODE_ISO8859_1
	&& book->character_code != EB_CHARCODE_JISX0208
	&& book->character_code != EB_CHARCODE_JISX0208_GB2312) {
	goto failed;
    }

    zio_close(&zio);
    LOG(("out: eb_load_language()"));

    return;

    /*
     * An error occurs...
     */
  failed:
    zio_close(&zio);
    LOG(("out: eb_load_language()"));
}


/*
 * Test whether `book' is bound.
 */
int
eb_is_bound(EB_Book *book)
{
    int is_bound;

    eb_lock(&book->lock);
    LOG(("in: eb_is_bound(book=%d)", (int)book->code));

    /*
     * Check for the current status.
     */
    is_bound = (book->path != NULL);

    LOG(("out: eb_is_bound() = %d", is_bound));
    eb_unlock(&book->lock);

    return is_bound;
}


/*
 * Return the bound path of `book'.
 */
EB_Error_Code
eb_path(EB_Book *book, char *path)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_path(book=%d)", (int)book->code));

    /*
     * Check for the current status.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Copy the path to `path'.
     */
    strcpy(path, book->path);

    LOG(("out: eb_path(path=%s) = %s", path, eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *path = '\0';
    LOG(("out: eb_path() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Inspect a disc type.
 */
EB_Error_Code
eb_disc_type(EB_Book *book, EB_Disc_Code *disc_code)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_disc_type(book=%d)", (int)book->code));

    /*
     * Check for the current status.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Copy the disc code to `disc_code'.
     */
    *disc_code = book->disc_code;

    LOG(("out: eb_disc_type(disc_code=%d) = %s", (int)*disc_code,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *disc_code = EB_DISC_INVALID;
    LOG(("out: eb_disc_type() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Inspect a character code used in the book.
 */
EB_Error_Code
eb_character_code(EB_Book *book, EB_Character_Code *character_code)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_character_code(book=%d)", (int)book->code));

    /*
     * Check for the current status.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Copy the character code to `character_code'.
     */
    *character_code = book->character_code;

    LOG(("out: eb_character_code(character_code=%d) = %s",
	(int)*character_code, eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *character_code = EB_CHARCODE_INVALID;
    LOG(("out: eb_character_code() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


