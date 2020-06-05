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
#include "build-post.h"

/*
 * Compare `word' and `pattern'.
 * `word' must be terminated by `\0' and `pattern' is assumed to be
 * `length' characters long.
 *
 * When `word' is equal to `pattern', or equal to the beginning of
 * `pattern', 0 is returned.  A positive or negateive integer is
 * returned according as `pattern' is greater or less than `word'.
 */
int
eb_match_word(const char *word, const char *pattern, size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    int result;

    LOG(("in: eb_match_word(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = *word_p;
	    break;
	}
	if (*word_p == '\0') {
	    result = 0;
	    break;
	}

	if (*word_p != *pattern_p) {
	    result = *word_p - *pattern_p;
	    break;
	}

	word_p++;
	pattern_p++;
	i++;
    }

    LOG(("out: eb_match_word() = %d", result));
    return result;
}


/*
 * Compare `word' and `pattern' for pre-search.
 * `word' must be terminated by `\0' and `pattern' is assumed to be
 * `length' characters long.
 *
 * When `word' is equal to `pattern', or equal to the beginning of
 * `pattern', 0 is returned.  A positive or negateive integer is
 * returned according as `pattern' is greater or less than `word'.
 */
int
eb_pre_match_word(const char *word, const char *pattern, size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    int result;

    LOG(("in: eb_pre_match_word(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = 0;
	    break;
	}
	if (*word_p == '\0') {
	    result = 0;
	    break;
	}

	if (*word_p != *pattern_p) {
	    result = *word_p - *pattern_p;
	    break;
	}

	word_p++;
	pattern_p++;
	i++;
    }

    LOG(("out: eb_pre_match_word() = %d", result));
    return result;
}


/*
 * Compare `word' and `pattern' in JIS X 0208.
 * `word' must be terminated by `\0' and `pattern' is assumed to be
 * `length' characters long.
 *
 * When the word is equal to the pattern, 0 is returned.  A positive or
 * negateive integer is returned according as `pattern' is greater or
 * less than `word'.
 */
int
eb_exact_match_word_jis(const char *word, const char *pattern, size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    int result;

    LOG(("in: eb_exact_match_word_jis(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = *word_p;
	    break;
	}
	if (*word_p == '\0') {
	    /* ignore spaces in the tail of the pattern */
	    while (i < length && *pattern_p == '\0') {
		pattern_p++;
		i++;
	    }
	    result = (i - length);
	    break;
	}
	if (*word_p != *pattern_p) {
	    result = *word_p - *pattern_p;
	    break;
	}

	word_p++;
	pattern_p++;
	i++;
    }

    LOG(("out: eb_exact_match_word_jis() = %d", result));
    return result;
}


/*
 * Compare `word' and `pattern' in JIS X 0208 for pre-search.
 * `word' must be terminated by `\0' and `pattern' is assumed to be
 * `length' characters long.
 *
 * When the word is equal to the pattern, 0 is returned.  A positive or
 * negateive integer is returned according as `pattern' is greater or
 * less than `word'.
 */
int
eb_exact_pre_match_word_jis(const char *word, const char *pattern,
    size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    int result;

    LOG(("in: eb_exact_pre_match_word_jis(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = 0;
	    break;
	}
	if (*word_p == '\0') {
	    /* ignore spaces in the tail of the pattern */
	    while (i < length && *pattern_p == '\0') {
		pattern_p++;
		i++;
	    }
	    result = (i - length);
	    break;
	}
	if (*word_p != *pattern_p) {
	    result = *word_p - *pattern_p;
	    break;
	}

	word_p++;
	pattern_p++;
	i++;
    }

    LOG(("out: eb_exact_pre_match_word_jis() = %d", result));
    return result;
}


/*
 * Compare `word' and `pattern' in Latin1.
 * `word' must be terminated by `\0' and `pattern' is assumed to be
 * `length' characters long.
 *
 * When the word is equal to the pattern, 0 is returned.  A positive or
 * negateive integer is returned according as `pattern' is greater or
 * less than `word'.
 */
