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
#include "text.h"
#include "build-post.h"

/*
 * Page-ID macros.
 */
#define PAGE_ID_IS_LEAF_LAYER(page_id)		(((page_id) & 0x80) == 0x80)
#define PAGE_ID_IS_LAYER_START(page_id)		(((page_id) & 0x40) == 0x40)
#define PAGE_ID_IS_LAYER_END(page_id)		(((page_id) & 0x20) == 0x20)
#define PAGE_ID_HAVE_GROUP_ENTRY(page_id)	(((page_id) & 0x10) == 0x10)

/*
 * The maximum number of hit entries for tomporary hit lists.
 * This is used in eb_hit_list().
 */
#define EB_TMP_MAX_HITS		64

/*
 * Book-code of the book in which you want to search a word.
 */
static EB_Book_Code cache_book_code = EB_BOOK_NONE;

/*
 * Cache buffer for the current page.
 */
static char cache_buffer[EB_SIZE_PAGE];

/*
 * Cache buffer for the current page.
 */
static int cache_page;

/*
 * Mutex for cache variables.
 */
#ifdef ENABLE_PTHREAD
static pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * Unexported functions.
 */
static EB_Error_Code eb_hit_list_word(EB_Book *book,
    EB_Search_Context *context, int max_hit_count, EB_Hit *hit_list,
    int *hit_count);
static EB_Error_Code eb_hit_list_keyword(EB_Book *book,
    EB_Search_Context *context, int max_hit_count, EB_Hit *hit_list,
    int *hit_count);
static EB_Error_Code eb_hit_list_multi(EB_Book *book,
    EB_Search_Context *context, int max_hit_count, EB_Hit *hit_list,
    int *hit_count);
static void eb_and_hit_lists(EB_Hit and_list[EB_TMP_MAX_HITS],
    int *and_count, int max_and_count, int hit_list_count,
    EB_Hit hit_lists[EB_NUMBER_OF_SEARCH_CONTEXTS][EB_TMP_MAX_HITS],
    int hit_counts[EB_NUMBER_OF_SEARCH_CONTEXTS]);


/*
 * Intialize search contexts of `book'.
 */
void
eb_initialize_search_contexts(EB_Book *book)
{
    EB_Search_Context *context;
    int i;

    LOG(("in: eb_initialize_search_context(book=%d)", (int)book->code));

    for (i = 0, context = book->search_contexts;
	 i < EB_NUMBER_OF_SEARCH_CONTEXTS; i++, context++) {
	context->code = EB_SEARCH_NONE;
	context->compare_pre = NULL;
	context->compare_single = NULL;
	context->compare_group = NULL;
	context->comparison_result = -1;
	context->word[0] = '\0';
	context->canonicalized_word[0] = '\0';
	context->page = 0;
	context->offset = 0;
	context->page_id = 0;
	context->entry_count = 0;
	context->entry_index = 0;
	context->entry_length = 0;
	context->entry_arrangement = EB_ARRANGE_INVALID;
	context->in_group_entry = 0;
	context->keyword_heading.page = 0;
	context->keyword_heading.offset = 0;
    }

    LOG(("out: eb_initialize_search_context()"));
}


/*
 * Finalize search contexts of `book'.
 */
void
eb_finalize_search_contexts(EB_Book *book)
{
    LOG(("in+out: eb_finalize_search_context(book=%d)", (int)book->code));

    /* nothing to be done */
}


/*
 * Reset search context of `book'.
 */
void
eb_reset_search_contexts(EB_Book *book)
{
    LOG(("in: eb_reset_search_context(book=%d)", (int)book->code));

    eb_initialize_search_contexts(book);

    LOG(("out: eb_reset_search_context()"));
}


/*
 * Intialize a search element.
 */
void
eb_initialize_search(EB_Search *search)
{
    search->index_id = 0;
    search->start_page = 0;
    search->end_page = 0;
    search->candidates_page = 0;
    search->katakana = EB_INDEX_STYLE_CONVERT;
    search->lower = EB_INDEX_STYLE_CONVERT;
    search->mark = EB_INDEX_STYLE_DELETE;
    search->long_vowel = EB_INDEX_STYLE_CONVERT;
    search->double_consonant = EB_INDEX_STYLE_CONVERT;
    search->contracted_sound = EB_INDEX_STYLE_CONVERT;
    search->voiced_consonant = EB_INDEX_STYLE_CONVERT;
    search->small_vowel = EB_INDEX_STYLE_CONVERT;
    search->p_sound = EB_INDEX_STYLE_CONVERT;
    search->space = EB_INDEX_STYLE_DELETE;
    search->label[0] = '\0';
}


/*
 * Finalize a search element.
 */
void
eb_finalize_search(EB_Search *search)
{
    /* nothing to be done */
}


/*
 * Initialize all search elements in the current subbook.
 */
void
eb_initialize_searches(EB_Book *book)
{
    EB_Subbook *subbook;
    EB_Multi_Search *multi;
    EB_Search *entry;
    int i, j;

    LOG(("in: eb_initialize_searches(book=%d)", (int)book->code));

    subbook = book->subbook_current;

    eb_initialize_search(&subbook->word_alphabet);
    eb_initialize_search(&subbook->word_asis);
    eb_initialize_search(&subbook->word_kana);
    eb_initialize_search(&subbook->endword_alphabet);
    eb_initialize_search(&subbook->endword_asis);
    eb_initialize_search(&subbook->endword_kana);
    eb_initialize_search(&subbook->keyword);
    eb_initialize_search(&subbook->cross);
    eb_initialize_search(&subbook->menu);
    eb_initialize_search(&subbook->image_menu);
    eb_initialize_search(&subbook->copyright);
    eb_initialize_search(&subbook->text);
    eb_initialize_search(&subbook->sound);

    for (i = 0, multi = subbook->multis; i < EB_MAX_MULTI_SEARCHES;
	 i++, multi++) {
	eb_initialize_search(&multi->search);
	multi->title[0] = '\0';
	multi->entry_count = 0;
	for (j = 0, entry = multi->entries;
	     j < EB_MAX_MULTI_ENTRIES; j++, entry++) {
	    eb_initialize_search(entry);
	}
    }

    LOG(("out: eb_initialize_searches(book=%d)", (int)book->code));
}


