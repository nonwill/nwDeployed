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
#include "build-post.h"

/*
 * Get information about the current subbook.
 */
EB_Error_Code
eb_load_multi_searches(EB_Book *book)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    EB_Multi_Search *multi;
    EB_Search *entry;
    char buffer[EB_SIZE_PAGE];
    char *buffer_p;
    int index_count;
    int index_id;
    int i, j, k;

    LOG(("in: eb_load_multi_searches(book=%d)", book->code));

    subbook = book->subbook_current;

    for (i = 0, multi = subbook->multis; i < subbook->multi_count;
	 i++, multi++) {
	/*
	 * Read the index table page of the multi search.
	 */
	if (zio_lseek(&subbook->text_zio,
	    ((off_t) multi->search.start_page - 1) * EB_SIZE_PAGE, SEEK_SET)
	    < 0) {
	    error_code = EB_ERR_FAIL_SEEK_TEXT;
	    goto failed;
	}
	if (zio_read(&subbook->text_zio, buffer, EB_SIZE_PAGE)
	    != EB_SIZE_PAGE) {
	    error_code = EB_ERR_FAIL_READ_TEXT;
	    goto failed;
	}

	/*
	 * Get the number of entries in this multi search.
	 */
	multi->entry_count = eb_uint2(buffer);
	if (EB_MAX_MULTI_SEARCHES <= multi->entry_count) {
	    error_code = EB_ERR_UNEXP_TEXT;
	    goto failed;
	}

	buffer_p = buffer + 16;
	for (j = 0, entry = multi->entries;
	     j < multi->entry_count; j++, entry++) {
	    /*
	     * Get the number of indexes in this entry, and title
	     * of this entry.
	     */
	    index_count = eb_uint1(buffer_p);
	    strncpy(entry->label, buffer_p + 2, EB_MAX_MULTI_LABEL_LENGTH);
	    entry->label[EB_MAX_MULTI_LABEL_LENGTH] = '\0';
	    eb_jisx0208_to_euc(entry->label, entry->label);
	    buffer_p += EB_MAX_MULTI_LABEL_LENGTH + 2;

	    /*
	     * Initialize index page information of the entry.
	     */
	    for (k = 0; k < index_count; k++) {
		/*
		 * Get the index page information of the entry.
		 */
		index_id = eb_uint1(buffer_p);
		switch (index_id) {
		case 0x71:
		case 0x91:
		case 0xa1:
		    if (entry->start_page != 0 && entry->index_id != 0x71)
			break;
		    entry->start_page = eb_uint4(buffer_p + 2);
		    entry->end_page = entry->start_page
			+ eb_uint4(buffer_p + 6) - 1;
		    entry->index_id = index_id;
		    entry->katakana         = EB_INDEX_STYLE_ASIS;
		    entry->lower            = EB_INDEX_STYLE_CONVERT;
		    entry->mark             = EB_INDEX_STYLE_ASIS;
		    entry->long_vowel       = EB_INDEX_STYLE_ASIS;
		    entry->double_consonant = EB_INDEX_STYLE_ASIS;
		    entry->contracted_sound = EB_INDEX_STYLE_ASIS;
		    entry->voiced_consonant = EB_INDEX_STYLE_ASIS;
		    entry->small_vowel      = EB_INDEX_STYLE_ASIS;
		    entry->p_sound          = EB_INDEX_STYLE_ASIS;
		    entry->space            = EB_INDEX_STYLE_ASIS;
		    break;
		case 0x01:
		    entry->candidates_page = eb_uint4(buffer_p + 2);
		    break;
		}
		buffer_p += 16;
	    }
	}
    }

    LOG(("out: eb_load_multi_searches() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_load_multi_searches() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Default multi search titles (written in JIS X 0208).
 */
static const char *default_multi_titles_jisx0208[] = {
    "J#9g8!:w#1",    /* Multi search 1. */
    "J#9g8!:w#2",    /* Multi search 2. */
    "J#9g8!:w#3",    /* Multi search 3. */
    "J#9g8!:w#4",    /* Multi search 4. */
    "J#9g8!:w#5",    /* Multi search 5. */
    "J#9g8!:w#6",    /* Multi search 6. */
    "J#9g8!:w#7",    /* Multi search 7. */
    "J#9g8!:w#8",    /* Multi search 8. */
    "J#9g8!:w#9",    /* Multi search 9. */
    "J#9g8!:w#1#0",  /* Multi search 10. */
};

/*
 * Default multi search titles (written in ASCII, subset of ISO 8859-1).
 */
static const char *default_multi_titles_latin[] = {
    "Multi search 1",
    "Multi search 2",
    "Multi search 3",
    "Multi search 4",
    "Multi search 5",
    "Multi search 6",
    "Multi search 7",
    "Multi search 8",
    "Multi search 9",
    "Multi search 10",
};

/*
 * Load multi search titles.
 */
EB_Error_Code
eb_load_multi_titles(EB_Book *book)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    char buffer[EB_SIZE_PAGE];
    int title_count;
    char *title;
    size_t offset;
    int i;

    LOG(("in: eb_load_multi_searches(book=%d)", book->code));

    subbook = book->subbook_current;

    /*
     * Set default titles.
     */
    if (book->character_code == EB_CHARCODE_ISO8859_1
	|| book->character_code == EB_CHARCODE_UTF8) {
	for (i = 0; i < subbook->multi_count; i++) {
	    title = subbook->multis[i].title;
	    strcpy(title, default_multi_titles_latin[i]);
	}
    } else {
	for (i = 0; i < subbook->multi_count; i++) {
	    title = subbook->multis[i].title;
	    strcpy(title, default_multi_titles_jisx0208[i]);
	    eb_jisx0208_to_euc(title, title);
	}
    }

    if (book->disc_code != EB_DISC_EPWING || subbook->search_title_page == 0)
	goto succeeded;

    /*
     * Read the page of the multi search.
     */
    if (zio_lseek(&subbook->text_zio,
	((off_t) subbook->search_title_page - 1) * EB_SIZE_PAGE, SEEK_SET)
	< 0) {
	error_code = EB_ERR_FAIL_SEEK_TEXT;
	goto failed;
    }
    if (zio_read(&subbook->text_zio, buffer, EB_SIZE_PAGE) != EB_SIZE_PAGE) {
	error_code = EB_ERR_FAIL_READ_TEXT;
	goto failed;
    }

    title_count = eb_uint2(buffer);
    if (EB_MAX_SEARCH_TITLES < title_count)
	title_count = EB_MAX_SEARCH_TITLES;

    /*
     * We need titles for multi searches only.
     *     titles[ 0]: title for word and endword searches.
     *     titles[ 1]: title for keyword search.
     *     titles[ 2]: common title for all multi searches.
     *                (we don't need this)
     *     titles[ 3]: title for multi search 1.
     *         :
     *     titles[12]: title for multi search 10.
     *     titles[13]: title for menu search.
     *
     * The offset of titles[3] is:
     *     the number of entries(2bytes)
     *     + reserved 1 (68bytes)
     *     + title for word and endword searches (70bytes)
     *     + title for keyword search (70bytes)
     *     + common title for all multi searches (70bytes)
     *     + reserved 2 (70bytes)
     *     = 2 + 68 + 70 + 70 + 70 + 70 = 350
     */
    for (i = 4, offset = 350; i < EB_MAX_SEARCH_TITLES; i++, offset += 70) {
	if (subbook->multi_count <= i - 4)
	    break;
	if (eb_uint2(buffer + offset) != 0x02)
	    continue;

	/*
	 * Each titles[] consists of
	 *    parameter (2bytes)
	 *    short title (16bytes)
	 *    long title (32bytes)
	 * We get long title rather than short one.
	 */
	title = subbook->multis[i - 4].title;
	strncpy(title, buffer + offset + 2 + 16, EB_MAX_MULTI_TITLE_LENGTH);
	title[EB_MAX_MULTI_TITLE_LENGTH] = '\0';
	eb_jisx0208_to_euc(title, title);
    }

succeeded:
    LOG(("out: eb_load_multi_titles() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_load_multi_titles() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Examine whether the current subbook in `book' supports `MULTI SEARCH'
 * or not.
 */
int
eb_have_multi_search(EB_Book *book)
{
    eb_lock(&book->lock);
    LOG(("in: eb_have_multi_search(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL)
	goto failed;

    if (book->subbook_current->multi_count == 0)
	goto failed;

    LOG(("out: eb_have_multi_search() = %d", 1));
    eb_unlock(&book->lock);

    return 1;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_have_multi_search() = %d", 0));
    eb_unlock(&book->lock);
    return 0;
}


/*
 * Return a title of the multi search `multi_id'.
 */
EB_Error_Code
eb_multi_title(EB_Book *book, EB_Multi_Search_Code multi_id, char *title)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;

    eb_lock(&book->lock);
    LOG(("in: eb_multi_title(book=%d, multi_id=%d)",
	(int)book->code, (int)multi_id));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Current subbook must have been set.
     */
    subbook = book->subbook_current;
    if (subbook == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * `multi_id' must be a valid code.
     */
    if (multi_id < 0 || subbook->multi_count <= multi_id) {
	error_code = EB_ERR_NO_SUCH_MULTI_ID;
	goto failed;
    }

    strcpy(title, subbook->multis[multi_id].title);

    LOG(("out: eb_multi_title(title=%s) = %s", title,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *title = '\0';
    LOG(("out: eb_multi_title() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Return a list of multi search ids in `book'.
 */
EB_Error_Code
eb_multi_search_list(EB_Book *book, EB_Multi_Search_Code *search_list,
    int *search_count)
{
    EB_Error_Code error_code;
    EB_Subbook_Code *list_p;
    int i;

    eb_lock(&book->lock);
    LOG(("in: eb_multi_search_list(book=%d)", (int)book->code));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    *search_count = book->subbook_current->multi_count;
    for (i = 0, list_p = search_list; i < *search_count; i++, list_p++)
	*list_p = i;

    LOG(("out: eb_multi_search_list(search_count=%d) = %s", *search_count,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *search_count = 0;
    LOG(("out: eb_multi_search_list() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Return the number of entries that the multi search `multi_id' in `book'.
 */
EB_Error_Code
eb_multi_entry_count(EB_Book *book, EB_Multi_Search_Code multi_id,
    int *entry_count)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_multi_entry_count(book=%d, multi_id=%d)", (int)book->code,
	(int)multi_id));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * `multi_id' must be a valid code.
     */
    if (multi_id < 0 || book->subbook_current->multi_count <= multi_id) {
	error_code = EB_ERR_NO_SUCH_MULTI_ID;
	goto failed;
    }

    *entry_count = book->subbook_current->multis[multi_id].entry_count;

    LOG(("out: eb_multi_entry_count(entry_count=%d) = %s", (int)*entry_count,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *entry_count = 0;
    LOG(("out: eb_multi_entry_count() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Return a list of entries that the multi search `multi_id' in `book' has.
 * (Legacy function)
 */
EB_Error_Code
eb_multi_entry_list(EB_Book *book, EB_Multi_Search_Code multi_id,
    int *entry_list, int *entry_count)
{
    EB_Error_Code error_code;
    EB_Subbook_Code *list_p;
    int i;

    error_code = eb_multi_entry_count(book, multi_id, entry_count);
    if (error_code != EB_SUCCESS)
	return error_code;

    for (i = 0, list_p = entry_list; i < *entry_count; i++, list_p++)
	*list_p = i;

    return EB_SUCCESS;
}


/*
 * Return a lable of the entry `entry_index' in the multi search `multi_id'.
 */
EB_Error_Code
eb_multi_entry_label(EB_Book *book, EB_Multi_Search_Code multi_id,
    int entry_index, char *label)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;

    eb_lock(&book->lock);
    LOG(("in: eb_multi_entry_label(book=%d, multi_id=%d, entry_index=%d)",
	(int)book->code, (int)multi_id, entry_index));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Current subbook must have been set.
     */
    subbook = book->subbook_current;
    if (subbook == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * `multi_id' must be a valid code.
     */
    if (multi_id < 0 || subbook->multi_count <= multi_id) {
	error_code = EB_ERR_NO_SUCH_MULTI_ID;
	goto failed;
    }

    /*
     * `entry_index' must be a valid code.
     */
    if (entry_index < 0
	|| subbook->multis[multi_id].entry_count <= entry_index) {
	error_code = EB_ERR_NO_SUCH_ENTRY_ID;
	goto failed;
    }

    strcpy(label, subbook->multis[multi_id].entries[entry_index].label);

    LOG(("out: eb_multi_entry_label(label=%s) = %s", label,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *label = '\0';
    LOG(("out: eb_multi_entry_label() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Whether the entry `entry_index' in the multi search `multi_id' has
 * candidates or not.
 */
int
eb_multi_entry_have_candidates(EB_Book *book, EB_Multi_Search_Code multi_id,
    int entry_index)
{
    EB_Multi_Search *multi;

    eb_lock(&book->lock);
    LOG(("in: eb_multi_entry_have_candidates(book=%d, multi_id=%d, \
entry_index=%d)",
	(int)book->code, (int)multi_id, entry_index));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL)
	goto failed;

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL)
	goto failed;

    /*
     * `multi_id' must be a valid code.
     */
    if (multi_id < 0 || book->subbook_current->multi_count <= multi_id)
	goto failed;

    /*
     * `entry_index' must be a valid code.
     */
    multi = book->subbook_current->multis + multi_id;
    if (entry_index < 0 || multi->entry_count <= entry_index)
	goto failed;

    if (multi->entries[entry_index].candidates_page == 0)
	goto failed;

    LOG(("out: eb_multi_entry_have_candidates() = %d", 1));
    eb_unlock(&book->lock);

    return 1;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_multi_entry_have_candidates() = %d", 0));
    eb_unlock(&book->lock);
    return 0;
}


/*
 * Return a position of candidates for the entry `entry_index' in the multi
 * search `multi_id'.
 */
EB_Error_Code
eb_multi_entry_candidates(EB_Book *book, EB_Multi_Search_Code multi_id,
    int entry_index, EB_Position *position)
{
    EB_Error_Code error_code;
    EB_Multi_Search *multi;

    eb_lock(&book->lock);
    LOG(("in: eb_multi_entry_candidates(book=%d, multi_id=%d, entry_index=%d)",
	(int)book->code, (int)multi_id, entry_index));

    /*
     * The book must have been bound.
     */
    if (book->path == NULL) {
	error_code = EB_ERR_UNBOUND_BOOK;
	goto failed;
    }

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * `multi_id' must be a valid code.
     */
    if (multi_id < 0 || book->subbook_current->multi_count <= multi_id) {
	error_code = EB_ERR_NO_SUCH_MULTI_ID;
	goto failed;
    }

    /*
     * `entry_index' must be a valid code.
     */
    multi = book->subbook_current->multis + multi_id;
    if (entry_index < 0 || multi->entry_count <= entry_index) {
	error_code = EB_ERR_NO_SUCH_ENTRY_ID;
	goto failed;
    }

    if (multi->entries[entry_index].candidates_page == 0) {
	error_code = EB_ERR_NO_CANDIDATES;
	goto failed;
    }

    position->page = multi->entries[entry_index].candidates_page;
    position->offset = 0;

    LOG(("out: eb_multi_entry_candidates(position={%d,%d}) = %s",
	position->page, position->offset, eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_multi_entry_candidates() = %s",
	eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Multi search.
 */
EB_Error_Code
eb_search_multi(EB_Book *book, EB_Multi_Search_Code multi_id,
    const char * const input_words[])
{
    EB_Error_Code error_code;
    EB_Search_Context *context;
    EB_Search *entry;
    EB_Word_Code word_code;
    int word_count;
    int i;

    eb_lock(&book->lock);
    LOG(("in: eb_search_multi(book=%d, multi_id=%d, input_words=[below])",
	(int)book->code, (int)multi_id));

    if (eb_log_flag) {
	for (i = 0; i < EB_MAX_KEYWORDS && input_words[i] != NULL; i++) {
	    LOG(("    input_words[%d]=%s", i,
		eb_quoted_string(input_words[i])));
	}
	LOG(("    input_words[%d]=NULL", i));
    }

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Check whether the current subbook has keyword search.
     */
    if (multi_id < 0 || book->subbook_current->multi_count <= multi_id) {
	error_code = EB_ERR_NO_SUCH_SEARCH;
	goto failed;
    }

    /*
     * Attach a search context for each keyword, and pre-search the
     * keywords.
     */
    eb_reset_search_contexts(book);
    word_count = 0;

    for (i = 0, entry = book->subbook_current->multis[multi_id].entries;
	 i < book->subbook_current->multis[multi_id].entry_count;
	 i++, entry++) {

	if (input_words[i] == NULL)
	    break;

	/*
	 * Initialize search context.
	 */
	context = book->search_contexts + word_count;
	context->code = EB_SEARCH_MULTI;

	/*
	 * Choose comparison functions.
	 */
	if (entry->candidates_page == 0) {
	    if (book->character_code == EB_CHARCODE_ISO8859_1
		|| book->character_code == EB_CHARCODE_UTF8) {
		context->compare_pre    = eb_pre_match_word;
		context->compare_single = eb_match_word;
		context->compare_group  = eb_match_word;
	    } else {
		context->compare_pre    = eb_pre_match_word;
		context->compare_single = eb_match_word;
		context->compare_group  = eb_match_word_kana_group;
	    }
	} else {
	    if (book->character_code == EB_CHARCODE_ISO8859_1
		|| book->character_code == EB_CHARCODE_UTF8) {
		context->compare_pre    = eb_exact_pre_match_word_latin;
		context->compare_single = eb_exact_match_word_latin;
		context->compare_group  = eb_exact_match_word_latin;
	    } else {
		context->compare_pre    = eb_exact_pre_match_word_jis;
		context->compare_single = eb_exact_match_word_jis;
		context->compare_group  = eb_exact_match_word_kana_group;
	    }
	}
	context->page = entry->start_page;
	if (context->page == 0)
	    continue;

	/*
	 * Make a fixed word and a canonicalized word to search from
	 * `input_words[i]'.
	 */
	error_code = eb_set_multiword(book, multi_id, i, input_words[i],
	    context->word, context->canonicalized_word, &word_code);
	if (error_code == EB_ERR_EMPTY_WORD)
	    continue;
	else if (error_code != EB_SUCCESS)
	    goto failed;

	/*
	 * Pre-search.
	 */
	error_code = eb_presearch_word(book, context);
	if (error_code != EB_SUCCESS)
	    goto failed;

	word_count++;
    }
    if (word_count == 0) {
	error_code = EB_ERR_NO_WORD;
	goto failed;
    } else if (book->subbook_current->multis[multi_id].entry_count <= i
	&& input_words[i] != NULL) {
	error_code =  EB_ERR_TOO_MANY_WORDS;
	goto failed;
    }

    /*
     * Set `EB_SEARCH_NONE' to the rest unused search context.
     */
    for (i = word_count; i < EB_MAX_KEYWORDS; i++)
	(book->search_contexts + i)->code = EB_SEARCH_NONE;

    LOG(("out: eb_search_multi() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_reset_search_contexts(book);
    LOG(("out: eb_search_multi() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