int
eb_exact_match_word_latin(const char *word, const char *pattern, size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    int result;

    LOG(("in: eb_exact_match_word_latin(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = *word_p;
	    break;
	}
	if (*word_p == '\0') {
	    /* ignore spaces in the tail of the pattern */
	    while (i < length && (*pattern_p == ' ' || *pattern_p == '\0')) {
		pattern_p++;
		i++;
	    }
	    result = (i - length);
	    break;
	}
	if (*word_p != *pattern_p) {
	    result = *word_p - *pattern_p;
	    break;
	}

	word_p++;
	pattern_p++;
	i++;
    }

    LOG(("out: eb_exact_match_word_latin() = %d", result));
    return result;
}


/*
 * Compare `word' and `pattern' in Latin1 for pre-search.
 * `word' must be terminated by `\0' and `pattern' is assumed to be
 * `length' characters long.
 *
 * When the word is equal to the pattern, 0 is returned.  A positive or
 * negateive integer is returned according as `pattern' is greater or
 * less than `word'.
 */
int
eb_exact_pre_match_word_latin(const char *word, const char *pattern,
    size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    int result;

    LOG(("in: eb_exact_pre_match_word_latin(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = 0;
	    break;
	}
	if (*word_p == '\0') {
	    /* ignore spaces in the tail of the pattern */
	    while (i < length && (*pattern_p == ' ' || *pattern_p == '\0')) {
		pattern_p++;
		i++;
	    }
	    result = (i - length);
	    break;
	}
	if (*word_p != *pattern_p) {
	    result = *word_p - *pattern_p;
	    break;
	}

	word_p++;
	pattern_p++;
	i++;
    }

    LOG(("out: eb_exact_pre_match_word_latin() = %d", result));
    return result;
}


/*
 * Compare `word' and `pattern' in JIS X 0208.
 *
 * This function is equivalent to eb_match_word() except that this function
 * ignores differences of kana (katakana and hiragana).  The order of
 * hiragana and katakana characters is:
 *
 * If `word' and `pattern' differ, the function compares their characters
 * with the following rule:
 *
 *    HIRAGANA `KA' < HIRAGANA `GA' < KATAKANA `KA' < KATAKANA `GA'
 */
int
eb_match_word_kana_group(const char *word, const char *pattern, size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    unsigned char wc0, wc1, pc0, pc1;
    int result;

    LOG(("in: eb_match_word_kana_group(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = *word_p;
	    break;
	}
	if (*word_p == '\0') {
	    result = 0;
	    break;
	}
	if (length <= i + 1 || *(word_p + 1) == '\0') {
	    result = *word_p - *pattern_p;
	    break;
	}

	wc0 = *word_p;
	wc1 = *(word_p + 1);
	pc0 = *pattern_p;
	pc1 = *(pattern_p + 1);

	if ((wc0 == 0x24 || wc0 == 0x25) && (pc0 == 0x24 || pc0 == 0x25)) {
	    if (wc1 != pc1) {
		result = ((wc0 << 8) + wc1) - ((pc0 << 8) + pc1);
		break;
	    }
	} else {
	    if (wc0 != pc0 || wc1 != pc1) {
		result = ((wc0 << 8) + wc1) - ((pc0 << 8) + pc1);
		break;
	    }
	}
	word_p += 2;
	pattern_p += 2;
	i += 2;
    }

    LOG(("out: eb_match_word_kana_group() = %d", result));
    return result;
}


/*
 * Compare `word' and `pattern' in JIS X 0208.
 *
 * This function is equivalent to eb_match_word() except that this function
 * ignores differences of kana (katakana and hiragana).  The order of
 * hiragana and katakana characters is:
 *
 * If `word' and `pattern' differ, the function compares their characters
 * with the following rule:
 *
 *    HIRAGANA `KA' == KATAKANA `KA' < HIRAGANA `GA' == KATAKANA `GA'.
 */
int
eb_match_word_kana_single(const char *word, const char *pattern, size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    unsigned char wc0, wc1, pc0, pc1;
    int result;

    LOG(("in: eb_match_word_kana_single(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = *word_p;
	    break;
	}
	if (*word_p == '\0') {
	    result = 0;
	    break;
	}
	if (length <= i + 1 || *(word_p + 1) == '\0') {
	    result = *word_p - *pattern_p;
	    break;
	}

	wc0 = *word_p;
	wc1 = *(word_p + 1);
	pc0 = *pattern_p;
	pc1 = *(pattern_p + 1);

	if ((wc0 == 0x24 || wc0 == 0x25) && (pc0 == 0x24 || pc0 == 0x25)) {
	    if (wc1 != pc1) {
		result = wc1 - pc1;
		break;
	    }
	} else {
	    if (wc0 != pc0 || wc1 != pc1) {
		result = ((wc0 << 8) + wc1) - ((pc0 << 8) + pc1);
		break;
	    }
	}
	word_p += 2;
	pattern_p += 2;
	i += 2;
    }

    LOG(("out: eb_match_word_kana_single() = %d", result));
    return result;
}