/*
 * Finalize all search elements in the current subbook.
 */
void
eb_finalize_searches(EB_Book *book)
{
    EB_Subbook *subbook;
    EB_Multi_Search *multi;
    EB_Search *entry;
    int i, j;

    LOG(("in: eb_finalize_searches(book=%d)", (int)book->code));

    subbook = book->subbook_current;

    eb_finalize_search(&subbook->word_alphabet);
    eb_finalize_search(&subbook->word_asis);
    eb_finalize_search(&subbook->word_kana);
    eb_finalize_search(&subbook->endword_alphabet);
    eb_finalize_search(&subbook->endword_asis);
    eb_finalize_search(&subbook->endword_kana);
    eb_finalize_search(&subbook->keyword);
    eb_finalize_search(&subbook->menu);
    eb_finalize_search(&subbook->image_menu);
    eb_finalize_search(&subbook->copyright);
    eb_finalize_search(&subbook->text);
    eb_finalize_search(&subbook->sound);

    for (i = 0, multi = subbook->multis; i < EB_MAX_KEYWORDS;
	 i++, multi++) {
	eb_finalize_search(&multi->search);
	multi->entry_count = 0;
	for (j = 0, entry = multi->entries;
	     j < multi->entry_count; j++, entry++) {
	    eb_finalize_search(entry);
	}
    }

    LOG(("out: eb_finalize_searches()"));
}


/*
 * Pre-search for a word described in the current search context.
 * It descends intermediate indexes and reached to a leaf page that
 * may have the word.
 * If succeeded, 0 is returned.  Otherwise -1 is returned.
 */
