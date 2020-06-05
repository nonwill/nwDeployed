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
#include "ebutils.h"

/*
 * File name to be deleted and file to be closed when signal is received.
 */
static const char *trap_file_name = NULL;
static int trap_file = -1;

/*
 * Unexported function.
 */
static void trap(int signal_number);


/*
 * Copy a file.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
int
ebzip_copy_file(const char *out_file_name, const char *in_file_name)
{
    unsigned char buffer[EB_SIZE_PAGE];
    off_t total_length;
    struct stat in_status, out_status;
    int in_file = -1, out_file = -1;
    ssize_t in_length;
    int progress_interval;
    int total_slices;
    struct utimbuf utim;
    int i;

    /*
     * Output file name information.
     */
    if (!ebzip_quiet_flag) {
	fprintf(stderr, _("==> copy %s <==\n"), in_file_name);
	fprintf(stderr, _("output to %s\n"), out_file_name);
	fflush(stderr);
    }

    /*
     * Check for the input file.
     */
    if (stat(in_file_name, &in_status) != 0 || !S_ISREG(in_status.st_mode)) {
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
     * When test mode, return immediately.
     */
    if (ebzip_test_flag) {
#if defined(PRINTF_LL_MODIFIER)
	fprintf(stderr, _("completed (%llu / %llu bytes)\n"),
	    (unsigned long long) in_status.st_size,
	    (unsigned long long) in_status.st_size);
#elif defined(PRINTF_I64_MODIFIER)
	fprintf(stderr, _("completed (%I64u / %I64u bytes)\n"),
	    (unsigned __int64) in_status.st_size,
	    (unsigned __int64) in_status.st_size);
#else
	fprintf(stderr, _("completed (%lu / %lu bytes)\n"),
	    (unsigned long) in_status.st_size,
	    (unsigned long) in_status.st_size);
#endif
	fputc('\n', stderr);
	fflush(stderr);
	return 0;
    }

    /*
     * If the file to be output already exists, confirm and unlink it.
     */
    if (stat(out_file_name, &out_status) == 0
	&& S_ISREG(out_status.st_mode)) {
	if (ebzip_overwrite_mode == EBZIP_OVERWRITE_NO) {
	    if (!ebzip_quiet_flag) {
		fputs(_("already exists, skip the file\n\n"), stderr);
		fflush(stderr);
	    }
	    return 0;
	}
	if (ebzip_overwrite_mode == EBZIP_OVERWRITE_CONFIRM) {
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
    in_file = open(in_file_name, O_RDONLY | O_BINARY);
    if (in_file < 0) {
	fprintf(stderr, _("%s: failed to open the file, %s: %s\n"),
	    invoked_name, strerror(errno), in_file_name);
	goto failed;
    }

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
	fprintf(stderr, _("%s: failed to open the file, %s: %s\n"),
	    invoked_name, strerror(errno), out_file_name);
	goto failed;
    }
    trap_file = out_file;

    /*
     * Read data from the input file, compress the data, and then
     * write them to the output file.
     */
    total_length = 0;
    total_slices = (in_status.st_size + EB_SIZE_PAGE - 1) / EB_SIZE_PAGE;
    progress_interval = EBZIP_PROGRESS_INTERVAL_FACTOR;
    for (i = 0; i < total_slices; i++) {
	/*
	 * Read data from `in_file', and write them to `out_file'.
	 */
	in_length = read(in_file, buffer, EB_SIZE_PAGE);
	if (in_length < 0) {
	    fprintf(stderr, _("%s: failed to read from the file, %s: %s\n"),
		invoked_name, strerror(errno), in_file_name);
	    goto failed;
	} else if (in_length == 0) {
	    fprintf(stderr, _("%s: unexpected EOF: %s\n"),
		invoked_name, in_file_name);
	    goto failed;
	} else if (in_length != EB_SIZE_PAGE
	    && total_length + in_length != in_status.st_size) {
	    fprintf(stderr, _("%s: unexpected EOF: %s\n"),
		invoked_name, in_file_name);
	    goto failed;
	}

	/*
	 * Write decoded data to `out_file'.
	 */
	if (write(out_file, buffer, in_length) != in_length) {
	    fprintf(stderr, _("%s: failed to write to the file, %s: %s\n"),
		invoked_name, strerror(errno), out_file_name);
	    goto failed;
	}
	total_length += in_length;

	/*
	 * Output status information unless `quiet' mode.
	 */
	if (!ebzip_quiet_flag && (i + 1) % progress_interval == 0) {
#if defined(PRINTF_LL_MODIFIER)
	    fprintf(stderr, _("%4.1f%% done (%llu / %llu bytes)\n"),
		(double) (i + 1) * 100.0 / (double) total_slices,
		(unsigned long long) total_length,
		(unsigned long long) in_status.st_size);
#elif defined(PRINTF_I64_MODIFIER)
	    fprintf(stderr, _("%4.1f%% done (%I64u / %I64u bytes)\n"),
		(double) (i + 1) * 100.0 / (double) total_slices,
		(unsigned __int64) total_length,
		(unsigned __int64) in_status.st_size);
#else
	    fprintf(stderr, _("%4.1f%% done (%lu / %lu bytes)\n"),
		(double) (i + 1) * 100.0 / (double) total_slices,
		(unsigned long) total_length,
		(unsigned long) in_status.st_size);
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
	    (unsigned long long) total_length,
	    (unsigned long long) in_status.st_size);
#elif defined(PRINTF_I64_MODIFIER)
	fprintf(stderr, _("completed (%I64u / %I64u bytes)\n"),
	    (unsigned __int64) total_length,
	    (unsigned __int64) in_status.st_size);
#else
	fprintf(stderr, _("completed (%lu / %lu bytes)\n"),
	    (unsigned long) total_length,
	    (unsigned long) in_status.st_size);
#endif
	fputc('\n', stderr);
	fflush(stderr);
    }

    /*
     * Close files.
     */
    close(in_file);
    in_file = -1;

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

    /*
     * Delete an original file unless the keep flag is set.
     */
    if (!ebzip_test_flag && !ebzip_keep_flag)
	unlink_files_add(in_file_name);

    /*
     * Set owner, group, permission, atime and mtime of `out_file'.
     * We ignore return values of `chown', `chmod' and `utime'.
     */
    utim.actime = in_status.st_atime;
    utim.modtime = in_status.st_mtime;
    utime(out_file_name, &utim);

    return 0;

    /*
     * An error occurs...
     */
  failed:
    if (0 <= in_file)
	close(in_file);
    if (0 <= out_file)
	close(out_file);

    fputc('\n', stderr);
    fflush(stderr);

    return -1;
}


/*
 * Copy all files in a directory.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
int
ebzip_copy_files_in_directory(const char *out_directory_name,
    const char *in_directory_name)
{
    struct dirent *entry;
    DIR *dir = NULL;
    struct stat in_status;
    char in_path_name[PATH_MAX + 1];
    char out_path_name[PATH_MAX + 1];

    /*
     * Check for the input directory.
     */
    if (stat(in_directory_name, &in_status) != 0
	|| !S_ISDIR(in_status.st_mode))
	return 0;


    /*
     * Make the output directory if missing.
     */
    if (!ebzip_test_flag
	&& make_missing_directory(out_directory_name, 0777 ^ get_umask())
	< 0) {
	fprintf(stderr, _("%s: failed to create a directory, %s: %s\n"),
	    invoked_name, strerror(errno), out_directory_name);
	goto failed;
    }

    /*
     * Open the directory `path'.
     */
    dir = opendir(in_directory_name);
    if (dir == NULL) {
	fprintf(stderr, _("%s: failed to open the directory, %s: %s\n"),
	    invoked_name, strerror(errno), in_directory_name);
        goto failed;
    }

    for (;;) {
        /*
         * Read the directory entry.
         */
        entry = readdir(dir);
        if (entry == NULL)
            break;

	eb_compose_path_name(in_directory_name, entry->d_name, in_path_name);
	eb_compose_path_name(out_directory_name, entry->d_name, out_path_name);

	if (stat(in_path_name, &in_status) != 0
	    || !S_ISREG(in_status.st_mode))
	    continue;

	ebzip_copy_file(out_path_name, in_path_name);
    }

    closedir(dir);
    return 0;

    /*
     * An error occurs...
     */
  failed:
    if (dir != NULL)
        closedir(dir);
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
