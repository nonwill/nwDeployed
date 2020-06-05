/*
 * Copyright (c) 2009  Kazuhiro Ito
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
#include "font.h"
#include "build-post.h"

char *eb_normalize_utf8 (EB_Book *book, int code) {
    EB_Subbook *subbook;
    EB_UTF8_Table *table;
    int floor = -1 , ceil;
    int current;

    subbook = book->subbook_current;
    table = subbook->table;
    ceil = subbook->table_count;

    while (floor + 1 != ceil) {
	current = (floor + ceil) / 2;
	if (table[current].code == code) {
	    return table[current].string;
	}

	if (table[current].code < code) {
	    floor = current;
	} else {
	    ceil = current;
	}
    }

    return NULL;
}

EB_Error_Code eb_load_utf8_table (EB_Book *book) {
    EB_Error_Code error_code;
    EB_Subbook *subbook;
    size_t buffer_size;
    unsigned int count, i;
    char *buffer, *read;

    LOG(("in: eb_load_utf8_table(book=%d)", (int)book->code));

    subbook = book->subbook_current;

    /*
     * If table is already loaded, do nothing.
     */
    if (subbook->table_buffer) return EB_SUCCESS;

    /*
     * Allocate memory for table buffer.
     */
    buffer_size = subbook->table_size * EB_SIZE_PAGE;
    subbook->table_buffer = malloc(buffer_size);
    if (subbook->table_buffer == NULL) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }
    buffer = subbook->table_buffer;

    /*
     * Read normalization table.
     */
    if (zio_lseek(&subbook->text_zio,
	     ((off_t) subbook->table_page - 1) * EB_SIZE_PAGE, SEEK_SET) < 0) {
	error_code = EB_ERR_FAIL_SEEK_TEXT;
	goto failed;
    }
    if (zio_read(&subbook->text_zio, buffer, buffer_size) != buffer_size) {
	error_code = EB_ERR_FAIL_READ_TEXT;
	goto failed;
    }

    /*
     * Check version and etc.
     */
    if (eb_uint4(buffer) != 1) {
	error_code = EB_ERR_UNEXP_TEXT;
	goto failed;
    }
    subbook->table_count = eb_uint4(buffer + 4);
    if (subbook->table_count > 0x10ffff) {
	error_code = EB_ERR_UNEXP_TEXT;
	goto failed;
    }

    /*
     * Allocate memory for table elements.
     */
    subbook->table = malloc(subbook->table_count * sizeof(EB_UTF8_Table));
    if (subbook->table == NULL) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }

    read = buffer + 8;
    for (i = 0; i < subbook->table_count; i++) {
	read += eb_read_utf8(read, &(subbook->table[i].code));
	if (subbook->table[i].code == 0) {
	    error_code = EB_ERR_UNEXP_TEXT;
	    goto failed;
	}
	subbook->table[i].string = read;
	read += strlen(read) + 1;
    }

    LOG(("out: eb_load_utf8_table() = %s", eb_error_string(EB_SUCCESS)));
    return EB_SUCCESS;
  failed:
    if (subbook->table_buffer) {
	free(subbook->table_buffer);
	subbook->table_buffer = NULL;
    }
    if (subbook->table) {
	free(subbook->table);
	subbook->table = NULL;
    }

    LOG(("out: eb_load_utf8_table() = %s", eb_error_string(error_code)));
    return error_code;
}


void eb_initialize_utf8_table (EB_Book *book) {
    EB_Subbook *subbook;

    subbook = book->subbook_current;

    subbook->table_buffer = NULL;
    subbook->table = NULL;
    subbook->table_count = 0;
}

void eb_finalize_utf8_table (EB_Book *book) {
    EB_Subbook *subbook;

    subbook = book->subbook_current;

    if (subbook->table_buffer) {
	free(subbook->table_buffer);
	subbook->table_buffer = NULL;
    }
    if (subbook->table) {
	free(subbook->table);
	subbook->table = NULL;
    }
    subbook->table_count = 0;
}

int eb_read_utf8 (const char *buffer, int *code) {
    if (!(((unsigned char)*buffer) & 0x80)) {
	*code = *buffer;
	return 1;
    } else if (!(((unsigned char)*buffer) & 0x20)) {
	*code =
	    ((((unsigned char)buffer[0]) & 0x1f) <<  6) +
	    ((((unsigned char)buffer[1]) & 0x3f)      );
	return 2;
    } else if (!(((unsigned char)*buffer) & 0x10)) {
	*code =
	    ((((unsigned char)buffer[0]) & 0x0f) << 12) +
	    ((((unsigned char)buffer[1]) & 0x3f) <<  6) + 
	    ((((unsigned char)buffer[2]) & 0x3f)      );
	return 3;
    } else if (!(((unsigned char)*buffer) & 0x08)) {
	*code =
	    ((((unsigned char)buffer[0]) & 0x07) << 18) +
	    ((((unsigned char)buffer[1]) & 0x3f) << 12) + 
	    ((((unsigned char)buffer[2]) & 0x3f) <<  6) + 
	    ((((unsigned char)buffer[3]) & 0x3f)      );
	return 4;
    }

    /* invalid string */
    *code = 0;
    return 0;
}
