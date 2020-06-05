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
 * The maximum number of arguments for an escape sequence.
 */
#define EB_MAX_ARGV 	7

/*
 * Read next when the length of cached data is shorter than this value.
 */
#define SIZE_FEW_REST	48

/*
 * Special skip-code that represents `no skip-code is set'.
 */
#define SKIP_CODE_NONE  -1

/*
 * Cache data buffer.
 */
static char cache_buffer[EB_SIZE_PAGE];

/*
 * Book code of which `cache_buffer' records data.
 */
static EB_Book_Code cache_book_code = EB_BOOK_NONE;

/*
 * Location of cache data loaded in `cache_buffer'.
 */
static off_t cache_location;

/*
 * Length of cache data loaded in `cache_buffer'.
 */
static size_t cache_length;

/*
 * Null hook.
 */
static const EB_Hook null_hook = {EB_HOOK_NULL, NULL};

/*
 * Mutex for cache variables.
 */
#ifdef ENABLE_PTHREAD
static pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * Unexported functions.
 */
static EB_Error_Code eb_read_text_internal(EB_Book *book,
    EB_Appendix *appendix, EB_Hookset *hookset, void *container,
    size_t text_max_length, char *text, ssize_t *text_length,
    int forward_only);
static int eb_is_stop_code(EB_Book *book, EB_Appendix *appendix,
    unsigned int code0, unsigned int code1);


/*
 * Initialize text context of `book'.
 */
void
eb_initialize_text_context(EB_Book *book)
{
    LOG(("in: eb_initialize_text_context(book=%d)", (int)book->code));

    book->text_context.code = EB_TEXT_INVALID;
    book->text_context.location = -1;
    book->text_context.out = NULL;
    book->text_context.out_rest_length = 0;
    book->text_context.unprocessed = NULL;
    book->text_context.unprocessed_size = 0;
    book->text_context.out_step = 0;
    book->text_context.narrow_flag = 0;
    book->text_context.printable_count = 0;
    book->text_context.file_end_flag = 0;
    book->text_context.text_status = EB_TEXT_STATUS_CONTINUED;
    book->text_context.skip_code = SKIP_CODE_NONE;
    book->text_context.auto_stop_code = -1;
    book->text_context.candidate[0] = '\0';
    book->text_context.is_candidate = 0;
    book->text_context.ebxac_gaiji_flag = 0;

    LOG(("out: eb_initialize_text_context()"));
}


/*
 * Finalize text context of `book'.
 */
void
eb_finalize_text_context(EB_Book *book)
{
    LOG(("in: eb_finalize_text_context(book=%d)", (int)book->code));

    if (book->text_context.unprocessed != NULL)
	free(book->text_context.unprocessed);

    LOG(("out: eb_finalize_text_context()"));
}


/*
 * Reset text context of `book'.
 * Note that `contexxt_code' and `context_location' are unchanged.
 */
void
eb_reset_text_context(EB_Book *book)
{
    LOG(("in: eb_reset_text_context(book=%d)", (int)book->code));

    eb_finalize_text_context(book);

    book->text_context.out = NULL;
    book->text_context.out_rest_length = 0;
    book->text_context.unprocessed = NULL;
    book->text_context.unprocessed_size = 0;
    book->text_context.out_step = 0;
    book->text_context.narrow_flag = 0;
    book->text_context.printable_count = 0;
    book->text_context.file_end_flag = 0;
    book->text_context.text_status = EB_TEXT_STATUS_CONTINUED;
    book->text_context.skip_code = SKIP_CODE_NONE;
    book->text_context.auto_stop_code = -1;
    book->text_context.candidate[0] = '\0';
    book->text_context.is_candidate = 0;
    book->text_context.ebxac_gaiji_flag = 0;

    LOG(("out: eb_reset_text_context()"));
}


/*
 * Invalidate text context of `book'.
 */
void
eb_invalidate_text_context(EB_Book *book)
{
    LOG(("in: eb_invalidate_text_context(book=%d)", (int)book->code));

    eb_finalize_text_context(book);
    eb_initialize_text_context(book);
    book->text_context.code = EB_TEXT_INVALID;

    LOG(("out: eb_invalidate_text_context()"));
}


/*
 * Reposition the offset of the subbook file.
 */
