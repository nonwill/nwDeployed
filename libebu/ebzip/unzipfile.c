/*                                                            -*- C -*-
 * Copyright (c) 1998-2006  Motoyuki Kasahara
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

#include "ebzip.h"

#include "getumask.h"
#include "makedir.h"
#include "samefile.h"
#include "yesno.h"

/*
 * Tricks for gettext.
 */
#ifdef ENABLE_NLS
#define _(string) gettext(string)
#ifdef gettext_noop
#define N_(string) gettext_noop(string)
#else
#define N_(string) (string)
#endif
#else
#define _(string) (string)
#define N_(string) (string)
#endif

/*
 * File name to be deleted and file to be closed when signal is received.
 */
static const char *trap_file_name = NULL;
static int trap_file = -1;

/*
 * Unexported function.
 */
static int ebzip_unzip_file_internal(const char *out_file_name,
    const char *in_file_name, Zio_Code in_zio_code, int index_page);
static void trap(int signal_number);


/*
 * Uncompress a file `in_file_name'.
 * For START file, use ebzip_unzip_start_file() instead.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
int
ebzip_unzip_file(const char *out_file_name, const char *in_file_name,
    Zio_Code in_zio_code)
{
    return ebzip_unzip_file_internal(out_file_name, in_file_name,
	in_zio_code, 0);
}

/*
 * Uncompress START file `in_file_name'.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
int
ebzip_unzip_start_file(const char *out_file_name, const char *in_file_name,
    Zio_Code in_zio_code, int index_page)
{
    return ebzip_unzip_file_internal(out_file_name, in_file_name,
	in_zio_code, index_page);
}

/*
 * Internal function for ebzip_unzip_file() and ebzip_unzip_sebxa_start().
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
static int
ebzip_unzip_file_internal(const char *out_file_name, const char *in_file_name,
    Zio_Code in_zio_code, int index_page)
{
    Zio in_zio;
    unsigned char *buffer = NULL;
    off_t total_length;
    int out_file = -1;
    ssize_t length;
    struct stat in_status, out_status;
    unsigned int crc = 1;
    int progress_interval;
    int total_slices;
    int i;

    zio_initialize(&in_zio);

    /*
     * Simply copy a file, when an input file is not compressed.
     */
    if (in_zio_code == ZIO_PLAIN)
	return ebzip_copy_file(out_file_name, in_file_name);

    /*
     * Output file name information.
     */
    if (!ebzip_quiet_flag) {
	fprintf(stderr, _("==> uncompress %s <==\n"), in_file_name);
	fprintf(stderr, _("output to %s\n"), out_file_name);
	fflush(stderr);
    }

    /*
     * Get status of the input file.
     */
    if (stat(in_file_name, &in_status) < 0 || !S_ISREG(in_status.st_mode)) {
        fprintf(stderr, _("%s: no such file: %s\n"), invoked_name,
            in_file_name);
        goto failed;
    }

    /*
     * Do nothing if the `in_file_name' and `out_file_name' are the same.
     */
    if (is_same_file(out_file_name, in_file_name)) {
	if (!ebzip_quiet_flag) {
	    fprintf(stderr,
		_("the input and output files are the same, skipped.\n\n"));
	    fflush(stderr);
	}
	return 0;
    }

    /*
     * Allocate memories for in/out buffers.
     */
    buffer = (unsigned char *)malloc(EB_SIZE_PAGE << ZIO_MAX_EBZIP_LEVEL);
    if (buffer == NULL) {
	fprintf(stderr, _("%s: memory exhausted\n"), invoked_name);
	goto failed;
    }

    /*
     * If the file `out_file_name' already exists, confirm and unlink it.
     */
    if (!ebzip_test_flag
	&& stat(out_file_name, &out_status) == 0
	&& S_ISREG(out_status.st_mode)) {
	if (ebzip_overwrite_mode == EBZIP_OVERWRITE_NO) {
	    if (!ebzip_quiet_flag) {
		fputs(_("already exists, skip the file\n\n"), stderr);
		fflush(stderr);
	    }
	    return 0;
	} else if (ebzip_overwrite_mode == EBZIP_OVERWRITE_CONFIRM) {
	    int y_or_n;

	    fprintf(stderr, _("\nthe file already exists: %s\n"),
		out_file_name);
	    y_or_n = query_y_or_n(_("do you wish to overwrite (y or n)? "));
	    fputc('\n', stderr);
	    fflush(stderr);
	    if (!y_or_n)
		return 0;
        }
	if (unlink(out_file_name) < 0) {
	    fprintf(stderr, _("%s: failed to unlink the file: %s\n"),
		invoked_name, out_file_name);
	    goto failed;
	}
    }

    /*
     * Open files.
     */
    if (zio_open(&in_zio, in_file_name, in_zio_code) < 0) {
	fprintf(stderr, _("%s: failed to open the file: %s\n"),
	    invoked_name, in_file_name);
	goto failed;
    }
    if (in_zio_code == ZIO_SEBXA) {
	off_t index_location;
	off_t index_base;
	off_t zio_start_location;
	off_t zio_end_location;

	if (get_sebxa_indexes(in_file_name, index_page, &index_location,
	    &index_base, &zio_start_location, &zio_end_location) < 0) {
	    goto failed;
	}
	zio_set_sebxa_mode(&in_zio, index_location, index_base,
	    zio_start_location, zio_end_location);
    }

    if (!ebzip_test_flag) {
	trap_file_name = out_file_name;
#ifdef SIGHUP
	signal(SIGHUP, trap);
#endif
	signal(SIGINT, trap);
#ifdef SIGQUIT
	signal(SIGQUIT, trap);
#endif
#ifdef SIGTERM
	signal(SIGTERM, trap);
#endif

#ifdef O_CREAT
	out_file = open(out_file_name, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
	    0666 ^ get_umask());
#else
	out_file = creat(out_file_name, 0666 ^ get_umask());
#endif
	if (out_file < 0) {
	    fprintf(stderr, _("%s: failed to open the file: %s\n"),
		invoked_name, out_file_name);
	    goto failed;
	}
	trap_file = out_file;
    }

    /*
     * Read a slice from the input file, uncompress it if required,
     * and then write it to the output file.
     */
    total_length = 0;
    total_slices = (in_zio.file_size + in_zio.slice_size - 1)
	/ in_zio.slice_size;
    progress_interval = EBZIP_PROGRESS_INTERVAL_FACTOR;
    if (((total_slices + 999) / 1000) > progress_interval)
	progress_interval = ((total_slices + 999) / 1000);

    for (i = 0; i < total_slices; i++) {
	/*
	 * Read a slice.
	 */
	if (zio_lseek(&in_zio, total_length, SEEK_SET) < 0) {
	    fprintf(stderr, _("%s: failed to seek the file: %s\n"),
		invoked_name, in_file_name);
	    goto failed;
	}
	length = zio_read(&in_zio, (char *)buffer, in_zio.slice_size);
	if (length < 0) {
	    fprintf(stderr, _("%s: failed to read from the file: %s\n"),
		invoked_name, in_file_name);
	    goto failed;
	} else if (length == 0) {
	    fprintf(stderr, _("%s: unexpected EOF: %s\n"),
		invoked_name, in_file_name);
	    goto failed;
	} else if (length != in_zio.slice_size
	    && total_length + length != in_zio.file_size) {
	    fprintf(stderr, _("%s: unexpected EOF: %s\n"),
		invoked_name, in_file_name);
	    goto failed;
	}

	/*
	 * Update CRC.  (Calculate adler32 again.)
	 */
	if (in_zio.code == ZIO_EBZIP1) {
#ifdef ENABLE_LIBDEFLATE
	    crc = libdeflate_adler32((uint32_t)crc, buffer, (size_t)length);
#else
	    crc = adler32((uLong)crc, (Bytef *)buffer, (uInt)length);
#endif
	}

	/*
	 * Write the slice to `out_file'.
	 */
	if (!ebzip_test_flag) {
	    if (write(out_file, buffer, length) != length) {
		fprintf(stderr, _("%s: failed to write to the file, %s: %s\n"),
		    invoked_name, strerror(errno), out_file_name);
		goto failed;
	    }
	}
	total_length += length;

	/*
	 * Output status information unless `quiet' mode.
	 */
	if (!ebzip_quiet_flag && (i + 1) % progress_interval == 0) {
#if defined(PRINTF_LL_MODIFIER)
	    fprintf(stderr, _("%4.1f%% done (%llu / %llu bytes)\n"),
		(double) (i + 1) * 100.0 / (double) total_slices,
		(unsigned long long) total_length,
		(unsigned long long) in_zio.file_size);
#elif defined(PRINTF_I64_MODIFIER)
	    fprintf(stderr, _("%4.1f%% done (%I64u / %I64u bytes)\n"),
		(double) (i + 1) * 100.0 / (double) total_slices,
		(unsigned __int64) total_length,
		(unsigned __int64) in_zio.file_size);
#else
	    fprintf(stderr, _("%4.1f%% done (%lu / %lu bytes)\n"),
		(double) (i + 1) * 100.0 / (double) total_slices,
		(unsigned long) total_length,
		(unsigned long) in_zio.file_size);
#endif
	    fflush(stderr);
	}
    }

    /*
     * Output the result unless quiet mode.
     */
    if (!ebzip_quiet_flag) {
#if defined(PRINTF_LL_MODIFIER)
	fprintf(stderr, _("completed (%llu / %llu bytes)\n"),
	    (unsigned long long) in_zio.file_size,
	    (unsigned long long) in_zio.file_size);
#elif defined(PRINTF_I64_MODIFIER)
	fprintf(stderr, _("completed (%I64u / %I64u bytes)\n"),
	    (unsigned __int64) in_zio.file_size,
	    (unsigned __int64) in_zio.file_size);
#else
	fprintf(stderr, _("%lu -> %lu bytes\n\n"),
	    (unsigned long) in_status.st_size,
	    (unsigned long) total_length);
#endif
	fflush(stderr);
    }

    /*
     * Close files.
     */
    zio_close(&in_zio);
    zio_finalize(&in_zio);

    if (!ebzip_test_flag) {
	close(out_file);
	out_file = -1;
	trap_file = -1;
	trap_file_name = NULL;
#ifdef SIGHUP
	signal(SIGHUP, SIG_DFL);
#endif
	signal(SIGINT, SIG_DFL);
#ifdef SIGQUIT
	signal(SIGQUIT, SIG_DFL);
#endif
#ifdef SIGTERM
	signal(SIGTERM, SIG_DFL);
#endif
    }

    /*
     * Check for CRC.
     */
    if (in_zio.code == ZIO_EBZIP1 && in_zio.crc != crc) {
	fprintf(stderr, _("%s: CRC error: %s\n"), invoked_name, out_file_name);
	goto failed;
    }

    /*
     * Delete an original file unless the keep flag is set.
     */
    if (!ebzip_test_flag && !ebzip_keep_flag)
	unlink_files_add(in_file_name);

    /*
     * Set owner, group, permission, atime and mtime of `out_file'.
     * We ignore return values of `chown', `chmod' and `utime'.
     */
    if (!ebzip_test_flag) {
	struct utimbuf utim;

	utim.actime = in_status.st_atime;
	utim.modtime = in_status.st_mtime;
	utime(out_file_name, &utim);
    }

    /*
     * Dispose memories.
     */
    free(buffer);

    return 0;

    /*
     * An error occurs...
     */
  failed:
    if (buffer != NULL)
	free(buffer);

    zio_close(&in_zio);
    zio_finalize(&in_zio);

    if (0 <= out_file) {
	close(out_file);
	trap_file = -1;
	trap_file_name = NULL;
#ifdef SIGHUP
	signal(SIGHUP, SIG_DFL);
#endif
	signal(SIGINT, SIG_DFL);
#ifdef SIGQUIT
	signal(SIGQUIT, SIG_DFL);
#endif
#ifdef SIGTERM
	signal(SIGTERM, SIG_DFL);
#endif
    }

    fputc('\n', stderr);
    fflush(stderr);

    return -1;
}


/*
 * Signal handler.
 */
static void
trap(int signal_number)
{
    if (0 <= trap_file)
	close(trap_file);
    if (trap_file_name != NULL)
	unlink(trap_file_name);

    exit(1);
}
