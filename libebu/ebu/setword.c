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
 * Unexported functions.
 */
static void eb_fix_word(EB_Book *book, const EB_Search *search, char *word,
    char *canonicalized_word);
static EB_Error_Code eb_convert_latin(EB_Book *book, const char *input_word,
    char *word, EB_Word_Code *word_code);
static EB_Error_Code eb_convert_utf8(EB_Book *book, const char *input_word,
    char *word, EB_Word_Code *word_code);
static EB_Error_Code eb_convert_euc_jp(EB_Book *book, const char *input_word,
    char *word, EB_Word_Code *word_code);
static void eb_convert_katakana_jis(char *word);
static void eb_convert_hiragana_jis(char *word);
static void eb_convert_lower_latin(char *word);
static void eb_convert_lower_jis(char *word);
static void eb_delete_marks_jis(char *word);
static void eb_convert_long_vowels_jis(char *word);
static void eb_delete_long_vowels_jis(char *word);
static void eb_convert_double_consonants_jis(char *word);
static void eb_convert_contracted_sounds_jis(char *word);
static void eb_convert_small_vowels_jis(char *word);
static void eb_convert_voiced_consonants_jis(char *word);
static void eb_convert_p_sounds_jis(char *word);
static void eb_delete_spaces_latin(char *word);
static void eb_delete_spaces_jis(char *word);
static void eb_reverse_word_latin(char *word);
static void eb_reverse_word_utf8(char *word);
static void eb_reverse_word_jis(char *word);


/*
 * Make a fixed word and a cannonicalized word for `WORD SEARCH'.
 *
 * If `inputword' is a KANA word,  EB_WORD_KANA is returned.
 * If `inputword' is a alphabetic word, EB_WORD_ALPHABET is returned.
 * Otherwise, -1 is returned.  It means that an error occurs.
 */
