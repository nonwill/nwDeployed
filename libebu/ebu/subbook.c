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
#include "binary.h"
#include "font.h"
#include "build-post.h"

/*
 * Unexported functions.
 */
static EB_Error_Code eb_load_subbook(EB_Book *book);
static EB_Error_Code eb_load_subbook_indexes(EB_Book *book);
static EB_Error_Code eb_set_subbook_eb(EB_Book *book,
    EB_Subbook_Code subbook_code);
static EB_Error_Code eb_set_subbook_epwing(EB_Book *book,
    EB_Subbook_Code subbook_code);


/*
 * Initialize all subbooks in `book'.
 */
void
eb_initialize_subbooks(EB_Book *book)
{
    EB_Subbook *subbook;
    EB_Subbook *saved_subbook_current;
    int i;

    LOG(("in: eb_initialize_subbooks(book=%d)", (int)book->code));

    saved_subbook_current = book->subbook_current;

    for (i = 0, subbook = book->subbooks; i < book->subbook_count;
	 i++, subbook++) {
	book->subbook_current = subbook;

        subbook->initialized = 0;
	subbook->index_page = 1;
        subbook->code = i;
        zio_initialize(&subbook->text_zio);
        zio_initialize(&subbook->graphic_zio);
        zio_initialize(&subbook->sound_zio);
        zio_initialize(&subbook->movie_zio);
        subbook->title[0] = '\0';
        subbook->directory_name[0] = '\0';
        subbook->data_directory_name[0] = '\0';
        subbook->gaiji_directory_name[0] = '\0';
        subbook->movie_directory_name[0] = '\0';

        subbook->text_file_name[0] = '\0';
        subbook->graphic_file_name[0] = '\0';
        subbook->sound_file_name[0] = '\0';

	subbook->text_hint_zio_code = ZIO_PLAIN;
	subbook->graphic_hint_zio_code = ZIO_PLAIN;
	subbook->sound_hint_zio_code = ZIO_PLAIN;

	subbook->search_title_page = 0;
	eb_initialize_searches(book);
	subbook->multi_count = 0;

	eb_initialize_fonts(book);
	subbook->narrow_current = NULL;
	subbook->wide_current = NULL;

	eb_initialize_utf8_table(book);
    }

    book->subbook_current = saved_subbook_current;

    LOG(("out: eb_initialize_subbooks()"));
}


/*
 * Finalize all subbooks in `book'.
 */
void
eb_finalize_subbooks(EB_Book *book)
{
    EB_Subbook *subbook;
    EB_Subbook *saved_subbook_current;
    int i;

    LOG(("in: eb_finalize_subbooks(book=%d)", (int)book->code));

    saved_subbook_current = book->subbook_current;

    for (i = 0, subbook = book->subbooks; i < book->subbook_count;
	 i++, subbook++) {
	book->subbook_current = subbook;

        zio_finalize(&subbook->text_zio);
        zio_finalize(&subbook->graphic_zio);
        zio_finalize(&subbook->sound_zio);
        zio_finalize(&subbook->movie_zio);

	eb_finalize_searches(book);
 	eb_finalize_fonts(book);
 	eb_finalize_utf8_table(book);

	subbook->narrow_current = NULL;
	subbook->wide_current = NULL;
    }

    book->subbook_current = saved_subbook_current;

    LOG(("out: eb_finalize_subbooks()"));
}


/*
 * Get information about the current subbook.
 */
