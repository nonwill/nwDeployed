/* automatically generated from narwalt.c. */
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
#include "appendix.h"
#include "build-post.h"

/*
 * Unexported functions.
 */
static EB_Error_Code eb_wide_character_text_jis(EB_Appendix *appendix,
    int character_number, char *text);
static EB_Error_Code eb_wide_character_text_latin(EB_Appendix *appendix,
    int character_number, char *text);

/*
 * Hash macro for cache data.
 */
#define EB_HASH_ALT_CACHE(c)	((c) & 0x0f)


/*
 * Examine whether the current subbook in `book' has a wide font
 * alternation or not.
 */
int
eb_have_wide_alt(EB_Appendix *appendix)
{
    eb_lock(&appendix->lock);
    LOG(("in: eb_have_wide_alt(appendix=%d)", (int)appendix->code));

    /*
     * Current subbook must have been set.
     */
    if (appendix->subbook_current == NULL)
	goto failed;

    if (appendix->subbook_current->wide_page == 0)
	goto failed;

    LOG(("out: eb_have_wide_alt() = %d", 1));
    eb_unlock(&appendix->lock);

    return 1;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_have_wide_alt() = %d", 0));
    eb_unlock(&appendix->lock);
    return 0;
}


/*
 * Look up the character number of the start of the wide font alternation
 * of the current subbook in `book'.
 */
