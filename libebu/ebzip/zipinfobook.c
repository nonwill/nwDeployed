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

#include "ebzip.h"
#include "ebutils.h"

/*
 * Unexported functions.
 */
static int ebzip_zipinfo_book_eb(EB_Book *book, const char *book_path,
    EB_Subbook_Code *subbook_list, int subbook_count);
static int ebzip_zipinfo_book_epwing(EB_Book *book, const char *book_path,
    EB_Subbook_Code *subbook_list, int subbook_count);


/*
 * Hints of language file names in a book.
 */
#define EB_HINT_INDEX_LANGUAGE		0
#define EB_HINT_INDEX_LANGUAGE_EBZ	1

/*
 * List compressed book information.
 * If is succeeds, 0 is returned.  Otherwise -1 is returned.
 */
int
ebzip_zipinfo_book(const char *book_path,
    char subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count)
{
    EB_Book book;
    EB_Error_Code error_code;
    EB_Subbook_Code subbook_list[EB_MAX_SUBBOOKS];
    EB_Subbook_Code subbook_code;
    int subbook_count = 0;
    int result;
    int i;

    eb_initialize_book(&book);

    /*
     * Bind a book.
     */
    error_code = eb_bind(&book, book_path);
    if (error_code != EB_SUCCESS) {
	fprintf(stderr, "%s: %s\n", invoked_name,
	    eb_error_message(error_code));
	fflush(stderr);
	return -1;
    }

    /*
     * For each targe subbook, convert a subbook-names to a subbook-codes.
     * If no subbook is specified by `--subbook'(`-S'), set all subbooks
     * as the target.
     */
    if (subbook_name_count == 0) {
	error_code = eb_subbook_list(&book, subbook_list, &subbook_count);
	if (error_code != EB_SUCCESS) {
	    fprintf(stderr, "%s: %s\n", invoked_name,
		eb_error_message(error_code));
	    fflush(stderr);
	    return -1;
	}
    } else {
	for (i = 0; i < subbook_name_count; i++) {
	    error_code = find_subbook(&book, *(subbook_name_list + i),
		&subbook_code);
	    if (error_code != EB_SUCCESS) {
		fprintf(stderr, _("%s: unknown subbook name `%s'\n"),
		    invoked_name, subbook_name_list[i]);
		return -1;
	    }
	    subbook_list[subbook_count++] = subbook_code;
	}
    }

    /*
     * List compressed book information.
     */
    if (book.disc_code == EB_DISC_EB) {
	result = ebzip_zipinfo_book_eb(&book, book_path, subbook_list,
	    subbook_count);
    } else {
	result = ebzip_zipinfo_book_epwing(&book, book_path, subbook_list,
	    subbook_count);
    }

    eb_finalize_book(&book);

    return result;
}


/*
 * Internal function for `zipinfo_book'.
 * This is used to list files in an EB book.
 */
static int
ebzip_zipinfo_book_eb(EB_Book *book, const char *book_path,
    EB_Subbook_Code *subbook_list, int subbook_count)
{
    EB_Subbook *subbook;
    EB_Error_Code error_code;
    char in_path_name[PATH_MAX + 1];
    char catalog_file_name[EB_MAX_FILE_NAME_LENGTH];
    char language_file_name[EB_MAX_FILE_NAME_LENGTH];
    Zio_Code in_zio_code;
    int i;

    /*
     * If `book_path' represents "/", replace it to an empty string.
     */
    if (strcmp(book_path, "/") == 0)
	book_path++;

    /*
     * Initialize variables.
     */
    error_code = eb_load_all_subbooks(book);
    if (error_code != EB_SUCCESS) {
	fprintf(stderr, "%s: %s\n", invoked_name,
	    eb_error_message(error_code));
    }

    /*
     * Inspect a book.
     */
    for (i = 0; i < subbook_count; i++) {
	subbook = book->subbooks + subbook_list[i];

	in_zio_code = zio_mode(&subbook->text_zio);
	eb_compose_path_name2(book->path, subbook->directory_name,
	    subbook->text_file_name, in_path_name);

	if (in_zio_code != ZIO_INVALID) {
	    ebzip_zipinfo_start_file(in_path_name, in_zio_code,
		subbook->index_page);
	}
    }

    /*
     * Inspect a language file.
     */
    if (eb_find_file_name(book->path, "language", language_file_name)
	== EB_SUCCESS) {
	eb_compose_path_name(book->path, language_file_name, in_path_name);
	eb_path_name_zio_code(in_path_name, ZIO_PLAIN, &in_zio_code);
	ebzip_zipinfo_file(in_path_name, in_zio_code);
    }

    /*
     * Inspect CATALOG file.
     */
    if (eb_find_file_name(book->path, "catalog", catalog_file_name)
	== EB_SUCCESS) {
	eb_compose_path_name(book->path, catalog_file_name, in_path_name);
	ebzip_zipinfo_file(in_path_name, ZIO_PLAIN);
    }

    return 0;
}