EB_Error_Code
eb_seek_text(EB_Book *book, const EB_Position *position)
{
    EB_Error_Code error_code;

    pthread_mutex_lock(&cache_mutex);
    eb_lock(&book->lock);
    LOG(("in: eb_seek_text(book=%d, position={%d,%d})", (int)book->code,
	position->page, position->offset));

    /*
     * Current subbook must have been set and START file must exist.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_TEXT;
	goto failed;
    }

    if (position->page <= 0 || position->offset < 0) {
	error_code = EB_ERR_FAIL_SEEK_TEXT;
	goto failed;
    }

    /*
     * Set text-context variables.
     */
    eb_reset_text_context(book);
    book->text_context.code = EB_TEXT_SEEKED;
    book->text_context.location = ((off_t) position->page - 1) * EB_SIZE_PAGE
	+ position->offset;

    /*
     * Unlock cache data and the book.
     */
    LOG(("out: eb_seek_text() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);
    pthread_mutex_unlock(&cache_mutex);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_invalidate_text_context(book);
    LOG(("out: eb_seek_text() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    pthread_mutex_unlock(&cache_mutex);
    return error_code;
}


/*
 * Get the current text position of the subbook file.
 */
EB_Error_Code
eb_tell_text(EB_Book *book, EB_Position *position)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_tell_text(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set and START file must exist.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_TEXT;
	goto failed;
    }

    position->page = book->text_context.location / EB_SIZE_PAGE + 1;
    position->offset = book->text_context.location % EB_SIZE_PAGE;

    LOG(("out: eb_seek_text(position={%d,%d}) = %s",
	position->page, position->offset, eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_invalidate_text_context(book);
    LOG(("out: eb_seek_text() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Get text in the current subbook in `book'.
 */
EB_Error_Code
eb_read_text(EB_Book *book, EB_Appendix *appendix, EB_Hookset *hookset,
    void *container, size_t text_max_length, char *text, ssize_t *text_length)
{
    EB_Error_Code error_code;
    const EB_Hook *hook;
    EB_Position position;

    eb_lock(&book->lock);
    if (appendix != NULL)
	eb_lock(&appendix->lock);
    if (hookset != NULL)
	eb_lock(&hookset->lock);
    LOG(("in: eb_read_text(book=%d, appendix=%d, text_max_length=%ld)",
	(int)book->code, (appendix != NULL) ? (int)appendix->code : 0,
	(long)text_max_length));

    /*
     * Current subbook must have been set and START file must exist.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_TEXT;
	goto failed;
    }

    /*
     * Use `eb_default_hookset' when `hookset' is `NULL'.
     */
    if (hookset == NULL)
	hookset = &eb_default_hookset;

    /*
     * Set text mode to `text'.
     */
    if (book->text_context.code == EB_TEXT_INVALID) {
	error_code = EB_ERR_NO_PREV_SEEK;
	goto failed;
    } else if (book->text_context.code == EB_TEXT_SEEKED) {
	eb_tell_text(book, &position);
	eb_reset_text_context(book);

	if (book->subbook_current->menu.start_page <= position.page
	    && position.page <= book->subbook_current->menu.end_page)
	    book->text_context.code = EB_TEXT_OPTIONAL_TEXT;
	else if (book->subbook_current->image_menu.start_page <= position.page
	    && position.page <= book->subbook_current->image_menu.end_page)
	    book->text_context.code = EB_TEXT_OPTIONAL_TEXT;
	else if (book->subbook_current->copyright.start_page <= position.page
	    && position.page <= book->subbook_current->copyright.end_page)
	    book->text_context.code = EB_TEXT_OPTIONAL_TEXT;
	else
	    book->text_context.code = EB_TEXT_MAIN_TEXT;

	hook = hookset->hooks + EB_HOOK_INITIALIZE;
	if (hook->function != NULL) {
	    error_code = hook->function(book, appendix, container,
		EB_HOOK_INITIALIZE, 0, NULL);
	    if (error_code != EB_SUCCESS)
		goto failed;
	}
    } else if (book->text_context.code != EB_TEXT_MAIN_TEXT
	&& book->text_context.code != EB_TEXT_OPTIONAL_TEXT) {
	error_code = EB_ERR_DIFF_CONTENT;
	goto failed;
    }

    error_code = eb_read_text_internal(book, appendix, hookset, container,
	text_max_length, text, text_length, 0);
    if (error_code != EB_SUCCESS)
	goto failed;

    LOG(("out: eb_read_text(text_length=%ld) = %s", (long)*text_length,
	eb_error_string(EB_SUCCESS)));
    if (hookset != &eb_default_hookset)
	eb_unlock(&hookset->lock);
    if (appendix != NULL)
	eb_unlock(&appendix->lock);
    eb_unlock(&book->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_invalidate_text_context(book);
    LOG(("out: eb_read_text() = %s", eb_error_string(error_code)));
    if (hookset != &eb_default_hookset)
	eb_unlock(&hookset->lock);
    if (appendix != NULL)
	eb_unlock(&appendix->lock);
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Get text in the current subbook in `book'.
 */
EB_Error_Code
eb_read_heading(EB_Book *book, EB_Appendix *appendix, EB_Hookset *hookset,
    void *container, size_t text_max_length, char *text, ssize_t *text_length)
{
    EB_Error_Code error_code;
    const EB_Hook *hook;

    eb_lock(&book->lock);
    if (appendix != NULL)
	eb_lock(&appendix->lock);
    if (hookset != NULL)
	eb_lock(&hookset->lock);
    LOG(("in: eb_read_heading(book=%d, appendix=%d, text_max_length=%ld)",
	(int)book->code, (appendix != NULL) ? (int)appendix->code : 0,
	(long)text_max_length));

    /*
     * Current subbook must have been set and START file must exist.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_TEXT;
	goto failed;
    }

    /*
     * Use `eb_default_hookset' when `hookset' is `NULL'.
     */
    if (hookset == NULL)
	hookset = &eb_default_hookset;

    /*
     * Set text mode to `heading'.
     */
    if (book->text_context.code == EB_TEXT_INVALID) {
	error_code = EB_ERR_NO_PREV_SEEK;
	goto failed;
    } else if (book->text_context.code == EB_TEXT_SEEKED) {
	eb_reset_text_context(book);
	book->text_context.code = EB_TEXT_HEADING;

	hook = hookset->hooks + EB_HOOK_INITIALIZE;
	if (hook->function != NULL) {
	    error_code = hook->function(book, appendix, container,
		EB_HOOK_INITIALIZE, 0, NULL);
	    if (error_code != EB_SUCCESS)
		goto failed;
	}
    } else if (book->text_context.code != EB_TEXT_HEADING) {
	error_code = EB_ERR_DIFF_CONTENT;
	goto failed;
    }

    error_code = eb_read_text_internal(book, appendix, hookset, container,
	text_max_length, text, text_length, 0);
    if (error_code != EB_SUCCESS)
	goto failed;

    LOG(("out: eb_read_heading(text_length=%ld) = %s", (long)*text_length,
	eb_error_string(EB_SUCCESS)));
    if (hookset != &eb_default_hookset)
	eb_unlock(&hookset->lock);
    if (appendix != NULL)
	eb_unlock(&appendix->lock);
    eb_unlock(&book->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_invalidate_text_context(book);
    LOG(("out: eb_read_heading() = %s", eb_error_string(error_code)));
    if (hookset != &eb_default_hookset)
	eb_unlock(&hookset->lock);
    if (appendix != NULL)
	eb_unlock(&appendix->lock);
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Read data from the subbook file directly.
 */
EB_Error_Code
eb_read_rawtext(EB_Book *book, size_t text_max_length, char *text,
    ssize_t *text_length)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_read_rawtext(book=%d, text_max_length=%ld)",
	(int)book->code, (long)text_max_length));

    /*
     * Current subbook must have been set and START file must exist.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_TEXT;
	goto failed;
    }

    /*
     * Set text mode to `rawtext'.
     */
    if (book->text_context.code == EB_TEXT_INVALID) {
	error_code = EB_ERR_NO_PREV_SEEK;
	goto failed;
    } else if (book->text_context.code == EB_TEXT_SEEKED) {
	eb_reset_text_context(book);
	book->text_context.code = EB_TEXT_RAWTEXT;
    } else if (book->text_context.code != EB_TEXT_RAWTEXT) {
	error_code = EB_ERR_DIFF_CONTENT;
	goto failed;
    }

    /*
     * Seek START file and read data.
     */
    if (zio_lseek(&book->subbook_current->text_zio,
	book->text_context.location, SEEK_SET) == -1) {
	error_code = EB_ERR_FAIL_SEEK_TEXT;
	goto failed;
    }
    *text_length = zio_read(&book->subbook_current->text_zio, text,
	text_max_length);
    book->text_context.location += *text_length;
    if (*text_length < 0) {
	error_code = EB_ERR_FAIL_READ_TEXT;
	goto failed;
    }

    LOG(("out: eb_read_rawtext(text_length=%ld) = %s", (long)*text_length,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *text_length = -1;
    eb_invalidate_text_context(book);
    LOG(("out: eb_read_rawtext() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Get text or heading.
 */
static EB_Error_Code
eb_read_text_internal(EB_Book *book, EB_Appendix *appendix,
    EB_Hookset *hookset, void *container, size_t text_max_length, char *text,
    ssize_t *text_length, int forward_only)
{
    EB_Error_Code error_code;
    EB_Text_Context *context;
    unsigned char c1, c2;
    char *cache_p;
    const EB_Hook *hook;
    unsigned char *candidate_p;
    size_t candidate_length;
    size_t cache_rest_length;
    size_t in_step;
    unsigned int argv[EB_MAX_ARGV];
    int argc;

    pthread_mutex_lock(&cache_mutex);
    LOG(("in: eb_read_text_internal(book=%d, appendix=%d, \
text_max_length=%ld, forward=%d)",
	(int)book->code, (appendix != NULL) ? (int)appendix->code : 0,
	(long)text_max_length, forward_only));

    /*
     * Initialize variables.
     */
    context = &book->text_context;
    context->out = text;
    context->out_rest_length = text_max_length;
    if (context->is_candidate) {
	candidate_length = strlen(context->candidate);
	candidate_p = (unsigned char *)context->candidate + candidate_length;
    } else {
	candidate_length = 0;
	candidate_p = NULL;
    }

    /*
     * If unprocessed string are rest in `context->unprocessed',
     * copy them to `context->out'.
     */
    if (context->unprocessed != NULL) {
	if (!forward_only) {
	    if (context->out_rest_length < context->unprocessed_size)
		goto succeeded;
	    memcpy(context->out, context->unprocessed,
		context->unprocessed_size);
	    context->out += context->unprocessed_size;
	    context->out_rest_length -= context->unprocessed_size;
	}
	free(context->unprocessed);
	context->unprocessed = NULL;
	context->unprocessed_size = 0;
    }

    /*
     * Return immediately if text-end-flag has been set.
     */
    if (context->text_status != EB_TEXT_STATUS_CONTINUED)
	goto succeeded;

    /*
     * Check for cache data.
     * If cache data is not what we need, discard it.
     */
    if (book->code == cache_book_code
	&& cache_location <= context->location
	&& context->location < cache_location + cache_length) {
	cache_p = cache_buffer + (context->location - cache_location);
	cache_rest_length = cache_length
	    - (context->location - cache_location);
    } else {
	cache_book_code = EB_BOOK_NONE;
	cache_p = cache_buffer;
	cache_length = 0;
	cache_rest_length = 0;
    }

    for (;;) {
	in_step = 0;
	context->out_step = 0;
	argc = 1;

	/*
	 * If it reaches to the near of the end of the cache buffer,
	 * then moves remaind cache text to the beginning of the cache
	 * buffer, and reads a next chunk from a file.
	 */
	if (cache_rest_length < SIZE_FEW_REST && !context->file_end_flag) {
	    ssize_t read_result;

	    if (0 < cache_rest_length)
		memmove(cache_buffer, cache_p, cache_rest_length);
	    if (zio_lseek(&book->subbook_current->text_zio,
		context->location + cache_rest_length, SEEK_SET) == -1) {
		error_code = EB_ERR_FAIL_SEEK_TEXT;
		goto failed;
	    }

	    read_result = zio_read(&book->subbook_current->text_zio,
		cache_buffer + cache_rest_length,
		EB_SIZE_PAGE - cache_rest_length);
	    if (read_result < 0) {
		error_code = EB_ERR_FAIL_READ_TEXT;
		goto failed;
	    } else if (read_result != EB_SIZE_PAGE - cache_rest_length)
		context->file_end_flag = 1;

	    cache_book_code = book->code;
	    cache_location = context->location;
	    cache_length = cache_rest_length + read_result;
	    cache_p = cache_buffer;
	    cache_rest_length = cache_length;
	}

	/*
	 * Get 1 byte from the buffer.
	 */
	if (cache_rest_length < 1) {
	    error_code = EB_ERR_UNEXP_TEXT;
	    goto failed;
	}
	c1 = eb_uint1(cache_p);

	if (c1 == 0x1f) {
	    hook = &null_hook;

	    /*
	     * This is escape sequences.
	     */
	    if (cache_rest_length < 2) {
		error_code = EB_ERR_UNEXP_TEXT;
		goto failed;
	    }
	    argv[0] = eb_uint2(cache_p);
	    c2 = eb_uint1(cache_p + 1);

	    switch (c2) {
	    case 0x02:
		/* beginning of text */
		in_step = 2;
		break;

	    case 0x03:
		/* end of text (don't set `in_step') */
		context->text_status = EB_TEXT_STATUS_HARD_STOP;
		if (forward_only) {
		    error_code = EB_ERR_END_OF_CONTENT;
		    goto failed;
		}
		goto succeeded;

	    case 0x04:
		/* beginning of NARROW */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_BEGIN_NARROW;
		context->narrow_flag = 1;
		break;

	    case 0x05:
		/* end of NARROW */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_END_NARROW;
		context->narrow_flag = 0;
		break;

	    case 0x06:
		/* beginning of subscript */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_BEGIN_SUBSCRIPT;
		break;

	    case 0x07:
		/* end of subscript */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_END_SUBSCRIPT;
		break;

	    case 0x09:
		/* set indent */
		in_step = 4;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 2;
		argv[1] = eb_uint2(cache_p + 2);

		if (0 < context->printable_count
		    && context->code == EB_TEXT_MAIN_TEXT) {
		    if (eb_is_stop_code(book, appendix, argv[0], argv[1])) {
			context->text_status = EB_TEXT_STATUS_SOFT_STOP;
			goto succeeded;
		    }
		}

		hook = hookset->hooks + EB_HOOK_SET_INDENT;
		break;

	    case 0x0a:
		/* newline */
		in_step = 2;
		if (context->code == EB_TEXT_HEADING) {
		    context->text_status = EB_TEXT_STATUS_SOFT_STOP;
		    context->location += in_step;
		    goto succeeded;
		}
		hook = hookset->hooks + EB_HOOK_NEWLINE;
		break;

	    case 0x0b:
		/* beginning of unicode */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_BEGIN_UNICODE;
		break;

	    case 0x0c:
		/* end of unicode */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_END_UNICODE;
		break;

	    case 0x0e:
		/* beginning of superscript */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_BEGIN_SUPERSCRIPT;
		break;

	    case 0x0f:
		/* end of superscript */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_END_SUPERSCRIPT;
		break;

	    case 0x10:
		/* beginning of newline prohibition */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_BEGIN_NO_NEWLINE;
		break;

	    case 0x11:
		/* end of newline prohibition */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_END_NO_NEWLINE;
		break;

	    case 0x12:
		/* beginning of emphasis */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_BEGIN_EMPHASIS;
		break;

	    case 0x13:
		/* end of emphasis */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_END_EMPHASIS;
		break;

	    case 0x14:
		in_step = 4;
		context->skip_code = 0x15;
		break;

	    case 0x1a: case 0x1b: case 0x1e: case 0x1f:
		/* emphasis; described in JIS X 4081-1996 */
		in_step = 4;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		/* Some old EB books don't take an argument. */
		if (book->disc_code != EB_DISC_EPWING
		    && eb_uint1(cache_p + 2) >= 0x1f)
		    in_step = 2;
		break;

	    case 0x1c:
		if (book->character_code == EB_CHARCODE_JISX0208_GB2312) {
		    /* beginning of EBXA-C gaiji */
		    in_step = 2;
		    hook = hookset->hooks + EB_HOOK_BEGIN_EBXAC_GAIJI;
		    context->ebxac_gaiji_flag = 1;
		} else {
		    in_step = 4;
		    if (cache_rest_length < in_step) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }
		    /* Some old EB books don't take an argument. */
		    if (book->disc_code != EB_DISC_EPWING
			&& eb_uint1(cache_p + 2) >= 0x1f)
			in_step = 2;
		}
		break;

	    case 0x1d:
		if (book->character_code == EB_CHARCODE_JISX0208_GB2312) {
		    /* end of EBXA-C gaiji */
		    in_step = 2;
		    hook = hookset->hooks + EB_HOOK_END_EBXAC_GAIJI;
		    context->ebxac_gaiji_flag = 0;
		} else {
		    in_step = 4;
		    if (cache_rest_length < in_step) {
			error_code = EB_ERR_UNEXP_TEXT;
			goto failed;
		    }
		    /* Some old EB books don't take an argument. */
		    if (book->disc_code != EB_DISC_EPWING
			&& eb_uint1(cache_p + 2) >= 0x1f)
			in_step = 2;
		}
		break;

	    case 0x32:
		/* beginning of reference to monochrome graphic */
		in_step = 2;
		argc = 4;
		argv[1] = 0;
		argv[2] = 0;
		argv[3] = 0;
		hook = hookset->hooks + EB_HOOK_BEGIN_MONO_GRAPHIC;
		break;

	    case 0x39:
		/* beginning of MPEG movie */
		in_step = 46;
		argc = 6;
		argv[1] = eb_uint4(cache_p + 2);
		argv[2] = eb_uint4(cache_p + 22);
		argv[3] = eb_uint4(cache_p + 26);
		argv[4] = eb_uint4(cache_p + 30);
		argv[5] = eb_uint4(cache_p + 34);
		hook = hookset->hooks + EB_HOOK_BEGIN_MPEG;
		break;

	    case 0x3c:
		/* beginning of inline color graphic */
		in_step = 20;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 4;
		argv[1] = eb_uint2(cache_p + 2);
		argv[2] = eb_bcd4(cache_p + 14);
		argv[3] = eb_bcd2(cache_p + 18);
		if (argv[1] >> 8 == 0x00)
		    hook = hookset->hooks + EB_HOOK_BEGIN_IN_COLOR_BMP;
		else
		    hook = hookset->hooks + EB_HOOK_BEGIN_IN_COLOR_JPEG;
		break;

	    case 0x35: case 0x36: case 0x37: case 0x38: case 0x3a:
	    case 0x3b: case 0x3d: case 0x3e: case 0x3f:
		in_step = 2;
		context->skip_code = eb_uint1(cache_p + 1) + 0x20;
		break;

	    case 0x41:
		/* beginning of keyword */
		in_step = 4;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 2;
		argv[1] = eb_uint2(cache_p + 2);

		if (0 < context->printable_count
		    && context->code == EB_TEXT_MAIN_TEXT) {
		    if (eb_is_stop_code(book, appendix, argv[0], argv[1])) {
			context->text_status = EB_TEXT_STATUS_SOFT_STOP;
			goto succeeded;
		    }
		}
		if (context->auto_stop_code < 0)
		    context->auto_stop_code = eb_uint2(cache_p + 2);

		hook = hookset->hooks + EB_HOOK_BEGIN_KEYWORD;
		break;

	    case 0x42:
		/* beginning of reference */
		in_step = 4;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		if (eb_uint1(cache_p + 2) != 0x00)
		    in_step -= 2;
		hook = hookset->hooks + EB_HOOK_BEGIN_REFERENCE;
		break;

	    case 0x43:
		/* beginning of an entry of a candidate */
		in_step = 2;
		if (context->skip_code == SKIP_CODE_NONE) {
		    context->candidate[0] = '\0';
		    context->is_candidate = 1;
		    candidate_length = 0;
		    candidate_p = (unsigned char *)context->candidate;
		}
		hook = hookset->hooks + EB_HOOK_BEGIN_CANDIDATE;
		break;

	    case 0x44:
		/* beginning of monochrome graphic */
		in_step = 12;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 4;
		argv[1] = eb_uint2(cache_p + 2);
		argv[2] = eb_bcd4(cache_p + 4);
		argv[3] = eb_bcd4(cache_p + 8);
		if (0 < argv[2] && 0 < argv[3])
		    hook = hookset->hooks + EB_HOOK_BEGIN_MONO_GRAPHIC;
		break;

	    case 0x45:
		/* beginning of graphic block */
		in_step = 4;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		if (eb_uint1(cache_p + 2) != 0x1f) {
		    argc = 2;
		    argv[1] = eb_bcd4(cache_p + 2);
		} else {
		    in_step = 2;
		}
		break;

	    case 0x4a:
		/* beginning of WAVE sound */
		in_step = 18;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 6;
		argv[1] = eb_uint4(cache_p + 2);
		argv[2] = eb_bcd4(cache_p + 6);
		argv[3] = eb_bcd2(cache_p + 10);
		argv[4] = eb_bcd4(cache_p + 12);
		argv[5] = eb_bcd2(cache_p + 16);
		hook = hookset->hooks + EB_HOOK_BEGIN_WAVE;
		break;

	    case 0x4b:
		/* beginning of paged reference */
		in_step = 8;
		if (cache_rest_length < in_step + 2) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 3;
		argv[1] = eb_bcd4(cache_p + 2);
		argv[2] = eb_bcd2(cache_p + 6);
		if (cache_p[8]==0x1f && cache_p[9]==0x6b) {
		    context->text_status = EB_TEXT_STATUS_SOFT_STOP;
		    hook = hookset->hooks + EB_HOOK_GRAPHIC_REFERENCE;
		    in_step = 10;
		} else {
		    hook = hookset->hooks + EB_HOOK_BEGIN_GRAPHIC_REFERENCE;
		}
		break;

	    case 0x4c:
		/* beginning of image page */
		in_step = 4;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		hook = hookset->hooks + EB_HOOK_BEGIN_IMAGE_PAGE;
		break;

	    case 0x4d:
		/* beginning of color graphic (BMP or JPEG) */
		in_step = 20;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 4;
		argv[1] = eb_uint2(cache_p + 2);
		argv[2] = eb_bcd4(cache_p + 14);
		argv[3] = eb_bcd2(cache_p + 18);
		if (argv[1] >> 8 == 0x00)
		    hook = hookset->hooks + EB_HOOK_BEGIN_COLOR_BMP;
		else
		    hook = hookset->hooks + EB_HOOK_BEGIN_COLOR_JPEG;
		break;

	    case 0x4f:
	        /* beginning of clickable area */
	        in_step = 34;
	        if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 7;
		argv[1] = eb_bcd2(cache_p + 8);	  
		argv[2] = eb_bcd2(cache_p + 10);	  
		argv[3] = eb_bcd2(cache_p + 12);	  
		argv[4] = eb_bcd2(cache_p + 14);	  
		argv[5] = eb_bcd4(cache_p + 28);	  
		argv[6] = eb_bcd2(cache_p + 32);	  
		hook = hookset->hooks + EB_HOOK_BEGIN_CLICKABLE_AREA;
	      break;

	    case 0x49: case 0x4e:

		in_step = 2;
		context->skip_code = eb_uint1(cache_p + 1) + 0x20;
		break;

	    case 0x52:
		/* end of reference to monochrome graphic */
		in_step = 8;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 3;
		argv[1] = eb_bcd4(cache_p + 2);
		argv[2] = eb_bcd2(cache_p + 6);
		hook = hookset->hooks + EB_HOOK_END_MONO_GRAPHIC;
		break;

	    case 0x53:
		/* end of EB sound */
		in_step = 10;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		break;

	    case 0x59:
		/* end of MPEG movie */
		in_step = 2;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		hook = hookset->hooks + EB_HOOK_END_MPEG;
		break;

	    case 0x5c:
		/* end of inline color graphic */
		in_step = 2;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		hook = hookset->hooks + EB_HOOK_END_IN_COLOR_GRAPHIC;
		break;

	    case 0x61:
		/* end of keyword */
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_END_KEYWORD;
		break;

	    case 0x62:
		/* end of reference */
		in_step = 8;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 3;
		argv[1] = eb_bcd4(cache_p + 2);
		argv[2] = eb_bcd2(cache_p + 6);
		hook = hookset->hooks + EB_HOOK_END_REFERENCE;
		break;

	    case 0x63:
		/* end of an entry of a candidate */
		in_step = 8;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 3;
		argv[1] = eb_bcd4(cache_p + 2);
		argv[2] = eb_bcd2(cache_p + 6);
		if (argv[1] == 0 && argv[2] == 0)
		    hook = hookset->hooks + EB_HOOK_END_CANDIDATE_LEAF;
		else
		    hook = hookset->hooks + EB_HOOK_END_CANDIDATE_GROUP;
		break;

	    case 0x64:
		/* end of monochrome graphic */
		in_step = 8;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 3;
		argv[1] = eb_bcd4(cache_p + 2);
		argv[2] = eb_bcd2(cache_p + 6);
		hook = hookset->hooks + EB_HOOK_END_MONO_GRAPHIC;
		break;

	    case 0x6b:
		/* end of paged reference */
		in_step = 2;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		hook = hookset->hooks + EB_HOOK_END_GRAPHIC_REFERENCE;
		break;

	    case 0x6a:
		/* end of WAVE sound */
		in_step = 2;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		hook = hookset->hooks + EB_HOOK_END_WAVE;
		break;

	    case 0x6c:
		/* end of image page */
		in_step = 2;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		context->text_status = EB_TEXT_STATUS_SOFT_STOP;
		hook = hookset->hooks + EB_HOOK_END_IMAGE_PAGE;
		break;

	    case 0x6d:
		/* end of color graphic (BMP or JPEG) */
		in_step = 2;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		hook = hookset->hooks + EB_HOOK_END_COLOR_GRAPHIC;
		break;

	    case 0x6f:
		/* end of clickable area */
		in_step = 2;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		hook = hookset->hooks + EB_HOOK_END_CLICKABLE_AREA;
		break;

	    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75:
	    case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b:
	    case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	    case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85:
	    case 0x86: case 0x87: case 0x88: case 0x89: case 0x8a: case 0x8b:
	    case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		in_step = 2;
		context->skip_code = eb_uint1(cache_p + 1) + 0x20;
		break;

	    case 0xd0:
		/* halfwidth local character for UTF-8 text */
		in_step = 4;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		argv[0] = eb_uint2(cache_p + 2) | 0x8080;
		hook = hookset->hooks + EB_HOOK_NARROW_FONT;
		if (forward_only) {
		    ; /* do nothing */
		} else if (hook->function == NULL) {
		    ; /* do nothing */
		} else {
		    error_code = hook->function(book, appendix, container,
						EB_HOOK_NARROW_FONT, argc, argv);
		    if (error_code != EB_SUCCESS)
			goto failed;
		}
		break;
	    case 0xd1:
		/* fullwidth local character for UTF-8 text */
		in_step = 4;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		argv[0] = eb_uint2(cache_p + 2) | 0x8080;
		hook = hookset->hooks + EB_HOOK_WIDE_FONT;
		if (forward_only) {
		    ; /* do nothing */
		} else if (hook->function == NULL) {
		    ; /* do nothing */
		} else {
		    error_code = hook->function(book, appendix, container,
						EB_HOOK_WIDE_FONT, argc, argv);
		    if (error_code != EB_SUCCESS)
			goto failed;
		}
	    case 0xe0:
		/* character modification */
		in_step = 4;
		if (cache_rest_length < in_step) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}
		argc = 2;
		argv[1] = eb_uint2(cache_p + 2);
		hook = hookset->hooks + EB_HOOK_BEGIN_DECORATION;

		/* Some old EB books don't take an argument. */
		if (book->disc_code != EB_DISC_EPWING
		    && eb_uint1(cache_p + 2) >= 0x1f) {
		    in_step = 2;
		    hook = &null_hook;
		}
		break;

	    case 0xe1:
		in_step = 2;
		hook = hookset->hooks + EB_HOOK_END_DECORATION;
		break;

	    case 0xe4: case 0xe6: case 0xe8: case 0xea: case 0xec: case 0xee:
	    case 0xf0: case 0xf2: case 0xf4: case 0xf6: case 0xf8: case 0xfa:
	    case 0xfc: case 0xfe:
		in_step = 2;
		context->skip_code = eb_uint1(cache_p + 1) + 0x01;
		break;

	    default:
		in_step = 2;
		if (context->skip_code == eb_uint1(cache_p + 1))
		    context->skip_code = SKIP_CODE_NONE;
		break;
	    }

	    if (context->skip_code == SKIP_CODE_NONE
		&& hook->function != NULL
		&& !forward_only) {
		error_code = hook->function(book, appendix, container,
		    hook->code, argc, argv);
		if (error_code != EB_SUCCESS)
		    goto failed;
	    }

	    /*
	     * Post process.  Clean a candidate.
	     */
	    if (c2 == 0x63) {
		/* end of an entry of candidate */
		context->is_candidate = 0;
	    }

	} else if (book->character_code == EB_CHARCODE_ISO8859_1) {
	    /*
	     * The book is mainly written in ISO 8859 1.
	     */
	    context->printable_count++;

	    if ((0x20 <= c1 && c1 < 0x7f) || (0xa0 <= c1 && c1 <= 0xff)) {
		/*
		 * This is an ISO 8859 1 character.
		 */
		in_step = 1;
		argv[0] = eb_uint1(cache_p);

		if (context->skip_code == SKIP_CODE_NONE) {
		    if (context->is_candidate
			&& candidate_length < EB_MAX_WORD_LENGTH) {
			*candidate_p++ = c1 | 0x80;
			*candidate_p = '\0';
			candidate_length++;
		    }

		    hook = hookset->hooks + EB_HOOK_ISO8859_1;
		    if (forward_only) {
			; /* do nothing */
		    } else if (hook->function == NULL) {
			error_code = eb_write_text_byte1(book, c1);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    } else {
			error_code = hook->function(book, appendix, container,
			    EB_HOOK_ISO8859_1, argc, argv);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    }
		}
	    } else {
		/*
		 * This is a local character.
		 */
		if (cache_rest_length < 2) {
		    error_code = EB_ERR_UNEXP_TEXT;
		    goto failed;
		}

		in_step = 2;
		argv[0] = eb_uint2(cache_p);
		if (context->skip_code == SKIP_CODE_NONE) {
		    hook = hookset->hooks + EB_HOOK_NARROW_FONT;
		    if (forward_only) {
			; /* do nothing */
		    } else if (hook->function == NULL) {
			error_code = eb_write_text_byte1(book, c1);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    } else {
			error_code = hook->function(book, appendix, container,
			    EB_HOOK_NARROW_FONT, argc, argv);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    }
		}
	    }

	} else if (book->character_code == EB_CHARCODE_UTF8) {
	    /*
	     * The book is mainly written in UTF-8.
	     */
	    context->printable_count++;

	    if (!(c1 & 0x80)) {
		argv[0] = c1;
		in_step = 1;
	    } else if ((cache_rest_length >= 2) && !(c1 & 0x20)) {
		argv[0] = ((c1 & 0x1f) <<  6) +
		    ((eb_uint1(cache_p + 1) & 0x3f));
		in_step = 2;
	    } else if ((cache_rest_length >= 3) && !(c1 & 0x10)) {
		argv[0] = ((c1 & 0x0f) << 12) +
		    ((eb_uint1(cache_p + 1) & 0x3f) <<  6) + 
		    ((eb_uint1(cache_p + 2) & 0x3f)      );
		in_step = 3;
	    } else if ((cache_rest_length >= 4) && !(c1 & 0x08)) {
		argv[0] = ((c1 & 0x07) << 18) +
		    ((eb_uint1(cache_p + 1) & 0x3f) << 12) + 
		    ((eb_uint1(cache_p + 2) & 0x3f) <<  6) + 
		    ((eb_uint1(cache_p + 3) & 0x3f)      );
		in_step = 4;
	    } else {
		error_code = EB_ERR_UNEXP_TEXT;
		goto failed;
	    }

	    if (context->skip_code == SKIP_CODE_NONE) {
		if (context->is_candidate
		    && candidate_length + in_step <= EB_MAX_WORD_LENGTH) {
		    memcpy(candidate_p, cache_p, in_step);
		    candidate_p += in_step;
		    *candidate_p = '\0';
		    candidate_length += in_step;
		}

		if (!forward_only) {
		    if (in_step == 1) {
			hook = hookset->hooks + EB_HOOK_ISO8859_1;
			if (hook->function == NULL) {
			    error_code = eb_write_text_byte1(book, c1);
			    if (error_code != EB_SUCCESS)
				goto failed;
			} else {
			    error_code = hook->function(book, appendix,
				     container, EB_HOOK_ISO8859_1, argc, argv);
			    if (error_code != EB_SUCCESS)
				goto failed;
			}
		    } else {
			hook = hookset->hooks + EB_HOOK_UNICODE;
			if (hook->function == NULL) {
			    error_code = eb_write_text(book, cache_p, in_step);
			    if (error_code != EB_SUCCESS)
				goto failed;
			} else {
			    error_code = hook->function(book, appendix,
				     container, EB_HOOK_UNICODE, argc, argv);
			    if (error_code != EB_SUCCESS)
				goto failed;
			}
		    }
		}
	    }
	} else {
	    /*
	     * The book is written in JIS X 0208 or JIS X 0208 + GB 2312.
	     */
	    context->printable_count++;
	    in_step = 2;

	    if (cache_rest_length < 2) {
		error_code = EB_ERR_UNEXP_TEXT;
		goto failed;
	    }

	    c2 = eb_uint1(cache_p + 1);

	    if (context->skip_code != SKIP_CODE_NONE) {
		/* nothing to be done. */
	    } else if (0x20 < c1 && c1 < 0x7f && 0x20 < c2 && c2 < 0x7f) {
		/*
		 * This is a JIS X 0208 KANJI character.
		 */
		argv[0] = eb_uint2(cache_p) | 0x8080;

		if (context->is_candidate
		    && candidate_length < EB_MAX_WORD_LENGTH - 1) {
		    *candidate_p++ = c1 | 0x80;
		    *candidate_p++ = c2 | 0x80;
		    *candidate_p = '\0';
		    candidate_length += 2;
		}

		if (context->ebxac_gaiji_flag) {
		    hook = hookset->hooks + EB_HOOK_EBXAC_GAIJI;
		    if (forward_only) {
			; /* do nothing */
		    } else if (hook->function == NULL) {
			error_code = eb_write_text_byte2(book, c1 | 0x80,
			    c2 | 0x80);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    } else {
			error_code = hook->function(book, appendix, container,
			    EB_HOOK_EBXAC_GAIJI, 0, argv);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    }
		} else if (context->narrow_flag) {
		    hook = hookset->hooks + EB_HOOK_NARROW_JISX0208;
		    if (forward_only) {
			; /* do nothing */
		    } else if (hook->function == NULL) {
			error_code = eb_write_text_byte2(book, c1 | 0x80,
			    c2 | 0x80);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    } else {
			error_code = hook->function(book, appendix, container,
			    EB_HOOK_NARROW_JISX0208, 0, argv);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    }
		} else {
		    hook = hookset->hooks + EB_HOOK_WIDE_JISX0208;
		    if (forward_only) {
			; /* do nothing */
		    } else if (hook->function == NULL) {
			error_code = eb_write_text_byte2(book, c1 | 0x80,
			    c2 | 0x80);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    } else {
			error_code = hook->function(book, appendix, container,
			    EB_HOOK_WIDE_JISX0208, argc, argv);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    }
		}
	    } else if (0x20 < c1 && c1 < 0x7f && 0xa0 < c2 && c2 < 0xff) {
		/*
		 * This is a GB 2312 HANJI character.
		 */
		argv[0] = eb_uint2(cache_p) | 0x8000;

		if (context->is_candidate
		    && candidate_length < EB_MAX_WORD_LENGTH - 1) {
		    *candidate_p++ = c1 | 0x80;
		    *candidate_p++ = c2;
		    *candidate_p   = '\0';
		    candidate_length += 2;
		}

		hook = hookset->hooks + EB_HOOK_GB2312;
		if (forward_only) {
		    ; /* do nothing */
		} else if (hook->function == NULL) {
		    error_code = eb_write_text_byte2(book, c1 | 0x80, c2);
		    if (error_code != EB_SUCCESS)
			goto failed;
		} else {
		    error_code = hook->function(book, appendix, container,
			EB_HOOK_GB2312, 0, argv);
		    if (error_code != EB_SUCCESS)
			goto failed;
		}
	    } else if (0xa0 < c1 && c1 < 0xff && 0x20 < c2 && c2 < 0x7f) {
		/*
		 * This is a local character.
		 */
		argv[0] = eb_uint2(cache_p);

		if (context->narrow_flag) {
		    hook = hookset->hooks + EB_HOOK_NARROW_FONT;
		    if (forward_only) {
			; /* do nothing */
		    } else if (hook->function == NULL) {
			error_code = eb_write_text_byte2(book, c1, c2);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    } else {
			error_code = hook->function(book, appendix, container,
			    EB_HOOK_NARROW_FONT, argc, argv);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    }
		} else {
		    hook = hookset->hooks + EB_HOOK_WIDE_FONT;
		    if (forward_only) {
			; /* do nothing */
		    } else if (hook->function == NULL) {
			error_code = eb_write_text_byte2(book, c1, c2);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    } else {
			error_code = hook->function(book, appendix, container,
			    EB_HOOK_WIDE_FONT, argc, argv);
			if (error_code != EB_SUCCESS)
			    goto failed;
		    }
		}
	    }
	}

	/*
	 * Update variables.
	 */
	cache_p += in_step;
	cache_rest_length -= in_step;
	context->location += in_step;
	in_step = 0;

	/*
	 * Break if an unprocessed character is remained.
	 */
	if (context->unprocessed != NULL)
	    break;
	/*
	 * Break if EB_TEXT_STATUS_SOFT_STOP is set.
	 */
	if (context->text_status == EB_TEXT_STATUS_SOFT_STOP)
	  break;
    }

  succeeded:
    if (!forward_only) {
	*text_length = (context->out - text);
	*(context->out) = '\0';
    }

    LOG(("out: eb_read_text_internal(text_length=%ld) = %s",
	(text_length == NULL) ? 0L : (long)*text_length,
	eb_error_string(EB_SUCCESS)));
    pthread_mutex_unlock(&cache_mutex);

    return EB_SUCCESS;

    /*
     * An error occurs...
     * Discard cache if read error occurs.
     */
  failed:
    if (!forward_only) {
	*text_length = -1;
	*text = '\0';
    }
    if (error_code == EB_ERR_FAIL_READ_TEXT)
	cache_book_code = EB_BOOK_NONE;
    LOG(("out: eb_read_text_internal() = %s", eb_error_string(error_code)));
    pthread_mutex_unlock(&cache_mutex);
    return error_code;
}


/*
 * Check whether an escape sequence is stop-code or not.
 */
static int
eb_is_stop_code(EB_Book *book, EB_Appendix *appendix, unsigned int code0,
    unsigned int code1)
{
    int result;

    if (appendix == NULL
	|| appendix->subbook_current == NULL
	|| appendix->subbook_current->stop_code0 == 0) {
	result = (code0 == 0x1f41
	    && code1 == book->text_context.auto_stop_code);
    } else {
	result = (code0 == appendix->subbook_current->stop_code0
	    && code1 == appendix->subbook_current->stop_code1);
    }

    return result;
}


/*
 * Have the current text context reached the end of text?
 */
int
eb_is_text_stopped(EB_Book *book)
{
    int is_stopped = 0;

    eb_lock(book);
    LOG(("in: eb_is_text_stopped(book=%d)", (int)book->code));

    if (book->subbook_current != NULL) {
	if (book->text_context.code == EB_TEXT_HEADING
	    || book->text_context.code == EB_TEXT_MAIN_TEXT
	    || book->text_context.code == EB_TEXT_OPTIONAL_TEXT) {
	    if (book->text_context.text_status != EB_TEXT_STATUS_CONTINUED
		&& book->text_context.unprocessed == NULL) {
		is_stopped = 1;
	    }
	}
    }

    LOG(("out: eb_is_text_stopped() = %d", is_stopped));
    return is_stopped;
}


/*
 * Write a byte to a text buffer.
 */
EB_Error_Code
eb_write_text_byte1(EB_Book *book, int byte1)
{
    EB_Error_Code error_code;
    char stream[1];

    LOG(("in: eb_write_text_byte1(book=%d, byte1=%d)",
	(int)book->code, byte1));

    /*
     * If the text buffer has enough space to write `byte1',
     * save the byte in `book->text_context.unprocessed'.
     */
    if (book->text_context.unprocessed != NULL
	|| book->text_context.out_rest_length < 1) {
	*(unsigned char *)stream = byte1;
	error_code = eb_write_text(book, stream, 1);
	if (error_code != EB_SUCCESS)
	    goto failed;
    } else {
	*(book->text_context.out) = byte1;
	book->text_context.out++;
	book->text_context.out_rest_length--;
	book->text_context.out_step++;
    }

    LOG(("out: eb_write_text_byte1() = %s", eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_write_text_byte1() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Write two bytes to a text buffer for output.
 */
EB_Error_Code
eb_write_text_byte2(EB_Book *book, int byte1, int byte2)
{
    EB_Error_Code error_code;
    char stream[2];

    LOG(("in: eb_write_text_byte2(book=%d, byte1=%d, byte2=%d)",
	(int)book->code, byte1, byte2));

    /*
     * If the text buffer has enough space to write `byte1' and `byte2',
     * save the bytes in `book->text_context.unprocessed'.
     */
    if (book->text_context.unprocessed != NULL
	|| book->text_context.out_rest_length < 2) {
	*(unsigned char *)stream = byte1;
	*(unsigned char *)(stream + 1) = byte2;
	error_code = eb_write_text(book, stream, 2);
	if (error_code != EB_SUCCESS)
	    goto failed;
    } else {
	*(book->text_context.out) = byte1;
	book->text_context.out++;
	*(book->text_context.out) = byte2;
	book->text_context.out++;
	book->text_context.out_rest_length -= 2;
	book->text_context.out_step += 2;
    }

    LOG(("out: eb_write_text_byte2() = %s", eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_write_text_byte2() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Write a string to a text buffer.
 */
EB_Error_Code
eb_write_text_string(EB_Book *book, const char *string)
{
    EB_Error_Code error_code;
    size_t string_length;

    LOG(("in: eb_write_text_string(book=%d, string=%s)",
	(int)book->code, eb_quoted_string(string)));

    /*
     * If the text buffer has enough space to write `sting',
     * save the string in `book->text_context.unprocessed'.
     */
    string_length = strlen(string);

    if (book->text_context.unprocessed != NULL
	|| book->text_context.out_rest_length < string_length) {
	error_code = eb_write_text(book, string, string_length);
	if (error_code != EB_SUCCESS)
	    goto failed;
    } else {
	memcpy(book->text_context.out, string, string_length);
	book->text_context.out += string_length;
	book->text_context.out_rest_length -= string_length;
	book->text_context.out_step += string_length;
    }

    LOG(("out: eb_write_text_string() = %s", eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_write_text_string() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Write a stream with `length' bytes to a text buffer.
 */
EB_Error_Code
eb_write_text(EB_Book *book, const char *stream, size_t stream_length)
{
    EB_Error_Code error_code;
    char *reallocated;

    LOG(("in: eb_write_text(book=%d, stream=%s)",
	(int)book->code, eb_quoted_stream(stream, stream_length)));

    /*
     * If the text buffer has enough space to write `stream',
     * save the stream in `book->text_context.unprocessed'.
     */
    if (book->text_context.unprocessed != NULL) {
	reallocated = (char *)realloc(book->text_context.unprocessed,
	    book->text_context.unprocessed_size + stream_length);
	if (reallocated == NULL) {
	    free(book->text_context.unprocessed);
	    book->text_context.unprocessed = NULL;
	    book->text_context.unprocessed_size = 0;
	    error_code = EB_ERR_MEMORY_EXHAUSTED;
	    goto failed;
	}
	memcpy(reallocated + book->text_context.unprocessed_size, stream,
	    stream_length);
	book->text_context.unprocessed = reallocated;
	book->text_context.unprocessed_size += stream_length;

    } else if (book->text_context.out_rest_length < stream_length) {
	book->text_context.unprocessed
	    = (char *)malloc(book->text_context.out_step + stream_length);
	if (book->text_context.unprocessed == NULL) {
	    error_code = EB_ERR_MEMORY_EXHAUSTED;
	    goto failed;
	}
	book->text_context.unprocessed_size
	    = book->text_context.out_step + stream_length;
	memcpy(book->text_context.unprocessed,
	    book->text_context.out - book->text_context.out_step,
	    book->text_context.out_step);
	memcpy(book->text_context.unprocessed + book->text_context.out_step,
	    stream, stream_length);
	book->text_context.out -= book->text_context.out_step;
	book->text_context.out_step = 0;

    } else {
	memcpy(book->text_context.out, stream, stream_length);
	book->text_context.out += stream_length;
	book->text_context.out_rest_length -= stream_length;
	book->text_context.out_step += stream_length;
    }

    LOG(("out: eb_write_text() = %s", eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_write_text() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Get the current candidate word for multi search.
 */
const char *
eb_current_candidate(EB_Book *book)
{
    LOG(("in: eb_current_candidate(book=%d)", (int)book->code));

    if (!book->text_context.is_candidate)
	book->text_context.candidate[0] = '\0';

    LOG(("out: eb_current_candidate() = %s",
	eb_quoted_string(book->text_context.candidate)));

    return book->text_context.candidate;
}


/*
 * Forward text position to the next paragraph.
 */
EB_Error_Code
eb_forward_text(EB_Book *book, EB_Appendix *appendix)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_forward_text(book=%d, appendix=%d)", (int)book->code,
	(appendix != NULL) ? (int)appendix->code : 0));

    /*
     * Current subbook must have been set and START file must exist.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_TEXT;
	goto failed;
    }

    if (book->text_context.code == EB_TEXT_SEEKED) {
	book->text_context.code = EB_TEXT_MAIN_TEXT;
    } else if (book->text_context.code == EB_TEXT_INVALID) {
	error_code = EB_ERR_NO_PREV_SEEK;
	goto failed;
    } else if (book->text_context.code != EB_TEXT_MAIN_TEXT
	&& book->text_context.code != EB_TEXT_OPTIONAL_TEXT) {
	error_code = EB_ERR_DIFF_CONTENT;
	goto failed;
    }

    if (book->text_context.text_status == EB_TEXT_STATUS_SOFT_STOP) {
	book->text_context.text_status = EB_TEXT_STATUS_CONTINUED;
	goto succeeded;
    } else if (book->text_context.text_status == EB_TEXT_STATUS_HARD_STOP) {
	error_code = EB_ERR_END_OF_CONTENT;
	goto failed;
    }

    /*
     * Forward text.
     */
    error_code = eb_read_text_internal(book, appendix, &eb_default_hookset,
	NULL, EB_SIZE_PAGE, NULL, NULL, 1);
    if (error_code != EB_SUCCESS)
	goto failed;

    /*
     * Unlock the book and hookset.
     */
  succeeded:
    eb_reset_text_context(book);
    LOG(("out: eb_forward_text() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    if (error_code != EB_ERR_END_OF_CONTENT)
	eb_invalidate_text_context(book);
    LOG(("out: eb_forward_text() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Forward heading position to the next paragraph.
 * (for keyword search.)
 */
EB_Error_Code
eb_forward_heading(EB_Book *book)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_forward_heading(book=%d)", (int)book->code));

    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_TEXT;
	goto failed;
    }

    if (book->text_context.code == EB_TEXT_SEEKED) {
	book->text_context.code = EB_TEXT_HEADING;
    } else if (book->text_context.code == EB_TEXT_INVALID) {
	error_code = EB_ERR_NO_PREV_SEEK;
	goto failed;
    } else if (book->text_context.code != EB_TEXT_HEADING) {
	error_code = EB_ERR_DIFF_CONTENT;
	goto failed;
    }

    if (book->text_context.text_status == EB_TEXT_STATUS_SOFT_STOP) {
	book->text_context.text_status = EB_TEXT_STATUS_CONTINUED;
	goto succeeded;
    } else if (book->text_context.text_status == EB_TEXT_STATUS_HARD_STOP) {
	error_code = EB_ERR_END_OF_CONTENT;
	goto failed;
    }

    /*
     * Forward text.
     */
    error_code = eb_read_text_internal(book, NULL, &eb_default_hookset,
	NULL, EB_SIZE_PAGE, NULL, NULL, 1);
    if (error_code != EB_SUCCESS)
	goto failed;

    eb_reset_text_context(book);

    /*
     * Unlock cache data.
     */
  succeeded:
    LOG(("out: eb_forward_heading() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    if (error_code != EB_ERR_END_OF_CONTENT)
	eb_invalidate_text_context(book);
    LOG(("out: eb_forward_heading() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Backward text position to the previous paragraph.
 */
EB_Error_Code
eb_backward_text(EB_Book *book, EB_Appendix *appendix)
{
    EB_Error_Code error_code;
    EB_Text_Context saved_context;
    off_t current_location;
    off_t forward_location;
    off_t read_location;
    off_t backward_location = -1;
    char text_buffer[EB_SIZE_PAGE];
    char *text_buffer_p;
    ssize_t read_result;
    int stop_code0, stop_code1;

    eb_lock(&book->lock);
    LOG(("in: eb_backward_text(book=%d, appendix=%d)", (int)book->code,
	(appendix != NULL) ? (int)appendix->code : 0));

    /*
     * Current subbook must have been set and START file must exist.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_TEXT;
	goto failed;
    }

    if (book->text_context.code == EB_TEXT_SEEKED) {
	book->text_context.code = EB_TEXT_MAIN_TEXT;
    } else if (book->text_context.code == EB_TEXT_INVALID) {
	error_code = EB_ERR_NO_PREV_SEEK;
	goto failed;
    } else if (book->text_context.code != EB_TEXT_MAIN_TEXT
	&& book->text_context.code != EB_TEXT_OPTIONAL_TEXT) {
	error_code = EB_ERR_DIFF_CONTENT;
	goto failed;
    }

    /*
     * Forward text to get auto-stop-code and location where the current
     * text stops.
     */
    if (book->text_context.text_status != EB_TEXT_STATUS_CONTINUED) {
	forward_location = book->text_context.location;
    } else {
	memcpy(&saved_context, &book->text_context, sizeof(EB_Text_Context));
	error_code = eb_read_text_internal(book, NULL, &eb_default_hookset,
	    NULL, EB_SIZE_PAGE, NULL, NULL, 1);
	if (error_code != EB_SUCCESS && error_code != EB_ERR_END_OF_CONTENT)
	    goto failed;
	forward_location = book->text_context.location;
	saved_context.auto_stop_code = book->text_context.auto_stop_code;
	memcpy(&book->text_context, &saved_context, sizeof(EB_Text_Context));
    }

    /*
     * Determine stop-code.
     */
    if (appendix == NULL
	|| appendix->subbook_current == NULL
	|| appendix->subbook_current->stop_code0 == 0) {
	stop_code0 = 0x1f41;
	stop_code1 = book->text_context.auto_stop_code;
    } else {
	stop_code0 = appendix->subbook_current->stop_code0;
	stop_code1 = appendix->subbook_current->stop_code1;
    }

    /*
     * If the text locator has pointed to `0x1f02' (beginning of text),
     * we cannot backward.
     */
    if (zio_lseek(&book->subbook_current->text_zio,
	book->text_context.location, SEEK_SET) == -1) {
	error_code = EB_ERR_FAIL_SEEK_TEXT;
	goto failed;
    }
    if (zio_read(&book->subbook_current->text_zio, text_buffer, 2) != 2) {
	error_code = EB_ERR_FAIL_READ_TEXT;
	goto failed;
    }
    if (eb_uint2(text_buffer) == 0x1f02) {
	error_code = EB_ERR_END_OF_CONTENT;
	goto failed;
    }

    /*
     * Backward text.
     */
    current_location = book->text_context.location;

    while (0 < book->text_context.location) {
	size_t backward_distance;
	int i;

	/*
	 * Seek and read text.
	 *
	 * Since a stop code occupies 4 bytes and we start scanning
	 * stop-code at preceding byte of the current location, we read
	 * text in front of the current location and following 3 bytes.
	 *
	 *                  start scanning
	 *                  |    current location
	 *                  |    |
	 *    [..] [..] [..] [1F] [41] [00] [01]
	 *                   ===================
	 *                    may be stop-code
	 */
	if (book->text_context.location < EB_SIZE_PAGE + 3)
	    read_location = 0;
	else
	    read_location = book->text_context.location - EB_SIZE_PAGE + 3;
	backward_distance = book->text_context.location - read_location;

	if (zio_lseek(&book->subbook_current->text_zio, read_location,
	    SEEK_SET) == -1) {
	    error_code = EB_ERR_FAIL_SEEK_TEXT;
	    goto failed;
	}

	memset(text_buffer, 0x00, EB_SIZE_PAGE);
	read_result = zio_read(&book->subbook_current->text_zio, text_buffer,
	    EB_SIZE_PAGE);
	if (read_result < 0 || read_result < backward_distance) {
	    error_code = EB_ERR_FAIL_READ_TEXT;
	    goto failed;
	}

	/*
	 * Scan stop-code.
	 */
	text_buffer_p = text_buffer + backward_distance - 1;
	i = backward_distance - 1;

	while (0 <= i) {
	    if (eb_uint2(text_buffer_p) == 0x1f02) {
		book->text_context.location = read_location + i;
		if (current_location <= book->text_context.location + 2) {
		    error_code = EB_ERR_END_OF_CONTENT;
		    goto failed;
		}
		backward_location = book->text_context.location;
		goto loop_end;
	    }
	    if (book->text_context.code != EB_TEXT_MAIN_TEXT
		|| eb_uint2(text_buffer_p) != stop_code0
		|| eb_uint2(text_buffer_p + 2) != stop_code1) {
		text_buffer_p--;
		i--;
		continue;
	    }

	    eb_reset_text_context(book);
	    book->text_context.location = read_location + i;
	    error_code = eb_read_text_internal(book, appendix,
		&eb_default_hookset, NULL, EB_SIZE_PAGE, NULL, NULL, 1);
	    if (error_code != EB_SUCCESS
		&& error_code != EB_ERR_END_OF_CONTENT)
		goto failed;

	    if (book->text_context.location >= current_location - 4
		&& book->text_context.location <= current_location + 4
		&& backward_location < 0)
		forward_location = current_location;
	    if (book->text_context.location >= forward_location - 4
		&& book->text_context.location <= forward_location + 4)
		backward_location = read_location + i;
	    else if (book->text_context.location < forward_location)
		goto loop_end;

	    text_buffer_p--;
	    i--;
	}

	book->text_context.location = read_location - 1;
    }

  loop_end:
    if (backward_location < 0) {
	error_code = EB_ERR_UNEXP_TEXT;
	goto failed;
    }

    eb_reset_text_context(book);

    /*
     * Unlock the book and hookset.
     */
    LOG(("out: eb_forward_text() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_invalidate_text_context(book);
    LOG(("out: eb_backward_text() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