EB_Error_Code
eb_wide_alt_start(EB_Appendix *appendix, int *start)
{
    EB_Error_Code error_code;

    eb_lock(&appendix->lock);
    LOG(("in: eb_wide_alt_start(appendix=%d)", (int)appendix->code));

    /*
     * Current subbook must have been set.
     */
    if (appendix->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_APPSUB;
	goto failed;
    }

    if (appendix->subbook_current->wide_page == 0) {
	error_code = EB_ERR_NO_ALT;
	goto failed;
    }

    *start = appendix->subbook_current->wide_start;

    LOG(("out: eb_wide_alt_start(start=%d) = %s", *start,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&appendix->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *start = -1;
    LOG(("out: eb_wide_alt_start() = %s", eb_error_string(error_code)));
    eb_unlock(&appendix->lock);
    return error_code;
}


/*
 * Return the character number of the end of the wide font alternation
 * of the current subbook in `book'.
 */
EB_Error_Code
eb_wide_alt_end(EB_Appendix *appendix, int *end)
{
    EB_Error_Code error_code;

    eb_lock(&appendix->lock);
    LOG(("in: eb_wide_alt_end(appendix=%d)", (int)appendix->code));

    /*
     * Current subbook must have been set.
     */
    if (appendix->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_APPSUB;
	goto failed;
    }

    if (appendix->subbook_current->wide_page == 0) {
	error_code = EB_ERR_NO_ALT;
	goto failed;
    }

    *end = appendix->subbook_current->wide_end;

    LOG(("out: eb_wide_alt_end(end=%d) = %s", *end,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&appendix->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *end = -1;
    LOG(("out: eb_wide_alt_end() = %s", eb_error_string(error_code)));
    eb_unlock(&appendix->lock);
    return error_code;
}


/*
 * Get the alternation text of the character number `character_number'.
 */
EB_Error_Code
eb_wide_alt_character_text(EB_Appendix *appendix, int character_number,
    char *text)
{
    EB_Error_Code error_code;

    eb_lock(&appendix->lock);
    LOG(("in: eb_wide_alt_character_text(appendix=%d, character_number=%d)",
	(int)appendix->code, character_number));

    /*
     * Current subbook must have been set.
     */
    if (appendix->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_APPSUB;
	goto failed;
    }

    /*
     * The wide font must exist in the current subbook.
     */
    if (appendix->subbook_current->wide_page == 0) {
	error_code = EB_ERR_NO_ALT;
	goto failed;
    }

    if (appendix->subbook_current->character_code == EB_CHARCODE_ISO8859_1) {
	error_code = eb_wide_character_text_latin(appendix,
	    character_number, text);
    } else {
	error_code = eb_wide_character_text_jis(appendix, character_number,
	    text);
    }
    if (error_code != EB_SUCCESS)
	goto failed;

    LOG(("out: eb_wide_alt_character_text(text=%s) = %s",
	eb_quoted_string(text), eb_error_string(EB_SUCCESS)));
    eb_unlock(&appendix->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *text = '\0';
    LOG(("out: eb_wide_alt_character_text() = %s",
	eb_error_string(error_code)));
    eb_unlock(&appendix->lock);
    return error_code;
}


/*
 * Get the alternation text of the character number `character_number'.
 */
static EB_Error_Code
eb_wide_character_text_jis(EB_Appendix *appendix, int character_number,
    char *text)
{
    EB_Error_Code error_code;
    int start;
    int end;
    off_t location;
    EB_Alternation_Cache *cachep;

    LOG(("in: eb_wide_alt_character_text_jis(appendix=%d, \
character_number=%d)",
	(int)appendix->code, character_number));

    start = appendix->subbook_current->wide_start;
    end = appendix->subbook_current->wide_end;

    /*
     * Check for `character_number'.  Is it in a font?
     * This test works correctly even when the font doesn't exist in
     * the current subbook because `start' and `end' have set to -1
     * in the case.
     */
    if (character_number < start
	|| end < character_number
	|| (character_number & 0xff) < 0x21
	|| 0x7e < (character_number & 0xff)) {
	error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
	goto failed;
    }

    /*
     * Calculate the location of alternation data.
     */
    location
	= (appendix->subbook_current->wide_page - 1) * EB_SIZE_PAGE
	+ (((character_number >> 8) - (start >> 8)) * 0x5e
	    + (character_number & 0xff) - (start & 0xff))
	* (EB_MAX_ALTERNATION_TEXT_LENGTH + 1);

    /*
     * Check for the cache data.
     */
    cachep = appendix->wide_cache + EB_HASH_ALT_CACHE(character_number);
    if (cachep->character_number == character_number) {
	memcpy(text, cachep->text, EB_MAX_ALTERNATION_TEXT_LENGTH + 1);
	goto succeeded;
    }

    /*
     * Read the alternation data.
     */
    if (zio_lseek(&appendix->subbook_current->zio, location, SEEK_SET) < 0) {
	error_code = EB_ERR_FAIL_SEEK_APP;
	goto failed;
    }
    cachep->character_number = -1;
    if (zio_read(&appendix->subbook_current->zio, cachep->text,
	EB_MAX_ALTERNATION_TEXT_LENGTH + 1)
	!= EB_MAX_ALTERNATION_TEXT_LENGTH + 1) {
	error_code = EB_ERR_FAIL_READ_APP;
	goto failed;
    }

    /*
     * Update cache data.
     */
    memcpy(text, cachep->text, EB_MAX_ALTERNATION_TEXT_LENGTH + 1);
    cachep->text[EB_MAX_ALTERNATION_TEXT_LENGTH] = '\0';
    cachep->character_number = character_number;

  succeeded:
    LOG(("out: eb_wide_alt_character_text_jis(text=%s) = %s",
	eb_quoted_string(text), eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *text = '\0';
    LOG(("out: eb_wide_alt_character_text_jis() = %s",
	eb_error_string(error_code)));
    return error_code;
}


/*
 * Get the alternation text of the character number `character_number'.
 */
static EB_Error_Code
eb_wide_character_text_latin(EB_Appendix *appendix, int character_number,
    char *text)
{
    EB_Error_Code error_code;
    int start;
    int end;
    off_t location;
    EB_Alternation_Cache *cache_p;

    LOG(("in: eb_wide_alt_character_text_latin(appendix=%d, \
character_number=%d)",
	(int)appendix->code, character_number));

    start = appendix->subbook_current->wide_start;
    end = appendix->subbook_current->wide_end;

    /*
     * Check for `character_number'.  Is it in a font?
     * This test works correctly even when the font doesn't exist in
     * the current subbook because `start' and `end' have set to -1
     * in the case.
     */
    if (character_number < start
	|| end < character_number
	|| (character_number & 0xff) < 0x01
	|| 0xfe < (character_number & 0xff)) {
	error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
	goto failed;
    }

    /*
     * Calculate the location of alternation data.
     */
    location
	= (appendix->subbook_current->wide_page - 1) * EB_SIZE_PAGE
	+ (((character_number >> 8) - (start >> 8)) * 0xfe
	    + (character_number & 0xff) - (start & 0xff))
	* (EB_MAX_ALTERNATION_TEXT_LENGTH + 1);

    /*
     * Check for the cache data.
     */
    cache_p = appendix->wide_cache + EB_HASH_ALT_CACHE(character_number);
    if (cache_p->character_number == character_number) {
	memcpy(text, cache_p->text, EB_MAX_ALTERNATION_TEXT_LENGTH + 1);
	goto succeeded;
    }

    /*
     * Read the alternation data.
     */
    if (zio_lseek(&appendix->subbook_current->zio, location, SEEK_SET) < 0) {
	error_code = EB_ERR_FAIL_SEEK_APP;
	goto failed;
    }
    cache_p->character_number = -1;
    if (zio_read(&appendix->subbook_current->zio, cache_p->text,
	EB_MAX_ALTERNATION_TEXT_LENGTH + 1)
	!= EB_MAX_ALTERNATION_TEXT_LENGTH + 1) {
	error_code = EB_ERR_FAIL_READ_APP;
	goto failed;
    }

    /*
     * Update cache data.
     */
    memcpy(text, cache_p->text, EB_MAX_ALTERNATION_TEXT_LENGTH + 1);
    cache_p->text[EB_MAX_ALTERNATION_TEXT_LENGTH] = '\0';
    cache_p->character_number = character_number;

  succeeded:
    LOG(("out: eb_wide_alt_character_text_latin(text=%s) = %s",
	eb_quoted_string(text), eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *text = '\0';
    LOG(("out: eb_wide_alt_character_text_latin() = %s",
	eb_error_string(error_code)));
    return error_code;
}


/*
 * Return next `n'th character number from `*character_number'.
 */
EB_Error_Code
eb_forward_wide_alt_character(EB_Appendix *appendix, int n,
    int *character_number)
{
    EB_Error_Code error_code;
    int start;
    int end;
    int i;

    if (n < 0) {
	return eb_backward_wide_alt_character(appendix, -n,
	    character_number);
    }

    eb_lock(&appendix->lock);
    LOG(("in: eb_forward_wide_alt_character(appendix=%d, n=%d, \
character_number=%d)",
	(int)appendix->code, n, *character_number));

    /*
     * Current subbook must have been set.
     */
    if (appendix->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_APPSUB;
	goto failed;
    }

    /*
     * The wide font must exist in the current subbook.
     */
    if (appendix->subbook_current->wide_page == 0) {
	error_code = EB_ERR_NO_ALT;
	goto failed;
    }

    start = appendix->subbook_current->wide_start;
    end = appendix->subbook_current->wide_end;

    if (appendix->subbook_current->character_code == EB_CHARCODE_ISO8859_1) {
	/*
	 * Check for `*character_number'. (ISO 8859 1)
	 */
	if (*character_number < start
	    || end < *character_number
	    || (*character_number & 0xff) < 0x01
	    || 0xfe < (*character_number & 0xff)) {
	    error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
	    goto failed;
	}

	/*
	 * Get character number. (ISO 8859 1)
	 */
	for (i = 0; i < n; i++) {
	    if (0xfe <= (*character_number & 0xff))
		*character_number += 3;
	    else
		*character_number += 1;
	    if (end < *character_number) {
		error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
		goto failed;
	    }
	}
    } else {
	/*
	 * Check for `*character_number'. (JIS X 0208)
	 */
	if (*character_number < start
	    || end < *character_number
	    || (*character_number & 0xff) < 0x21
	    || 0x7e < (*character_number & 0xff)) {
	    error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
	    goto failed;
	}

	/*
	 * Get character number. (JIS X 0208)
	 */
	for (i = 0; i < n; i++) {
	    if (0x7e <= (*character_number & 0xff))
		*character_number += 0xa3;
	    else
		*character_number += 1;
	    if (end < *character_number) {
		error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
		goto failed;
	    }
	}
    }

    LOG(("out: eb_forkward_wide_alt_character(character_number=%d) = %s",
	*character_number, eb_error_string(EB_SUCCESS)));
    eb_unlock(&appendix->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *character_number = -1;
    LOG(("out: eb_forward_wide_alt_character() = %s",
	eb_error_string(error_code)));
    eb_unlock(&appendix->lock);
    return error_code;
}


/*
 * Return previous `n'th character number from `*character_number'.
 */
EB_Error_Code
eb_backward_wide_alt_character(EB_Appendix *appendix, int n,
    int *character_number)
{
    EB_Error_Code error_code;
    int start;
    int end;
    int i;

    if (n < 0) {
	return eb_forward_wide_alt_character(appendix, -n, character_number);
    }

    eb_lock(&appendix->lock);
    LOG(("in: eb_backward_wide_alt_character(appendix=%d, n=%d, \
character_number=%d)",
	(int)appendix->code, n, *character_number));

    /*
     * Current subbook must have been set.
     */
    if (appendix->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_APPSUB;
	goto failed;
    }

    /*
     * The wide font must exist in the current subbook.
     */
    if (appendix->subbook_current->wide_page == 0) {
	error_code = EB_ERR_NO_ALT;
	goto failed;
    }

    start = appendix->subbook_current->wide_start;
    end = appendix->subbook_current->wide_end;

    if (appendix->subbook_current->character_code == EB_CHARCODE_ISO8859_1) {
	/*
	 * Check for `*character_number'. (ISO 8859 1)
	 */
	if (*character_number < start
	    || end < *character_number
	    || (*character_number & 0xff) < 0x01
	    || 0xfe < (*character_number & 0xff)) {
	    error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
	    goto failed;
	}

	/*
	 * Get character number. (ISO 8859 1)
	 */
	for (i = 0; i < n; i++) {
	    if ((*character_number & 0xff) <= 0x01)
		*character_number -= 3;
	    else
		*character_number -= 1;
	    if (*character_number < start) {
		error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
		goto failed;
	    }
	}
    } else {
	/*
	 * Check for `*character_number'. (JIS X 0208)
	 */
	if (*character_number < start
	    || end < *character_number
	    || (*character_number & 0xff) < 0x21
	    || 0x7e < (*character_number & 0xff)) {
	    error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
	    goto failed;
	}

	/*
	 * Get character number. (JIS X 0208)
	 */
	for (i = 0; i < n; i++) {
	    if ((*character_number & 0xff) <= 0x21)
		*character_number -= 0xa3;
	    else
		*character_number -= 1;
	    if (*character_number < start) {
		error_code = EB_ERR_NO_SUCH_CHAR_TEXT;
		goto failed;
	    }
	}
    }

    LOG(("out: eb_backward_wide_alt_character(character_number=%d) = %s",
	*character_number, eb_error_string(EB_SUCCESS)));
    eb_unlock(&appendix->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *character_number = -1;
    LOG(("out: eb_backward_wide_alt_character() = %s",
	eb_error_string(error_code)));
    eb_unlock(&appendix->lock);
    return error_code;
}


