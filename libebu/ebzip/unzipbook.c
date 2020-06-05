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

#include "getumask.h"
#include "makedir.h"
#include "strlist.h"

/*
 * Unexported function.
 */
static int ebzip_unzip_book_eb(EB_Book *book, const char *out_top_path,
    const char *book_path, EB_Subbook_Code *subbook_list, int subbook_count);
static int ebzip_unzip_book_epwing(EB_Book *book, const char *out_top_path,
    const char *book_path, EB_Subbook_Code *subbook_list, int subbook_count);


/*
 * Uncompress files in `book' and output them under `out_top_path'.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
int
ebzip_unzip_book(const char *out_top_path, const char *book_path,
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
	    error_code = find_subbook(&book, subbook_name_list[i],
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
     * Uncompress the book.
     */
    if (book.disc_code == EB_DISC_EB) {
	result = ebzip_unzip_book_eb(&book, out_top_path, book_path,
	    subbook_list, subbook_count);
    } else {
	result = ebzip_unzip_book_epwing(&book, out_top_path, book_path,
	    subbook_list, subbook_count);
    }

    eb_finalize_book(&book);

    return result;
}


/*
 * Internal function for `unzip_book'.
 * This is used to compress an EB book.
 */
static int
ebzip_unzip_book_eb(EB_Book *book, const char *out_top_path,
    const char *book_path, EB_Subbook_Code *subbook_list, int subbook_count)
{
    EB_Subbook *subbook;
    EB_Error_Code error_code;
    String_List string_list;
    char in_path_name[PATH_MAX + 1];
    char out_sub_path[PATH_MAX + 1];
    char out_path_name[PATH_MAX + 1];
    char catalog_file_name[EB_MAX_FILE_NAME_LENGTH];
    char language_file_name[EB_MAX_FILE_NAME_LENGTH];
    mode_t out_directory_mode;
    Zio_Code in_zio_code;
    int i;

    /*
     * If `out_top_path' and/or `book_path' represents "/", replace it
     * to an empty string.
     */
    if (strcmp(out_top_path, "/") == 0)
	out_top_path++;
    if (strcmp(book_path, "/") == 0)
	book_path++;

    /*
     * Initialize variables.
     */
    out_directory_mode = 0777 ^ get_umask();
    string_list_initialize(&string_list);

    error_code = eb_load_all_subbooks(book);
    if (error_code != EB_SUCCESS) {
	fprintf(stderr, "%s: %s\n", invoked_name,
	    eb_error_message(error_code));
    }

    /*
     * Uncompress a book.
     */
    for (i = 0; i < subbook_count; i++) {
	subbook = book->subbooks + subbook_list[i];

	/*
	 * Make an output directory for the current subbook.
	 */
	eb_compose_path_name(out_top_path, subbook->directory_name,
	    out_sub_path);
	if (!ebzip_test_flag
	    && make_missing_directory(out_sub_path, out_directory_mode) < 0) {
	    fprintf(stderr, _("%s: failed to create a directory, %s: %s\n"),
		invoked_name, strerror(errno), out_sub_path);
	    goto failed;
	}

	/*
	 * Uncompress START file.
	 */
	in_zio_code = zio_mode(&subbook->text_zio);
	eb_compose_path_name2(book->path, subbook->directory_name,
	    subbook->text_file_name, in_path_name);
	eb_compose_path_name2(out_top_path, subbook->directory_name,
	    subbook->text_file_name, out_path_name);
	eb_fix_path_name_suffix(out_path_name, EBZIP_SUFFIX_NONE);

	if (in_zio_code != ZIO_INVALID
	    && !string_list_find(&string_list, in_path_name)) {
	    if (ebzip_unzip_start_file(out_path_name, in_path_name,
		in_zio_code, subbook->index_page) < 0)
		goto failed;
	}

	if (!ebzip_test_flag
	    && rewrite_sebxa_start(out_path_name, subbook->index_page) < 0)
	    goto failed;
	string_list_add(&string_list, in_path_name);
    }

    /*
     * Uncompress a language file.
     */
    if (eb_find_file_name(book->path, "language", language_file_name)
	== EB_SUCCESS) {
	eb_compose_path_name(book->path, language_file_name, in_path_name);
	eb_compose_path_name(out_top_path, language_file_name, out_path_name);
	eb_path_name_zio_code(in_path_name, ZIO_PLAIN, &in_zio_code);
	eb_fix_path_name_suffix(out_path_name, EBZIP_SUFFIX_NONE);
	if (ebzip_unzip_file(out_path_name, in_path_name, in_zio_code) < 0)
	    goto failed;
    }

    /*
     * Copy CATALOG file.
     */
    if (eb_find_file_name(book->path, "catalog", catalog_file_name)
	== EB_SUCCESS) {
	eb_compose_path_name(book->path, catalog_file_name, in_path_name);
	eb_compose_path_name(out_top_path, catalog_file_name, out_path_name);
	if (ebzip_copy_file(out_path_name, in_path_name) < 0)
	    goto failed;
    }

    string_list_finalize(&string_list);
    return 0;

    /*
     * An error occurs...
     */
  failed:
    string_list_finalize(&string_list);
    return -1;
}