static EB_Error_Code
eb_load_subbook(EB_Book *book)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;

    LOG(("in: eb_load_subbook(book=%d)", (int)book->code));

    subbook = book->subbook_current;

    /*
     * Current subbook must have been set.
     */
    if (subbook == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Reset contexts.
     */
    eb_reset_search_contexts(book);
    eb_reset_text_context(book);
    eb_reset_binary_context(book);

    /*
     * If the subbook has already initialized, return immediately.
     */
    if (subbook->initialized)
	goto succeeded;

    if (0 <= zio_file(&subbook->text_zio)) {
	/*
	 * Read index information.
	 */
	error_code = eb_load_subbook_indexes(book);
	if (error_code != EB_SUCCESS)
	    goto failed;

	/*
	 * Read mutli search information.
	 */
	error_code = eb_load_multi_searches(book);
	if (error_code != EB_SUCCESS)
	    goto failed;
	error_code = eb_load_multi_titles(book);
	if (error_code != EB_SUCCESS)
	    goto failed;

	/*
	 * Rewind the file descriptor of the start file.
	 */
	if (zio_lseek(&subbook->text_zio,
	    ((off_t) subbook->index_page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
	    error_code = EB_ERR_FAIL_SEEK_TEXT;
	    goto failed;
	}
    }

  succeeded:
    LOG(("out: eb_load_subbook() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_load_subbook() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Get information about all subbooks in the book.
 */
EB_Error_Code
eb_load_all_subbooks(EB_Book *book)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    int i;

    eb_lock(&book->lock);
    LOG(("in: eb_load_all_subbooks(book=%d)", (int)book->code));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Initialize each subbook.
     */
    for (i = 0, subbook = book->subbooks; i < book->subbook_count;
	 i++, subbook++) {
	error_code = eb_set_subbook(book, subbook->code);
	if (error_code != EB_SUCCESS)
	    goto failed;
    }
    eb_unset_subbook(book);

    LOG(("out: eb_load_all_subbooks() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_unset_subbook(book);
    LOG(("out: eb_load_all_subbooks() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Get index information in the current subbook.
 *
 * If succeeds, the number of indexes is returned.
 * Otherwise, -1 is returned.
 */
static EB_Error_Code
eb_load_subbook_indexes(EB_Book *book)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    EB_Search search;
    char buffer[EB_SIZE_PAGE];
    char *buffer_p;
    int index_count;
    int availability;
    int global_availability;
    EB_Search sebxa_zip_index;
    EB_Search sebxa_zip_text;
    int i;

    LOG(("in: eb_load_subbook_indexes(book=%d)", (int)book->code));

    eb_initialize_search(&sebxa_zip_index);
    eb_initialize_search(&sebxa_zip_text);

    subbook = book->subbook_current;

    /*
     * Read the index table in the subbook.
     */
    if (zio_lseek(&subbook->text_zio,
	    ((off_t) subbook->index_page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
	error_code = EB_ERR_FAIL_SEEK_TEXT;
	goto failed;
    }
    if (zio_read(&subbook->text_zio, buffer, EB_SIZE_PAGE) != EB_SIZE_PAGE) {
	error_code = EB_ERR_FAIL_READ_TEXT;
	goto failed;
    }

    /*
     * Get start page numbers of the indexes in the subbook.
     */
    index_count = eb_uint1(buffer + 1);
    if (EB_SIZE_PAGE / 16 - 1 <= index_count) {
	error_code = EB_ERR_UNEXP_TEXT;
	goto failed;
    }

    /*
     * Get availavility flag of the index information.
     */
    global_availability = eb_uint1(buffer + 4);
    if (0x02 < global_availability)
	global_availability = 0;

    /*
     * Set each search method information.
     */
    for (i = 0, buffer_p = buffer + 16; i < index_count; i++, buffer_p += 16) {
	/*
	 * Set index style.
	 */
	eb_initialize_search(&search);

	search.index_id = eb_uint1(buffer_p);
	search.start_page = eb_uint4(buffer_p + 2);
	search.end_page   = search.start_page + eb_uint4(buffer_p + 6) - 1;

	/*
	 * Set canonicalization flags.
	 */
	availability = eb_uint1(buffer_p + 10);
	if ((global_availability == 0x00 && availability == 0x02)
	    || global_availability == 0x02) {
	    unsigned int flags;

	    flags = eb_uint3(buffer_p + 11);
	    search.katakana = (flags & 0xc00000) >> 22;
	    search.lower = (flags & 0x300000) >> 20;
	    if ((flags & 0x0c0000) >> 18 == 0)
		search.mark = EB_INDEX_STYLE_DELETE;
	    else
		search.mark = EB_INDEX_STYLE_ASIS;
	    search.long_vowel = (flags & 0x030000) >> 16;
	    search.double_consonant = (flags & 0x00c000) >> 14;
	    search.contracted_sound = (flags & 0x003000) >> 12;
	    search.small_vowel = (flags & 0x000c00) >> 10;
	    search.voiced_consonant = (flags & 0x000300) >> 8;
	    search.p_sound = (flags & 0x0000c0) >> 6;

	} else if (search.index_id == 0x70 || search.index_id == 0x90) {
	    search.katakana = EB_INDEX_STYLE_CONVERT;
	    search.lower = EB_INDEX_STYLE_CONVERT;
	    search.mark = EB_INDEX_STYLE_DELETE;
	    search.long_vowel = EB_INDEX_STYLE_CONVERT;
	    search.double_consonant = EB_INDEX_STYLE_CONVERT;
	    search.contracted_sound = EB_INDEX_STYLE_CONVERT;
	    search.small_vowel = EB_INDEX_STYLE_CONVERT;
	    search.voiced_consonant = EB_INDEX_STYLE_CONVERT;
	    search.p_sound = EB_INDEX_STYLE_CONVERT;

	} else {
	    search.katakana = EB_INDEX_STYLE_ASIS;
	    search.lower = EB_INDEX_STYLE_CONVERT;
	    search.mark = EB_INDEX_STYLE_ASIS;
	    search.long_vowel = EB_INDEX_STYLE_ASIS;
	    search.double_consonant = EB_INDEX_STYLE_ASIS;
	    search.contracted_sound = EB_INDEX_STYLE_ASIS;
	    search.small_vowel = EB_INDEX_STYLE_ASIS;
	    search.voiced_consonant = EB_INDEX_STYLE_ASIS;
	    search.p_sound = EB_INDEX_STYLE_ASIS;
	}

	if (book->character_code == EB_CHARCODE_ISO8859_1
	    || search.index_id == 0x72
	    || search.index_id == 0x92) {
	    search.space = EB_INDEX_STYLE_ASIS;
	} else {
            search.space = EB_INDEX_STYLE_DELETE;
	}

	/*
	 * Identify search method.
	 */
	switch (search.index_id) {
	case 0x00:
	  if (book->disc_code != EB_DISC_EPWING
	      || book->character_code != EB_CHARCODE_UTF8) {
	    memcpy(&subbook->text, &search, sizeof(EB_Search));
	  }
	    break;
	case 0x01:
	  if (book->disc_code != EB_DISC_EPWING
	      || book->character_code != EB_CHARCODE_UTF8) {
	    memcpy(&subbook->menu, &search, sizeof(EB_Search));
	  }
	    break;
	case 0x02:
	  if (book->disc_code != EB_DISC_EPWING
	      || book->character_code != EB_CHARCODE_UTF8) {
	    memcpy(&subbook->copyright, &search, sizeof(EB_Search));
	  }
	    break;
	case 0x10:
	    memcpy(&subbook->image_menu, &search, sizeof(EB_Search));
	    break;
	case 0x16:
	    if (book->disc_code == EB_DISC_EPWING)
		subbook->search_title_page = search.start_page;
	    break;
	case 0x21:
	    if (book->disc_code == EB_DISC_EB
		&& zio_mode(&subbook->text_zio) == ZIO_PLAIN)
		memcpy(&sebxa_zip_text, &search, sizeof(EB_Search));
	    break;
	case 0x22:
	    if (book->disc_code == EB_DISC_EB
		&& zio_mode(&subbook->text_zio) == ZIO_PLAIN)
		memcpy(&sebxa_zip_index, &search, sizeof(EB_Search));
	    break;
	case 0x70:
	  if (book->disc_code != EB_DISC_EPWING
	      || book->character_code != EB_CHARCODE_UTF8) {
	    memcpy(&subbook->endword_kana, &search, sizeof(EB_Search));
	  }
	    break;
	case 0x71:
	  if (book->disc_code != EB_DISC_EPWING
	      || book->character_code != EB_CHARCODE_UTF8) {
	    memcpy(&subbook->endword_asis, &search, sizeof(EB_Search));
	  }
	    break;
	case 0x72:
	    memcpy(&subbook->endword_alphabet, &search, sizeof(EB_Search));
	    break;
	case 0x80:
	  if (book->disc_code != EB_DISC_EPWING
	      || book->character_code != EB_CHARCODE_UTF8) {
	    memcpy(&subbook->keyword, &search, sizeof(EB_Search));
	  }
	    break;
	case 0x81:
	  if (book->disc_code != EB_DISC_EPWING
	      || book->character_code != EB_CHARCODE_UTF8) {
	    memcpy(&subbook->cross, &search, sizeof(EB_Search));
	  }
	    break;
	case 0x90:
	  if (book->disc_code != EB_DISC_EPWING
	      || book->character_code != EB_CHARCODE_UTF8) {
	    memcpy(&subbook->word_kana, &search, sizeof(EB_Search));
	  }
	    break;
	case 0x91:
	  if (book->disc_code != EB_DISC_EPWING
	      || book->character_code != EB_CHARCODE_UTF8) {
	    memcpy(&subbook->word_asis, &search, sizeof(EB_Search));
	  }
	    break;
	case 0x92:
	    memcpy(&subbook->word_alphabet, &search, sizeof(EB_Search));
	    break;
	case 0xc0:
	  if (book->disc_code == EB_DISC_EPWING
	      && book->character_code == EB_CHARCODE_UTF8) {
	    memcpy(&subbook->text, &search, sizeof(EB_Search));
	  }
	  break;
	case 0xc1:
	  if (book->disc_code == EB_DISC_EPWING
	      && book->character_code == EB_CHARCODE_UTF8) {
	    memcpy(&subbook->menu, &search, sizeof(EB_Search));
	  }
	  break;
	case 0xc2:
	  if (book->disc_code == EB_DISC_EPWING
	      && book->character_code == EB_CHARCODE_UTF8) {
	    memcpy(&subbook->copyright, &search, sizeof(EB_Search));
	  }
	  break;
	case 0xc6:
	  if (book->disc_code == EB_DISC_EPWING
	      && book->character_code == EB_CHARCODE_UTF8) {
	    memcpy(&subbook->word_asis, &search, sizeof(EB_Search));
	  }
	  break;
	case 0xc7:
	  if (book->disc_code == EB_DISC_EPWING
	      && book->character_code == EB_CHARCODE_UTF8) {
	    memcpy(&subbook->endword_asis, &search, sizeof(EB_Search));
	  }
	  break;
	case 0xc8:
	  if (book->disc_code == EB_DISC_EPWING
	      && book->character_code == EB_CHARCODE_UTF8) {
	    memcpy(&subbook->keyword, &search, sizeof(EB_Search));
	  }
	  break;
	case 0xc9:
	  if (book->disc_code == EB_DISC_EPWING
	      && book->character_code == EB_CHARCODE_UTF8) {
	    memcpy(&subbook->cross, &search, sizeof(EB_Search));
	  }
	  break;
	case 0xca:
	    if (subbook->multi_count < EB_MAX_MULTI_SEARCHES
		&& book->character_code == EB_CHARCODE_UTF8) {
		memcpy(&subbook->multis[subbook->multi_count].search, &search,
		    sizeof(EB_Search));
		subbook->multi_count++;
	    }
	    break;
	case 0xcb:
	    if (book->disc_code == EB_DISC_EPWING
		&& book->character_code == EB_CHARCODE_UTF8) {
		subbook->table_page = search.start_page;
		subbook->table_size = search.end_page - search.start_page + 1;
		error_code = eb_load_utf8_table(book);
		if (error_code != EB_SUCCESS) goto failed;
	    }
	    break;
	case 0xd8:
	    memcpy(&subbook->sound, &search, sizeof(EB_Search));
	    break;
	case 0xf1:
	    if (book->disc_code == EB_DISC_EB) {
		subbook->wide_fonts[EB_FONT_16].page = search.start_page;
		subbook->wide_fonts[EB_FONT_16].font_code = EB_FONT_16;
	    }
	    break;
	case 0xf2:
	    if (book->disc_code == EB_DISC_EB) {
		subbook->narrow_fonts[EB_FONT_16].page = search.start_page;
		subbook->narrow_fonts[EB_FONT_16].font_code = EB_FONT_16;
	    }
	    break;
	case 0xf3:
	    if (book->disc_code == EB_DISC_EB) {
		subbook->wide_fonts[EB_FONT_24].page = search.start_page;
		subbook->wide_fonts[EB_FONT_24].font_code = EB_FONT_24;
	    }
	    break;
	case 0xf4:
	    if (book->disc_code == EB_DISC_EB) {
		subbook->narrow_fonts[EB_FONT_24].page = search.start_page;
		subbook->narrow_fonts[EB_FONT_24].font_code = EB_FONT_24;
	    }
	    break;
	case 0xf5:
	    if (book->disc_code == EB_DISC_EB) {
		subbook->wide_fonts[EB_FONT_30].page = search.start_page;
		subbook->wide_fonts[EB_FONT_30].font_code = EB_FONT_30;
	    }
	    break;
	case 0xf6:
	    if (book->disc_code == EB_DISC_EB) {
		subbook->narrow_fonts[EB_FONT_30].page = search.start_page;
		subbook->narrow_fonts[EB_FONT_30].font_code = EB_FONT_30;
	    }
	    break;
	case 0xf7:
	    if (book->disc_code == EB_DISC_EB) {
		subbook->wide_fonts[EB_FONT_48].page = search.start_page;
		subbook->wide_fonts[EB_FONT_48].font_code = EB_FONT_48;
	    }
	    break;
	case 0xf8:
	    if (book->disc_code == EB_DISC_EB) {
		subbook->narrow_fonts[EB_FONT_48].page = search.start_page;
		subbook->narrow_fonts[EB_FONT_48].font_code = EB_FONT_48;
	    }
	    break;
	case 0xff:
	    if (subbook->multi_count < EB_MAX_MULTI_SEARCHES
		&& book->character_code != EB_CHARCODE_UTF8) {
		memcpy(&subbook->multis[subbook->multi_count].search, &search,
		    sizeof(EB_Search));
		subbook->multi_count++;
	    }
	    break;
	}

	eb_finalize_search(&sebxa_zip_text);
    }

    /*
     * Set S-EBXA compression flag.
     */
    if (book->disc_code == EB_DISC_EB
	&& zio_mode(&subbook->text_zio) == ZIO_PLAIN
	&& subbook->text.start_page != 0
	&& sebxa_zip_index.start_page != 0
	&& sebxa_zip_text.start_page != 0) {
	zio_set_sebxa_mode(&subbook->text_zio,
	    (sebxa_zip_index.start_page - 1) * EB_SIZE_PAGE,
	    (sebxa_zip_text.start_page - 1) * EB_SIZE_PAGE,
	    (subbook->text.start_page - 1) * EB_SIZE_PAGE,
	    subbook->text.end_page * EB_SIZE_PAGE - 1);
    }

    eb_finalize_search(&sebxa_zip_index);
    eb_finalize_search(&sebxa_zip_text);

    LOG(("out: eb_load_subbook_indexes() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_finalize_search(&sebxa_zip_index);
    eb_finalize_search(&sebxa_zip_text);
    LOG(("out: eb_load_subbook_indexes() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Make a subbook list in the book.
 */
EB_Error_Code
eb_subbook_list(EB_Book *book, EB_Subbook_Code *subbook_list,
    int *subbook_count)
{
    EB_Error_Code error_code;
    EB_Subbook_Code *list_p;
    int i;

    eb_lock(&book->lock);
    LOG(("in: eb_subbook_list(book=%d)", (int)book->code));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    for (i = 0, list_p = subbook_list; i < book->subbook_count; i++, list_p++)
	*list_p = i;
    *subbook_count = book->subbook_count;

    LOG(("out: eb_subbook_list(subbook_count=%d) = %s", *subbook_count,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *subbook_count = 0;
    LOG(("out: eb_subbook_list() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Return a subbook-code of the current subbook.
 */
EB_Error_Code
eb_subbook(EB_Book *book, EB_Subbook_Code *subbook_code)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_subbook(book=%d)", (int)book->code));

    /*
     * The current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    *subbook_code = book->subbook_current->code;

    LOG(("out: eb_subbook(subbook_code=%d) = %s", *subbook_code,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *subbook_code = EB_SUBBOOK_INVALID;
    LOG(("out: eb_subbook() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Return a title of the current subbook.
 */
EB_Error_Code
eb_subbook_title(EB_Book *book, char *title)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_subbook_title(book=%d)", (int)book->code));

    /*
     * The current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    strcpy(title, book->subbook_current->title);

    LOG(("out: eb_subbook_title(title=%s) = %s", title,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *title = '\0';
    LOG(("out: eb_subbook_title() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Return a title of the specified subbook `subbook_code'.
 */
EB_Error_Code
eb_subbook_title2(EB_Book *book, EB_Subbook_Code subbook_code, char *title)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_subbook_title2(book=%d, subbook_code=%d)",
	(int)book->code, (int)subbook_code));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Check for the subbook-code.
     */
    if (subbook_code < 0 || book->subbook_count <= subbook_code) {
	error_code = EB_ERR_NO_SUCH_SUB;
	goto failed;
    }

    strcpy(title, (book->subbooks + subbook_code)->title);

    LOG(("out: eb_subbook_title2(title=%s) = %s", title,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *title = '\0';
    LOG(("out: eb_subbook_title2() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Return a directory name of the current subbook.
 */
EB_Error_Code
eb_subbook_directory(EB_Book *book, char *directory)
{
    EB_Error_Code error_code;
    char *p;

    eb_lock(&book->lock);
    LOG(("in: eb_subbook_directory(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Copy directory name.
     * Upper letters are converted to lower letters.
     */
    strcpy(directory, book->subbook_current->directory_name);
    for (p = directory; *p != '\0'; p++) {
	if ('A' <= *p && *p <= 'Z')
	    *p = ASCII_TOLOWER(*p);
    }

    LOG(("out: eb_subbook_directory(directory=%s) = %s", directory,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *directory = '\0';
    LOG(("out: eb_subbook_directory() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Return a directory name of the specified subbook `subbook_code'.
 */
EB_Error_Code
eb_subbook_directory2(EB_Book *book, EB_Subbook_Code subbook_code,
    char *directory)
{
    EB_Error_Code error_code;
    char *p;

    eb_lock(&book->lock);
    LOG(("in: eb_subbook_directory2(book=%d, subbook_code=%d)",
	(int)book->code, (int)subbook_code));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Check for the subbook-code.
     */
    if (subbook_code < 0 || book->subbook_count <= subbook_code) {
	error_code = EB_ERR_NO_SUCH_SUB;
	goto failed;
    }

    /*
     * Copy directory name.
     * Upper letters are converted to lower letters.
     */
    strcpy(directory, (book->subbooks + subbook_code)->directory_name);
    for (p = directory; *p != '\0'; p++) {
	if ('A' <= *p && *p <= 'Z')
	    *p = ASCII_TOLOWER(*p);
    }

    LOG(("out: eb_subbook_directory2(directory=%s) = %s", directory,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *directory = '\0';
    LOG(("out: eb_subbook_directory2() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Set the subbook `subbook_code' as the current subbook.
 */
EB_Error_Code
eb_set_subbook(EB_Book *book, EB_Subbook_Code subbook_code)
{
    EB_Error_Code error_code = EB_SUCCESS;

    eb_lock(&book->lock);
    LOG(("in: eb_set_subbook(book=%d, subbook_code=%d)",
	(int)book->code, (int)subbook_code));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Check for the subbook-code.
     */
    if (subbook_code < 0 || book->subbook_count <= subbook_code) {
	error_code = EB_ERR_NO_SUCH_SUB;
	goto failed;
    }

    /*
     * If the subbook has already been set as the current subbook,
     * there is nothing to be done.
     * Otherwise close the previous subbook.
     */
    if (book->subbook_current != NULL) {
	if (book->subbook_current->code == subbook_code)
	    goto succeeded;
	eb_unset_subbook(book);
    }

    /*
     * Set the current subbook.
     */
    book->subbook_current = book->subbooks + subbook_code;

    /*
     * Dispatch.
     */
    if (book->disc_code == EB_DISC_EB)
	error_code = eb_set_subbook_eb(book, subbook_code);
    else
	error_code = eb_set_subbook_epwing(book, subbook_code);
    if (error_code != EB_SUCCESS)
	goto failed;

    /*
     * Load the subbook.
     */
    error_code = eb_load_subbook(book);
    if (error_code != EB_SUCCESS)
	goto failed;

    /*
     * Load font files.
     */
    eb_load_font_headers(book);

  succeeded:
    book->subbook_current->initialized = 1;
    LOG(("out: eb_set_subbook() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return error_code;

    /*
     * An error occurs...
     */
  failed:
    if (book->subbook_current != NULL) {
	zio_close(&book->subbook_current->text_zio);
	zio_close(&book->subbook_current->graphic_zio);
	zio_close(&book->subbook_current->sound_zio);
	zio_close(&book->subbook_current->movie_zio);
    }
    book->subbook_current = NULL;
    LOG(("out: eb_set_subbook() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Set the subbook `subbook_code' as the current subbook.
 */
static EB_Error_Code
eb_set_subbook_eb(EB_Book *book, EB_Subbook_Code subbook_code)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    char text_path_name[EB_MAX_PATH_LENGTH + 1];
    char graphic_path_name[EB_MAX_PATH_LENGTH + 1];
    Zio_Code text_zio_code;
    Zio_Code graphic_zio_code;

    LOG(("in: eb_set_subbook_eb(book=%d, subbook_code=%d)",
	(int)book->code, (int)subbook_code));

    subbook = book->subbook_current;

    /*
     * Open a text file if exists.
     */
    text_zio_code = ZIO_INVALID;

    if (subbook->initialized) {
	if (zio_mode(&subbook->text_zio) != ZIO_INVALID)
	    text_zio_code = ZIO_REOPEN;
    } else {
	eb_canonicalize_file_name(subbook->text_file_name);
	if (eb_find_file_name2(book->path, subbook->directory_name,
	    EB_FILE_NAME_START, subbook->text_file_name) == EB_SUCCESS) {
	    eb_path_name_zio_code(subbook->text_file_name, ZIO_PLAIN,
		&text_zio_code);
	}
    }

    if (text_zio_code != ZIO_INVALID) {
	eb_compose_path_name2(book->path, subbook->directory_name,
	    subbook->text_file_name, text_path_name);
	if (zio_open(&subbook->text_zio, text_path_name, text_zio_code) < 0) {
	    error_code = EB_ERR_FAIL_OPEN_TEXT;
	    goto failed;
	}
	text_zio_code = zio_mode(&subbook->text_zio);
    }

    /*
     * Open a graphic file if exists.
     */
    graphic_zio_code = ZIO_INVALID;

    if (subbook->initialized) {
	if (zio_mode(&subbook->graphic_zio) != ZIO_INVALID)
	    graphic_zio_code = ZIO_REOPEN;
    } else if (text_zio_code != ZIO_INVALID) {
	strcpy(subbook->graphic_file_name, subbook->text_file_name);
	graphic_zio_code = text_zio_code;
    }

    if (graphic_zio_code != ZIO_INVALID) {
	eb_compose_path_name2(book->path, subbook->directory_name,
	    subbook->graphic_file_name, graphic_path_name);
	if (zio_open(&subbook->graphic_zio, graphic_path_name,
	    graphic_zio_code) < 0) {
	    error_code = EB_ERR_FAIL_OPEN_BINARY;
	    goto failed;
	}
	graphic_zio_code = zio_mode(&subbook->graphic_zio);
    }

    LOG(("out: eb_set_subbook_eb() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_set_subbook_eb() = %s", eb_error_string(error_code)));
    return error_code;
}

/*
 * Set the subbook `subbook_code' as the current subbook.
 */
static EB_Error_Code
eb_set_subbook_epwing(EB_Book *book, EB_Subbook_Code subbook_code)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    char text_path_name[EB_MAX_PATH_LENGTH + 1];
    char graphic_path_name[EB_MAX_PATH_LENGTH + 1];
    char sound_path_name[EB_MAX_PATH_LENGTH + 1];
    Zio_Code text_zio_code;
    Zio_Code graphic_zio_code;
    Zio_Code sound_zio_code;

    LOG(("in: eb_set_subbook_epwing(book=%d, subbook_code=%d)",
	(int)book->code, (int)subbook_code));

    subbook = book->subbook_current;

    if (!subbook->initialized) {
	/*
	 * Adjust directory names.
	 */
	strcpy(subbook->data_directory_name, EB_DIRECTORY_NAME_DATA);
	eb_fix_directory_name2(book->path, subbook->directory_name,
	    subbook->data_directory_name);

	strcpy(subbook->gaiji_directory_name, EB_DIRECTORY_NAME_GAIJI);
	eb_fix_directory_name2(book->path, subbook->directory_name,
	    subbook->gaiji_directory_name);

	strcpy(subbook->movie_directory_name, EB_DIRECTORY_NAME_MOVIE);
	eb_fix_directory_name2(book->path, subbook->directory_name,
	    subbook->movie_directory_name);
    }

    /*
     * Open a text file if exists.
     *
     * If a subbook has stream data only, its index_page has been set
     * to 0.  In this case, we must not try to open a text file of
     * the subbook, since the text file may be for another subbook.
     * Remember that subbooks can share a `data' sub-directory.
     */
    text_zio_code = ZIO_INVALID;

    if (subbook->initialized) {
	if (zio_mode(&subbook->text_zio) != ZIO_INVALID)
	    text_zio_code = ZIO_REOPEN;
    } else if (subbook->index_page > 0) {
	eb_canonicalize_file_name(subbook->text_file_name);
	if (eb_find_file_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->text_file_name,
	    subbook->text_file_name) == EB_SUCCESS) {
	    eb_path_name_zio_code(subbook->text_file_name,
		subbook->text_hint_zio_code, &text_zio_code);
	}
    }

    if (text_zio_code != ZIO_INVALID) {
	eb_compose_path_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->text_file_name,
	    text_path_name);
	if (zio_open(&subbook->text_zio, text_path_name, text_zio_code) < 0) {
	    subbook = NULL;
	    error_code = EB_ERR_FAIL_OPEN_TEXT;
	    goto failed;
	}
	text_zio_code = zio_mode(&subbook->text_zio);
    }

    /*
     * Open a graphic file if exists.
     */
    graphic_zio_code = ZIO_INVALID;

    if (subbook->initialized) {
	if (zio_mode(&subbook->graphic_zio) != ZIO_INVALID)
	    graphic_zio_code = ZIO_REOPEN;
    } else if (text_zio_code != ZIO_INVALID) {
	eb_canonicalize_file_name(subbook->graphic_file_name);
	if (eb_find_file_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->graphic_file_name,
	    subbook->graphic_file_name) == EB_SUCCESS) {
	    eb_path_name_zio_code(subbook->graphic_file_name,
		subbook->graphic_hint_zio_code, &graphic_zio_code);
	}
    }

    if (graphic_zio_code != ZIO_INVALID) {
	eb_compose_path_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->graphic_file_name,
	    graphic_path_name);
	if (zio_open(&subbook->graphic_zio, graphic_path_name,
	    graphic_zio_code) < 0) {
	    error_code = EB_ERR_FAIL_OPEN_BINARY;
	    goto failed;
	}
	graphic_zio_code = zio_mode(&subbook->graphic_zio);
    }

    /*
     * Open a sound file if exists.
     */
    sound_zio_code = ZIO_INVALID;

    if (subbook->initialized) {
	if (zio_mode(&subbook->sound_zio) != ZIO_INVALID)
	    sound_zio_code = ZIO_REOPEN;
    } else if (text_zio_code != ZIO_INVALID) {
	eb_canonicalize_file_name(subbook->sound_file_name);
	if (eb_find_file_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->sound_file_name,
	    subbook->sound_file_name) == EB_SUCCESS) {
	    eb_path_name_zio_code(subbook->sound_file_name,
		subbook->sound_hint_zio_code, &sound_zio_code);
	}
    }

    if (sound_zio_code != ZIO_INVALID) {
	eb_compose_path_name3(book->path, subbook->directory_name,
	    subbook->data_directory_name, subbook->sound_file_name,
	    sound_path_name);
	if (zio_open(&subbook->sound_zio, sound_path_name,
	    sound_zio_code) < 0) {
	    error_code = EB_ERR_FAIL_OPEN_BINARY;
	    goto failed;
	}
	sound_zio_code = zio_mode(&subbook->sound_zio);
    }

    LOG(("out: eb_set_subbook_epwing() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_set_subbook_epwing() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Unset the current subbook.
 */
void
eb_unset_subbook(EB_Book *book)
{
    eb_lock(&book->lock);
    LOG(("in: eb_unset_subbook(book=%d)", (int)book->code));

    /*
     * Close the file of the current subbook.
     */
    if (book->subbook_current != NULL) {
	eb_unset_font(book);
	eb_unset_binary(book);
	zio_close(&book->subbook_current->text_zio);
	zio_close(&book->subbook_current->graphic_zio);
	zio_close(&book->subbook_current->sound_zio);
	zio_close(&book->subbook_current->movie_zio);
	book->subbook_current = NULL;
    }

    LOG(("out: eb_unset_subbook()"));
    eb_unlock(&book->lock);
}


