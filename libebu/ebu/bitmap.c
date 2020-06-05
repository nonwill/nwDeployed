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

#ifdef ENABLE_LIBDEFLATE
#include <libdeflate.h>
#else
#include <zlib.h>
#endif

/*
 * Unexported functions.
 */
static unsigned long png_crc(const char *buf, size_t len);
static int png_compress(const char *src, int width, int height, char *dest,
    size_t *dest_len);


/*
 * Return required buffer size for a narrow font character converted
 * to XBM image format.
 */
EB_Error_Code
eb_narrow_font_xbm_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_narrow_font_xbm_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_NARROW_FONT_16_XBM;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_NARROW_FONT_24_XBM;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_NARROW_FONT_30_XBM;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_NARROW_FONT_48_XBM;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_narrow_font_xbm_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_narrow_font_xbm_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return required buffer size for a narrow font character converted
 * to XPM image format.
 */
EB_Error_Code
eb_narrow_font_xpm_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_narrow_font_xpm_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_NARROW_FONT_16_XPM;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_NARROW_FONT_24_XPM;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_NARROW_FONT_30_XPM;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_NARROW_FONT_48_XPM;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_narrow_font_xpm_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_narrow_font_xpm_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return required buffer size for a narrow font character converted
 * to GIF image format.
 */
EB_Error_Code
eb_narrow_font_gif_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_narrow_font_gif_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_NARROW_FONT_16_GIF;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_NARROW_FONT_24_GIF;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_NARROW_FONT_30_GIF;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_NARROW_FONT_48_GIF;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_narrow_font_gif_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_narrow_font_gif_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return required buffer size for a narrow font character converted
 * to BMP image format.
 */
EB_Error_Code
eb_narrow_font_bmp_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_narrow_font_bmp_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_NARROW_FONT_16_BMP;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_NARROW_FONT_24_BMP;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_NARROW_FONT_30_BMP;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_NARROW_FONT_48_BMP;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_narrow_font_bmp_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_narrow_font_bmp_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return required buffer size for a narrow font character converted
 * to PNG image format.
 */
EB_Error_Code
eb_narrow_font_png_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_narrow_font_png_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_NARROW_FONT_16_PNG;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_NARROW_FONT_24_PNG;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_NARROW_FONT_30_PNG;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_NARROW_FONT_48_PNG;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_narrow_font_png_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_narrow_font_png_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return required buffer size for a wide font character converted
 * to XBM image format.
 */
EB_Error_Code
eb_wide_font_xbm_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_wide_font_xbm_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_WIDE_FONT_16_XBM;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_WIDE_FONT_24_XBM;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_WIDE_FONT_30_XBM;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_WIDE_FONT_48_XBM;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_wide_font_xbm_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_wide_font_xbm_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return required buffer size for a wide font character converted
 * to XPM image format.
 */
EB_Error_Code
eb_wide_font_xpm_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_wide_font_xpm_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_WIDE_FONT_16_XPM;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_WIDE_FONT_24_XPM;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_WIDE_FONT_30_XPM;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_WIDE_FONT_48_XPM;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_wide_font_xpm_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_wide_font_xpm_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return required buffer size for a wide font character converted
 * to GIF image format.
 */
EB_Error_Code
eb_wide_font_gif_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_wide_font_gif_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_WIDE_FONT_16_GIF;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_WIDE_FONT_24_GIF;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_WIDE_FONT_30_GIF;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_WIDE_FONT_48_GIF;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_wide_font_gif_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_wide_font_gif_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return required buffer size for a wide font character converted
 * to BMP image format.
 */
EB_Error_Code
eb_wide_font_bmp_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_wide_font_bmp_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_WIDE_FONT_16_BMP;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_WIDE_FONT_24_BMP;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_WIDE_FONT_30_BMP;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_WIDE_FONT_48_BMP;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_wide_font_bmp_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_wide_font_bmp_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Return required buffer size for a wide font character converted
 * to PNG image format.
 */
