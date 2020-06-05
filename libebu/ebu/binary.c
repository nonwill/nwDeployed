/*                                                            -*- C -*-
 * Copyright (c) 2001-2006  Motoyuki Kasahara
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
#include "build-post.h"

/*
 * Unexported function.
 */
static EB_Error_Code eb_read_binary_generic(EB_Book *book,
    size_t binary_max_length, char *binary, ssize_t *binary_length);
static EB_Error_Code eb_read_binary_wave(EB_Book *book,
    size_t binary_max_length, char *binary, ssize_t *binary_length);
static EB_Error_Code eb_read_binary_mono_graphic(EB_Book *book,
    size_t binary_max_length, char *binary, ssize_t *binary_length);
static EB_Error_Code eb_read_binary_gray_graphic(EB_Book *book,
    size_t binary_max_length, char *binary, ssize_t *binary_length);


/*
 * Initialize binary context of `book'.
 */
void
eb_initialize_binary_context(EB_Book *book)
{
    LOG(("in: eb_initialize_binary_context(book=%d)", (int)book->code));

    book->binary_context.code = EB_BINARY_INVALID;
    book->binary_context.zio = NULL;
    book->binary_context.location = -1;
    book->binary_context.size = 0;
    book->binary_context.cache_length = 0;
    book->binary_context.cache_offset = 0;
    book->binary_context.width = 0;

    LOG(("out: eb_initialize_binary_context()"));
}


/*
 * Finalize binary context of `book'.
 */
void
eb_finalize_binary_context(EB_Book *book)
{
    LOG(("in+out: eb_finalize_binary_context(book=%d)", (int)book->code));

    /* nothing to be done */
}


/*
 * Reset binary context of `book'.
 */
void
eb_reset_binary_context(EB_Book *book)
{
    LOG(("in: eb_reset_binary_context(book=%d)", (int)book->code));

    eb_initialize_binary_context(book);

    LOG(("out: eb_reset_binary_context()"));
}


/*
 * Template of BMP preamble for 2 colors monochrome graphic.
 */
#define MONO_BMP_PREAMBLE_LENGTH	62

static const unsigned char mono_bmp_preamble[] = {
    /* Type. */
    'B', 'M',

    /* File size. (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /* Reserved. */
    0x00, 0x00, 0x00, 0x00,

    /* Offset of bitmap bits part. */
    0x3e, 0x00, 0x00, 0x00,

    /* Size of bitmap info part. */
    0x28, 0x00, 0x00, 0x00,

    /* Width. (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /* Height. (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /* Planes. */
    0x01, 0x00,

    /* Bits per pixels. */
    0x01, 0x00,

    /* Compression mode. */
    0x00, 0x00, 0x00, 0x00,

    /* Size of bitmap bits part. (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /* X Pixels per meter. */
    0x6d, 0x0b, 0x00, 0x00,

    /* Y Pixels per meter. */
    0x6d, 0x0b, 0x00, 0x00,

    /* Colors */
    0x02, 0x00, 0x00, 0x00,

    /* Important colors */
    0x02, 0x00, 0x00, 0x00,

    /* RGB quad of color 0   RGB quad of color 1 */
    0xff, 0xff, 0xff, 0x00,  0x00, 0x00, 0x00, 0x00,
};

/*
 * Set monochrome bitmap picture as the current binary data.
 */