/*
 * Internal function for `unzip_book'.
 * This is used to compress an EPWING book.
 */
static int
ebzip_unzip_book_epwing(EB_Book *book, const char *out_top_path,
    const char *book_path, EB_Subbook_Code *subbook_list, int subbook_count)
{
    EB_Subbook *subbook;
    EB_Error_Code error_code;
    EB_Font *font;
    String_List string_list;
    char in_path_name[PATH_MAX + 1];
    char out_sub_path[PATH_MAX + 1];
    char out_path_name[PATH_MAX + 1];
    char catalogs_file_name[EB_MAX_FILE_NAME_LENGTH];
    mode_t out_directory_mode;
    Zio_Code in_zio_code;
    int i, j;

    /*
     * If `out_top_path' and/or `book_path' represents "/", replace it
     * to an empty string.
     */
    if (strcmp(out_top_path, "/") == 0)
	out_top_path++;
    if (strcmp(book_path, "/") == 0)
	book_path++;

    /*
     * Initialize variables.
     */
    out_directory_mode = 0777 ^ get_umask();
    string_list_initialize(&string_list);

    error_code = eb_load_all_subbooks(book);
    if (error_code != EB_SUCCESS) {
	fprintf(stderr, "%s: %s\n", invoked_name,
	    eb_error_message(error_code));
    }

    /*
     * Uncompress a book.
     */
    for (i = 0; i < subbook_count; i++) {
	subbook = book->subbooks + subbook_list[i];

	/*
	 * Make an output directory for the current subbook.
	 */
	eb_compose_path_name(out_top_path, subbook->directory_name,
	    out_sub_path);
	if (!ebzip_test_flag
	    && make_missing_directory(out_sub_path, out_directory_mode) < 0) {
	    fprintf(stderr, _("%s: failed to create a directory, %s: %s\n"),
		invoked_name, strerror(errno), out_sub_path);
	    goto failed;
	}

	/*
	 * Make `data' sub directory for the current subbook.
	 */
	eb_compose_path_name2(out_top_path, subbook->directory_name,
	    subbook->data_directory_name, out_sub_path);
	if (!ebzip_test_flag
	    && make_missing_directory(out_sub_path, out_directory_mode) < 0) {
	    fprintf(stderr, _("%s: failed to create a directory, %s: %s\n"),
		invoked_name, strerror(errno), out_sub_path);
	    goto failed;
	}

	/*
	 * Uncompress HONMON/HONMON2 file.
	 */
	in_zio_code = zio_mode(&subbook->text_zio);
	eb_compose_path_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->text_file_name,
	    in_path_name);
	eb_compose_path_name3(out_top_path, subbook->directory_name,
	    subbook->data_directory_name, subbook->text_file_name,
	    out_path_name);

	if (in_zio_code != ZIO_INVALID
	    && !string_list_find(&string_list, in_path_name)) {
	    if (strncasecmp(subbook->text_file_name, "honmon2", 7) == 0)
		eb_fix_path_name_suffix(out_path_name, EBZIP_SUFFIX_ORG);
	    else
		eb_fix_path_name_suffix(out_path_name, EBZIP_SUFFIX_NONE);
	    if (ebzip_unzip_file(out_path_name, in_path_name, in_zio_code) < 0)
		goto failed;
	    string_list_add(&string_list, in_path_name);
	}

	/*
	 * Uncompress HONMONS file.
	 */
	in_zio_code = zio_mode(&subbook->sound_zio);
	eb_compose_path_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->sound_file_name,
	    in_path_name);
	eb_compose_path_name3(out_top_path, subbook->directory_name,
	    subbook->data_directory_name, subbook->sound_file_name,
	    out_path_name);
	eb_fix_path_name_suffix(out_path_name, EBZIP_SUFFIX_NONE);

	if (!ebzip_skip_flag_sound
	    && in_zio_code != ZIO_INVALID
	    && !string_list_find(&string_list, in_path_name)) {
	    if (ebzip_unzip_file(out_path_name, in_path_name, in_zio_code) < 0)
		goto failed;
	    string_list_add(&string_list, in_path_name);
	}

	/*
	 * Uncompress HONMONG file.
	 */
	in_zio_code = zio_mode(&subbook->graphic_zio);
	eb_compose_path_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->graphic_file_name,
	    in_path_name);
	eb_compose_path_name3(out_top_path, subbook->directory_name,
	    subbook->data_directory_name, subbook->graphic_file_name,
	    out_path_name);
	eb_fix_path_name_suffix(out_path_name, EBZIP_SUFFIX_NONE);

	if (!ebzip_skip_flag_graphic
	    && in_zio_code != ZIO_INVALID
	    && !string_list_find(&string_list, in_path_name)) {
	    if (ebzip_unzip_file(out_path_name, in_path_name, in_zio_code) < 0)
		goto failed;
	    string_list_add(&string_list, in_path_name);
	}

	/*
	 * Make `gaiji' sub directory for the current subbook.
	 */
	if (!ebzip_skip_flag_font) {
	    eb_compose_path_name2(out_top_path, subbook->directory_name,
		subbook->gaiji_directory_name, out_sub_path);
	    if (!ebzip_test_flag
		&& make_missing_directory(out_sub_path, out_directory_mode)
		< 0) {
		fprintf(stderr,
		    _("%s: failed to create a directory, %s: %s\n"),
		    invoked_name, strerror(errno), out_sub_path);
		goto failed;
	    }

	    /*
	     * Uncompress narrow font files.
	     */
	    for (j = 0; j < EB_MAX_FONTS; j++) {
		font = subbook->narrow_fonts + j;
		if (font->font_code == EB_FONT_INVALID)
		    continue;

		in_zio_code = zio_mode(&font->zio);
		eb_compose_path_name3(book->path, subbook->directory_name,
		    subbook->gaiji_directory_name, font->file_name,
		    in_path_name);
		eb_compose_path_name3(out_top_path, subbook->directory_name,
		    subbook->gaiji_directory_name, font->file_name,
		    out_path_name);
		eb_fix_path_name_suffix(out_path_name, EBZIP_SUFFIX_NONE);

		if (in_zio_code != ZIO_INVALID
		    && !string_list_find(&string_list, in_path_name)) {
		    if (ebzip_unzip_file(out_path_name, in_path_name,
			in_zio_code) < 0)
			goto failed;
		    string_list_add(&string_list, in_path_name);
		}
	    }

	    /*
	     * Uncompress wide font files.
	     */
	    for (j = 0; j < EB_MAX_FONTS; j++) {
		font = subbook->wide_fonts + j;
		if (font->font_code == EB_FONT_INVALID)
		    continue;

		in_zio_code = zio_mode(&font->zio);
		eb_compose_path_name3(book->path, subbook->directory_name,
		    subbook->gaiji_directory_name,
		    font->file_name, in_path_name);
		eb_compose_path_name3(out_top_path, subbook->directory_name,
		    subbook->gaiji_directory_name, font->file_name,
		    out_path_name);
		eb_fix_path_name_suffix(out_path_name, EBZIP_SUFFIX_NONE);

		if (in_zio_code != ZIO_INVALID
		    && !string_list_find(&string_list, in_path_name)) {
		    if (ebzip_unzip_file(out_path_name, in_path_name,
			in_zio_code) < 0)
			goto failed;
		    string_list_add(&string_list, in_path_name);
		}
	    }
	}

	/*
	 * Copy movie files.
	 */
	if (!ebzip_skip_flag_movie) {
	    eb_compose_path_name2(book->path, subbook->directory_name,
		subbook->movie_directory_name, in_path_name);
	    eb_compose_path_name2(out_top_path, subbook->directory_name,
		subbook->movie_directory_name, out_path_name);
	    if (ebzip_copy_files_in_directory(out_path_name, in_path_name) < 0)
		goto failed;
	}
    }

    /*
     * Copy CATALOGS file.
     */
    if (eb_find_file_name(book->path, "catalogs", catalogs_file_name)
	== EB_SUCCESS) {
	eb_compose_path_name(book->path, catalogs_file_name, in_path_name);
	eb_compose_path_name(out_top_path, catalogs_file_name, out_path_name);
	if (ebzip_copy_file(out_path_name, in_path_name) < 0)
	    goto failed;
    }

    string_list_finalize(&string_list);
    return 0;

    /*
     * An error occurs...
     */
  failed:
    string_list_finalize(&string_list);
    return -1;
}