EB_Error_Code
eb_wide_font_png_size(EB_Font_Code height, size_t *size)
{
    EB_Error_Code error_code;

    LOG(("in: eb_wide_font_png_size(height=%d)", (int)height));

    switch (height) {
    case EB_FONT_16:
        *size = EB_SIZE_WIDE_FONT_16_PNG;
	break;
    case EB_FONT_24:
        *size = EB_SIZE_WIDE_FONT_24_PNG;
	break;
    case EB_FONT_30:
        *size = EB_SIZE_WIDE_FONT_30_PNG;
	break;
    case EB_FONT_48:
        *size = EB_SIZE_WIDE_FONT_48_PNG;
	break;
    default:
	error_code = EB_ERR_NO_SUCH_FONT;
	goto failed;
    }

    LOG(("out: eb_wide_font_png_size(size=%ld) = %s", (long)*size,
	eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    *size = 0;
    LOG(("out: eb_wide_font_png_size() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * The maximum number of octets in a line in a XBM file.
 */
#define XBM_MAX_OCTETS_A_LINE		12

/*
 * The base name of a XBM file.
 */
#define XBM_BASE_NAME			"default"

/*
 * Convert a bitmap image to XBM format.
 *
 * It requires four arguements.  `xbm' is buffer to store the XBM
 * image data.  `bitmap', `width', and `height' are bitmap data,
 * width, and height of the bitmap image.
 */
EB_Error_Code
eb_bitmap_to_xbm(const char *bitmap, int width, int height, char *xbm,
    size_t *xbm_length)
{
    char *xbm_p = xbm;
    const unsigned char *bitmap_p = (const unsigned char *)bitmap;
    int bitmap_size = (width + 7) / 8 * height;
    int hex;
    int i;

    LOG(("in: eb_bitmap_to_xbm(width=%d, height=%d)", width, height));

    /*
     * Output a header.
     */
    sprintf(xbm_p, "#define %s_width %4d\n", XBM_BASE_NAME, width);
    xbm_p = strchr(xbm_p, '\n') + 1;
    sprintf(xbm_p, "#define %s_height %4d\n", XBM_BASE_NAME, height);
    xbm_p = strchr(xbm_p, '\n') + 1;
    sprintf(xbm_p, "static unsigned char %s_bits[] = {\n", XBM_BASE_NAME);
    xbm_p = strchr(xbm_p, '\n') + 1;

    /*
     * Output image data.
     */
    for (i = 0; i < bitmap_size; i++) {
	hex = 0;
        hex |= (*bitmap_p & 0x80) ? 0x01 : 0x00;
        hex |= (*bitmap_p & 0x40) ? 0x02 : 0x00;
        hex |= (*bitmap_p & 0x20) ? 0x04 : 0x00;
        hex |= (*bitmap_p & 0x10) ? 0x08 : 0x00;
        hex |= (*bitmap_p & 0x08) ? 0x10 : 0x00;
        hex |= (*bitmap_p & 0x04) ? 0x20 : 0x00;
        hex |= (*bitmap_p & 0x02) ? 0x40 : 0x00;
        hex |= (*bitmap_p & 0x01) ? 0x80 : 0x00;
	bitmap_p++;

	if (i % XBM_MAX_OCTETS_A_LINE != 0) {
	    sprintf(xbm_p, ", 0x%02x", hex);
	    xbm_p += 6;
	} else if (i == 0) {
	    sprintf(xbm_p, "   0x%02x", hex);
	    xbm_p += 7;
	} else {
	    sprintf(xbm_p, ",\n   0x%02x", hex);
	    xbm_p += 9;
	}
    }

    /*
     * Output a footer.
     */
    memcpy(xbm_p, "};\n", 3);
    xbm_p += 3;

    *xbm_length = xbm_p - xbm;

    LOG(("out: eb_bitmap_to_xbm(xbm_length=%ld) = %s",
	(long)(xbm_p - xbm), eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;
}


/*
 * The base name of a XPM file.
 */
#define XPM_BASE_NAME		"default"

/*
 * The foreground and background colors of XPM image.
 */
#define XPM_FOREGROUND_COLOR		"Black"
#define XPM_BACKGROUND_COLOR		"None"

/*
 * Convert a bitmap image to XPM format.
 *
 * It requires four arguements.  `xpm' is buffer to store the XPM
 * image data.  `bitmap', `width', and `height' are bitmap data,
 * width, and height of the bitmap image.
 */
EB_Error_Code
eb_bitmap_to_xpm(const char *bitmap, int width, int height, char *xpm,
    size_t *xpm_length)
{
    char *xpm_p = xpm;
    const unsigned char *bitmap_p = (const unsigned char *)bitmap;
    int i, j;

    LOG(("in: eb_bitmap_to_xpm(width=%d, height=%d)", width, height));

    /*
     * Output a header.
     */
    sprintf(xpm_p, "/* XPM */\n");
    xpm_p = strchr(xpm_p, '\n') + 1;

    sprintf(xpm_p, "static char * %s[] = {\n", XPM_BASE_NAME);
    xpm_p = strchr(xpm_p, '\n') + 1;

    sprintf(xpm_p, "\"%d %d 2 1\",\n", width, height);
    xpm_p = strchr(xpm_p, '\n') + 1;

    sprintf(xpm_p, "\" 	c %s\",\n", XPM_BACKGROUND_COLOR);
    xpm_p = strchr(xpm_p, '\n') + 1;

    sprintf(xpm_p, "\". 	c %s\",\n", XPM_FOREGROUND_COLOR);
    xpm_p = strchr(xpm_p, '\n') + 1;

    /*
     * Output image data.
     */
    for (i = 0; i < height; i++) {
	if (0 < i) {
	    strcpy(xpm_p, "\",\n\"");
	    xpm_p += 4;
	} else {
	    *xpm_p++ = '\"';
	}

	for (j = 0; j + 7 < width; j += 8, bitmap_p++) {
	    *xpm_p++ = (*bitmap_p & 0x80) ? '.' : ' ';
	    *xpm_p++ = (*bitmap_p & 0x40) ? '.' : ' ';
	    *xpm_p++ = (*bitmap_p & 0x20) ? '.' : ' ';
	    *xpm_p++ = (*bitmap_p & 0x10) ? '.' : ' ';
	    *xpm_p++ = (*bitmap_p & 0x08) ? '.' : ' ';
	    *xpm_p++ = (*bitmap_p & 0x04) ? '.' : ' ';
	    *xpm_p++ = (*bitmap_p & 0x02) ? '.' : ' ';
	    *xpm_p++ = (*bitmap_p & 0x01) ? '.' : ' ';
	}

	if (j < width) {
	    if (j++ < width)
		*xpm_p++ = (*bitmap_p & 0x80) ? '.' : ' ';
	    if (j++ < width)
		*xpm_p++ = (*bitmap_p & 0x40) ? '.' : ' ';
	    if (j++ < width)
		*xpm_p++ = (*bitmap_p & 0x20) ? '.' : ' ';
	    if (j++ < width)
		*xpm_p++ = (*bitmap_p & 0x10) ? '.' : ' ';
	    if (j++ < width)
		*xpm_p++ = (*bitmap_p & 0x08) ? '.' : ' ';
	    if (j++ < width)
		*xpm_p++ = (*bitmap_p & 0x04) ? '.' : ' ';
	    if (j++ < width)
		*xpm_p++ = (*bitmap_p & 0x02) ? '.' : ' ';
	    if (j++ < width)
		*xpm_p++ = (*bitmap_p & 0x01) ? '.' : ' ';
	    bitmap_p++;
	}
    }

    /*
     * Output a footer.
     */
    memcpy(xpm_p, "\"};\n", 4);
    xpm_p += 4;

    if (xpm_length != NULL)
	*xpm_length = xpm_p - xpm;

    LOG(("out: eb_bitmap_to_xpm(xpm_length=%ld) = %s",
	(long)(xpm_p - xpm), eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;
}


/*
 * The Foreground and background colors of GIF image.
 */
#define GIF_FOREGROUND_COLOR		0x000000
#define GIF_BACKGROUND_COLOR		0xffffff

/*
 * The preamble of GIF image.
 */
#define GIF_PREAMBLE_LENGTH	38

static const unsigned char gif_preamble[GIF_PREAMBLE_LENGTH] = {
    /*
     * Header. (6 bytes)
     */
    'G', 'I', 'F', '8', '9', 'a',

    /*
     * Logical Screen Descriptor. (7 bytes)
     *   global color table flag = 1.
     *   color resolution = 1 - 1 = 0.
     *   sort flag = 0.
     *   size of global color table = 1 - 1 = 0.
     *   background color index = 0.
     *   the pixel aspect ratio = 0 (unused)
     * Logical screen width and height are set at run time.
     */
    0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,

    /*
     * Global Color Table. (6 bytes)
     * These are set at run time.
     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /*
     * Graphic Control Extension. (8 bytes)
     *   disposal method = 0.
     *   user input flag = 0.
     *   transparency flag = 1.
     *   delay time = 0.
     *   transparent color index = 0.
     */
    0x21, 0xf9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,

    /*
     * Image Descriptor. (10 bytes)
     *   image left position = 0.
     *   image top position = 0.
     *   local color table flag = 0.
     *   interlace flag = 0.
     *   sort flag = 0.
     *   size of local color table = 0.
     * Image width and height are set at run time.
     */
    0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /*
     * Code size. (1byte)
     */
    0x03
};


/*
 * Convert a bitmap image to GIF format.
 *
 * It requires four arguements.  `gif' is buffer to store the GIF
 * image data.  `bitmap', `width', and `height' are bitmap data,
 * width, and height of the bitmap image.
 *
 * Note: This GIF image doesn't use LZW because of patent.
 */
EB_Error_Code
eb_bitmap_to_gif(const char *bitmap, int width, int height, char *gif,
    size_t *gif_length)
{
    unsigned char *gif_p = (unsigned char *)gif;
    const unsigned char *bitmap_p = (const unsigned char *)bitmap;
    int i, j;

    LOG(("in: eb_bitmap_to_gif(width=%d, height=%d)", width, height));

    /*
     * Copy the default preamble.
     */
    memcpy(gif_p, gif_preamble, GIF_PREAMBLE_LENGTH);

    /*
     * Set logical screen width and height.
     */
    gif_p[6] = width & 0xff;
    gif_p[7] = (width >> 8) & 0xff;
    gif_p[8] = height & 0xff;
    gif_p[9] = (height >> 8) & 0xff;

    /*
     * Set global colors.
     */
    gif_p[13] = (GIF_BACKGROUND_COLOR >> 16) & 0xff;
    gif_p[14] = (GIF_BACKGROUND_COLOR >> 8) & 0xff;
    gif_p[15] = GIF_BACKGROUND_COLOR & 0xff;
    gif_p[16] = (GIF_FOREGROUND_COLOR >> 16) & 0xff;
    gif_p[17] = (GIF_FOREGROUND_COLOR >> 8) & 0xff;
    gif_p[18] = GIF_FOREGROUND_COLOR & 0xff;

    /*
     * Set image width and height.
     */
    gif_p[32] = width & 0xff;
    gif_p[33] = (width >> 8) & 0xff;
    gif_p[34] = height & 0xff;
    gif_p[35] = (height >> 8) & 0xff;

    gif_p += GIF_PREAMBLE_LENGTH;

    /*
     * Output image data.
     */
    for (i = 0;  i < height; i++) {
	*gif_p++ = (unsigned char)width;
	for (j = 0; j + 7 < width; j += 8, bitmap_p++) {
	    *gif_p++ = (*bitmap_p & 0x80) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x40) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x20) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x10) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x08) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x04) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x02) ? 0x81 : 0x80;
	    *gif_p++ = (*bitmap_p & 0x01) ? 0x81 : 0x80;
	}

	if (j < width) {
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x80) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x40) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x20) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x10) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x08) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x04) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x02) ? 0x81 : 0x80;
	    if (j++ < width)
		*gif_p++ = (*bitmap_p & 0x01) ? 0x81 : 0x80;
	    bitmap_p++;
	}
    }

    /*
     * Output a trailer.
     */
    memcpy(gif_p, "\001\011\000\073", 4);
    gif_p += 4;

    if (gif_length != NULL)
	*gif_length = ((char *)gif_p - gif);

    LOG(("out: eb_bitmap_to_gif(gif_length=%ld) = %s",
	(long)((char *)gif_p - gif), eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;
}


/*
 * The preamble of BMP image.
 */
#define BMP_PREAMBLE_LENGTH	62

static const unsigned char bmp_preamble[] = {
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
 * Convert a bitmap image to BMP format.
 *
 * It requires four arguements.  `bmp' is buffer to store the BMP
 * image data.  `bitmap', `width', and `height' are bitmap data,
 * width, and height of the bitmap image.
 */
EB_Error_Code
eb_bitmap_to_bmp(const char *bitmap, int width, int height, char *bmp,
    size_t *bmp_length)
{
    unsigned char *bmp_p = (unsigned char *)bmp;
    size_t data_size;
    size_t file_size;
    size_t line_pad_length;
    size_t bitmap_line_length;
    int i, j;

    LOG(("in: eb_bitmap_to_bmp(width=%d, height=%d)", width, height));

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

    data_size = (width / 2 + line_pad_length) * height;
    file_size = data_size + BMP_PREAMBLE_LENGTH;

    /*
     * Set BMP preamble.
     */
    memcpy(bmp_p, bmp_preamble, BMP_PREAMBLE_LENGTH);

    bmp_p[2] =  file_size        & 0xff;
    bmp_p[3] = (file_size >> 8)  & 0xff;
    bmp_p[4] = (file_size >> 16) & 0xff;
    bmp_p[5] = (file_size >> 24) & 0xff;

    bmp_p[18] =  width        & 0xff;
    bmp_p[19] = (width >> 8)  & 0xff;
    bmp_p[20] = (width >> 16) & 0xff;
    bmp_p[21] = (width >> 24) & 0xff;

    bmp_p[22] =  height        & 0xff;
    bmp_p[23] = (height >> 8)  & 0xff;
    bmp_p[24] = (height >> 16) & 0xff;
    bmp_p[25] = (height >> 24) & 0xff;

    bmp_p[34] =  data_size        & 0xff;
    bmp_p[35] = (data_size >> 8)  & 0xff;
    bmp_p[36] = (data_size >> 16) & 0xff;
    bmp_p[37] = (data_size >> 24) & 0xff;

    bmp_p += BMP_PREAMBLE_LENGTH;
    bitmap_line_length = (width + 7) / 8;

    for (i = height - 1; 0 <= i; i--) {
	memcpy(bmp_p, bitmap + bitmap_line_length * i, bitmap_line_length);
	bmp_p += bitmap_line_length;
	for (j = 0; j < line_pad_length; j++, bmp_p++)
	    *bmp_p = 0x00;
    }

    if (bmp_length != NULL)
	*bmp_length = ((char *)bmp_p - bmp);

    LOG(("out: eb_bitmap_to_bmp(bmp_length=%ld) = %s",
	(long)((char *)bmp_p - bmp), eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;
}


/*
 * The Foreground and background colors of PNG image.
 */
#define PNG_FOREGROUND_COLOR		0x000000
#define PNG_BACKGROUND_COLOR		0xffffff

/*
 * The preamble of PNG image.
 */
static const unsigned char png_preamble[] = {
    /*
     * PNG file signature (8 bytes)
     */
    0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n',
    /*
     * IHDR(Image Header) Chunk (25 bytes)
     */
    /* Size of IHDR. */
    0x00, 0x00, 0x00, 0x0d,
    'I', 'H', 'D', 'R',
    /* Width. (set at run time) */
    0x00, 0x00, 0x00, 0x00,
    /* Height. (set at run time) */
    0x00, 0x00, 0x00, 0x00,
    /* misc. */
    0x01, 0x03, 0x00, 0x00, 0x00,
    /* CRC (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /*
     * PLTE(Palette) Chunk (18 bytes)
     */
    /* Size of PLTE */
    0x00, 0x00, 0x00, 0x06,
    'P', 'L', 'T', 'E',
    /* RGB for palette index 0 */
    0xff, 0xff, 0xff,
    /* RGB for palette index 1 */
    0x00, 0x00, 0x00,
    /* CRC (set at run time) */
    0x00, 0x00, 0x00, 0x00,

    /*
     * tRNS(Transparency) Chunk (13 bytes)
     */
    /* Size of tRNS */
    0x00, 0x00, 0x00, 0x01,
    't', 'R', 'N', 'S',
    /* Alpha for palette index 0 */
    0x00,
    /* CRC */
    0x40, 0xe6, 0xd8, 0x66,

    /*
     * IDAT(Image Data) Chunk (12+ bytes)
     */
    /* Size of IDAT (set at run time) */
    0x00, 0x00, 0x00, 0x00,
    'I', 'D', 'A', 'T',
};

static const unsigned char png_trailer[] = {
    /* CRC (set at run time) */
    0x00, 0x00, 0x00, 0x00,
    /*
     * IEND(Image End) Chunk (12 bytes)
     */
    /* Size of IEND */
    0x00, 0x00, 0x00, 0x00,
    'I', 'E', 'N', 'D',
    /* CRC */
    0xae, 0x42, 0x60, 0x82,
};

/*
 * Table of CRCs of all 8-bit messages.
 */
static const unsigned long png_crc_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,   /* 0x00 - 0x03 */
    0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,   /* 0x04 - 0x07 */
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,   /* 0x08 - 0x0b */
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,   /* 0x0c - 0x0f */
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,   /* 0x10 - 0x13 */
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,   /* 0x14 - 0x17 */
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,   /* 0x18 - 0x1b */
    0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,   /* 0x1c - 0x1f */
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,   /* 0x20 - 0x23 */
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,   /* 0x24 - 0x27 */
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,   /* 0x28 - 0x2b */
    0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,   /* 0x2c - 0x2f */
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,   /* 0x30 - 0x33 */
    0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,   /* 0x34 - 0x37 */
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,   /* 0x38 - 0x3b */
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,   /* 0x3c - 0x3f */
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,   /* 0x40 - 0x43 */
    0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,   /* 0x44 - 0x47 */
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,   /* 0x48 - 0x4b */
    0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,   /* 0x4c - 0x4f */
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,   /* 0x50 - 0x53 */
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,   /* 0x54 - 0x57 */
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,   /* 0x58 - 0x5b */
    0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,   /* 0x5c - 0x5f */
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,   /* 0x60 - 0x63 */
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,   /* 0x64 - 0x67 */
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,   /* 0x68 - 0x6b */
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,   /* 0x6c - 0x6f */
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,   /* 0x70 - 0x73 */
    0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,   /* 0x74 - 0x77 */
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,   /* 0x78 - 0x7b */
    0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,   /* 0x7c - 0x7f */
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,   /* 0x80 - 0x83 */
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,   /* 0x84 - 0x87 */
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,   /* 0x88 - 0x8b */
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,   /* 0x8c - 0x8f */
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,   /* 0x90 - 0x93 */
    0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,   /* 0x94 - 0x97 */
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,   /* 0x98 - 0x9b */
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,   /* 0x9c - 0x9f */
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,   /* 0xa0 - 0xa3 */
    0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,   /* 0xa4 - 0xa7 */
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,   /* 0xa8 - 0xab */
    0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,   /* 0xac - 0xaf */
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,   /* 0xb0 - 0xb3 */
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,   /* 0xb4 - 0xb7 */
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,   /* 0xb8 - 0xbb */
    0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,   /* 0xbc - 0xbf */
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,   /* 0xc0 - 0xc3 */
    0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,   /* 0xc4 - 0xc7 */
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,   /* 0xc8 - 0xcb */
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,   /* 0xcc - 0xcf */
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,   /* 0xd0 - 0xd3 */
    0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,   /* 0xd4 - 0xd7 */
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,   /* 0xd8 - 0xdb */
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,   /* 0xdc - 0xdf */
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,   /* 0xe0 - 0xe3 */
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,   /* 0xe4 - 0xe7 */
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,   /* 0xe8 - 0xeb */
    0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,   /* 0xec - 0xef */
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,   /* 0xf0 - 0xf3 */
    0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,   /* 0xf4 - 0xf7 */
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,   /* 0xf8 - 0xfb */
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d    /* 0xfc - 0xff */
};

static unsigned long
png_crc(const char *buf, size_t len)
{
    unsigned long c = 0xffffffffL;
    int n;

    for (n = 0; n < len; n++)
	c = png_crc_table[(c ^ *((unsigned char *)buf + n)) & 0xff] ^ (c >> 8);
    return c ^ 0xffffffffL;
}


static int
png_compress(const char *src, int width, int height, char *dest,
    size_t *dest_len)
{
#ifdef ENABLE_LIBDEFLATE
    int line_size = (width  + 7) / 8;
    char *in_buffer;
    struct libdeflate_compressor *compressor;
    int i;
    size_t in_buffer_size = (line_size + 1) * height;
    size_t result;

    in_buffer = malloc (in_buffer_size);
    if (in_buffer == NULL) goto failed;
    compressor = libdeflate_alloc_compressor(1);
    if (compressor == NULL) goto failed;

    for (i = 0; i < height; i++) {
	*(in_buffer + (line_size + 1) * i) = 0;
	memcpy(in_buffer + (line_size + 1) * i + 1, src + line_size * i, line_size);
    }

    result = libdeflate_zlib_compress(compressor, in_buffer, in_buffer_size,
				      dest, (line_size + 1) * height + 12 + 256);
    if (result == 0) goto failed;

    *dest_len = result;

    free(in_buffer);
    libdeflate_free_compressor(compressor);
    return 0;

  failed:
    if (in_buffer) free(in_buffer);
    if (compressor) libdeflate_free_compressor(compressor);
    return -1;
#else
    int line_size = (width  + 7) / 8;
    z_stream z;
    int z_result;
    unsigned char byte_zero = 0x00;
    int i;

    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
    z_result = deflateInit(&z, Z_NO_COMPRESSION);
    if (z_result != Z_OK)
	return -1;

    /*
     * Exactly to say, `z.avail_out' must be:
     *     avail_out > (sizeof(src) + 12) * 1.001
     * but we use an approximation here.
     */
    z.next_out = (unsigned char *)dest;
    z.avail_out = (line_size + 1) * height + 12 + 256;
    for (i = 0; i < height - 1; i++) {
	z.next_in = &byte_zero;
	z.avail_in = 1;
	z_result = deflate(&z, Z_NO_FLUSH);
	if (z_result != Z_OK || z.avail_in != 0)
	    goto failed;

	z.next_in = (unsigned char *)src + (line_size * i);
	z.avail_in = line_size;
	z_result = deflate(&z, Z_NO_FLUSH);
	if (z_result != Z_OK || z.avail_in != 0)
	    goto failed;
    }

    z.next_in = &byte_zero;
    z.avail_in = 1;
    z_result = deflate(&z, Z_NO_FLUSH);
    if (z_result != Z_OK || z.avail_in != 0)
	goto failed;

    z.next_in = (unsigned char *)src + (line_size * i);
    z.avail_in = line_size;
    if (deflate(&z, Z_FINISH) != Z_STREAM_END)
	goto failed;

    z_result = deflateEnd(&z);
    if (z_result != Z_OK)
	return -1;

    *dest_len = (z.next_out - (unsigned char *)dest);
    return 0;

    /*
     * An error occurs...
     */
  failed:
    deflateEnd(&z);
    return -1;
#endif
}


#define INT2CHARS(p, i) do { \
     *(unsigned char *)(p) = ((i) >> 24) & 0xff; \
     *((unsigned char *)(p) + 1) = ((i) >> 16) & 0xff; \
     *((unsigned char *)(p) + 2) = ((i) >> 8) & 0xff; \
     *((unsigned char *)(p) + 3) = (i) & 0xff; \
} while (0);