EB_Error_Code
eb_set_word(EB_Book *book, const char *input_word, char *word,
    char *canonicalized_word, EB_Word_Code *word_code)
{
    EB_Error_Code error_code;
    const EB_Search *search;

    LOG(("in: eb_set_word(book=%d, input_word=%s)", (int)book->code,
	eb_quoted_string(input_word)));

    /*
     * Make a fixed word and a canonicalized word from `input_word'.
     */
    if (book->character_code == EB_CHARCODE_ISO8859_1)
	error_code = eb_convert_latin(book, input_word, word, word_code);
    else if (book->character_code == EB_CHARCODE_UTF8)
	error_code = eb_convert_utf8(book, input_word, word, word_code);
    else
	error_code = eb_convert_euc_jp(book, input_word, word, word_code);
    if (error_code != EB_SUCCESS)
	goto failed;
    strcpy(canonicalized_word, word);

    /*
     * Determine search method.
     */
    switch (*word_code) {
    case EB_WORD_ALPHABET:
	if (book->subbook_current->word_alphabet.start_page != 0)
	    search = &book->subbook_current->word_alphabet;
	else if (book->subbook_current->word_asis.start_page != 0)
	    search = &book->subbook_current->word_asis;
	else {
	    error_code = EB_ERR_NO_SUCH_SEARCH;
	    goto failed;
	}
	break;

    case EB_WORD_KANA:
	if (book->subbook_current->word_kana.start_page != 0)
	    search = &book->subbook_current->word_kana;
	else if (book->subbook_current->word_asis.start_page != 0)
	    search = &book->subbook_current->word_asis;
	else {
	    error_code = EB_ERR_NO_SUCH_SEARCH;
	    goto failed;
	}
	break;

    case EB_WORD_OTHER:
	if (book->subbook_current->word_asis.start_page != 0)
	    search = &book->subbook_current->word_asis;
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
     * Fix the word.
     */
    eb_fix_word(book, search, word, canonicalized_word);

    LOG(("out: eb_set_word(word=%s, canonicalized_word=%s, word_code=%d) = %s",
	eb_quoted_string(word), eb_quoted_string(canonicalized_word),
	(int)*word_code, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *word = '\0';
    *canonicalized_word = '\0';
    *word_code = EB_WORD_INVALID;
    LOG(("out: eb_set_word() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Make a fixed word and a cannonicalized word for `ENDWORD SEARCH'.
 *
 * If `input_word' is a KANA word,  EB_WORD_KANA is retuend.
 * If `input_word' is a alphabetic word,  EB_WORD_ALPHABET is retuend.
 * Otherwise, -1 is returned.  It means that an error occurs.
 */
EB_Error_Code
eb_set_endword(EB_Book *book, const char *input_word, char *word,
    char *canonicalized_word, EB_Word_Code *word_code)
{
    EB_Error_Code error_code;
    const EB_Search *search;

    LOG(("in: eb_set_endword(book=%d, input_word=%s)", (int)book->code,
	eb_quoted_string(input_word)));

    /*
     * Make a fixed word and a canonicalized word from `input_word'.
     */
    if (book->character_code == EB_CHARCODE_ISO8859_1)
	error_code = eb_convert_latin(book, input_word, word, word_code);
    else if (book->character_code == EB_CHARCODE_UTF8)
	error_code = eb_convert_utf8(book, input_word, word, word_code);
    else
	error_code = eb_convert_euc_jp(book, input_word, word, word_code);
    if (error_code != EB_SUCCESS)
	goto failed;
    strcpy(canonicalized_word, word);

    /*
     * Determine search method.
     */
    switch (*word_code) {
    case EB_WORD_ALPHABET:
	if (book->subbook_current->endword_alphabet.start_page != 0)
	    search = &book->subbook_current->endword_alphabet;
	else if (book->subbook_current->endword_asis.start_page != 0)
	    search = &book->subbook_current->endword_asis;
	else {
	    error_code = EB_ERR_NO_SUCH_SEARCH;
	    goto failed;
	}
	break;

    case EB_WORD_KANA:
	if (book->subbook_current->endword_kana.start_page != 0)
	    search = &book->subbook_current->endword_kana;
	else if (book->subbook_current->endword_asis.start_page != 0)
	    search = &book->subbook_current->endword_asis;
	else {
	    error_code = EB_ERR_NO_SUCH_SEARCH;
	    goto failed;
	}
	break;

    case EB_WORD_OTHER:
	if (book->subbook_current->endword_asis.start_page != 0)
	    search = &book->subbook_current->endword_asis;
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
     * Fix the word.
     */
    eb_fix_word(book, search, word, canonicalized_word);

    /*
     * Reverse the word.
     */
    if (book->character_code == EB_CHARCODE_ISO8859_1) {
	eb_reverse_word_latin(word);
	eb_reverse_word_latin(canonicalized_word);
    } else if (book->character_code == EB_CHARCODE_UTF8) {
	eb_reverse_word_utf8(word);
	eb_reverse_word_utf8(canonicalized_word);
    } else {
	eb_reverse_word_jis(word);
	eb_reverse_word_jis(canonicalized_word);
    }

    LOG(("out: eb_set_endword(word=%s, canonicalized_word=%s, word_code=%d) \
= %s",
	eb_quoted_string(word), eb_quoted_string(canonicalized_word),
	(int)*word_code, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *word = '\0';
    *canonicalized_word = '\0';
    *word_code = EB_WORD_INVALID;
    LOG(("out: eb_set_endword() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Make a fixed word and a cannonicalized word for `KEYWORD SEARCH'
 * or `CROSS SEARCH'.
 *
 * If `inputword' is a KANA word,  EB_WORD_KANA is returned.
 * If `inputword' is a alphabetic word, EB_WORD_ALPHABET is returned.
 * Otherwise, -1 is returned.  It means that an error occurs.
 */
EB_Error_Code
eb_set_keyword(EB_Book *book, const char *input_word, char *word,
    char *canonicalized_word, EB_Word_Code *word_code)
{
    EB_Error_Code error_code;

    LOG(("in: eb_set_keyword(book=%d, input_word=%s)", (int)book->code,
	eb_quoted_string(input_word)));

    /*
     * Make a fixed word and a canonicalized word from `input_word'.
     */
    if (book->character_code == EB_CHARCODE_ISO8859_1)
	error_code = eb_convert_latin(book, input_word, word, word_code);
    else if (book->character_code == EB_CHARCODE_UTF8)
	error_code = eb_convert_utf8(book, input_word, word, word_code);
    else
	error_code = eb_convert_euc_jp(book, input_word, word, word_code);
    if (error_code != EB_SUCCESS)
	goto failed;
    strcpy(canonicalized_word, word);

    /*
     * Fix the word.
     */
    eb_fix_word(book, &book->subbook_current->keyword, word,
	canonicalized_word);

    LOG(("out: eb_set_keyword(word=%s, canonicalized_word=%s, word_code=%d) \
= %s",
	eb_quoted_string(word), eb_quoted_string(canonicalized_word),
	(int)*word_code, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *word = '\0';
    *canonicalized_word = '\0';
    *word_code = EB_WORD_INVALID;
    LOG(("out: eb_set_keyword() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Make a fixed word and a cannonicalized word for `MULTI SEARCH'.
 *
 * If `inputword' is a KANA word,  EB_WORD_KANA is returned.
 * If `inputword' is a alphabetic word, EB_WORD_ALPHABET is returned.
 * Otherwise, -1 is returned.  It means that an error occurs.
 */
EB_Error_Code
eb_set_multiword(EB_Book *book, EB_Multi_Search_Code multi_id,
    EB_Multi_Entry_Code entry_id, const char *input_word, char *word,
    char *canonicalized_word, EB_Word_Code *word_code)
{
    EB_Error_Code error_code;
    EB_Search *search;

    LOG(("in: eb_set_multiword(book=%d, input_word=%s)", (int)book->code,
	eb_quoted_string(input_word)));

    /*
     * Make a fixed word and a canonicalized word from `input_word'.
     */
    if (book->character_code == EB_CHARCODE_ISO8859_1)
	error_code = eb_convert_latin(book, input_word, word, word_code);
    else if (book->character_code == EB_CHARCODE_UTF8)
	error_code = eb_convert_utf8(book, input_word, word, word_code);
    else
	error_code = eb_convert_euc_jp(book, input_word, word, word_code);
    if (error_code != EB_SUCCESS)
	goto failed;
    strcpy(canonicalized_word, word);

    /*
     * Fix the word.
     */
    search = &book->subbook_current->multis[multi_id].entries[entry_id];
    eb_fix_word(book, search, word, canonicalized_word);

    LOG(("out: eb_set_multiword(word=%s, canonicalized_word=%s, word_code=%d) \
= %s",
	eb_quoted_string(word), eb_quoted_string(canonicalized_word),
	(int)*word_code, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *word = '\0';
    *canonicalized_word = '\0';
    *word_code = EB_WORD_INVALID;
    LOG(("out: eb_set_multiword() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Fix `canonicalized_word' and `word' according with `book->character_code'
 * and `search'.
 */
static void
eb_fix_word(EB_Book *book, const EB_Search *search, char *word,
    char *canonicalized_word)
{
    LOG(("in: eb_fix_word(book=%d, word=%s, canonicalized_word=%s)",
	(int)book->code, eb_quoted_string(word),
	eb_quoted_string(canonicalized_word)));

    if (search->index_id == 0xa1 && search->candidates_page != 0)
	return;

    if (book->character_code == EB_CHARCODE_UTF8) {
	return;

    } else if (book->character_code == EB_CHARCODE_ISO8859_1) {
	if (search->space == EB_INDEX_STYLE_DELETE)
	    eb_delete_spaces_latin(canonicalized_word);

	if (search->lower == EB_INDEX_STYLE_CONVERT)
	    eb_convert_lower_latin(canonicalized_word);

    } else {
	if (search->space == EB_INDEX_STYLE_DELETE)
	    eb_delete_spaces_jis(canonicalized_word);

	if (search->katakana == EB_INDEX_STYLE_CONVERT)
	    eb_convert_katakana_jis(canonicalized_word);
	else if (search->katakana == EB_INDEX_STYLE_REVERSED_CONVERT)
	    eb_convert_hiragana_jis(canonicalized_word);

	if (search->lower == EB_INDEX_STYLE_CONVERT)
	    eb_convert_lower_jis(canonicalized_word);

	if (search->mark == EB_INDEX_STYLE_DELETE)
	    eb_delete_marks_jis(canonicalized_word);

	if (search->long_vowel == EB_INDEX_STYLE_CONVERT)
	    eb_convert_long_vowels_jis(canonicalized_word);
	else if (search->long_vowel == EB_INDEX_STYLE_DELETE)
	    eb_delete_long_vowels_jis(canonicalized_word);

	if (search->double_consonant == EB_INDEX_STYLE_CONVERT)
	    eb_convert_double_consonants_jis(canonicalized_word);

	if (search->contracted_sound == EB_INDEX_STYLE_CONVERT)
	    eb_convert_contracted_sounds_jis(canonicalized_word);

	if (search->small_vowel == EB_INDEX_STYLE_CONVERT)
	    eb_convert_small_vowels_jis(canonicalized_word);

	if (search->voiced_consonant == EB_INDEX_STYLE_CONVERT)
	    eb_convert_voiced_consonants_jis(canonicalized_word);

	if (search->p_sound == EB_INDEX_STYLE_CONVERT)
	    eb_convert_p_sounds_jis(canonicalized_word);
    }

    if (search->index_id != 0x70 && search->index_id != 0x90)
	strcpy(word, canonicalized_word);

    LOG(("out: eb_fix_word(word=%s, canonicalized_word=%s)",
	eb_quoted_string(word), eb_quoted_string(canonicalized_word)));
}


/*
 * Convert `input_word' to ISO 8859 1 and put it into `word'.
 *
 * If `input_word' is a valid string to search, EB_WORD_ALPHABET is returned.
 * Otherwise, -1 is returned.
 */
static EB_Error_Code
eb_convert_latin(EB_Book *book, const char *input_word, char *word,
    EB_Word_Code *word_code)
{
    EB_Error_Code error_code;
    unsigned char *wp = (unsigned char *) word;
    const unsigned char *inp = (const unsigned char *) input_word;
    const unsigned char *tail;
    unsigned char c1;
    int word_length = 0;

    LOG(("in: eb_convert_latin(book=%d, input_word=%s)", (int)book->code,
	eb_quoted_string(input_word)));

    /*
     * Find the tail of `input_word'.
     */
    tail = (const unsigned char *) input_word + strlen(input_word) - 1;
    while ((const unsigned char *)input_word <= tail
	&& (*tail == ' ' || *tail == '\t'))
	tail--;
    tail++;

    /*
     * Ignore spaces and tabs in the beginning of `input_word'.
     */
    while (*inp == ' ' || *inp == '\t')
	inp++;

    while (inp < tail) {
	/*
	 * Check for the length of the word.
	 * If exceeds, return with an error code.
	 */
	if (EB_MAX_WORD_LENGTH < word_length + 1) {
	    error_code = EB_ERR_TOO_LONG_WORD;
	    goto failed;
	}

	c1 = *inp++;

	/*
	 * Tabs are translated to spaces.
	 */
	if (c1 == '\t')
	    c1 = ' ';

	*wp++ = c1;

	/*
	 * Skip successive spaces and tabs.
	 */
	if (c1 == ' ') {
	    while (*inp == '\t' || *inp == ' ')
		inp++;
	}

	word_length++;
    }
    *wp = '\0';

    if (word_length == 0) {
	error_code = EB_ERR_EMPTY_WORD;
	goto failed;
    }
    *word_code = EB_WORD_ALPHABET;

    LOG(("out: eb_convert_latin(word=%s, word_code=%d) = %s",
	eb_quoted_string(word), (int)*word_code, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *word = '\0';
    *word_code = EB_WORD_INVALID;
    LOG(("out: eb_convert_latin() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Table used to convert JIS X 0208 to ASCII.
 */
static const unsigned int jisx0208_table[] = {
    /* 0x20 -- 0x2f */
    0x2121, 0x212a, 0x2149, 0x2174, 0x2170, 0x2173, 0x2175, 0x2147,
    0x214a, 0x214b, 0x2176, 0x215c, 0x2124, 0x215d, 0x2125, 0x213f,
    /* 0x30 -- 0x3f */
    0x2330, 0x2331, 0x2332, 0x2333, 0x2334, 0x2335, 0x2336, 0x2337,
    0x2338, 0x2339, 0x2127, 0x2128, 0x2163, 0x2161, 0x2164, 0x2129,
    /* 0x40 -- 0x4f */
    0x2177, 0x2341, 0x2342, 0x2343, 0x2344, 0x2345, 0x2346, 0x2347,
    0x2348, 0x2349, 0x234a, 0x234b, 0x234c, 0x234d, 0x234e, 0x234f,
    /* 0x50 -- 0x5f */
    0x2350, 0x2351, 0x2352, 0x2353, 0x2354, 0x2355, 0x2356, 0x2357,
    0x2358, 0x2359, 0x235a, 0x214e, 0x2140, 0x214f, 0x2130, 0x2132,
    /* 0x60 -- 0x6f */
    0x2146, 0x2361, 0x2362, 0x2363, 0x2364, 0x2365, 0x2366, 0x2367,
    0x2368, 0x2369, 0x236a, 0x236b, 0x236c, 0x236d, 0x236e, 0x236f,
    /* 0x70 -- 0x7e */
    0x2370, 0x2371, 0x2372, 0x2373, 0x2374, 0x2375, 0x2376, 0x2377,
    0x2378, 0x2379, 0x237a, 0x2150, 0x2143, 0x2151, 0x2141
};

/*
 * Table used to convert JIS X 0201 KATAKANA to JIS X 0208.
 */
static const unsigned int jisx0201_table[] = {
    /* 0xa0 -- 0xaf */
    0x0000, 0x2123, 0x2156, 0x2157, 0x2122, 0x2126, 0x2572, 0x2521,
    0x2523, 0x2525, 0x2527, 0x2529, 0x2563, 0x2565, 0x2567, 0x2543,
    /* 0xb0 -- 0xbf */
    0x213c, 0x2522, 0x2524, 0x2526, 0x2528, 0x252a, 0x252b, 0x252d,
    0x252f, 0x2531, 0x2533, 0x2535, 0x2537, 0x2539, 0x253b, 0x253d,
    /* 0xc0 -- 0xcf */
    0x253f, 0x2541, 0x2544, 0x2546, 0x2548, 0x254a, 0x254b, 0x254c,
    0x254d, 0x254e, 0x254f, 0x2552, 0x2555, 0x2558, 0x255b, 0x255e,
    /* 0xd0 -- 0xdf */
    0x255f, 0x2560, 0x2561, 0x2562, 0x2564, 0x2566, 0x2568, 0x2569,
    0x256a, 0x256b, 0x256c, 0x256d, 0x256f, 0x2573, 0x212b, 0x212c
};

/*
 * Convert `input_word' to JIS X0208 and put it into `word'.
 *
 * If `input_word' is a valid string to search, EB_WORD_ALPHABET or
 * EB_WORD_KANA is returned.
 * Otherwise, -1 is returned.
 */
static EB_Error_Code
eb_convert_euc_jp(EB_Book *book, const char *input_word, char *word,
    EB_Word_Code *word_code)
{
    EB_Error_Code error_code;
    unsigned char *wp = (unsigned char *) word;
    const unsigned char *inp = (const unsigned char *) input_word;
    const unsigned char *tail;
    unsigned char c1 = 0, c2 = 0;
    int kana_count = 0;
    int alphabet_count = 0;
    int kanji_count = 0;
    int word_length = 0;

    LOG(("in: eb_convert_euc_jp(book=%d, input_word=%s)", (int)book->code,
	eb_quoted_string(input_word)));

    /*
     * Find the tail of `input_word'.
     */
    tail = (const unsigned char *) input_word + strlen(input_word) - 1;
    for (;;) {
	if (inp < tail && (*tail == ' ' || *tail == '\t'))
	    tail--;
	else if (inp < tail - 1 && *tail == 0xa1 && *(tail - 1) == 0xa1)
	    tail -= 2;
	else
	    break;
    }
    tail++;

    /*
     * Ignore spaces and tabs in the beginning of `input_word'.
     */
    for (;;) {
	if (*inp == ' ' || *inp == '\t')
	    inp++;
	else if (*inp == 0xa1 && *(inp + 1) == 0xa1)
	    inp += 2;
	else
	    break;
    }

    while (inp < tail) {
	/*
	 * Check for the length of the word.
	 * If exceeds, return with an error code.
	 */
	if (EB_MAX_WORD_LENGTH < word_length + 2) {
	    error_code = EB_ERR_TOO_LONG_WORD;
	    goto failed;
	}

	/*
	 * Tabs are translated to spaces.
	 */
	c1 = *inp++;
	if (c1 == '\t')
	    c1 = ' ';

	if (0x20 <= c1 && c1 <= 0x7e) {
	    /*
	     * `c1' is a character in ASCII.
	     */
	    unsigned int c = jisx0208_table[c1 - 0x20];
	    c1 = c >> 8;
	    c2 = c & 0xff;
	} else if (0xa1 <= c1 && c1 <= 0xfe) {
	    /*
	     * `c1' is a character in JIS X 0208, or local character.
	     */
	    c2 = *inp++;

	    if (0xa1 <= c2 && c2 <= 0xfe) {
		c1 &= 0x7f;
		c2 &= 0x7f;
	    } else if (c2 < 0x20 || 0x7e < c2) {
		error_code = EB_ERR_BAD_WORD;
		goto failed;
	    }
	} else if (c1 == 0x8e) {
	    /*
	     * `c1' is SS2.
	     */
	    if (c2 < 0xa1 || 0xdf < c2) {
		error_code = EB_ERR_BAD_WORD;
		goto failed;
	    }
	    c2 = jisx0201_table[c2 - 0xa0];
	    c1 = 0x25;
	} else {
	    error_code = EB_ERR_BAD_WORD;
	    goto failed;
	}

	/*
	 * The following characters are recognized as alphabet.
	 *   2330 - 2339: `0' .. `9'
	 *   2341 - 235a: `A' .. `Z'
	 *   2361 - 237a: `a' .. `z' (convert to upper cases)
	 */
	*wp++ = c1;
	*wp++ = c2;

	if (c1 == 0x23)
	    alphabet_count++;
	else if (c1 == 0x24 || c1 == 0x25)
	    kana_count++;
	else if (c1 != 0x21)
	    kanji_count++;

	word_length += 2;
    }
    *wp = '\0';

    if (word_length == 0) {
	error_code = EB_ERR_EMPTY_WORD;
	goto failed;
    }
    if (alphabet_count == 0 && kana_count != 0 && kanji_count == 0)
	*word_code = EB_WORD_KANA;
    else if (alphabet_count != 0 && kana_count == 0 && kanji_count == 0)
	*word_code = EB_WORD_ALPHABET;
    else
	*word_code = EB_WORD_OTHER;

    LOG(("out: eb_convert_euc_jp(word=%s, word_code=%d) = %s",
	eb_quoted_string(word), (int)*word_code, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *word = '\0';
    *word_code = EB_WORD_INVALID;
    LOG(("out: eb_convert_euc_jp() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Convert KATAKANA to HIRAGANA in `word'.
 */
static void
eb_convert_katakana_jis(char *word)
{
    unsigned char *wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_convert_katakana_jis(word=%s)", eb_quoted_string(word)));

    while (*wp != '\0' && *(wp + 1) != '\0') {
	c1 = *wp;
	c2 = *(wp + 1);

	if (c1 == 0x25 && 0x21 <= c2 && c2 <= 0x76) {
	    /*
	     * This is a KATAKANA.  Convert to corresponding HIRAGANA.
	     */
	    *wp = 0x24;
	}
	wp += 2;
    }
    *wp = '\0';

    LOG(("out: eb_convert_katakana_jis()"));
}


/*
 * Convert HIRAGANA to KATAKANA in `word'.
 */
static void
eb_convert_hiragana_jis(char *word)
{
    unsigned char *wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_convert_hiragana_jis(word=%s)", eb_quoted_string(word)));

    while (*wp != '\0' && *(wp + 1) != '\0') {
	c1 = *wp;
	c2 = *(wp + 1);

	if (c1 == 0x24 && 0x21 <= c2 && c2 <= 0x76) {
	    /*
	     * This is a HIRAGANA.  Convert to corresponding KATAKANA.
	     */
	    *wp = 0x25;
	}
	wp += 2;
    }
    *wp = '\0';

    LOG(("out: eb_convert_hiragana_jis()"));
}


/*
 * Convert lower case to upper case in `word'.
 */
static void
eb_convert_lower_latin(char *word)
{
    unsigned char *wp = (unsigned char *) word;

    LOG(("in: eb_convert_lower_latin(word=%s)", eb_quoted_string(word)));

    while (*wp != '\0') {
	if (('a' <= *wp && *wp <= 'z')
	    || (0xe0 <= *wp && *wp <= 0xf6) || (0xf8 <= *wp && *wp <= 0xfe)) {
	    /*
	     * This is a lower case letter.  Convert to upper case.
	     */
	    *wp -= 0x20;
	}
	wp++;
    }
    *wp = '\0';

    LOG(("out: eb_convert_lower_latin()"));
}


/*
 * Convert lower case to upper case in `word'.
 */
static void
eb_convert_lower_jis(char *word)
{
    unsigned char *wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_convert_lower_jis(word=%s)", eb_quoted_string(word)));

    while (*wp != '\0' && *(wp + 1) != '\0') {
	c1 = *wp;
	c2 = *(wp + 1);

	if (c1 == 0x23 && 0x61 <= c2 && c2 <= 0x7a) {
	    /*
	     * This is a lower case letter.  Convert to upper case.
	     */
	    *(wp + 1) = c2 - 0x20;
	}
	wp += 2;
    }
    *wp = '\0';

    LOG(("out: eb_convert_lower_jis()"));
}


/*
 * Delete some marks in `word'.
 */
static void
eb_delete_marks_jis(char *word)
{
    unsigned char *in_wp = (unsigned char *) word;
    unsigned char *out_wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_delete_marks_jis(word=%s)", eb_quoted_string(word)));

    while (*in_wp != '\0' && *(in_wp + 1) != '\0') {
	c1 = *in_wp;
	c2 = *(in_wp + 1);

	if (c1 != 0x21
	    || (c2 != 0x26 && c2 != 0x3e && c2 != 0x47 && c2 != 0x5d)) {
	    /*
	     * This is not a character to be deleted.
	     */
	    *out_wp = c1;
	    *(out_wp + 1) = c2;
	    out_wp += 2;
	}
	in_wp += 2;
    }
    *out_wp = '\0';

    LOG(("out: eb_delete_marks_jis()"));
}


/*
 * The table is used to convert long vowel marks.
 */
static const char long_vowel_table[] = {
    0x22, /* a(21) -> A(22) */		0x22, /* A(22) -> A(22) */
    0x24, /* i(23) -> I(24) */		0x24, /* I(24) -> I(24) */
    0x26, /* u(25) -> U(26) */		0x26, /* U(26) -> U(26) */
    0x28, /* e(27) -> E(28) */		0x28, /* E(28) -> E(28) */
    0x2a, /* o(29) -> O(2a) */		0x2a, /* O(2a) -> O(2a) */
    0x22, /* KA(2b) -> A(22) */		0x22, /* GA(2c) -> A(22) */
    0x24, /* KI(2d) -> I(24) */		0x24, /* GI(2e) -> I(24) */
    0x26, /* KU(2f) -> U(26) */		0x26, /* GU(30) -> U(26) */
    0x28, /* KE(31) -> E(28) */		0x28, /* GE(32) -> E(28) */
    0x2a, /* KO(33) -> O(2a) */		0x2a, /* GO(34) -> O(2a) */
    0x22, /* SA(35) -> A(22) */		0x22, /* ZA(36) -> A(22) */
    0x24, /* SI(37) -> I(24) */		0x24, /* ZI(38) -> I(24) */
    0x26, /* SU(39) -> U(26) */		0x26, /* ZU(3a) -> U(26) */
    0x28, /* SE(3b) -> E(28) */		0x28, /* ZE(3c) -> E(28) */
    0x2a, /* SO(3d) -> O(2a) */		0x2a, /* ZO(3e) -> O(2a) */
    0x22, /* TA(3f) -> A(22) */		0x22, /* DA(40) -> A(22) */
    0x24, /* TI(41) -> I(24) */		0x24, /* DI(42) -> I(24) */
    0x26, /* tu(43) -> U(26) */		0x26, /* TU(44) -> U(26) */
    0x26, /* DU(45) -> U(26) */		0x28, /* TE(46) -> E(28) */
    0x28, /* DE(47) -> E(28) */		0x2a, /* TO(48) -> O(2a) */
    0x2a, /* DO(49) -> O(2a) */		0x22, /* NA(4a) -> A(22) */
    0x24, /* NI(4b) -> I(24) */		0x26, /* NU(4c) -> U(26) */
    0x28, /* NE(4d) -> E(28) */		0x2a, /* NO(4e) -> O(2a) */
    0x22, /* HA(4f) -> A(22) */		0x22, /* BA(50) -> A(22) */
    0x22, /* PA(51) -> A(22) */		0x24, /* HI(52) -> I(24) */
    0x24, /* BI(53) -> I(24) */		0x24, /* PI(54) -> I(24) */
    0x26, /* HU(55) -> U(26) */		0x26, /* BU(56) -> U(26) */
    0x26, /* PU(57) -> U(26) */		0x28, /* HE(58) -> E(28) */
    0x28, /* BE(59) -> E(28) */		0x28, /* PE(5a) -> E(28) */
    0x2a, /* HO(5b) -> O(2a) */		0x2a, /* BO(5c) -> O(2a) */
    0x2a, /* PO(5d) -> O(2a) */		0x22, /* MA(5e) -> A(22) */
    0x24, /* MI(5f) -> I(24) */		0x26, /* MU(60) -> U(26) */
    0x28, /* ME(61) -> E(28) */		0x2a, /* MO(62) -> O(2a) */
    0x22, /* ya(63) -> A(22) */		0x22, /* YA(64) -> A(22) */
    0x26, /* yu(65) -> U(26) */		0x26, /* YU(66) -> U(26) */
    0x2a, /* yo(67) -> O(2a) */		0x2a, /* YO(68) -> O(2a) */
    0x22, /* RA(69) -> A(22) */		0x24, /* RI(6a) -> I(24) */
    0x26, /* RU(6b) -> U(26) */		0x28, /* RE(6c) -> E(28) */
    0x2a, /* RO(6d) -> O(2a) */		0x22, /* wa(6e) -> A(22) */
    0x22, /* WA(6f) -> A(22) */		0x24, /* WI(70) -> I(24) */
    0x28, /* WE(71) -> E(28) */		0x2a, /* WO(72) -> O(2a) */
    0x73, /* N (73) -> N(73) */		0x26, /* VU(74) -> U(26) */
    0x22, /* ka(75) -> A(22) */		0x28  /* ke(76) -> E(28) */
};


/*
 * Convert long vowel marks in `word' to the previous vowels.
 */
static void
eb_convert_long_vowels_jis(char *word)
{
    unsigned char *wp = (unsigned char *) word;
    unsigned char c1, c2;
    unsigned char previous_c1 = '\0', previous_c2 = '\0';

    LOG(("in: eb_convert_long_vowels_jis(word=%s)", eb_quoted_string(word)));

    while (*wp != '\0' && *(wp + 1) != '\0') {
	c1 = *wp;
	c2 = *(wp + 1);

	if (c1 == 0x21 && c2 == 0x3c) {
	    /*
	     * The is a long vowel mark.
	     * Convert to a vowel of the prev_ KANA character.
	     * If prev_ character is not KANA, the conversion is
	     * not done.
	     */
	    if ((previous_c1 == 0x24 || previous_c1 == 0x25)
		&& 0x21 <= previous_c2 && previous_c2 <= 0x76) {
		*wp = previous_c1;
		*(wp + 1) = long_vowel_table[previous_c2 - 0x21];
	    }
	}
	previous_c1 = c1;
	previous_c2 = c2;
	wp += 2;
    }
    *wp = '\0';

    LOG(("out: eb_convert_long_vowels_jis()"));
}


/*
 * Delete long vowel marks in `word'.
 */
static void
eb_delete_long_vowels_jis(char *word)
{
    unsigned char *in_wp = (unsigned char *) word;
    unsigned char *out_wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_delete_long_vowels_jis(word=%s)", eb_quoted_string(word)));

    while (*in_wp != '\0' && *(in_wp + 1) != '\0') {
	c1 = *in_wp;
	c2 = *(in_wp + 1);

	if (c1 != 0x21 || c2 != 0x3c) {
	    /*
	     * The is not a long vowel mark.
	     */
	    *out_wp = c1;
	    *(out_wp + 1) = c2;
	    out_wp += 2;
	}
	in_wp += 2;
    }
    *out_wp = '\0';

    LOG(("out: eb_delete_long_vowels_jis()"));
}


/*
 * Convert the double consonant mark `tu' to `TU'.
 */
static void
eb_convert_double_consonants_jis(char *word)
{
    unsigned char *wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_convert_double_consonants_jis(word=%s)",
	eb_quoted_string(word)));

    while (*wp != '\0' && *(wp + 1) != '\0') {
	c1 = *wp;
	c2 = *(wp + 1);

	if ((c1 == 0x24 || c1 == 0x25) && c2 == 0x43) {
	    /*
	     * This is a double sound mark.  Convert to the corresponding
	     * sound mark.
	     */
	    *(wp + 1) = c2 + 1;
	}
	wp += 2;
    }
    *wp = '\0';

    LOG(("out: eb_convert_double_consonants_jis()"));
}


/*
 * Convert the contracted sound marks to the corresponding
 * non-contracted sound marks.
 * (`ya', `yu', `yo', `wa', `ka', `ke' -> `YA', `YU', `YO', `WA', `KA', `KE')
 */
static void
eb_convert_contracted_sounds_jis(char *word)
{
    unsigned char *wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_convert_contracted_sounds_jis(word=%s)",
	eb_quoted_string(word)));

    while (*wp != '\0' && *(wp + 1) != '\0') {
	c1 = *wp;
	c2 = *(wp + 1);

	if (c1 == 0x24 || c1 == 0x25) {
	    /*
	     * This is HIRAGANA or KANAKANA.
	     * If this is a contracted sound mark, convert to the
	     * corresponding uncontracted sound mark.
	     */
	    if (c2 == 0x63 || c2 == 0x65 || c2 == 0x67 || c2 == 0x6e)
		*(wp + 1) = c2 + 1;
	    else if (c2 == 0x75)
		*(wp + 1) = 0x2b;
	    else if (c2 == 0x76)
		*(wp + 1) = 0x31;
	}
	wp += 2;
    }
    *wp = '\0';

    LOG(("in: eb_convert_contracted_sounds_jis()"));
}


/*
 * Convert the small vowels to the normal vowels.
 * (`a', `i', `u', `e', `o' -> `A', `I', `U', `E', `O')
 */
static void
eb_convert_small_vowels_jis(char *word)
{
    unsigned char *wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_convert_small_vowels_jis(word=%s)", eb_quoted_string(word)));

    while (*wp != '\0' && *(wp + 1) != '\0') {
	c1 = *wp;
	c2 = *(wp + 1);

	if (c1 == 0x24 || c1 == 0x25) {
	    /*
	     * This is HIRAGANA or KANAKANA.
	     * If this is a small vowel mark, convert to a normal vowel.
	     */
	    if (c2 == 0x21 || c2 == 0x23 || c2 == 0x25 || c2 == 0x27
		|| c2 == 0x29)
		*(wp + 1) = c2 + 1;
	}
	wp += 2;
    }
    *wp = '\0';

    LOG(("out: eb_convert_small_vowels_jis()"));
}


/*
 * The table is used to convert voiced consonant marks.
 */
static const char voiced_consonant_table[] = {
    0x21, /* a(21) -> a(22) */		0x22, /* A(22) -> A(22) */
    0x23, /* i(23) -> i(24) */		0x24, /* I(24) -> I(24) */
    0x25, /* u(25) -> u(26) */		0x26, /* U(26) -> U(26) */
    0x27, /* e(27) -> e(28) */		0x28, /* E(28) -> E(28) */
    0x29, /* o(29) -> o(2a) */		0x2a, /* O(2a) -> O(2a) */
    0x2b, /* KA(2b) -> KA(2b) */	0x2b, /* GA(2c) -> KA(2b) */
    0x2d, /* KI(2d) -> KI(2d) */	0x2d, /* GI(2e) -> KI(2d) */
    0x2f, /* KU(2f) -> KU(2f) */	0x2f, /* GU(30) -> KU(2f) */
    0x31, /* KE(31) -> KE(31) */	0x31, /* GE(32) -> KE(31) */
    0x33, /* KO(33) -> KO(33) */	0x33, /* GO(34) -> KO(33) */
    0x35, /* SA(35) -> SA(35) */	0x35, /* ZA(36) -> SA(35) */
    0x37, /* SI(37) -> SI(37) */	0x37, /* ZI(38) -> SI(37) */
    0x39, /* SU(39) -> SU(39) */	0x39, /* ZU(3a) -> SU(39) */
    0x3b, /* SE(3b) -> SE(3b) */	0x3b, /* ZE(3c) -> SE(3b) */
    0x3d, /* SO(3d) -> SO(3d) */	0x3d, /* ZO(3e) -> SO(3d) */
    0x3f, /* TA(3f) -> TA(3f) */	0x3f, /* DA(40) -> TA(3f) */
    0x41, /* TI(41) -> TI(41) */	0x41, /* DI(42) -> TI(41) */
    0x43, /* tu(43) -> TU(43) */	0x44, /* TU(44) -> TU(44) */
    0x44, /* DU(45) -> TU(44) */	0x46, /* TE(46) -> TE(46) */
    0x46, /* DE(47) -> TE(46) */	0x48, /* TO(48) -> TO(48) */
    0x48, /* DO(49) -> TO(48) */	0x4a, /* NA(4a) -> NA(4a) */
    0x4b, /* NI(4b) -> NI(4b) */	0x4c, /* NU(4c) -> NU(4c) */
    0x4d, /* NE(4d) -> NE(4d) */	0x4e, /* NO(4e) -> NO(4e) */
    0x4f, /* HA(4f) -> HA(4f) */	0x4f, /* BA(50) -> HA(4f) */
    0x51, /* PA(51) -> PA(51) */	0x52, /* HI(52) -> HI(52) */
    0x52, /* BI(53) -> HI(52) */	0x54, /* PI(54) -> PU(54) */
    0x55, /* HU(55) -> HU(55) */	0x55, /* BU(56) -> HU(55) */
    0x57, /* PU(57) -> PU(57) */	0x58, /* HE(58) -> HE(58) */
    0x58, /* BE(59) -> HE(58) */	0x5a, /* PE(5a) -> PE(5a) */
    0x5b, /* HO(5b) -> HO(5b) */	0x5b, /* BO(5c) -> HO(5b) */
    0x5d, /* PO(5d) -> PO(5d) */	0x5e, /* MA(5e) -> MA(5e) */
    0x5f, /* MI(5f) -> MI(5f) */	0x60, /* MU(60) -> MU(60) */
    0x61, /* ME(61) -> ME(61) */	0x62, /* MO(62) -> MO(62) */
    0x64, /* ya(63) -> ya(63) */	0x64, /* YA(64) -> YA(64) */
    0x66, /* yu(65) -> yu(65) */	0x66, /* YU(66) -> YU(66) */
    0x68, /* yo(67) -> yo(67) */	0x68, /* YO(68) -> YO(68) */
    0x69, /* RA(69) -> TA(69) */	0x6a, /* RI(6a) -> RI(6a) */
    0x6b, /* RU(6b) -> RU(6b) */	0x6c, /* RE(6c) -> RE(6c) */
    0x6d, /* RO(6d) -> RO(6d) */	0x6e, /* wa(6e) -> wa(6e) */
    0x6f, /* WA(6f) -> WA(6f) */	0x70, /* WI(70) -> WI(70) */
    0x71, /* WE(71) -> WE(71) */	0x72, /* WO(72) -> WO(72) */
    0x73, /* N(73) -> N(73) */		0x26, /* VU(74) -> U(26) */
    0x75, /* ka(75) -> ka(75) */	0x76  /* ke(76) -> ke(76) */
};

/*
 * Convert the contracted sound marks to the corresponding
 * non-contracted sound marks (e.g. `GA' to `KA').
 */
static void
eb_convert_voiced_consonants_jis(char *word)
{
    unsigned char *wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_convert_voiced_consonants_jis(word=%s)",
	eb_quoted_string(word)));

    while (*wp != '\0' && *(wp + 1) != '\0') {
	c1 = *wp;
	c2 = *(wp + 1);

	if ((c1 == 0x24 || c1 == 0x25) && 0x21 <= c2 && c2 <= 0x76) {
	    /*
	     * This is a voiced constonat mark.  Convert to the
	     * corresponding unvoiced constonant mark.
	     */
	    *(wp + 1) = voiced_consonant_table[c2 - 0x21];
	}
	wp += 2;
    }
    *wp = '\0';

    LOG(("out: eb_convert_voiced_consonants_jis()"));
}


/*
 * Convert the p sound marks
 * (`PA', `PI', `PU', `PE', `PO' -> `HA', `HI', `HU', `HE', `HO')
 */
static void
eb_convert_p_sounds_jis(char *word)
{
    unsigned char *wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_convert_p_sounds_jis(word=%s)", eb_quoted_string(word)));

    while (*wp != '\0' && *(wp + 1) != '\0') {
	c1 = *wp;
	c2 = *(wp + 1);

	if (c1 == 0x24 || c1 == 0x25) {
	    /*
	     * This is HIRAGANA or KANAKANA.
	     * If this is a p-sound mark, convert to the corresponding
	     * unvoiced consonant mark.
	     */
	    if (c2 == 0x51 || c2 == 0x54 || c2 == 0x57 || c2 == 0x5a
		|| c2 == 0x5d)
		*(wp + 1) = c2 - 2;
	}
	wp += 2;
    }
    *wp = '\0';

    LOG(("out: eb_convert_p_sounds_jis()"));
}


/*
 * Delete spaces in `word'.
 */
static void
eb_delete_spaces_latin(char *word)
{
    unsigned char *in_wp = (unsigned char *) word;
    unsigned char *out_wp = (unsigned char *) word;

    LOG(("in: eb_delete_space_latin(word=%s)", eb_quoted_string(word)));

    while (*in_wp != '\0') {
	if (*in_wp != ' ') {
	    /*
	     * This is not a space character of ISO 8859 1.
	     */
	    *out_wp = *in_wp;
	    out_wp++;
	}
	in_wp++;
    }
    *out_wp = '\0';

    LOG(("out: eb_delete_space_latin()"));
}


/*
 * Delete spaces in `word'.
 */
static void
eb_delete_spaces_jis(char *word)
{
    unsigned char *in_wp = (unsigned char *) word;
    unsigned char *out_wp = (unsigned char *) word;
    unsigned char c1, c2;

    LOG(("in: eb_delete_space_jis(word=%s)", eb_quoted_string(word)));

    while (*in_wp != '\0' && *(in_wp + 1) != '\0') {
	c1 = *in_wp;
	c2 = *(in_wp + 1);

	if (c1 != 0x21 || c2 != 0x21) {
	    /*
	     * This is not a space character of JIS X 0208.
	     */
	    *out_wp = c1;
	    *(out_wp + 1) = c2;
	    out_wp += 2;
	}
	in_wp += 2;
    }
    *out_wp = '\0';

    LOG(("out: eb_delete_space_jis()"));
}


/*
 * Reverse a word for ENDWORD SEARCH.
 *
 * `word' is a word to reverse.  It must be an alphabetic word.
 * The reversed word is also put into `word'.
 */
static void
eb_reverse_word_latin(char *word)
{
    char *p1, *p2;
    int word_length;
    char c;

    LOG(("in: eb_reverse_word_latin(word=%s)", eb_quoted_string(word)));

    word_length = strlen(word);
    if (word_length == 0)
	return;
    for (p1 = word, p2 = word + word_length - 1; p1 < p2; p1++, p2--) {
	c = *p1;
	*p1 = *p2;
	*p2 = c;
    }

    LOG(("out: eb_reverse_word_latin()"));
}


/*
 * Reverse a word for ENDWORD SEARCH.
 *
 * `word' is a word to reverse.  It must be a KANA word.
 * The reversed word is also put into `word'.
 */
static void
eb_reverse_word_jis(char *word)
{
    char *p1, *p2;
    int word_length;
    char c;

    LOG(("in: eb_reverse_word_jis(word=%s)", eb_quoted_string(word)));

    word_length = strlen(word);
    if (word_length % 2 == 1) {
	*(word + word_length - 1) = '\0';
	word_length--;
    }
    for (p1 = word, p2 = word + word_length - 2; p1 < p2; p1 += 2, p2 -= 2) {
	c = *p1;
	*p1 = *p2;
	*p2 = c;
	c = *(p1 + 1);
	*(p1 + 1) = *(p2 + 1);
	*(p2 + 1) = c;
    }

    LOG(("out: eb_reverse_word_jis()"));
}


/*
 * Convert `input_word' to UTF 8 and put it into `word'.
 *
 * If `input_word' is a valid string to search, EB_WORD_OTHER is returned.
 * Otherwise, -1 is returned.
 */
static EB_Error_Code
eb_convert_utf8(EB_Book *book, const char *input_word, char *word,
    EB_Word_Code *word_code)
{
    EB_Error_Code error_code;
    unsigned char *wp = (unsigned char *) word;
    const unsigned char *inp = (const unsigned char *) input_word;
    const unsigned char *tail;
    int word_length = 0;
    int count = 0, chr;
    char *result;

    LOG(("in: eb_convert_utf8(book=%d, input_word=%s)", (int)book->code,
	 eb_quoted_string(input_word)));

    tail = (const unsigned char *) input_word + strlen(input_word);

    while (inp < tail) {
	if (*inp < 0x20) {
	    inp++;
	    continue;
	}

	count = eb_read_utf8 ((const char *)inp, &chr);
	if (!count) {
	    error_code = EB_ERR_BAD_WORD;
	    goto failed;
	}

/*
      if (*inp < 0x80) {
	count = 1;
	chr = *inp;
      } else if (0xc0 <= *inp       && *inp       <= 0xdf && 
		 0x80 <= *(inp + 1) && *(inp + 1) <= 0xbf) {
	count = 2;
	chr = ((*(inp + 0) & 0x1f) <<  6)
	  +    (*(inp + 1) & 0x3f);
      } else if (0xe0 <= *inp       && *inp       <= 0xef && 
		 0x80 <= *(inp + 1) && *(inp + 1) <= 0xbf &&
		 0x80 <= *(inp + 2) && *(inp + 2) <= 0xbf) {
	count = 3;
	chr = ((*(inp + 0) & 0x0f) << 12)
	  +   ((*(inp + 1) & 0x3f) <<  6)
	  +    (*(inp + 2) & 0x3f);
      } else if (0xf0 <= *inp       && *inp       <= 0xf7 && 
		 0x80 <= *(inp + 1) && *(inp + 1) <= 0xbf &&
		 0x80 <= *(inp + 2) && *(inp + 2) <= 0xbf &&
		 0x80 <= *(inp + 3) && *(inp + 3) <= 0xbf) {
	count = 4;
	chr = ((*(inp + 0) & 0x07) << 18)
	  +   ((*(inp + 1) & 0x3f) << 12)
	  +   ((*(inp + 2) & 0x3f) <<  6)
	  +    (*(inp + 2) & 0x3f);
      } else {
	error_code = EB_ERR_BAD_WORD;
	goto failed;
      }
*/
	result = eb_normalize_utf8(book, chr);

      if (result == NULL) {
	if (EB_MAX_WORD_LENGTH < word_length + count) {
	  error_code = EB_ERR_TOO_LONG_WORD;
	  goto failed;
	}
	memcpy (wp, inp, count);
	wp += count;
	word_length += count;
      } else if (strlen(result)) {
	if (EB_MAX_WORD_LENGTH < word_length + strlen(result)) {
	  error_code = EB_ERR_TOO_LONG_WORD;
	  goto failed;
	}
	memcpy (wp, result, strlen(result));
	wp += strlen(result);
	word_length += strlen(result);
      }
      inp += count;
    }

    if (word_length == 0) {
      wp = (unsigned char *) word;
      inp = (const unsigned char *) input_word;
      while (inp < tail) {
	if (*inp <= 0x20) {
	  inp++;
	  continue;
	}
	*wp = *inp;
	wp++;
	inp++;
	word_length++;
      }
    }

    if (word_length == 0) {
      error_code = EB_ERR_EMPTY_WORD;
      goto failed;
    }

    *wp = '\0';
    *word_code = EB_WORD_OTHER;

    LOG(("out: eb_convert_utf8(word=%s, word_code=%d) = %s",
	eb_quoted_string(word), (int)*word_code, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *word = '\0';
    *word_code = EB_WORD_INVALID;
    LOG(("out: eb_convert_utf8() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Reverse a word for ENDWORD SEARCH.
 *
 * `word' is a word to reverse.  It must be an alphabetic word.
 * The reversed word is also put into `word'.
 */
static void
eb_reverse_word_utf8(char *word)
{
    char *p1, *p2, *p3;
    int word_length;
    char tmp[EB_MAX_WORD_LENGTH];

    LOG(("in: eb_reverse_word_utf8(word=%s)", eb_quoted_string(word)));

    word_length = strlen(word);
    if (word_length == 0)
	return;

    p1 = word + word_length - 1;
    p2 = tmp;
    p3 = p1;

    while (p1 >= word) {
	while (!((unsigned char) *p1 <= 0x7f
		 || (*p1 >= (char) 0xc0 && *p1 <= (char) 0xdf)
		 || (*p1 >= (char) 0xe0 && *p1 <= (char) 0xef)
		 || (*p1 >= (char) 0xf0 && *p1 <= (char) 0xf7)))
	       p1--;
	if (p1 < word) p1 = word;
	memcpy(p2, p1, p3 - p1 + 1);
	p2 += p3 - p1 + 1;
	p1--;
	p3 = p1;
    }

    memcpy(word, tmp, word_length);

    LOG(("out: eb_reverse_word_utf8(word=%s)", eb_quoted_string(word)));
}