EB_Error_Code
eb_set_binary_mono_graphic(EB_Book *book, const EB_Position *position,
    int width, int height)
{
    EB_Error_Code error_code;
    EB_Binary_Context *context;
    EB_Position real_position;
    unsigned char *buffer_p;
    size_t line_pad_length;
    size_t data_size;
    size_t file_size;

    eb_lock(&book->lock);
    LOG(("in: eb_set_binary_mono_graphic(book=%d, position={%d,%d}, \
width=%d, height=%d)",
	(int)book->code, position->page, position->offset, width, height));

    eb_reset_binary_context(book);

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Current subbook must have a graphic file.
     */
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_SUCH_BINARY;
	goto failed;
    }

    /*
     * If both width and height are 0,
     * we get real width, height and position of the graphic data.
     */
    if (position->page <= 0 || position->offset < 0) {
	error_code = EB_ERR_FAIL_SEEK_BINARY;
	goto failed;
    }

    if (width == 0 && height == 0) {
	char buffer[22];

	if (zio_lseek(&book->subbook_current->text_zio,
	    ((off_t) position->page - 1) * EB_SIZE_PAGE + position->offset,
	    SEEK_SET) < 0) {
	    error_code = EB_ERR_FAIL_SEEK_BINARY;
	    goto failed;
	}

	if (zio_read(&book->subbook_current->text_zio, buffer, 22) != 22) {
	    error_code = EB_ERR_FAIL_READ_BINARY;
	    goto failed;
	}
	if (eb_uint2(buffer) != 0x1f45 || eb_uint2(buffer + 4) != 0x1f31) {
	    error_code = EB_ERR_UNEXP_BINARY;
	    goto failed;
	}
	width = eb_bcd2(buffer + 8);
	height = eb_bcd2(buffer + 10);

	if (eb_uint2(buffer + 12) == 0x1f51) {
	    real_position.page = eb_bcd4(buffer + 14);
	    real_position.offset = eb_bcd2(buffer + 18);
	} else if (eb_uint2(buffer + 14) == 0x1f51) {
	    real_position.page = eb_bcd4(buffer + 16);
	    real_position.offset = eb_bcd2(buffer + 20);
	} else {
	    error_code = EB_ERR_UNEXP_BINARY;
	    goto failed;
	}

	position = &real_position;
    }

    if (width <= 0 || height <= 0) {
	error_code = EB_ERR_NO_SUCH_BINARY;
	goto failed;
    }

    /*
     * BMP requires that the number of bytes in a line must be multiple
     * of 4.  If not, 0x00 must be padded to end of each line.
     * `line_pad_length' (0...3) is the number of bytes to be padded.
     *
     * In case of EB_BINARY_MONO_GRAPHIC, a pixel is represented with
     * a bit.
     */
    if (width % 32 == 0)
	line_pad_length = 0;
    else if (width % 32 <= 8)
	line_pad_length = 3;
    else if (width % 32 <= 16)
	line_pad_length = 2;
    else if (width % 32 <= 24)
	line_pad_length = 1;
    else
	line_pad_length = 0;

    data_size = (width / 8 + line_pad_length) * height;
    file_size = data_size + MONO_BMP_PREAMBLE_LENGTH;

    /*
     * Set binary context.
     */
    context = &book->binary_context;
    context->code = EB_BINARY_MONO_GRAPHIC;
    context->zio = &book->subbook_current->text_zio;
    context->location = ((off_t) position->page - 1) * EB_SIZE_PAGE
	+ position->offset + (width + 7) / 8 * (height - 1);
    context->size = (width + 7) / 8 * height;
    context->offset = 0;
    context->cache_offset = 0;
    context->width = width;

    /*
     * Set BMP preamble.
     */
    context->cache_length = MONO_BMP_PREAMBLE_LENGTH;
    memcpy(context->cache_buffer, mono_bmp_preamble, MONO_BMP_PREAMBLE_LENGTH);

    buffer_p = (unsigned char *)context->cache_buffer + 2;
    *buffer_p++ =  file_size        & 0xff;
    *buffer_p++ = (file_size >> 8)  & 0xff;
    *buffer_p++ = (file_size >> 16) & 0xff;
    *buffer_p++ = (file_size >> 24) & 0xff;

    buffer_p = (unsigned char *)context->cache_buffer + 18;
    *buffer_p++ =  width        & 0xff;
    *buffer_p++ = (width >> 8)  & 0xff;
    *buffer_p++ = (width >> 16) & 0xff;
    *buffer_p++ = (width >> 24) & 0xff;

    *buffer_p++ =  height        & 0xff;
    *buffer_p++ = (height >> 8)  & 0xff;
    *buffer_p++ = (height >> 16) & 0xff;
    *buffer_p++ = (height >> 24) & 0xff;

    buffer_p = (unsigned char *)context->cache_buffer + 34;
    *buffer_p++ =  data_size        & 0xff;
    *buffer_p++ = (data_size >> 8)  & 0xff;
    *buffer_p++ = (data_size >> 16) & 0xff;
    *buffer_p++ = (data_size >> 24) & 0xff;

    /*
     * Seek graphic file.
     */
    if (zio_lseek(context->zio, context->location, SEEK_SET) < 0) {
	error_code = EB_ERR_FAIL_SEEK_BINARY;
	goto failed;
    }

    LOG(("out: eb_set_binary_mono_graphic() = %s",
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_reset_binary_context(book);
    LOG(("out: eb_set_binary_mono_graphic() = %s",
	eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Template of BMP preamble for gray scale graphic.
 */
#define GRAY_BMP_PREAMBLE_LENGTH	118

static const unsigned char gray_bmp_preamble[] = {
    /* Type. */
    'B', 'M',

    /* File size. (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /* Reserved. */
    0x00, 0x00, 0x00, 0x00,

    /* Offset of bitmap bits part. */
    0x3e, 0x00, 0x00, 0x00,

    /* Size of bitmap info part. */
    0x28, 0x00, 0x00, 0x00,

    /* Width. (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /* Height. (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /* Planes. */
    0x01, 0x00,

    /* Bits per pixels. */
    0x04, 0x00,

    /* Compression mode. */
    0x00, 0x00, 0x00, 0x00,

    /* Size of bitmap bits part. (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /* X Pixels per meter. */
    0x6d, 0x0b, 0x00, 0x00,

    /* Y Pixels per meter. */
    0x6d, 0x0b, 0x00, 0x00,

    /* Colors */
    0x10, 0x00, 0x00, 0x00,

    /* Important colors */
    0x10, 0x00, 0x00, 0x00,

    /* RGB quad of color 0x0   RGB quad of color 0x1 */
    0x00, 0x00, 0x00, 0x00,    0x11, 0x11, 0x11, 0x00,

    /* RGB quad of color 0x2   RGB quad of color 0x3 */
    0x22, 0x22, 0x22, 0x00,    0x33, 0x33, 0x33, 0x00,

    /* RGB quad of color 0x4   RGB quad of color 0x5 */
    0x44, 0x44, 0x44, 0x00,    0x55, 0x55, 0x55, 0x00,

    /* RGB quad of color 0x6   RGB quad of color 0x7 */
    0x66, 0x66, 0x66, 0x00,    0x77, 0x77, 0x77, 0x00,

    /* RGB quad of color 0x8   RGB quad of color 0x9 */
    0x88, 0x88, 0x88, 0x00,    0x99, 0x99, 0x99, 0x00,

    /* RGB quad of color 0xa   RGB quad of color 0xb */
    0xaa, 0xaa, 0xaa, 0x00,    0xbb, 0xbb, 0xbb, 0x00,

    /* RGB quad of color 0xc   RGB quad of color 0xd */
    0xcc, 0xcc, 0xcc, 0x00,    0xdd, 0xdd, 0xdd, 0x00,

    /* RGB quad of color 0xe   RGB quad of color 0xf */
    0xee, 0xee, 0xee, 0x00,    0xff, 0xff, 0xff, 0x00,
};

/*
 * Set monochrome bitmap picture as the current binary data.
 */
EB_Error_Code
eb_set_binary_gray_graphic(EB_Book *book, const EB_Position *position,
    int width, int height)
{
    EB_Error_Code error_code;
    EB_Binary_Context *context;
    EB_Position real_position;
    unsigned char *buffer_p;
    size_t line_pad_length;
    size_t data_size;
    size_t file_size;

    eb_lock(&book->lock);
    LOG(("in: eb_set_binary_gray_graphic(book=%d, position={%d,%d}, \
width=%d, height=%d)",
	(int)book->code, position->page, position->offset, width, height));

    eb_reset_binary_context(book);

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Current subbook must have a graphic file.
     */
    if (zio_file(&book->subbook_current->text_zio) < 0) {
	error_code = EB_ERR_NO_SUCH_BINARY;
	goto failed;
    }

    /*
     * If both width and height are 0,
     * we get real width, height and position of the graphic data.
     */
    if (position->page <= 0 || position->offset < 0) {
	error_code = EB_ERR_FAIL_SEEK_BINARY;
	goto failed;
    }

    if (width == 0 && height == 0) {
	char buffer[22];

	if (zio_lseek(&book->subbook_current->text_zio,
	    ((off_t) position->page - 1) * EB_SIZE_PAGE + position->offset,
	    SEEK_SET) < 0) {
	    error_code = EB_ERR_FAIL_SEEK_BINARY;
	    goto failed;
	}

	if (zio_read(&book->subbook_current->text_zio, buffer, 22) != 22) {
	    error_code = EB_ERR_FAIL_READ_BINARY;
	    goto failed;
	}
	if (eb_uint2(buffer) != 0x1f45
	    || eb_uint2(buffer + 4) != 0x1f31
	    || eb_uint2(buffer + 12) != 0x1f51
	    || eb_uint2(buffer + 20) != 0x1f65) {
	    error_code = EB_ERR_UNEXP_BINARY;
	    goto failed;
	}

	width = eb_bcd2(buffer + 8);
	height = eb_bcd2(buffer + 10);
	real_position.page = eb_bcd4(buffer + 14);
	real_position.offset = eb_bcd2(buffer + 18);
	position = &real_position;
    }

    if (width <= 0 || height <= 0) {
	error_code = EB_ERR_NO_SUCH_BINARY;
	goto failed;
    }

    /*
     * BMP requires that the number of bytes in a line must be multiple
     * of 4.  If not, 0x00 must be padded to end of each line.
     * `line_pad_length' (0...3) is the number of bytes to be padded.
     *
     * In case of EB_BINARY_GRAY_GRAPHIC, a pixel is represented with
     * 4 bits.
     */
    if (width % 8 == 0)
	line_pad_length = 0;
    else if (width % 8 <= 2)
	line_pad_length = 3;
    else if (width % 8 <= 4)
	line_pad_length = 2;
    else if (width % 8 <= 6)
	line_pad_length = 1;
    else
	line_pad_length = 0;

    data_size = (width / 2 + line_pad_length) * height;
    file_size = data_size + MONO_BMP_PREAMBLE_LENGTH;

    /*
     * Set binary context.
     */
    context = &book->binary_context;

    context->code = EB_BINARY_GRAY_GRAPHIC;
    context->zio = &book->subbook_current->text_zio;
    context->location = ((off_t) position->page - 1) * EB_SIZE_PAGE
	+ position->offset + (width + 1) / 2 * (height - 1);
    context->size = (width + 1) / 2 * height;
    context->offset = 0;
    context->cache_offset = 0;
    context->width = width;

    /*
     * Set BMP preamble.
     */
    context->cache_length = GRAY_BMP_PREAMBLE_LENGTH;
    memcpy(context->cache_buffer, gray_bmp_preamble,
	GRAY_BMP_PREAMBLE_LENGTH);

    buffer_p = (unsigned char *)context->cache_buffer + 2;
    *buffer_p++ =  file_size        & 0xff;
    *buffer_p++ = (file_size >> 8)  & 0xff;
    *buffer_p++ = (file_size >> 16) & 0xff;
    *buffer_p++ = (file_size >> 24) & 0xff;

    buffer_p = (unsigned char *)context->cache_buffer + 18;
    *buffer_p++ =  width        & 0xff;
    *buffer_p++ = (width >> 8)  & 0xff;
    *buffer_p++ = (width >> 16) & 0xff;
    *buffer_p++ = (width >> 24) & 0xff;

    *buffer_p++ =  height        & 0xff;
    *buffer_p++ = (height >> 8)  & 0xff;
    *buffer_p++ = (height >> 16) & 0xff;
    *buffer_p++ = (height >> 24) & 0xff;

    buffer_p = (unsigned char *)context->cache_buffer + 34;
    *buffer_p++ =  data_size        & 0xff;
    *buffer_p++ = (data_size >> 8)  & 0xff;
    *buffer_p++ = (data_size >> 16) & 0xff;
    *buffer_p++ = (data_size >> 24) & 0xff;

    /*
     * Seek graphic file.
     */
    if (zio_lseek(context->zio, context->location, SEEK_SET) < 0) {
	error_code = EB_ERR_FAIL_SEEK_BINARY;
	goto failed;
    }

    LOG(("out: eb_set_binary_gray_graphic() = %s",
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_reset_binary_context(book);
    LOG(("out: eb_set_binary_gray_graphic() = %s",
	eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Set WAVE sound as the current binary data.
 */
EB_Error_Code
eb_set_binary_wave(EB_Book *book, const EB_Position *start_position,
    const EB_Position *end_position)
{
    EB_Error_Code error_code;
    EB_Binary_Context *context;
    off_t start_location;
    off_t end_location;
    char temporary_buffer[4];

    eb_lock(&book->lock);
    LOG(("in: eb_set_binary_wave(book=%d, start_position={%d,%d}, \
end_position={%d,%d})",
	(int)book->code, start_position->page, start_position->offset,
	end_position->page, end_position->offset));

    eb_reset_binary_context(book);

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Current subbook must have a sound file.
     */
    if (zio_file(&book->subbook_current->sound_zio) < 0) {
	error_code = EB_ERR_NO_SUCH_BINARY;
	goto failed;
    }

    /*
     * Set binary context.
     */
    if (start_position->page <= 0 || start_position->offset < 0) {
	error_code = EB_ERR_FAIL_SEEK_BINARY;
	goto failed;
    }
    if (end_position->page <= 0 || end_position->offset < 0) {
	error_code = EB_ERR_FAIL_SEEK_BINARY;
	goto failed;
    }

    start_location = ((off_t) start_position->page - 1) * EB_SIZE_PAGE
	+ start_position->offset;
    end_location   = ((off_t) end_position->page - 1)   * EB_SIZE_PAGE
	+ end_position->offset;

    context = &book->binary_context;

    context->code = EB_BINARY_WAVE;
    context->zio = &book->subbook_current->sound_zio;
    context->location = start_location;
    if (start_location < end_location)
	context->size = end_location - start_location + 1;
    else {
	error_code = EB_ERR_UNEXP_BINARY;
	goto failed;
    }
    context->offset = 0;

    /*
     * Read 4bytes from the sound file to check whether the sound
     * data contains a header part or not.
     *
     * If the read data is "fmt ", the wave data has a header part.
     * Otherwise, we must read a header in another location.
     *
     * The wave data consists of:
     *
     *     "RIFF" wave-size(4bytes) "WAVE" header-fragment(28bytes)
     *     data-part-size(4bytes) data
     *
     * wave-size      = "WAVE" + header-fragment + data-part-size + data
     *                = 4 + 28 + 4 + data
     *                = 36 + data
     * data-part-size = length(data)
     */
    if (zio_lseek(context->zio, context->location, SEEK_SET) < 0) {
	error_code = EB_ERR_FAIL_SEEK_BINARY;
	goto failed;
    }
    if (zio_read(context->zio, temporary_buffer, 4) != 4) {
	error_code = EB_ERR_FAIL_READ_BINARY;
	goto failed;
    }

    if (memcmp(temporary_buffer, "fmt ", 4) == 0) {
	memcpy(context->cache_buffer + 12, temporary_buffer, 4);
	if (zio_read(context->zio, context->cache_buffer + 16, 28) != 28) {
	    error_code = EB_ERR_FAIL_READ_BINARY;
	    goto failed;
	}
	if (context->size >= 32)
	    context->size -= 32;
	else
	    context->size = 0;
    } else {
	if (zio_lseek(context->zio,
	    ((off_t) book->subbook_current->sound.start_page - 1)
		* EB_SIZE_PAGE + 32, SEEK_SET) < 0) {
	    error_code = EB_ERR_FAIL_SEEK_BINARY;
	    goto failed;
	}
	if (zio_read(context->zio, context->cache_buffer + 12, 28) != 28) {
	    error_code = EB_ERR_FAIL_SEEK_BINARY;
	    goto failed;
	}

	*(unsigned char *)(context->cache_buffer + 40)
	    = (context->size)       & 0xff;
	*(unsigned char *)(context->cache_buffer + 41)
	    = (context->size >> 8)  & 0xff;
	*(unsigned char *)(context->cache_buffer + 42)
	    = (context->size >> 16) & 0xff;
	*(unsigned char *)(context->cache_buffer + 43)
	    = (context->size >> 24) & 0xff;


	/*
	 * Seek sound file, again.
	 */
	if (zio_lseek(context->zio, context->location, SEEK_SET) < 0) {
	    error_code = EB_ERR_FAIL_SEEK_BINARY;
	    goto failed;
	}
    }
    context->cache_length = 44;

    /*
     * Read and compose a WAVE header.
     */
    memcpy(context->cache_buffer, "RIFF", 4);

    *(unsigned char *)(context->cache_buffer + 4)
	= (context->size + 36)         & 0xff;
    *(unsigned char *)(context->cache_buffer + 5)
	= ((context->size + 36) >> 8)  & 0xff;
    *(unsigned char *)(context->cache_buffer + 6)
	= ((context->size + 36) >> 16) & 0xff;
    *(unsigned char *)(context->cache_buffer + 7)
	= ((context->size + 36) >> 24) & 0xff;

    memcpy(context->cache_buffer + 8, "WAVE", 4);

    LOG(("out: eb_set_binary_wave() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_reset_binary_context(book);
    LOG(("out: eb_set_binary_wave() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Length of the color graphic header.
 */
#define EB_COLOR_GRAPHIC_HEADER_LENGTH	8

/*
 * Set color graphic (BMP or JPEG) as the current binary data.
 */
EB_Error_Code
eb_set_binary_color_graphic(EB_Book *book, const EB_Position *position)
{
    EB_Error_Code error_code;
    EB_Binary_Context *context;
    char buffer[EB_COLOR_GRAPHIC_HEADER_LENGTH];

    eb_lock(&book->lock);
    LOG(("in: eb_set_binary_color_graphic(book=%d, position={%d,%d})",
	(int)book->code, position->page, position->offset));

    eb_reset_binary_context(book);

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Current subbook must have a graphic file.
     */
    if (zio_file(&book->subbook_current->graphic_zio) < 0) {
	error_code = EB_ERR_NO_SUCH_BINARY;
	goto failed;
    }

    /*
     * Set binary context.
     */
    if (position->page <= 0 || position->offset < 0) {
	error_code = EB_ERR_FAIL_SEEK_BINARY;
	goto failed;
    }

    context = &book->binary_context;
    context->code = EB_BINARY_COLOR_GRAPHIC;
    context->zio = &book->subbook_current->graphic_zio;
    context->location = ((off_t) position->page - 1) * EB_SIZE_PAGE
	+ position->offset;
    context->offset = 0;
    context->cache_length = 0;
    context->cache_offset = 0;

    /*
     * Seek graphic file.
     */
    if (zio_lseek(context->zio, context->location, SEEK_SET) < 0) {
	error_code = EB_ERR_FAIL_SEEK_BINARY;
	goto failed;
    }

    /*
     * Read header of the graphic data.
     * Note that EB* JPEG file lacks the header.
     */
    if (zio_read(context->zio, buffer, EB_COLOR_GRAPHIC_HEADER_LENGTH)
	!= EB_COLOR_GRAPHIC_HEADER_LENGTH) {
	error_code = EB_ERR_FAIL_READ_BINARY;
	goto failed;
    }

    if (memcmp(buffer, "data", 4) == 0) {
	context->size = eb_uint4_le(buffer + 4);
	context->location += EB_COLOR_GRAPHIC_HEADER_LENGTH;
    } else {
	context->size = 0;
	if (zio_lseek(context->zio, context->location, SEEK_SET) < 0) {
	    error_code = EB_ERR_FAIL_SEEK_BINARY;
	    goto failed;
	}
    }

    LOG(("out: eb_set_binary_color_graphic() = %s",
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_reset_binary_context(book);
    LOG(("out: eb_set_binary_color_graphic() = %s",
	eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Set MPEG movie as the current binary data.
 */
EB_Error_Code
eb_set_binary_mpeg(EB_Book *book, const unsigned int *argv)
{
    /*
     * `movie_file_name' is base name, and `movie_path_name' is absolute
     * path of the movie.
     */
    char movie_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
    char movie_path_name[EB_MAX_PATH_LENGTH + 1];
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    Zio_Code zio_code;

    eb_lock(&book->lock);
    LOG(("in: eb_set_binary_mpeg(book=%d)", (int)book->code));

    eb_reset_binary_context(book);

    /*
     * Current subbook must have been set.
     */
    subbook = book->subbook_current;
    if (subbook == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Open the movie file and set binary context.
     */
    if (eb_compose_movie_file_name(argv, movie_file_name) != EB_SUCCESS) {
	error_code = EB_ERR_NO_SUCH_BINARY;
	goto failed;
    }
    LOG(("aux: eb_set_binary_mpeg(): movie_file_name=%s", movie_file_name));

    if (eb_find_file_name3(book->path, subbook->directory_name,
	subbook->movie_directory_name, movie_file_name, movie_file_name)
	!= EB_SUCCESS) {
	error_code = EB_ERR_NO_SUCH_BINARY;
	goto failed;
    }
    eb_compose_path_name3(book->path, subbook->directory_name,
	subbook->movie_directory_name, movie_file_name, movie_path_name);
    eb_path_name_zio_code(movie_path_name, ZIO_PLAIN, &zio_code);

    if (zio_open(&subbook->movie_zio, movie_path_name, zio_code) < 0) {
	subbook = NULL;
	error_code = EB_ERR_FAIL_OPEN_BINARY;
	goto failed;
    }

    book->binary_context.code = EB_BINARY_MPEG;
    book->binary_context.zio = &book->subbook_current->movie_zio;
    book->binary_context.location = 0;
    book->binary_context.size = 0;
    book->binary_context.offset = 0;
    book->binary_context.cache_length = 0;
    book->binary_context.cache_offset = 0;

    LOG(("out: eb_set_binary_mpeg() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    eb_reset_binary_context(book);
    LOG(("out: eb_set_binary_mpeg() = %s", eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Read binary data.
 */
EB_Error_Code
eb_read_binary(book, binary_max_length, binary, binary_length)
    EB_Book *book;
    size_t binary_max_length;
    char *binary;
    ssize_t *binary_length;

{
    EB_Error_Code error_code;

    eb_lock(&book->lock);
    LOG(("in: eb_read_binary(book=%d, binary_max_length=%ld)",
	(int)book->code, (long)binary_max_length));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }

    /*
     * Return immediately if `binary_max_length' is 0.
     */
    *binary_length = 0;

    switch (book->binary_context.code) {
    case EB_BINARY_COLOR_GRAPHIC:
    case EB_BINARY_MPEG:
	error_code = eb_read_binary_generic(book, binary_max_length, binary,
	    binary_length);
	break;
    case EB_BINARY_WAVE:
	error_code = eb_read_binary_wave(book, binary_max_length,
	    binary, binary_length);
	break;
    case EB_BINARY_MONO_GRAPHIC:
	error_code = eb_read_binary_mono_graphic(book, binary_max_length,
	    binary, binary_length);
	break;
    case EB_BINARY_GRAY_GRAPHIC:
	error_code = eb_read_binary_gray_graphic(book, binary_max_length,
	    binary, binary_length);
	break;
    default:
	error_code = EB_ERR_NO_CUR_BINARY;
	goto failed;
    }
    if (error_code != EB_SUCCESS)
	goto failed;

    LOG(("out: eb_read_binary(binary_length=%ld) = %s", (long)*binary_length,
	eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *binary_length = -1;
    eb_reset_binary_context(book);
    LOG(("out: eb_read_binary() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Read generic binary data.
 * This function is used for reading JPEG or BMP picture, and data part
 * of WAVE sound.
 */
static EB_Error_Code
eb_read_binary_generic(EB_Book *book, size_t binary_max_length,
    char *binary, ssize_t *binary_length)
{
    EB_Error_Code error_code;
    EB_Binary_Context *context;
    char *binary_p = binary;
    size_t read_length = 0;
    ssize_t read_result;

    LOG(("in: eb_read_binary_generic(book=%d, binary_max_length=%ld)",
	(int)book->code, (long)binary_max_length));

    *binary_length = 0;
    context = &book->binary_context;

    /*
     * Return immediately if `binary_max_length' is 0.
     */
    if (binary_max_length == 0)
	goto succeeded;

    /*
     * Read binary data if it is remained.
     * If context->size is 0, the binary data size is unknown.
     */
    if (0 < context->size && context->size <= context->offset)
	goto succeeded;

    if (context->size == 0)
	read_length = binary_max_length - *binary_length;
    else if (binary_max_length - *binary_length
	< context->size - context->offset)
	read_length = binary_max_length - *binary_length;
    else
	read_length = context->size - context->offset;

    read_result = zio_read(context->zio, binary_p, read_length);
    if ((0 < context->size && read_result != read_length) || read_result < 0) {
	error_code = EB_ERR_FAIL_READ_BINARY;
	goto failed;
    }

    *binary_length += read_result;
    context->offset += read_result;

  succeeded:
    LOG(("out: eb_read_binary_generic(binary_length=%ld) = %s",
	(long)*binary_length, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_read_binary_generic() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Read WAVE sound data.
 */
static EB_Error_Code
eb_read_binary_wave(EB_Book *book, size_t binary_max_length, char *binary,
    ssize_t *binary_length)
{
    EB_Error_Code error_code;
    EB_Binary_Context *context;
    char *binary_p = binary;
    size_t copy_length = 0;

    LOG(("in: eb_read_binary_wave(book=%d, binary_max_length=%ld)",
	(int)book->code, (long)binary_max_length));

    *binary_length = 0;
    context = &book->binary_context;

    /*
     * Return immediately if `binary_max_length' is 0.
     */
    if (binary_max_length == 0)
	goto succeeded;

    /*
     * Copy cached data (header part) to `binary' if exists.
     */
    if (0 < context->cache_length) {
	if (binary_max_length < context->cache_length - context->cache_offset)
	    copy_length = binary_max_length;
	else
	    copy_length = context->cache_length - context->cache_offset;

	memcpy(binary_p, context->cache_buffer + context->cache_offset,
	    copy_length);
	binary_p += copy_length;
	context->cache_offset += copy_length;

	if (context->cache_length <= context->cache_offset)
	    context->cache_length = 0;

	if (binary_max_length <= *binary_length)
	    goto succeeded;
    }

    error_code = eb_read_binary_generic(book, binary_max_length - copy_length,
	binary_p, binary_length);
    if (error_code !=EB_SUCCESS)
	goto failed;
    *binary_length += copy_length;

  succeeded:
    LOG(("out: eb_read_binary_wave(binary_length=%ld) = %s",
	(long)*binary_length, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_read_binary_wave() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Read monochrome graphic data.
 * The function also convert the graphic data to BMP.
 */
static EB_Error_Code
eb_read_binary_mono_graphic(EB_Book *book, size_t binary_max_length,
    char *binary, ssize_t *binary_length)
{
    EB_Error_Code error_code;
    EB_Binary_Context *context;
    unsigned char *binary_p = (unsigned char *)binary;
    size_t copy_length = 0;
    size_t read_length = 0;
    size_t line_length;
    size_t line_pad_length;

    LOG(("in: eb_read_binary_mono_graphic(book=%d, binary_max_length=%ld)",
	(int)book->code, (long)binary_max_length));

    *binary_length = 0;
    context = &book->binary_context;

    line_length = (context->width + 7) / 8;

    if (context->width % 32 == 0)
	line_pad_length = 0;
    else if (context->width % 32 <= 8)
	line_pad_length = 3;
    else if (context->width % 32 <= 16)
	line_pad_length = 2;
    else if (context->width % 32 <= 24)
	line_pad_length = 1;
    else
	line_pad_length = 0;

    /*
     * Return immediately if `binary_max_length' is 0.
     */
    if (binary_max_length == 0)
	goto succeeded;

    for (;;) {
	/*
	 * Copy cached data to `binary' if exists.
	 */
	if (0 < context->cache_length) {
	    if (binary_max_length - *binary_length
		< context->cache_length - context->cache_offset)
		copy_length = binary_max_length - *binary_length;
	    else
		copy_length = context->cache_length - context->cache_offset;

	    memcpy(binary_p, context->cache_buffer + context->cache_offset,
		copy_length);
	    binary_p += copy_length;
	    *binary_length += copy_length;
	    context->cache_offset += copy_length;

	    if (context->cache_length <= context->cache_offset)
		context->cache_length = 0;

	    if (binary_max_length <= *binary_length)
		goto succeeded;
	}

	/*
	 * Read binary data if it is remained.
	 * If padding is needed, read each line.
	 */
	read_length = line_length - context->offset % line_length;
	if (context->size - context->offset < read_length)
	    read_length = context->size - context->offset;
	if (binary_max_length - *binary_length < read_length)
	    read_length = binary_max_length - *binary_length;
	if (read_length == 0)
	    goto succeeded;

	/*
	 * Read binary data.
	 */
	if (context->offset != 0
	    && context->offset % line_length == 0
	    && zio_lseek(context->zio, (off_t) line_length * -2, SEEK_CUR)
	    < 0) {
	    error_code = EB_ERR_FAIL_SEEK_BINARY;
	    goto failed;
	}
	if (zio_read(context->zio, (char *)binary_p, read_length)
	    != read_length) {
	    error_code = EB_ERR_FAIL_READ_BINARY;
	    goto failed;
	}

	*binary_length += read_length;
	context->offset += read_length;
	binary_p += read_length;

	/*
	 * Pad 0x00 to BMP if needed.
	 */
	if (context->offset % line_length == 0) {
	    if (0 < line_pad_length) {
		if (binary_max_length - *binary_length < line_pad_length) {
		    memset(context->cache_buffer, 0, line_pad_length);
		    context->cache_length = line_pad_length;
		    context->cache_offset = 0;
		} else {
		    memset(binary_p, 0, line_pad_length);
		    binary_p += line_pad_length;
		    *binary_length += line_pad_length;
		}
	    }
	}
    }

  succeeded:
    LOG(("out: eb_read_binary_mono_graphic(binary_length=%ld) = %s",
	(long)*binary_length, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_read_binary_mono_graphic() = %s",
	eb_error_string(error_code)));
    return error_code;
}


/*
 * Read gray scale graphic data.
 * The function also convert the graphic data to BMP.
 */
static EB_Error_Code
eb_read_binary_gray_graphic(EB_Book *book, size_t binary_max_length,
    char *binary, ssize_t *binary_length)
{
    EB_Error_Code error_code;
    EB_Binary_Context *context;
    unsigned char *binary_p = (unsigned char *)binary;
    size_t copy_length = 0;
    size_t read_length = 0;
    size_t line_length;
    size_t line_pad_length;

    LOG(("in: eb_read_binary_gray_graphic(book=%d, binary_max_length=%ld)",
	(int)book->code, (long)binary_max_length));

    *binary_length = 0;
    context = &book->binary_context;

    line_length = (context->width + 1) / 2;

    if (context->width % 8 == 0)
	line_pad_length = 0;
    else if (context->width % 8 <= 2)
	line_pad_length = 3;
    else if (context->width % 8 <= 4)
	line_pad_length = 2;
    else if (context->width % 8 <= 6)
	line_pad_length = 1;
    else
	line_pad_length = 0;

    /*
     * Return immediately if `binary_max_length' is 0.
     */
    if (binary_max_length == 0)
	goto succeeded;

    for (;;) {
	/*
	 * Copy cached data to `binary' if exists.
	 */
	if (0 < context->cache_length) {
	    if (binary_max_length - *binary_length
		< context->cache_length - context->cache_offset)
		copy_length = binary_max_length - *binary_length;
	    else
		copy_length = context->cache_length - context->cache_offset;

	    memcpy(binary_p, context->cache_buffer + context->cache_offset,
		copy_length);
	    binary_p += copy_length;
	    *binary_length += copy_length;
	    context->cache_offset += copy_length;

	    if (context->cache_length <= context->cache_offset)
		context->cache_length = 0;

	    if (binary_max_length <= *binary_length)
		goto succeeded;
	}

	/*
	 * Read binary data if it is remained.
	 * If padding is needed, read each line.
	 */
	read_length = line_length - context->offset % line_length;
	if (context->size - context->offset < read_length)
	    read_length = context->size - context->offset;
	if (binary_max_length - *binary_length < read_length)
	    read_length = binary_max_length - *binary_length;
	if (read_length == 0)
	    goto succeeded;

	/*
	 * Read binary data.
	 */
	if (context->offset != 0
	    && context->offset % line_length == 0
	    && zio_lseek(context->zio, (off_t) line_length * -2, SEEK_CUR)
	    < 0) {
		error_code = EB_ERR_FAIL_SEEK_BINARY;
		goto failed;
	}
	if (zio_read(context->zio, (char *)binary_p, read_length)
	    != read_length) {
	    error_code = EB_ERR_FAIL_READ_BINARY;
	    goto failed;
	}

	*binary_length += read_length;
	context->offset += read_length;
	binary_p += read_length;

	/*
	 * Pad 0x00 to BMP if needed.
	 */
	if (context->offset % line_length == 0) {
	    if (0 < line_pad_length) {
		if (binary_max_length - *binary_length < line_pad_length) {
		    memset(context->cache_buffer, 0, line_pad_length);
		    context->cache_length = line_pad_length;
		    context->cache_offset = 0;
		} else {
		    memset(binary_p, 0, line_pad_length);
		    binary_p += line_pad_length;
		    *binary_length += line_pad_length;
		}
	    }
	}
    }

  succeeded:
    LOG(("out: eb_read_binary_gray_graphic(binary_length=%ld) = %s",
	(long)*binary_length, eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_read_binary_gray_graphic() = %s",
	eb_error_string(error_code)));
    return error_code;
}


/*
 * Unset current binary.
 */
void
eb_unset_binary(EB_Book *book)
{
    eb_lock(&book->lock);
    LOG(("in: eb_unset_binary(book=%d)", (int)book->code));

    eb_reset_binary_context(book);

    LOG(("out: eb_unset_binary()"));
    eb_unlock(&book->lock);
}