#define RGB2CHARS(p, i) do { \
     *(unsigned char *)(p) = ((i) >> 16) & 0xff; \
     *((unsigned char *)(p) + 1) = ((i) >> 8) & 0xff; \
     *((unsigned char *)(p) + 2) = (i) & 0xff; \
} while (0);

/*
 * Convert a bitmap image to PNG format.
 *
 * It requires four arguements.  `png' is buffer to store the PNG
 * image data.  `bitmap', `width', and `height' are bitmap data,
 * width, and height of the bitmap image.
 */
EB_Error_Code
eb_bitmap_to_png(const char *bitmap, int width, int height, char *png,
    size_t *png_length)
{
    EB_Error_Code error_code;
    char *png_p = png;
    char *idat_start;
    size_t idat_len;
    unsigned long crc;
    int z_result;

    LOG(("in: eb_bitmap_to_png(width=%d, height=%d)", width, height));

    /*
     * Copy the default preamble.
     */
    memcpy(png_p, png_preamble, sizeof(png_preamble));

    /*
     * Set image width and height.
     */
    INT2CHARS(png_p + 16, width);
    INT2CHARS(png_p + 20, height);

    crc = png_crc(png_p + 12, 17);
    INT2CHARS(png_p + 29, crc);

    /*
     * Set global colors.
     */
    RGB2CHARS(png_p + 41, PNG_BACKGROUND_COLOR);
    RGB2CHARS(png_p + 44, PNG_FOREGROUND_COLOR);
    crc = png_crc(png_p + 37, 10);
    INT2CHARS(png_p + 47, crc);

    /*
     * Output `bitmap'.
     * We assume memory allocation error occurs if png_compress() doesn't
     * return Z_STREAM_END.
     */
    idat_start = png_p + sizeof(png_preamble);
    z_result = png_compress(bitmap, width, height, idat_start, &idat_len);
    if (z_result != 0) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }
    INT2CHARS(png_p + 64, idat_len);
    crc = png_crc(idat_start - 4, idat_len + 4);
    png_p = idat_start + idat_len;

    /*
     * Output a trailer.
     */
    memcpy(png_p, png_trailer, sizeof(png_trailer));
    INT2CHARS(png_p, crc);
    png_p += sizeof(png_trailer);
    if (png_length != NULL)
	*png_length = ((char *)png_p - png);

    LOG(("out: eb_bitmap_to_png(png_length=%ld) = %s",
	(long)((char *)png_p - png), eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_bitmap_to_png(png_length=%ld) = %s",
	(long)((char *)png_p - png), eb_error_string(error_code)));
    return error_code;
}

