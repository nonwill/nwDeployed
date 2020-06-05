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
 * Examine whether the current subbook in `book' supports `KEYWORD SEARCH'
 * or not.
 */
int
eb_have_keyword_search(EB_Book *book)
{
    eb_lock(&book->lock);
    LOG(("in: eb_have_keyword_search(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL)
	goto failed;

    if (book->subbook_current->keyword.start_page == 0)
	goto failed;

    LOG(("out: eb_have_keyword_search() = %d", 1));
    eb_unlock(&book->lock);

    return 1;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_have_keyword_search() = %d", 0));
    eb_unlock(&book->lock);
    return 0;
}


/*
 * Keyword search.
 */
EB_Error_Code
eb_search_keyword(EB_Book *book, const char * const input_words[])
{
    EB_Error_Code error_code;
    EB_Search_Context *context;
    EB_Word_Code word_code;
    int word_count;
    int i;

    /*
     * Lock the book.
     */
    eb_lock(&book->lock);
    LOG(("in: eb_search_keyword(book=%d, input_words=[below])",
	(int)book->code));

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
    if (book->subbook_current->keyword.start_page == 0) {
	error_code = EB_ERR_NO_SUCH_SEARCH;
	goto failed;
    }

    /*
     * Attach a search context for each keyword, and pre-search the
     * keywords.
     */
    eb_reset_search_contexts(book);
    word_count = 0;

    for (i = 0; i < EB_MAX_KEYWORDS; i++) {
	if (input_words[i] == NULL)
	    break;

	/*
	 * Initialize search context.
	 */
	context = book->search_contexts + word_count;
	context->code = EB_SEARCH_KEYWORD;

	/*
	 * Choose comparison functions.
	 */
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
	context->page = book->subbook_current->keyword.start_page;

	/*
	 * Make a fixed word and a canonicalized word to search from
	 * `input_words[i]'.
	 */
	error_code = eb_set_keyword(book, input_words[i], context->word,
	    context->canonicalized_word, &word_code);
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
    } else if (EB_MAX_KEYWORDS <= i && input_words[i] != NULL) {
	error_code =  EB_ERR_TOO_MANY_WORDS;
	goto failed;
    }

    /*
     * Set `EB_SEARCH_NONE' to the rest unused search context.
     */
    for (i = word_count; i < EB_MAX_KEYWORDS; i++)
	(book->search_contexts + i)->code = EB_SEARCH_NONE;

    LOG(("out: eb_search_keyword() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_reset_search_contexts(book);
    LOG(("out: eb_search_keyword() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


