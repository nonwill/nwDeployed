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
#include "font.h"
#include "build-post.h"

/*
 * Unexported functions.
 */
static EB_Error_Code eb_narrow_character_bitmap_jis(EB_Book *book,
    int character_number, char *bitmap);
static EB_Error_Code eb_narrow_character_bitmap_latin(EB_Book *book,
    int character_number, char *bitmap);


/*
 * Open a font file.
 */
EB_Error_Code
eb_open_narrow_font_file(EB_Book *book, EB_Font_Code font_code)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    EB_Font *narrow_font;
    char font_path_name[EB_MAX_PATH_LENGTH + 1];
    Zio_Code zio_code;

    LOG(("in: eb_open_narrow_font(book=%d, font_code=%d)",
	(int)book->code, (int)font_code));

    subbook = book->subbook_current;
    narrow_font = subbook->narrow_fonts + font_code;

    if (narrow_font->font_code == EB_FONT_INVALID) {
	error_code = EB_ERR_FAIL_OPEN_FONT;
	goto failed;
    }
    if (0 <= zio_file(&narrow_font->zio))
	goto succeeded;

    /*
     * If the book is EBWING, open the narrow font file.
     * (In EB books, font data are stored in the `START' file.)
     */
    zio_code = ZIO_INVALID;

    if (book->disc_code == EB_DISC_EB) {
	if (narrow_font->initialized) {
	    if (zio_mode(&narrow_font->zio) != ZIO_INVALID)
		zio_code = ZIO_REOPEN;
	} else {
	    zio_code = zio_mode(&subbook->text_zio);
	}
	eb_compose_path_name2(book->path, subbook->directory_name,
	    subbook->text_file_name, font_path_name);

    } else {
	if (narrow_font->initialized) {
	    if (zio_mode(&narrow_font->zio) != ZIO_INVALID)
		zio_code = ZIO_REOPEN;
	    eb_compose_path_name3(book->path, subbook->directory_name,
		subbook->gaiji_directory_name, narrow_font->file_name,
		font_path_name);
	} else {
	    eb_canonicalize_file_name(narrow_font->file_name);
	    if (eb_find_file_name3(book->path, subbook->directory_name,
		subbook->gaiji_directory_name, narrow_font->file_name,
		narrow_font->file_name) != EB_SUCCESS) {
		error_code = EB_ERR_FAIL_OPEN_FONT;
		goto failed;
	    }

	    eb_compose_path_name3(book->path, subbook->directory_name,
		subbook->gaiji_directory_name, narrow_font->file_name,
		font_path_name);
	    eb_path_name_zio_code(font_path_name, ZIO_PLAIN, &zio_code);
	}
    }

    if (zio_code != ZIO_INVALID
	&& zio_open(&narrow_font->zio, font_path_name, zio_code) < 0) {
	error_code = EB_ERR_FAIL_OPEN_FONT;
	goto failed;
    }

  succeeded:
    LOG(("out: eb_open_narrow_font_file(file=%d) = %s",
	zio_file(&narrow_font->zio), eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_open_narrow_font_file() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Read font header.
 */
EB_Error_Code
eb_load_narrow_font_header(EB_Book *book, EB_Font_Code font_code)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    EB_Font *narrow_font;
    char buffer[16];
    int character_count;
    Zio *zio;

    LOG(("in: eb_load_narrow_font_header(book=%d, font_code=%d)",
	(int)book->code, (int)font_code));

    subbook = book->subbook_current;
    narrow_font = subbook->narrow_fonts + font_code;
    zio = &narrow_font->zio;

    if (narrow_font->initialized)
	goto succeeded;

    /*
     * Read information from the text file.
     */
    if (zio_lseek(zio, ((off_t) narrow_font->page - 1) * EB_SIZE_PAGE,
	SEEK_SET) < 0) {
	error_code = EB_ERR_FAIL_SEEK_FONT;
	goto failed;
    }
    if (zio_read(zio, buffer, 16) != 16) {
	error_code = EB_ERR_FAIL_READ_FONT;
	goto failed;
    }

    /*
     * If the number of characters (`character_count') is 0, the font
     * is unavailable.  We return EB_ERR_NO_SUCH_FONT.
     */
    character_count = eb_uint2(buffer + 12);
    if (character_count == 0) {
	zio_close(zio);
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    /*
     * Set the information.
     */
    narrow_font->start = eb_uint2(buffer + 10);
    if (book->character_code == EB_CHARCODE_ISO8859_1) {
	narrow_font->end = narrow_font->start
	    + ((character_count / 0xfe) << 8) + (character_count % 0xfe) - 1;
	if (0xfe < (narrow_font->end & 0xff))
	    narrow_font->end += 3;
    } else {
	narrow_font->end = narrow_font->start
	    + ((character_count / 0x5e) << 8) + (character_count % 0x5e) - 1;
	if (0x7e < (narrow_font->end & 0xff))
	    narrow_font->end += 0xa3;
    }

    if (book->character_code == EB_CHARCODE_ISO8859_1) {
	if ((narrow_font->start & 0xff) < 0x01
	    || 0xfe < (narrow_font->start & 0xff)
	    || narrow_font->start < 0x0001
	    || 0x1efe < narrow_font->end) {
	    error_code = EB_ERR_UNEXP_FONT;
	    goto failed;
	}
    } else {
	if ((narrow_font->start & 0xff) < 0x21
	    || 0x7e < (narrow_font->start & 0xff)
	    || narrow_font->start < 0xa121
	    || 0xfe7e < narrow_font->end) {
	    error_code = EB_ERR_UNEXP_FONT;
	    goto failed;
	}
    }

  succeeded:
    LOG(("out: eb_load_narrow_font_header()", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_load_narrow_font_header()", eb_error_string(error_code)));
    return error_code;
}


/*
 * Read font glyph data.
 */
EB_Error_Code
eb_load_narrow_font_glyphs(EB_Book *book, EB_Font_Code font_code)
{
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    EB_Font *narrow_font;
    int character_count;
    size_t glyph_size;
    size_t total_glyph_size;
    Zio *zio;

    LOG(("in: eb_load_narrow_font_glyphs(book=%d, font_code=%d)",
	(int)book->code, (int)font_code));

    subbook = book->subbook_current;
    narrow_font = subbook->narrow_fonts + font_code;
    zio = &narrow_font->zio;

    if (narrow_font->glyphs != NULL)
	goto succeeded;

    /*
     * Calculate size of glyph data (`total_glyph_size').
     *
     *    Set the number of local defined characters to `character_count'.
     *    Set the number of character glpyhs in a page to `page_glyph_count'.
     *    Set size of glyph data to `total_glyph_size'.
     */
    if (book->character_code == EB_CHARCODE_ISO8859_1) {
	character_count
	    = ((narrow_font->end >> 8) - (narrow_font->start >> 8)) * 0xfe
	    + ((narrow_font->end & 0xff) - (narrow_font->start & 0xff)) + 1;
    } else {
	character_count
	    = ((narrow_font->end >> 8) - (narrow_font->start >> 8)) * 0x5e
	    + ((narrow_font->end & 0xff) - (narrow_font->start & 0xff)) + 1;
    }

    eb_narrow_font_size2(font_code, &glyph_size);
    total_glyph_size
	= (character_count / (1024 / glyph_size)) * 1024
	+ (character_count % (1024 / glyph_size)) * glyph_size;

    /*
     * Allocate memory for glyph data.
     */
    narrow_font->glyphs = (char *) malloc(total_glyph_size);
    if (narrow_font->glyphs == NULL) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }

    /*
     * Read glyphs.
     */
    if (zio_lseek(zio, (off_t) narrow_font->page * EB_SIZE_PAGE, SEEK_SET)
	< 0) {
	error_code = EB_ERR_FAIL_SEEK_FONT;
	goto failed;
    }
    if (zio_read(zio, narrow_font->glyphs, total_glyph_size)
	!= total_glyph_size) {
	error_code = EB_ERR_FAIL_READ_FONT;
	goto failed;
    }

  succeeded:
    LOG(("out: eb_load_narrow_font_glyphs()", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_load_narrow_font_glyphs()", eb_error_string(error_code)));
    if (narrow_font->glyphs != NULL) {
	free(narrow_font->glyphs);
	narrow_font->glyphs = NULL;
    }
    return error_code;
}


/*
 * Examine whether the current subbook in `book' has a narrow font.
 */
int
eb_have_narrow_font(EB_Book *book)
{
    int i;

    eb_lock(&book->lock);
    LOG(("in: eb_have_narrow_font(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL)
	goto failed;

    /*
     * If the narrow font has already set, the subbook has narrow fonts.
     */
    if (book->subbook_current->narrow_current != NULL)
	goto succeeded;

    /*
     * Scan the font table.
     */
    for (i = 0; i < EB_MAX_FONTS; i++) {
	if (book->subbook_current->narrow_fonts[i].font_code
	    != EB_FONT_INVALID)
	    break;
    }

    if (EB_MAX_FONTS <= i)
	goto failed;

  succeeded:
    LOG(("out: eb_have_narrow_font() = %d", 1));
    eb_unlock(&book->lock);
    return 1;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_have_narrow_font() = %d", 0));
    eb_unlock(&book->lock);
    return 0;
}


/*
 * Get width of the font `font_code' in the current subbook of `book'.
 */
EB_Error_Code
eb_narrow_font_width(EB_Book *book, int *width)
{
    EB_Error_Code error_code;
    EB_Font_Code font_code;

    eb_lock(&book->lock);
    LOG(("in: eb_narrow_font_width(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * The narrow font must exist in the current subbook.
     */
    if (book->subbook_current->narrow_current == NULL) {
	error_code = EB_ERR_NO_CUR_FONT;
	goto failed;
    }

    /*
     * Calculate width.
     */
    font_code = book->subbook_current->narrow_current->font_code;
    error_code = eb_narrow_font_width2(font_code, width);
    if (error_code != EB_SUCCESS)
	goto failed;

    LOG(("out: eb_narrow_font_width(width=%d) = %s", (int)*width,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *width = 0;
    LOG(("out: eb_narrow_font_width() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Get width of the font `font_code'.
 */
EB_Error_Code
eb_narrow_font_width2(EB_Font_Code font_code, int *width)
{
    EB_Error_Code error_code;

    LOG(("in: eb_narrow_font_width2(font_code=%d)", (int)font_code));

    switch (font_code) {
    case EB_FONT_16:
	*width = EB_WIDTH_NARROW_FONT_16;
	break;
    case EB_FONT_24:
	*width = EB_WIDTH_NARROW_FONT_24;
	break;
    case EB_FONT_30:
	*width = EB_WIDTH_NARROW_FONT_30;
	break;
    case EB_FONT_48:
	*width = EB_WIDTH_NARROW_FONT_48;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_narrow_font_width2(width=%d) = %s", *width,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *width = 0;
    LOG(("out: eb_narrow_font_width2() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Get the bitmap size of the font `font_code' in the current subbook
 * of `book'.
 */
EB_Error_Code
eb_narrow_font_size(EB_Book *book, size_t *size)
{
    EB_Error_Code error_code;
    EB_Font_Code font_code;
    int width;
    int height;

    eb_lock(&book->lock);
    LOG(("in: eb_narrow_font_size(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * The narrow font must exist in the current subbook.
     */
    if (book->subbook_current->narrow_current == NULL) {
	error_code = EB_ERR_NO_CUR_FONT;
	goto failed;
    }

    /*
     * Calculate size.
     */
    font_code = book->subbook_current->narrow_current->font_code;
    error_code = eb_narrow_font_width2(font_code, &width);
    if (error_code != EB_SUCCESS)
	goto failed;
    error_code = eb_font_height2(font_code, &height);
    if (error_code != EB_SUCCESS)
	goto failed;
    *size = (width / 8) * height;

    LOG(("out: eb_narrow_font_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_narrow_font_size() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Get the bitmap size of a character in `font_code' of the current
 * subbook.
 */
EB_Error_Code
eb_narrow_font_size2(EB_Font_Code font_code, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_narrow_font_size2(font_code=%d)", (int)font_code));

    switch (font_code) {
    case EB_FONT_16:
	*size = EB_SIZE_NARROW_FONT_16;
	break;
    case EB_FONT_24:
	*size = EB_SIZE_NARROW_FONT_24;
	break;
    case EB_FONT_30:
	*size = EB_SIZE_NARROW_FONT_30;
	break;
    case EB_FONT_48:
	*size = EB_SIZE_NARROW_FONT_48;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_narrow_font_size2(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_narrow_font_size2() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Get the character number of the start of the narrow font of the current
 * subbook in `book'.
 */
EB_Error_Code
eb_narrow_font_start(EB_Book *book, int *start)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_narrow_font_start(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * The narrow font must exist in the current subbook.
     */
    if (book->subbook_current->narrow_current == NULL) {
	error_code = EB_ERR_NO_CUR_FONT;
	goto failed;
    }

    *start = book->subbook_current->narrow_current->start;

    LOG(("out: eb_narrow_font_start(start=%d) = %s", *start,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_narrow_font_start() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Get the character number of the end of the narrow font of the current
 * subbook in `book'.
 */
EB_Error_Code
eb_narrow_font_end(EB_Book *book, int *end)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_narrow_font_end(book=%d)", (int)book->code));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * The narrow font must exist in the current subbook.
     */
    if (book->subbook_current->narrow_current == NULL) {
	error_code = EB_ERR_NO_CUR_FONT;
	goto failed;
    }

    *end = book->subbook_current->narrow_current->end;

    LOG(("out: eb_narrow_font_end(end=%d) = %s", *end,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_narrow_font_end() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Get bitmap data of the character with character number `character_number'
 * in the current narrow font of the current subbook in `book'.
 */
EB_Error_Code
eb_narrow_font_character_bitmap(EB_Book *book, int character_number,
    char *bitmap)
{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_narrow_font_character_bitmap(book=%d, character_number=%d)",
	(int)book->code, character_number));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * The narrow font must exist in the current subbook.
     */
    if (book->subbook_current->narrow_current == NULL) {
	error_code = EB_ERR_NO_CUR_FONT;
	goto failed;
    }

    if (book->character_code == EB_CHARCODE_ISO8859_1) {
	error_code = eb_narrow_character_bitmap_latin(book, character_number,
	    bitmap);
    } else {
	error_code = eb_narrow_character_bitmap_jis(book, character_number,
	    bitmap);
    }
    if (error_code != EB_SUCCESS)
	goto failed;

    LOG(("out: eb_narrow_font_character_bitmap() = %s",
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *bitmap = '\0';
    LOG(("out: eb_narrow_font_character_bitmap() = %s",
	eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Get bitmap data of the character with character number `character_number'
 * in the current narrow font of the current subbook in `book'.
 */
static EB_Error_Code
eb_narrow_character_bitmap_jis(EB_Book *book, int character_number,
    char *bitmap)
{
    EB_Error_Code error_code;
    EB_Font *narrow_current;
    int start;
    int end;
    int character_index;
    off_t offset;
    size_t size;
    Zio *zio;

    LOG(("in: eb_narrow_font_character_bitmap_jis(book=%d, \
character_number=%d)",
	(int)book->code, character_number));

    start = book->subbook_current->narrow_current->start;
    end = book->subbook_current->narrow_current->end;
    narrow_current = book->subbook_current->narrow_current;

    /*
     * Check for `character_number'.  Is it in a range of bitmaps?
     * This test works correctly even when the font doesn't exist in
     * the current subbook because `start' and `end' have set to -1
     * in the case.
     */
    if (character_number < start
	|| end < character_number
	|| (character_number & 0xff) < 0x21
	|| 0x7e < (character_number & 0xff)) {
	error_code = EB_ERR_NO_SUCH_CHAR_BMP;
	goto failed;
    }

    /*
     * Calculate the size and the location of bitmap data.
     */
    error_code = eb_narrow_font_size(book, &size);
    if (error_code != EB_SUCCESS)
	goto failed;

    character_index = ((character_number >> 8) - (start >> 8)) * 0x5e
	+ ((character_number & 0xff) - (start & 0xff));
    offset
	= (character_index / (1024 / size)) * 1024
	+ (character_index % (1024 / size)) * size;

    /*
     * Read bitmap data.
     */
    if (narrow_current->glyphs == NULL) {
	zio = &narrow_current->zio;

	if (zio_lseek(zio,
		(off_t) narrow_current->page * EB_SIZE_PAGE + offset,
		SEEK_SET) < 0) {
	    error_code = EB_ERR_FAIL_SEEK_FONT;
	    goto failed;
	}
	if (zio_read(zio, bitmap, size) != size) {
	    error_code = EB_ERR_FAIL_READ_FONT;
	    goto failed;
	}
    } else {
	memcpy(bitmap, narrow_current->glyphs + offset, size);
    }

    LOG(("out: eb_narrow_font_character_bitmap_jis() = %s",
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *bitmap = '\0';
    LOG(("out: eb_narrow_font_character_bitmap_jis() = %s",
	eb_error_string(error_code)));
    return error_code;
}


/*
 * Get bitmap data of the character with character number `character_number'
 * in the current narrow font of the current subbook in `book'.
 */
static EB_Error_Code
eb_narrow_character_bitmap_latin(EB_Book *book, int character_number,
    char *bitmap)
{
    EB_Error_Code error_code;
    EB_Font *narrow_current;
    int start;
    int end;
    int character_index;
    off_t offset;
    size_t size;
    Zio *zio;

    LOG(("in: eb_narrow_font_character_bitmap_latin(book=%d, \
character_number=%d)",
	(int)book->code, character_number));

    start = book->subbook_current->narrow_current->start;
    end = book->subbook_current->narrow_current->end;
    narrow_current = book->subbook_current->narrow_current;

    /*
     * Check for `ch'.  Is it in a range of bitmaps?
     * This test works correctly even when the font doesn't exist in
     * the current subbook because `start' and `end' have set to -1
     * in the case.
     */
    if (character_number < start
	|| end < character_number
	|| (character_number & 0xff) < 0x01
	|| 0xfe < (character_number & 0xff)) {
	error_code = EB_ERR_NO_SUCH_CHAR_BMP;
	goto failed;
    }

    /*
     * Calculate the size and the location of bitmap data.
     */
    error_code = eb_narrow_font_size(book, &size);
    if (error_code != EB_SUCCESS)
	goto failed;

    character_index = ((character_number >> 8) - (start >> 8)) * 0xfe
	+ ((character_number & 0xff) - (start & 0xff));
    offset
	= (character_index / (1024 / size)) * 1024
	+ (character_index % (1024 / size)) * size;

    /*
     * Read bitmap data.
     */
    if (narrow_current->glyphs == NULL) {
	zio = &narrow_current->zio;

	if (zio_lseek(zio,
		(off_t) narrow_current->page * EB_SIZE_PAGE + offset,
		SEEK_SET) < 0) {
	    error_code = EB_ERR_FAIL_SEEK_FONT;
	    goto failed;
	}
	if (zio_read(zio, bitmap, size) != size) {
	    error_code = EB_ERR_FAIL_READ_FONT;
	    goto failed;
	}
    } else {
	memcpy(bitmap, narrow_current->glyphs + offset, size);
    }

    LOG(("out: eb_narrow_font_character_bitmap_latin() = %s",
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *bitmap = '\0';
    LOG(("out: eb_narrow_font_character_bitmap_latin() = %s",
	eb_error_string(error_code)));
    return error_code;
}


/*
 * Return next `n'th character number from `character_number'.
 */
EB_Error_Code
eb_forward_narrow_font_character(EB_Book *book, int n, int *character_number)
{
    EB_Error_Code error_code;
    int start;
    int end;
    int i;

    if (n < 0)
	return eb_backward_narrow_font_character(book, -n, character_number);

    eb_lock(&book->lock);
    LOG(("in: eb_forward_narrow_font_character(book=%d, n=%d, \
character_number=%d)",
	(int)book->code, n, *character_number));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * The narrow font must exist in the current subbook.
     */
    if (book->subbook_current->narrow_current == NULL) {
	error_code = EB_ERR_NO_CUR_FONT;
	goto failed;
    }

    start = book->subbook_current->narrow_current->start;
    end = book->subbook_current->narrow_current->end;

    if (book->character_code == EB_CHARCODE_ISO8859_1) {
	/*
	 * Check for `*character_number'. (ISO 8859 1)
	 */
	if (*character_number < start
	    || end < *character_number
	    || (*character_number & 0xff) < 0x01
	    || 0xfe < (*character_number & 0xff)) {
	    error_code = EB_ERR_NO_SUCH_CHAR_BMP;
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
		error_code = EB_ERR_NO_SUCH_CHAR_BMP;
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
	    error_code = EB_ERR_NO_SUCH_CHAR_BMP;
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
		error_code = EB_ERR_NO_SUCH_CHAR_BMP;
		goto failed;
	    }
	}
    }

    LOG(("out: eb_forward_narrow_font_character(character_number=%d) = %s",
	*character_number, eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *character_number = -1;
    LOG(("out: eb_forward_narrow_font_character() = %s",
	eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Return previous `n'th character number from `*character_number'.
 */
EB_Error_Code
eb_backward_narrow_font_character(EB_Book *book, int n, int *character_number)
{
    EB_Error_Code error_code;
    int start;
    int end;
    int i;

    if (n < 0)
	return eb_forward_narrow_font_character(book, -n, character_number);

    eb_lock(&book->lock);
    LOG(("in: eb_backward_narrow_font_character(book=%d, n=%d, \
character_number=%d)",
	(int)book->code, n, *character_number));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * The narrow font must exist in the current subbook.
     */
    if (book->subbook_current->narrow_current == NULL) {
	error_code = EB_ERR_NO_CUR_FONT;
	goto failed;
    }

    start = book->subbook_current->narrow_current->start;
    end = book->subbook_current->narrow_current->end;

    if (book->character_code == EB_CHARCODE_ISO8859_1) {
	/*
	 * Check for `*character_number'. (ISO 8859 1)
	 */
	if (*character_number < start
	    || end < *character_number
	    || (*character_number & 0xff) < 0x01
	    || 0xfe < (*character_number & 0xff)) {
	    error_code = EB_ERR_NO_SUCH_CHAR_BMP;
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
		error_code = EB_ERR_NO_SUCH_CHAR_BMP;
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
	    error_code = EB_ERR_NO_SUCH_CHAR_BMP;
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
		error_code = EB_ERR_NO_SUCH_CHAR_BMP;
		goto failed;
	    }
	}
    }

    LOG(("out: eb_backward_narrow_font_character(character_number=%d) = %s",
	*character_number, eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *character_number = -1;
    LOG(("out: eb_backward_narrow_font_character() = %s",
	eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