#undef INT2CHARS
#undef RGB2CHARS


#ifdef TEST

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define test_width 32
#define test_height 16
static unsigned char test_bitmap[] = {
   0xff, 0xff, 0xff, 0xff, 0x80, 0x81, 0x83, 0x01, 0x80, 0x81, 0x01, 0x01,
   0x80, 0x81, 0x01, 0x01, 0xe3, 0x8f, 0x11, 0xc7, 0xe3, 0x8f, 0x0f, 0xc7,
   0xe3, 0x81, 0x87, 0xc7, 0xe3, 0x81, 0xc3, 0xc7, 0xe3, 0x81, 0xe1, 0xc7,
   0xe3, 0x8f, 0x11, 0xc7, 0xe3, 0x8f, 0x11, 0xc7, 0xe3, 0x81, 0x01, 0xc7,
   0xe3, 0x81, 0x01, 0xc7, 0xe3, 0x81, 0x83, 0xc7, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff
};

int
main(int argc, char *argv[])
{
    char image[EB_SIZE_FONT_IMAGE];
    size_t image_size;
    int file;

    eb_bitmap_to_xbm(test_bitmap, test_width, test_height, image, &image_size);
    file = creat("test.xbm", 0644);
    if (file < 0)
	exit(1);
    if (write(file, image, image_size) != image_size) {
	close(file);
	exit(1);
    }

    eb_bitmap_to_xpm(test_bitmap, test_width, test_height, image, &image_size);
    file = creat("test.xpm", 0644);
    if (file < 0)
	exit(1);
    if (write(file, image, image_size) != image_size) {
	close(file);
	exit(1);
    }

    eb_bitmap_to_gif(test_bitmap, test_width, test_height, image, &image_size);
    file = creat("test.gif", 0644);
    if (file < 0)
	exit(1);
    if (write(file, image, image_size) != image_size) {
	close(file);
	exit(1);
    }

    eb_bitmap_to_bmp(test_bitmap, test_width, test_height, image, &image_size);
    file = creat("test.bmp", 0644);
    if (file < 0)
	exit(1);
    if (write(file, image, image_size) != image_size) {
	close(file);
	exit(1);
    }

    eb_bitmap_to_png(test_bitmap, test_width, test_height, image, &image_size);
    file = creat("test.png", 0644);
    if (file < 0)
	exit(1);
    if (write(file, image, image_size) != image_size) {
	close(file);
	exit(1);
    }

    return 0;
}

#endif /* TEST */