/*
 * Compare `word' and `pattern' in JIS X 0208.
 *
 * This function is equivalent to eb_exact_match_word_jis() except that
 * this function ignores differences of kana (katakana and hiragana).
 *
 * If `word' and `pattern' differ, the function compares their characters
 * with the following rule:
 *
 *    HIRAGANA `KA' < HIRAGANA `GA' < KATAKANA `KA' < KATAKANA `GA'
 */
int
eb_exact_match_word_kana_group(const char *word, const char *pattern,
    size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    unsigned char wc0, wc1, pc0, pc1;
    int result;

    LOG(("in: eb_exact_match_word_kana_group(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = *word_p;
	    break;
	}
	if (*word_p == '\0') {
	    result = - *pattern_p;
	    break;
	}
	if (length <= i + 1 || *(word_p + 1) == '\0') {
	    result = *word_p - *pattern_p;
	    break;
	}
	wc0 = *word_p;
	wc1 = *(word_p + 1);
	pc0 = *pattern_p;
	pc1 = *(pattern_p + 1);

	if ((wc0 == 0x24 || wc0 == 0x25) && (pc0 == 0x24 || pc0 == 0x25)) {
	    if (wc1 != pc1) {
		result = ((wc0 << 8) + wc1) - ((pc0 << 8) + pc1);
		break;
	    }
	} else {
	    if (wc0 != pc0 || wc1 != pc1) {
		result = ((wc0 << 8) + wc1) - ((pc0 << 8) + pc1);
		break;
	    }
	}
	word_p += 2;
	pattern_p += 2;
	i += 2;
    }

    LOG(("out: eb_exact_match_word_kana_group() = %d", result));
    return result;
}


/*
 * Compare `word' and `pattern' in JIS X 0208.
 *
 * This function is equivalent to eb_exact_match_word_jis() except that
 * this function ignores differences of kana (katakana and hiragana).
 * The order of hiragana and katakana characters is:
 *
 * If `word' and `pattern' differ, the function compares their characters
 * with the following rule:
 *
 *    HIRAGANA `KA' == KATAKANA `KA' < HIRAGANA `GA' == KATAKANA `GA'.
 */
int
eb_exact_match_word_kana_single(const char *word, const char *pattern,
    size_t length)
{
    int i = 0;
    unsigned char *word_p = (unsigned char *)word;
    unsigned char *pattern_p = (unsigned char *)pattern;
    unsigned char wc0, wc1, pc0, pc1;
    int result;

    LOG(("in: eb_exact_match_word_kana_single(word=%s, pattern=%s)",
	eb_quoted_stream(word, EB_MAX_WORD_LENGTH),
	eb_quoted_stream(pattern, length)));

    for (;;) {
	if (length <= i) {
	    result = *word_p;
	    break;
	}
	if (*word_p == '\0') {
	    result = - *pattern_p;
	    break;
	}
	if (length <= i + 1 || *(word_p + 1) == '\0') {
	    result = *word_p - *pattern_p;
	    break;
	}
	wc0 = *word_p;
	wc1 = *(word_p + 1);
	pc0 = *pattern_p;
	pc1 = *(pattern_p + 1);

	if ((wc0 == 0x24 || wc0 == 0x25) && (pc0 == 0x24 || pc0 == 0x25)) {
	    if (wc1 != pc1) {
		result = wc1 - pc1;
		break;
	    }
	} else {
	    if (wc0 != pc0 || wc1 != pc1) {
		result = ((wc0 << 8) + wc1) - ((pc0 << 8) + pc1);
		break;
	    }
	}
	word_p += 2;
	pattern_p += 2;
	i += 2;
    }

    LOG(("out: eb_exact_match_word_kana_single() = %d", result));
    return result;
}