/*
 * Internal function for `zipinfo_book'.
 * This is used to list files in an EPWING book.
 */
static int
ebzip_zipinfo_book_epwing(EB_Book *book, const char *book_path,
    EB_Subbook_Code *subbook_list, int subbook_count)
{
    EB_Subbook *subbook;
    EB_Error_Code error_code;
    EB_Font *font;
    char in_path_name[PATH_MAX + 1];
    char in_movie_path_name[PATH_MAX + 1];
    char catalogs_file_name[EB_MAX_FILE_NAME_LENGTH];
    Zio_Code in_zio_code;
    DIR *dir;
    int i, j;

    /*
     * If `book_path' represents "/", replace it to an empty string.
     */
    if (strcmp(book_path, "/") == 0)
	book_path++;

    /*
     * Initialize variables.
     */
    error_code = eb_load_all_subbooks(book);
    if (error_code != EB_SUCCESS) {
	fprintf(stderr, "%s: %s\n", invoked_name,
	    eb_error_message(error_code));
    }

    /*
     * Inspect a book.
     */
    for (i = 0; i < subbook_count; i++) {
	subbook = book->subbooks + subbook_list[i];

	/*
	 * Inspect HONMON/HONMON2 file.
	 */
	in_zio_code = zio_mode(&subbook->text_zio);
	eb_compose_path_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->text_file_name,
	    in_path_name);
	if (in_zio_code != ZIO_INVALID)
	    ebzip_zipinfo_file(in_path_name, in_zio_code);

	/*
	 * Inspect HONMONS file.
	 */
	in_zio_code = zio_mode(&subbook->sound_zio);
	eb_compose_path_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->sound_file_name,
	    in_path_name);
	if (!ebzip_skip_flag_sound
	    && in_zio_code != ZIO_INVALID
	    && strncasecmp(subbook->sound_file_name, "honmons", 7) == 0)
	    ebzip_zipinfo_file(in_path_name, in_zio_code);

	/*
	 * Inspect HONMONG file.
	 */
	in_zio_code = zio_mode(&subbook->graphic_zio);
	eb_compose_path_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->graphic_file_name,
	    in_path_name);
	if (!ebzip_skip_flag_graphic
	    && in_zio_code != ZIO_INVALID
	    && strncasecmp(subbook->graphic_file_name, "honmong", 7) == 0)
	    ebzip_zipinfo_file(in_path_name, in_zio_code);

	if (!ebzip_skip_flag_font) {
	    /*
	     * Inspect narrow font files.
	     */
	    for (j = 0; j < EB_MAX_FONTS; j++) {
		font = subbook->narrow_fonts + j;
		if (font->font_code == EB_FONT_INVALID)
		    continue;

		in_zio_code = zio_mode(&font->zio);

		if (in_zio_code != ZIO_INVALID) {
		    eb_compose_path_name3(book->path,
			subbook->directory_name, subbook->gaiji_directory_name,
			font->file_name, in_path_name);
		    ebzip_zipinfo_file(in_path_name, in_zio_code);
		}
	    }

	    /*
	     * Inspect wide font files.
	     */
	    for (j = 0; j < EB_MAX_FONTS; j++) {
		font = subbook->wide_fonts + j;
		if (font->font_code == EB_FONT_INVALID)
		    continue;

		in_zio_code = zio_mode(&font->zio);

		if (in_zio_code != ZIO_INVALID) {
		    eb_compose_path_name3(book->path,
			subbook->directory_name, subbook->gaiji_directory_name,
			font->file_name, in_path_name);
		    ebzip_zipinfo_file(in_path_name, in_zio_code);
		}
	    }
	}

	/*
	 * Inspect movie files.
	 */
	if (!ebzip_skip_flag_movie) {
	    eb_compose_path_name2(book->path, subbook->directory_name,
		subbook->movie_directory_name, in_movie_path_name);
	    dir = opendir(in_movie_path_name);
	    if (dir == NULL)
		continue;
	    for (;;) {
		struct dirent *entry;
		struct stat st;

		entry = readdir(dir);
		if (entry == NULL)
		    break;
		eb_compose_path_name3(book->path, subbook->directory_name,
		    subbook->movie_directory_name, entry->d_name,
		    in_path_name);
		if (stat(in_path_name, &st) < 0 || !S_ISREG(st.st_mode))
		    continue;
		eb_path_name_zio_code(in_path_name, ZIO_PLAIN, &in_zio_code);
		ebzip_zipinfo_file(in_path_name, in_zio_code);
	    }
	    closedir(dir);
	}
    }

    /*
     * Inspect CATALOGS file.
     */
    if (eb_find_file_name(book->path, "catalogs", catalogs_file_name)
	== EB_SUCCESS) {
	eb_compose_path_name(book->path, catalogs_file_name, in_path_name);
	eb_path_name_zio_code(in_path_name, ZIO_PLAIN, &in_zio_code);
	ebzip_zipinfo_file(in_path_name, in_zio_code);
    }

    return 0;
}