EB_Error_Code
eb_presearch_word(EB_Book *book, EB_Search_Context *context)
{
    EB_Error_Code error_code;
    int next_page;
    int index_depth;
    char *cache_p;

    pthread_mutex_lock(&cache_mutex);
    LOG(("in: eb_presearch_word(book=%d)", (int)book->code));

    /*
     * Discard cache data.
     */
    cache_book_code = EB_BOOK_NONE;

    /*
     * Search the word in intermediate indexes.
     * Find a page number of the leaf index page.
     */
    for (index_depth = 0; index_depth < EB_MAX_INDEX_DEPTH; index_depth++) {
	next_page = context->page;

	/*
	 * Seek and read a page.
	 */
	if (zio_lseek(&book->subbook_current->text_zio,
	    ((off_t) context->page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
	    cache_book_code = EB_BOOK_NONE;
	    error_code = EB_ERR_FAIL_SEEK_TEXT;
	    goto failed;
	}
	if (zio_read(&book->subbook_current->text_zio, cache_buffer,
	    EB_SIZE_PAGE) != EB_SIZE_PAGE) {
	    cache_book_code = EB_BOOK_NONE;
	    error_code = EB_ERR_FAIL_READ_TEXT;
	    goto failed;
	}

	/*
	 * Get some data from the read page.
	 */
	context->page_id = eb_uint1(cache_buffer);
	context->entry_length = eb_uint1(cache_buffer + 1);
	if (context->entry_length == 0)
	    context->entry_arrangement = EB_ARRANGE_VARIABLE;
	else
	    context->entry_arrangement = EB_ARRANGE_FIXED;
	context->entry_count = eb_uint2(cache_buffer + 2);
	context->offset = 4;
	cache_p = cache_buffer + 4;

	LOG(("aux: eb_presearch_word(page=%d, page_id=0x%02x, \
entry_length=%d, entry_arrangement=%d, entry_count=%d)",
	    context->page, context->page_id, context->entry_length,
	    context->entry_arrangement, context->entry_count));

	/*
	 * Exit the loop if it reached to the leaf index.
	 */
	if (PAGE_ID_IS_LEAF_LAYER(context->page_id))
	    break;

	/*
	 * Search a page of next level index.
	 */
	for (context->entry_index = 0;
	     context->entry_index < context->entry_count;
	     context->entry_index++) {
	    if (EB_SIZE_PAGE < context->offset + context->entry_length + 4) {
		error_code = EB_ERR_UNEXP_TEXT;
		goto failed;
	    }
	    if (context->compare_pre(context->canonicalized_word, cache_p,
		context->entry_length) <= 0) {
		next_page = eb_uint4(cache_p + context->entry_length);
		break;
	    }
	    cache_p += context->entry_length + 4;
	    context->offset += context->entry_length + 4;
	}
	if (context->entry_count <= context->entry_index
	    || context->page == next_page) {
	    context->comparison_result = -1;
	    goto succeeded;
	}
	context->page = next_page;
    }

    /*
     * Check for the index depth.
     */
    if (index_depth == EB_MAX_INDEX_DEPTH) {
	error_code = EB_ERR_UNEXP_TEXT;
	goto failed;
    }

    /*
     * Update search context and cache information.
     */
    context->entry_index = 0;
    context->comparison_result = 1;
    context->in_group_entry = 0;
    cache_book_code = book->code;
    cache_page = context->page;

  succeeded:
    LOG(("out: eb_presearch_word() = %s", eb_error_string(EB_SUCCESS)));
    pthread_mutex_unlock(&cache_mutex);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_presearch_word() = %s", eb_error_string(error_code)));
    pthread_mutex_unlock(&cache_mutex);
    return error_code;
}

/*
 * Get hit entries of a submitted search request.
 */
EB_Error_Code
eb_hit_list(EB_Book *book, int max_hit_count, EB_Hit *hit_list, int *hit_count)
{
    EB_Error_Code error_code;
    EB_Search_Context temporary_context;
    EB_Hit temporary_hit_lists[EB_NUMBER_OF_SEARCH_CONTEXTS][EB_TMP_MAX_HITS];
    int temporary_hit_counts[EB_NUMBER_OF_SEARCH_CONTEXTS];
    int more_hit_count;
    int i;

    /*
     * Lock cache data and the book.
     */
    pthread_mutex_lock(&cache_mutex);
    eb_lock(&book->lock);
    LOG(("in: eb_hit_list(book=%d, max_hit_count=%d)", (int)book->code,
	max_hit_count));

    if (max_hit_count == 0)
	goto succeeded;

    *hit_count = 0;

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Get a list of hit entries.
     */
    switch (book->search_contexts->code) {
    case EB_SEARCH_EXACTWORD:
    case EB_SEARCH_WORD:
    case EB_SEARCH_ENDWORD:
	/*
	 * In case of exactword, word of endword search.
	 */
	error_code = eb_hit_list_word(book, book->search_contexts,
	    max_hit_count, hit_list, hit_count);
	if (error_code != EB_SUCCESS)
	    goto failed;
	break;

    case EB_SEARCH_KEYWORD:
    case EB_SEARCH_CROSS:
	/*
	 * In case of keyword or cross search.
	 */
	for (;;) {
	    int search_is_over = 0;

	    for (i = 0; i < EB_MAX_KEYWORDS; i++) {
		if (book->search_contexts[i].code != EB_SEARCH_KEYWORD
		    && book->search_contexts[i].code != EB_SEARCH_CROSS)
		    break;
		memcpy(&temporary_context, book->search_contexts + i,
		    sizeof(EB_Search_Context));
		error_code = eb_hit_list_keyword(book, &temporary_context,
		    EB_TMP_MAX_HITS, temporary_hit_lists[i],
		    temporary_hit_counts + i);
		if (error_code != EB_SUCCESS)
		    goto failed;
		if (temporary_hit_counts[i] == 0) {
		    search_is_over = 1;
		    break;
		}
	    }
	    if (search_is_over)
		break;

	    eb_and_hit_lists(hit_list + *hit_count, &more_hit_count,
		max_hit_count - *hit_count, i, temporary_hit_lists,
		temporary_hit_counts);

	    for (i = 0; i < EB_MAX_MULTI_ENTRIES; i++) {
		if (book->search_contexts[i].code != EB_SEARCH_KEYWORD
		    && book->search_contexts[i].code != EB_SEARCH_CROSS)
		    break;
		error_code = eb_hit_list_keyword(book,
		    book->search_contexts + i, temporary_hit_counts[i],
		    temporary_hit_lists[i], temporary_hit_counts + i);
		if (error_code != EB_SUCCESS)
		    goto failed;
	    }

	    *hit_count += more_hit_count;
	    if (max_hit_count <= *hit_count)
		break;
	}
	break;

    case EB_SEARCH_MULTI:
	/*
	 * In case of multi search.
	 */
	for (;;) {
	    int search_is_over = 0;

	    for (i = 0; i < EB_MAX_MULTI_ENTRIES; i++) {
		if (book->search_contexts[i].code != EB_SEARCH_MULTI)
		    break;
		memcpy(&temporary_context, book->search_contexts + i,
		    sizeof(EB_Search_Context));
		error_code = eb_hit_list_multi(book, &temporary_context,
		    EB_TMP_MAX_HITS, temporary_hit_lists[i],
		    temporary_hit_counts + i);
		if (error_code != EB_SUCCESS)
		    goto failed;
		if (temporary_hit_counts[i] == 0) {
		    search_is_over = 1;
		    break;
		}
	    }
	    if (search_is_over)
		break;

	    eb_and_hit_lists(hit_list + *hit_count, &more_hit_count,
		max_hit_count - *hit_count, i, temporary_hit_lists,
		temporary_hit_counts);

	    for (i = 0; i < EB_MAX_MULTI_ENTRIES; i++) {
		if (book->search_contexts[i].code != EB_SEARCH_MULTI)
		    break;
		error_code = eb_hit_list_multi(book,
		    book->search_contexts + i, temporary_hit_counts[i],
		    temporary_hit_lists[i], temporary_hit_counts + i);
		if (error_code != EB_SUCCESS)
		    goto failed;
	    }

	    *hit_count += more_hit_count;
	    if (max_hit_count <= *hit_count)
		break;
	}
	break;

    default:
	/* not reached */
	error_code = EB_ERR_NO_PREV_SEARCH;
	goto failed;
    }

    /*
     * Unlock cache data and the book.
     */
  succeeded:
    LOG(("out: eb_hit_list(hit_count=%d) = %s",
	*hit_count, eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);
    pthread_mutex_unlock(&cache_mutex);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *hit_count = 0;
    LOG(("out: eb_hit_list() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    pthread_mutex_unlock(&cache_mutex);
    return error_code;
}


/*
 * Get hit entries of a submitted exactword/word/endword search request.
 */
static EB_Error_Code
eb_hit_list_word(EB_Book *book, EB_Search_Context *context, int max_hit_count,
    EB_Hit *hit_list, int *hit_count)
{
    EB_Error_Code error_code;
    EB_Hit *hit;
    int group_id;
    char *cache_p;

    LOG(("in: eb_hit_list_word(book=%d, max_hit_count=%d)", (int)book->code,
	max_hit_count));

    hit = hit_list;
    *hit_count = 0;

    /*
     * If the result of previous comparison is negative value, all
     * matched entries have been found.
     */
    if (context->comparison_result < 0 || max_hit_count <= 0)
	goto succeeded;

    for (;;) {
	/*
	 * Read a page to search, if the page is not on the cache buffer.
	 *
	 * Cache may be missed by the two reasons:
	 *   1. the search process reaches to the end of an index page,
	 *      and tries to read the next page.
	 *   2. Someone else used the cache buffer.
	 *
	 * At the case of 1, the search process reads the page and update
	 * the search context.  At the case of 2. it reads the page but
	 * must not update the context!
	 */
	if (cache_book_code != book->code || cache_page != context->page) {
	    if (zio_lseek(&book->subbook_current->text_zio,
		((off_t) context->page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
		error_code = EB_ERR_FAIL_SEEK_TEXT;
		goto failed;
	    }
	    if (zio_read(&book->subbook_current->text_zio,
		cache_buffer, EB_SIZE_PAGE) != EB_SIZE_PAGE) {
		error_code = EB_ERR_FAIL_READ_TEXT;
		goto failed;
	    }

	    /*
	     * Update search context.
	     */
	    if (context->entry_index == 0) {
		context->page_id = eb_uint1(cache_buffer);
		context->entry_length = eb_uint1(cache_buffer + 1);
		if (context->entry_length == 0)
		    context->entry_arrangement = EB_ARRANGE_VARIABLE;
		else
		    context->entry_arrangement = EB_ARRANGE_FIXED;
		context->entry_count = eb_uint2(cache_buffer + 2);
		context->entry_index = 0;
		context->offset = 4;
	    }

	    cache_book_code = book->code;
	    cache_page = context->page;
	}

	cache_p = cache_buffer + context->offset;

	LOG(("aux: eb_hit_list_word(page=%d, page_id=0x%02x, \
entry_length=%d, entry_arrangement=%d, entry_count=%d)",
	    context->page, context->page_id, context->entry_length,
	    context->entry_arrangement, context->entry_count));

	if (!PAGE_ID_IS_LEAF_LAYER(context->page_id)) {
	    /*
	     * Not a leaf index.  It is an error.
	     */
	    error_code = EB_ERR_UNEXP_TEXT;
	    goto failed;
	}

	if (!PAGE_ID_HAVE_GROUP_ENTRY(context->page_id)
	    && context->entry_arrangement == EB_ARRANGE_FIXED) {
	    /*
	     * The leaf index doesn't have a group entry.
	     * Find text and heading locations.
	     */
	    while (context->entry_index < context->entry_count) {
		if (EB_SIZE_PAGE
		    < context->offset + context->entry_length + 12) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		/*
		 * Compare word and pattern.
		 * If matched, add it to a hit list.
		 */
		context->comparison_result
		    = context->compare_single(context->word, cache_p,
			context->entry_length);
		if (context->comparison_result == 0) {
		    hit->heading.page
			= eb_uint4(cache_p + context->entry_length + 6);
		    hit->heading.offset
			= eb_uint2(cache_p + context->entry_length + 10);
		    hit->text.page
			= eb_uint4(cache_p + context->entry_length);
		    hit->text.offset
			= eb_uint2(cache_p + context->entry_length + 4);
		    hit++;
		    *hit_count += 1;
		}
		context->entry_index++;
		context->offset += context->entry_length + 12;
		cache_p += context->entry_length + 12;

		if (context->comparison_result < 0
		    || max_hit_count <= *hit_count)
		    goto succeeded;
	    }

	} else if (!PAGE_ID_HAVE_GROUP_ENTRY(context->page_id)
	    && context->entry_arrangement == EB_ARRANGE_VARIABLE) {

	    /*
	     * The leaf index doesn't have a group entry.
	     * Find text and heading locations.
	     */
	    while (context->entry_index < context->entry_count) {
		if (EB_SIZE_PAGE < context->offset + 1) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		context->entry_length = eb_uint1(cache_p);
		if (EB_SIZE_PAGE
		    < context->offset + context->entry_length + 13) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		/*
		 * Compare word and pattern.
		 * If matched, add it to a hit list.
		 */
		context->comparison_result
		    = context->compare_single(context->word, cache_p + 1,
			context->entry_length);
		if (context->comparison_result == 0) {
		    hit->heading.page
			= eb_uint4(cache_p + context->entry_length + 7);
		    hit->heading.offset
			= eb_uint2(cache_p + context->entry_length + 11);
		    hit->text.page
			= eb_uint4(cache_p + context->entry_length + 1);
		    hit->text.offset
			= eb_uint2(cache_p + context->entry_length + 5);
		    hit++;
		    *hit_count += 1;
		}
		context->entry_index++;
		context->offset += context->entry_length + 13;
		cache_p += context->entry_length + 13;

		if (context->comparison_result < 0
		    || max_hit_count <= *hit_count)
		    goto succeeded;
	    }

	} else {
	    /*
	     * The leaf index have a group entry.
	     * Find text and heading locations.
	     */
	    while (context->entry_index < context->entry_count) {
		if (EB_SIZE_PAGE < context->offset + 2) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		group_id = eb_uint1(cache_p);

		if (group_id == 0x00) {
		    /*
		     * 0x00 -- Single entry.
		     */
		    context->entry_length = eb_uint1(cache_p + 1);
		    if (EB_SIZE_PAGE
			< context->offset + context->entry_length + 14) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }

		    /*
		     * Compare word and pattern.
		     * If matched, add it to a hit list.
		     */
		    context->comparison_result
			= context->compare_single(context->canonicalized_word,
			    cache_p + 2, context->entry_length);
		    if (context->comparison_result == 0) {
			hit->heading.page
			    = eb_uint4(cache_p + context->entry_length + 8);
			hit->heading.offset
			    = eb_uint2(cache_p + context->entry_length + 12);
			hit->text.page
			    = eb_uint4(cache_p + context->entry_length + 2);
			hit->text.offset
			    = eb_uint2(cache_p + context->entry_length + 6);
			hit++;
			*hit_count += 1;
		    }
		    context->in_group_entry = 0;
		    context->offset += context->entry_length + 14;
		    cache_p += context->entry_length + 14;

		} else if (group_id == 0x80) {
		    /*
		     * 0x80 -- Start of group entry.
		     */
		    context->entry_length = eb_uint1(cache_p + 1);
		    if (EB_SIZE_PAGE
			< context->offset + context->entry_length + 4) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }
		    context->comparison_result
			= context->compare_single(context->canonicalized_word,
			    cache_p + 4, context->entry_length);
		    context->in_group_entry = 1;
		    cache_p += context->entry_length + 4;
		    context->offset += context->entry_length + 4;

		} else if (group_id == 0xc0) {
		    /*
		     * Element of the group entry
		     */
		    context->entry_length = eb_uint1(cache_p + 1);
		    if (EB_SIZE_PAGE < context->offset + 14) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }

		    /*
		     * Compare word and pattern.
		     * If matched, add it to a hit list.
		     */
		    if (context->comparison_result == 0
			&& context->in_group_entry
			&& context->compare_group(context->word, cache_p + 2,
			    context->entry_length) == 0) {
			hit->heading.page
			    = eb_uint4(cache_p + context->entry_length + 8);
			hit->heading.offset
			    = eb_uint2(cache_p + context->entry_length + 12);
			hit->text.page
			    = eb_uint4(cache_p + context->entry_length + 2);
			hit->text.offset
			    = eb_uint2(cache_p + context->entry_length + 6);
			hit++;
			*hit_count += 1;
		    }
		    context->offset += context->entry_length + 14;
		    cache_p += context->entry_length + 14;

		} else {
		    /*
		     * Unknown group ID.
		     */
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		context->entry_index++;
		if (context->comparison_result < 0
		    || max_hit_count <= *hit_count)
		    goto succeeded;
	    }
	}

	/*
	 * Go to a next page if available.
	 */
	if (PAGE_ID_IS_LAYER_END(context->page_id)) {
	    context->comparison_result = -1;
	    goto succeeded;
	}
	context->page++;
	context->entry_index = 0;
    }

  succeeded:
    LOG(("out: eb_hit_list_word(hit_count=%d) = %s",
	*hit_count, eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     * Discard cache if read error occurs.
     */
  failed:
    if (error_code == EB_ERR_FAIL_READ_TEXT)
	cache_book_code = EB_BOOK_NONE;
    *hit_count = 0;
    LOG(("out: eb_hit_list_word() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Get hit entries of a submitted keyword search request.
 */
static EB_Error_Code
eb_hit_list_keyword(EB_Book *book, EB_Search_Context *context,
    int max_hit_count, EB_Hit *hit_list, int *hit_count)
{
    EB_Error_Code error_code;
    EB_Text_Context text_context;
    EB_Hit *hit;
    int group_id;
    char *cache_p;

    LOG(("in: eb_hit_list_keyword(book=%d, max_hit_count=%d)",
	(int)book->code, max_hit_count));

    hit = hit_list;
    *hit_count = 0;

    /*
     * Backup the text context in `book'.
     */
    memcpy(&text_context, &book->text_context, sizeof(EB_Text_Context));

    /*
     * Seek text file.
     */
    if (context->in_group_entry && context->comparison_result == 0) {
	error_code = eb_seek_text(book, &context->keyword_heading);
	if (error_code != EB_SUCCESS)
	    goto failed;
    }

    /*
     * If the result of previous comparison is negative value, all
     * matched entries have been found.
     */
    if (context->comparison_result < 0 || max_hit_count <= 0)
	goto succeeded;

    for (;;) {
	/*
	 * Read a page to search, if the page is not on the cache buffer.
	 *
	 * Cache may be missed by the two reasons:
	 *   1. the search process reaches to the end of an index page,
	 *      and tries to read the next page.
	 *   2. Someone else used the cache buffer.
	 *
	 * At the case of 1, the search process reads the page and update
	 * the search context.  At the case of 2. it reads the page but
	 * must not update the context!
	 */
	if (cache_book_code != book->code || cache_page != context->page) {
	    if (zio_lseek(&book->subbook_current->text_zio,
		((off_t) context->page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
		error_code = EB_ERR_FAIL_SEEK_TEXT;
		goto failed;
	    }
	    if (zio_read(&book->subbook_current->text_zio, cache_buffer,
		EB_SIZE_PAGE) != EB_SIZE_PAGE) {
		error_code = EB_ERR_FAIL_READ_TEXT;
		goto failed;
	    }

	    /*
	     * Update search context.
	     */
	    if (context->entry_index == 0) {
		context->page_id = eb_uint1(cache_buffer);
		context->entry_length = eb_uint1(cache_buffer + 1);
		if (context->entry_length == 0)
		    context->entry_arrangement = EB_ARRANGE_VARIABLE;
		else
		    context->entry_arrangement = EB_ARRANGE_FIXED;
		context->entry_count = eb_uint2(cache_buffer + 2);
		context->entry_index = 0;
		context->offset = 4;
	    }

	    cache_book_code = book->code;
	    cache_page = context->page;
	}

	cache_p = cache_buffer + context->offset;

	LOG(("aux: eb_hit_list_keyword(page=%d, page_id=0x%02x, \
entry_length=%d, entry_arrangement=%d, entry_count=%d)",
	    context->page, context->page_id, context->entry_length,
	    context->entry_arrangement, context->entry_count));

	if (!PAGE_ID_IS_LEAF_LAYER(context->page_id)) {
	    /*
	     * Not a leaf index.  It is an error.
	     */
	    error_code = EB_ERR_UNEXP_TEXT;
	    goto failed;
	}

	if (!PAGE_ID_HAVE_GROUP_ENTRY(context->page_id)
	    && context->entry_arrangement == EB_ARRANGE_FIXED) {
	    /*
	     * The leaf index doesn't have a group entry.
	     * Find text and heading locations.
	     */
	    while (context->entry_index < context->entry_count) {
		if (EB_SIZE_PAGE
		    < context->offset + context->entry_length + 12) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		/*
		 * Compare word and pattern.
		 * If matched, add it to a hit list.
		 */
		context->comparison_result
		    = context->compare_single(context->word, cache_p,
			context->entry_length);
		if (context->comparison_result == 0) {
		    hit->heading.page
			= eb_uint4(cache_p + context->entry_length + 6);
		    hit->heading.offset
			= eb_uint2(cache_p + context->entry_length + 10);
		    hit->text.page
			= eb_uint4(cache_p + context->entry_length);
		    hit->text.offset
			= eb_uint2(cache_p + context->entry_length + 4);
		    hit++;
		    *hit_count += 1;
		}
		context->entry_index++;
		context->offset += context->entry_length + 12;
		cache_p += context->entry_length + 12;

		if (context->comparison_result < 0
		    || max_hit_count <= *hit_count)
		    goto succeeded;
	    }

	} else if (!PAGE_ID_HAVE_GROUP_ENTRY(context->page_id)
	    && context->entry_arrangement == EB_ARRANGE_VARIABLE) {
	    /*
	     * The leaf index doesn't have a group entry.
	     * Find text and heading locations.
	     */
	    while (context->entry_index < context->entry_count) {
		if (EB_SIZE_PAGE < context->offset + 1) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		context->entry_length = eb_uint1(cache_p);
		if (EB_SIZE_PAGE
		    < context->offset + context->entry_length + 13) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		/*
		 * Compare word and pattern.
		 * If matched, add it to a hit list.
		 */
		context->comparison_result
		    = context->compare_single(context->word, cache_p + 1,
			context->entry_length);
		if (context->comparison_result == 0) {
		    hit->heading.page
			= eb_uint4(cache_p + context->entry_length + 7);
		    hit->heading.offset
			= eb_uint2(cache_p + context->entry_length + 11);
		    hit->text.page
			= eb_uint4(cache_p + context->entry_length + 1);
		    hit->text.offset
			= eb_uint2(cache_p + context->entry_length + 5);
		    hit++;
		    *hit_count += 1;
		}
		context->entry_index++;
		context->offset += context->entry_length + 13;
		cache_p += context->entry_length + 13;

		if (context->comparison_result < 0
		    || max_hit_count <= *hit_count)
		    goto succeeded;
	    }

	} else {
	    /*
	     * The leaf index have a group entry.
	     * Find text and heading locations.
	     */
	    while (context->entry_index < context->entry_count) {
		if (EB_SIZE_PAGE < context->offset + 2) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		group_id = eb_uint1(cache_p);

		if (group_id == 0x00) {
		    /*
		     * 0x00 -- Single entry.
		     */
		    context->entry_length = eb_uint1(cache_p + 1);
		    if (EB_SIZE_PAGE
			< context->offset + context->entry_length + 14) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }

		    /*
		     * Compare word and pattern.
		     * If matched, add it to a hit list.
		     */
		    context->comparison_result
			= context->compare_single(context->canonicalized_word,
			    cache_p + 2, context->entry_length);
		    if (context->comparison_result == 0) {
			hit->heading.page
			    = eb_uint4(cache_p + context->entry_length + 8);
			hit->heading.offset
			    = eb_uint2(cache_p + context->entry_length + 12);
			hit->text.page
			    = eb_uint4(cache_p + context->entry_length + 2);
			hit->text.offset
			    = eb_uint2(cache_p + context->entry_length + 6);
			hit++;
			*hit_count += 1;
		    }
		    context->in_group_entry = 0;
		    context->offset += context->entry_length + 14;
		    cache_p += context->entry_length + 14;

		} else if (group_id == 0x80) {
		    /*
		     * 0x80 -- Start of group entry.
		     */
		    context->entry_length = eb_uint1(cache_p + 1);
		    if (EB_SIZE_PAGE
			< context->offset + context->entry_length + 12) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }
		    context->comparison_result
			= context->compare_single(context->word, cache_p + 6,
			    context->entry_length);
		    context->keyword_heading.page
			= eb_uint4(cache_p + context->entry_length + 6);
		    context->keyword_heading.offset
			= eb_uint2(cache_p + context->entry_length + 10);
		    context->in_group_entry = 1;
		    cache_p += context->entry_length + 12;
		    context->offset += context->entry_length + 12;

		    if (context->comparison_result == 0) {
			error_code
			    = eb_seek_text(book, &context->keyword_heading);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    }

		} else if (group_id == 0xc0) {
		    /*
		     * Element of the group entry.
		     */
		    if (EB_SIZE_PAGE < context->offset + 7) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }

		    /*
		     * Compare word and pattern.
		     * If matched, add it to a hit list.
		     */
		    if (context->in_group_entry
			&& context->comparison_result == 0) {
			error_code
			    = eb_tell_text(book, &context->keyword_heading);
			if (error_code != EB_SUCCESS)
			    goto failed;
			hit->heading.page   = context->keyword_heading.page;
			hit->heading.offset = context->keyword_heading.offset;
			hit->text.page      = eb_uint4(cache_p + 1);
			hit->text.offset    = eb_uint2(cache_p + 5);
			hit++;
			*hit_count += 1;
			error_code = eb_forward_heading(book);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    }
		    context->offset += 7;
		    cache_p += 7;

		} else {
		    /*
		     * Unknown group ID.
		     */
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		context->entry_index++;
		if (context->comparison_result < 0
		    || max_hit_count <= *hit_count)
		    goto succeeded;
	    }
	}

	/*
	 * Go to a next page if available.
	 */
	if (PAGE_ID_IS_LAYER_END(context->page_id)) {
	    context->comparison_result = -1;
	    goto succeeded;
	}
	context->page++;
	context->entry_index = 0;
    }

  succeeded:
    if (context->in_group_entry && context->comparison_result == 0) {
	error_code = eb_tell_text(book, &context->keyword_heading);
	if (error_code != EB_SUCCESS)
	    goto failed;
    }

    /*
     * Restore the text context in `book'.
     */
    memcpy(&book->text_context, &text_context, sizeof(EB_Text_Context));
    LOG(("out: eb_hit_list_keyword(hit_count=%d) = %s",
	*hit_count, eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     * Discard cache if read error occurs.
     */
  failed:
    if (error_code == EB_ERR_FAIL_READ_TEXT)
	cache_book_code = EB_BOOK_NONE;
    *hit_count = 0;
    memcpy(&book->text_context, &text_context, sizeof(EB_Text_Context));
    LOG(("out: eb_hit_list_keyword() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Get hit entries of a submitted multi search request.
 */
static EB_Error_Code
eb_hit_list_multi(EB_Book *book, EB_Search_Context *context, int max_hit_count,
    EB_Hit *hit_list, int *hit_count)
{
    EB_Error_Code error_code;
    EB_Hit *hit;
    int group_id;
    char *cache_p;

    LOG(("in: eb_hit_list_multi(book=%d, max_hit_count=%d)", (int)book->code,
	max_hit_count));

    hit = hit_list;
    *hit_count = 0;

    /*
     * If the result of previous comparison is negative value, all
     * matched entries have been found.
     */
    if (context->comparison_result < 0 || max_hit_count <= 0)
	goto succeeded;

    for (;;) {
	/*
	 * Read a page to search, if the page is not on the cache buffer.
	 *
	 * Cache may be missed by the two reasons:
	 *   1. the search process reaches to the end of an index page,
	 *      and tries to read the next page.
	 *   2. Someone else used the cache buffer.
	 *
	 * At the case of 1, the search process reads the page and update
	 * the search context.  At the case of 2. it reads the page but
	 * must not update the context!
	 */
	if (cache_book_code != book->code || cache_page != context->page) {
	    if (zio_lseek(&book->subbook_current->text_zio,
		((off_t) context->page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
		error_code = EB_ERR_FAIL_SEEK_TEXT;
		goto failed;
	    }
	    if (zio_read(&book->subbook_current->text_zio, cache_buffer,
		EB_SIZE_PAGE) != EB_SIZE_PAGE) {
		error_code = EB_ERR_FAIL_READ_TEXT;
		goto failed;
	    }

	    /*
	     * Update search context.
	     */
	    if (context->entry_index == 0) {
		context->page_id = eb_uint1(cache_buffer);
		context->entry_length = eb_uint1(cache_buffer + 1);
		if (context->entry_length == 0)
		    context->entry_arrangement = EB_ARRANGE_VARIABLE;
		else
		    context->entry_arrangement = EB_ARRANGE_FIXED;
		context->entry_count = eb_uint2(cache_buffer + 2);
		context->entry_index = 0;
		context->offset = 4;
	    }

	    cache_book_code = book->code;
	    cache_page = context->page;
	}

	cache_p = cache_buffer + context->offset;

	LOG(("aux: eb_hit_list_multi(page=%d, page_id=0x%02x, \
entry_length=%d, entry_arrangement=%d, entry_count=%d)",
	    context->page, context->page_id, context->entry_length,
	    context->entry_arrangement, context->entry_count));

	if (!PAGE_ID_IS_LEAF_LAYER(context->page_id)) {
	    /*
	     * Not a leaf index.  It is an error.
	     */
	    error_code = EB_ERR_UNEXP_TEXT;
	    goto failed;
	}

	if (!PAGE_ID_HAVE_GROUP_ENTRY(context->page_id)
	    && context->entry_arrangement == EB_ARRANGE_FIXED) {
	    /*
	     * The leaf index doesn't have a group entry.
	     * Find text and heading locations.
	     */
	    while (context->entry_index < context->entry_count) {
		if (EB_SIZE_PAGE
		    < context->offset + context->entry_length + 13) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		/*
		 * Compare word and pattern.
		 * If matched, add it to a hit list.
		 */
		context->comparison_result
		    = context->compare_single(context->word, cache_p,
			context->entry_length);
		if (context->comparison_result == 0) {
		    hit->heading.page
			= eb_uint4(cache_p + context->entry_length + 6);
		    hit->heading.offset
			= eb_uint2(cache_p + context->entry_length + 10);
		    hit->text.page
			= eb_uint4(cache_p + context->entry_length);
		    hit->text.offset
			= eb_uint2(cache_p + context->entry_length + 4);
		    hit++;
		    *hit_count += 1;
		}
		context->entry_index++;
		context->offset += context->entry_length + 12;
		cache_p += context->entry_length + 12;

		if (context->comparison_result < 0
		    || max_hit_count <= *hit_count)
		    goto succeeded;
	    }

	} else if (!PAGE_ID_HAVE_GROUP_ENTRY(context->page_id)
	    && context->entry_arrangement == EB_ARRANGE_VARIABLE) {
	    /*
	     * The leaf index doesn't have a group entry.
	     * Find text and heading locations.
	     */
	    while (context->entry_index < context->entry_count) {
		if (EB_SIZE_PAGE < context->offset + 1) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		context->entry_length = eb_uint1(cache_p);
		if (EB_SIZE_PAGE
		    < context->offset + context->entry_length + 13) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		/*
		 * Compare word and pattern.
		 * If matched, add it to a hit list.
		 */
		context->comparison_result
		    = context->compare_single(context->word, cache_p + 1,
			context->entry_length);
		if (context->comparison_result == 0) {
		    hit->heading.page
			= eb_uint4(cache_p + context->entry_length + 7);
		    hit->heading.offset
			= eb_uint2(cache_p + context->entry_length + 11);
		    hit->text.page
			= eb_uint4(cache_p + context->entry_length + 1);
		    hit->text.offset
			= eb_uint2(cache_p + context->entry_length + 5);
		    hit++;
		    *hit_count += 1;
		}
		context->entry_index++;
		context->offset += context->entry_length + 13;
		cache_p += context->entry_length + 13;

		if (context->comparison_result < 0
		    || max_hit_count <= *hit_count)
		    goto succeeded;
	    }

	} else {
	    /*
	     * The leaf index have a group entry.
	     * Find text and heading locations.
	     */
	    while (context->entry_index < context->entry_count) {
		if (EB_SIZE_PAGE < context->offset + 2) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		group_id = eb_uint1(cache_p);

		if (group_id == 0x00) {
		    /*
		     * 0x00 -- Single entry.
		     */
		    context->entry_length = eb_uint1(cache_p + 1);
		    if (EB_SIZE_PAGE
			< context->offset + context->entry_length + 14) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }

		    /*
		     * Compare word and pattern.
		     * If matched, add it to a hit list.
		     */
		    context->comparison_result
			= context->compare_single(context->canonicalized_word,
			    cache_p + 2, context->entry_length);
		    if (context->comparison_result == 0) {
			hit->heading.page
			    = eb_uint4(cache_p + context->entry_length + 8);
			hit->heading.offset
			    = eb_uint2(cache_p + context->entry_length + 12);
			hit->text.page
			    = eb_uint4(cache_p + context->entry_length + 2);
			hit->text.offset
			    = eb_uint2(cache_p + context->entry_length + 6);
			hit++;
			*hit_count += 1;
		    }
		    context->in_group_entry = 0;
		    context->offset += context->entry_length + 14;
		    cache_p += context->entry_length + 14;

		} else if (group_id == 0x80) {
		    /*
		     * 0x80 -- Start of group entry.
		     */
		    context->entry_length = eb_uint1(cache_p + 1);
		    if (EB_SIZE_PAGE
			< context->offset + context->entry_length + 6) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }
		    context->comparison_result
			= context->compare_single(context->word, cache_p + 6,
			    context->entry_length);
		    context->in_group_entry = 1;
		    cache_p += context->entry_length + 6;
		    context->offset += context->entry_length + 6;

		} else if (group_id == 0xc0) {
		    /*
		     * Element of the group entry.
		     */
		    if (EB_SIZE_PAGE < context->offset + 13) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }

		    /*
		     * Compare word and pattern.
		     * If matched, add it to a hit list.
		     */
		    if (context->in_group_entry
			&& context->comparison_result == 0) {
			hit->heading.page   = eb_uint4(cache_p + 7);
			hit->heading.offset = eb_uint2(cache_p + 11);
			hit->text.page      = eb_uint4(cache_p + 1);
			hit->text.offset    = eb_uint2(cache_p + 5);
			hit++;
			*hit_count += 1;
		    }
		    context->offset += 13;
		    cache_p += 13;

		} else {
		    /*
		     * Unknown group ID.
		     */
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		context->entry_index++;
		if (context->comparison_result < 0
		    || max_hit_count <= *hit_count)
		    goto succeeded;
	    }
	}

	/*
	 * Go to a next page if available.
	 */
	if (PAGE_ID_IS_LAYER_END(context->page_id)) {
	    context->comparison_result = -1;
	    goto succeeded;
	}
	context->page++;
	context->entry_index = 0;
    }

  succeeded:
    LOG(("out: eb_hit_list_multi(hit_count=%d) = %s",
	*hit_count, eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     * Discard cache if read error occurs.
     */
  failed:
    if (error_code == EB_ERR_FAIL_READ_TEXT)
	cache_book_code = EB_BOOK_NONE;
    *hit_count = 0;
    LOG(("out: eb_hit_list_multi() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Do AND operation of hit lists.
 * and_list = hit_lists[0] AND hit_lists[1] AND ...
 */
static void
eb_and_hit_lists(EB_Hit and_list[EB_TMP_MAX_HITS], int *and_count,
    int max_and_count, int hit_list_count,
    EB_Hit hit_lists[EB_NUMBER_OF_SEARCH_CONTEXTS][EB_TMP_MAX_HITS],
    int hit_counts[EB_NUMBER_OF_SEARCH_CONTEXTS])
{
    int hit_indexes[EB_NUMBER_OF_SEARCH_CONTEXTS];
    int greatest_list;
    int greatest_page;
    int greatest_offset;
    int current_page;
    int current_offset;
    int equal_count;
    int increment_count;
    int i;

    LOG(("in: eb_and_hit_lists(max_and_count=%d, hit_list_count=%d)",
	max_and_count, hit_list_count));

    /*
     * Initialize indexes for the hit_lists[].
     */
    for (i = 0; i < hit_list_count; i++)
	hit_indexes[i] = 0;

    /*
     * Generate the new list `and_list'.
     */
    *and_count = 0;
    while (*and_count < max_and_count) {
	/*
	 * Initialize variables.
	 */
	greatest_list = -1;
	greatest_page = 0;
	greatest_offset = 0;
	current_page = 0;
	current_offset = 0;
	equal_count = 0;

	/*
	 * Compare the current elements of the lists.
	 */
	for (i = 0; i < hit_list_count; i++) {
	    /*
	     * If we have been reached to the tail of the hit_lists[i],
	     * skip the list.
	     */
	    if (hit_counts[i] <= hit_indexes[i])
		continue;

	    /*
	     * Compare {current_page, current_offset} and {greatest_page,
	     * greatest_offset}.
	     */
	    current_page = hit_lists[i][hit_indexes[i]].text.page;
	    current_offset = hit_lists[i][hit_indexes[i]].text.offset;

	    if (greatest_list == -1) {
		greatest_page = current_page;
		greatest_offset = current_offset;
		greatest_list = i;
		equal_count++;
	    } else if (greatest_page < current_page) {
		greatest_page = current_page;
		greatest_offset = current_offset;
		greatest_list = i;
	    } else if (current_page == greatest_page
		&& greatest_offset < current_offset) {
		greatest_page = current_page;
		greatest_offset = current_offset;
		greatest_list = i;
	    } else if (current_page == greatest_page
		&& current_offset == greatest_offset) {
		equal_count++;
	    }
	}

	if (equal_count == hit_list_count) {
	    /*
	     * All the current elements of the lists point to the same
	     * position.  This is hit element.  Increase indexes of all
	     * lists.
	     */
	    memcpy(and_list + *and_count, hit_lists[0] + hit_indexes[0],
		sizeof(EB_Hit));
	    *and_count += 1;
	    for (i = 0; i < hit_list_count; i++) {
		if (hit_counts[i] <= hit_indexes[i])
		    continue;
		hit_indexes[i]++;
	    }
	} else {
	    /*
	     * This is not hit element.  Increase indexes of all lists
	     * except for greatest element(s).  If there is no list
	     * whose index is incremented, our job has been completed.
	     */
	    increment_count = 0;
	    for (i = 0; i < hit_list_count; i++) {
		if (hit_counts[i] <= hit_indexes[i])
		    continue;
		current_page = hit_lists[i][hit_indexes[i]].text.page;
		current_offset = hit_lists[i][hit_indexes[i]].text.offset;
		if (current_page != greatest_page
		    || current_offset != greatest_offset) {
		    hit_indexes[i]++;
		    increment_count++;
		}
	    }
	    if (increment_count == 0)
		break;
	}
    }

    /*
     * Update hit_counts[].
     * The hit counts of the lists are set to the current indexes.
     */
    for (i = 0; i < hit_list_count; i++)
	hit_counts[i] = hit_indexes[i];

    LOG(("out: eb_and_hit_lists(and_count=%d)", *and_count));
}
