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
 * Examine whether the current subbook in `book' supports `EXACTWORD SEARCH'
 * or not.
 */
int
eb_have_exactword_search(EB_Book *book)
{
    eb_lock(&book->lock);
    LOG(("in: eb_have_exactword_search(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL)
	goto failed;

    /*
     * Check for the index page of word search.
     */
    if (book->subbook_current->word_alphabet.start_page == 0
	&& book->subbook_current->word_asis.start_page == 0
	&& book->subbook_current->word_kana.start_page == 0)
	goto failed;

    LOG(("out: eb_have_exactword_search() = %d", 1));
    eb_unlock(&book->lock);

    return 1;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_have_exactword_search() = %d", 0));
    eb_unlock(&book->lock);
    return 0;
}


/*
 * Exactword search.
 */
EB_Error_Code
eb_search_exactword(EB_Book *book, const char *input_word)
{
    EB_Error_Code error_code;
    EB_Word_Code word_code;
    EB_Search_Context *context;

    eb_lock(&book->lock);
    LOG(("in: eb_search_exactword(book=%d, input_word=%s)", (int)book->code,
	eb_quoted_string(input_word)));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Initialize search context.
     */
    eb_reset_search_contexts(book);
    context = book->search_contexts;
    context->code = EB_SEARCH_EXACTWORD;

    /*
     * Make a fixed word and a canonicalized word to search from
     * `input_word'.
     */
    error_code = eb_set_word(book, input_word, context->word,
	context->canonicalized_word, &word_code);
    if (error_code != EB_SUCCESS)
	goto failed;

    /*
     * Get a page number.
     */
    switch (word_code) {
    case EB_WORD_ALPHABET:
	if (book->subbook_current->word_alphabet.start_page != 0)
	    context->page = book->subbook_current->word_alphabet.start_page;
	else if (book->subbook_current->word_asis.start_page != 0)
	    context->page = book->subbook_current->word_asis.start_page;
	else {
	    error_code = EB_ERR_NO_SUCH_SEARCH;
	    goto failed;
	}
	break;

    case EB_WORD_KANA:
	if (book->subbook_current->word_kana.start_page != 0)
	    context->page = book->subbook_current->word_kana.start_page;
	else if (book->subbook_current->word_asis.start_page != 0)
	    context->page = book->subbook_current->word_asis.start_page;
	else {
	    error_code = EB_ERR_NO_SUCH_SEARCH;
	    goto failed;
	}
	break;

    case EB_WORD_OTHER:
	if (book->subbook_current->word_asis.start_page != 0)
	    context->page = book->subbook_current->word_asis.start_page;
	else {
	    error_code = EB_ERR_NO_SUCH_SEARCH;
	    goto failed;
	}
	break;

    default:
	error_code = EB_ERR_NO_SUCH_SEARCH;
	goto failed;
    }

    /*
     * Choose comparison functions.
     */
    if (book->character_code == EB_CHARCODE_ISO8859_1
	|| book->character_code == EB_CHARCODE_UTF8) {
	context->compare_pre    = eb_exact_pre_match_word_latin;
	context->compare_single = eb_exact_match_word_latin;
	context->compare_group  = eb_exact_match_word_latin;
    } else if (context->page == book->subbook_current->word_kana.start_page) {
	context->compare_pre    = eb_exact_pre_match_word_jis;
	context->compare_single = eb_exact_match_word_kana_single;
	context->compare_group  = eb_exact_match_word_kana_group;
    } else {
	context->compare_pre    = eb_exact_pre_match_word_jis;
	context->compare_single = eb_exact_match_word_jis;
	context->compare_group  = eb_exact_match_word_kana_group;
    }

    /*
     * Pre-search.
     */
    error_code = eb_presearch_word(book, context);
    if (error_code != EB_SUCCESS)
	goto failed;

    LOG(("out: eb_search_exactword() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_reset_search_contexts(book);
    LOG(("out: eb_search_exactword() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}
