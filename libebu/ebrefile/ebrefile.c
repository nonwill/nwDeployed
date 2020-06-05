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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>

#ifdef ENABLE_NLS
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <libintl.h>
#endif

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *, const char *);
int strncasecmp(const char *, const char *, size_t);
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

/*
 * stat macros.
 */
#ifndef S_ISREG
#define S_ISREG(m)   (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m)   (((m) & S_IFMT) == S_IFDIR)
#endif

/*
 * The maximum length of path name.
 */
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX        MAXPATHLEN
#else /* not MAXPATHLEN */
#define PATH_MAX        1024
#endif /* not MAXPATHLEN */
#endif /* not PATH_MAX */

/*
 * rename() on Windows complains if the new file already exists.
 * We fake rename() here for Windows.
 */
#ifdef WIN32
#include <windows.h>
#define rename(old, new) \
    (MoveFileEx((old), (new), MOVEFILE_REPLACE_EXISTING) ? 0 : -1)
#endif

#include "ebu/eb.h"
#include "ebu/error.h"
#include "ebu/build-post.h"

#include "ebutils.h"

#ifdef HAVE_GETOPT_LOG
#include <getopt.h>
#else
#include "getopt.h"
#endif
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

#define DEFAULT_BOOK_DIRECTORY		"."
#define DEFAULT_OUTPUT_DIRECTORY	"."

/*
 * Unexported functions.
 */
static void output_help(void);
static int refile_book(const char *out_path, const char *in_path,
    char subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count);
static int refile_catalog(const char *out_catalog_name,
    const char *in_catalog_name, EB_Disc_Code disc_code,
    char subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count);
static int copy_file(const char *out_file_name, const char *in_file_name);
static void trap(int signal_number);
static int find_subbook_name(char
    subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count, const char *pattern);

/*
 * Command line options.
 */
static const char *short_options = "ho:S:v";
static struct option long_options[] = {
    {"help",              no_argument,       NULL, 'h'},
    {"output-directory",  required_argument, NULL, 'o'},
    {"subbook",           required_argument, NULL, 'S'},
    {"version",           no_argument,       NULL, 'v'},
    {NULL, 0, NULL, 0}
};

/*
 * Program name and version.
 */
const char *program_name = "ebrefile";
const char *program_version = VERSION;
const char *invoked_name;

/*
 * File names to be deleted when signal is received.
 */
static const char *trap_file_name = NULL;
static int trap_file = -1;

int
main(int argc, char *argv[])
{
    EB_Error_Code error_code;
    char out_path[PATH_MAX + 1];
    char book_path[PATH_MAX + 1];
    char subbook_name_list[EB_MAX_SUBBOOKS][EB_MAX_DIRECTORY_NAME_LENGTH + 1];
    int subbook_name_count = 0;
    int ch;

    invoked_name = argv[0];
    strcpy(out_path, DEFAULT_OUTPUT_DIRECTORY);

    /*
     * Initialize locale data.
     */
#ifdef ENABLE_NLS
#ifdef HAVE_SETLOCALE
    setlocale(LC_ALL, "");
#endif
    bindtextdomain(TEXT_DOMAIN_NAME, LOCALEDIR);
    textdomain(TEXT_DOMAIN_NAME);
#endif

    /*
     * Initialize `book'.
     */
    error_code = eb_initialize_library();
    if (error_code != EB_SUCCESS) {
	fprintf(stderr, "%s: %s\n", invoked_name,
	    eb_error_message(error_code));
	goto die;
    }

    /*
     * Parse command line options.
     */
    for (;;) {
        ch = getopt_long(argc, argv, short_options, long_options, NULL);
        if (ch == -1)
            break;
        switch (ch) {
        case 'h':
            /*
             * Option `-h'.  Display help message, then exit.
             */
            output_help();
            exit(0);

        case 'o':
            /*
             * Option `-o'.  Output files under DIRECOTRY.
	     * The length of the file name
	     *    "<out_path>/catalogs.bak;1"
	     * must not exceed PATH_MAX.
             */
            if (PATH_MAX < strlen(optarg)) {
                fprintf(stderr, _("%s: too long output directory path\n"),
                    invoked_name);
                exit(1);
            }
            strcpy(out_path, optarg);
	    canonicalize_path(out_path);
	    if (PATH_MAX
		< strlen(out_path) + (1 + EB_MAX_DIRECTORY_NAME_LENGTH + 6)) {
		fprintf(stderr, _("%s: too long output directory path\n"),
		    invoked_name);
		goto die;
	    }
	    break;

        case 'S':
            /*
             * Option `-S'.  Specify target subbooks.
             */
            if (parse_subbook_name_argument(invoked_name, optarg,
		subbook_name_list, &subbook_name_count) < 0)
                exit(1);
            break;

        case 'v':
            /*
             * Option `-v'.  Display version number, then exit.
             */
            output_version(program_name, program_version);
            exit(0);

        default:
            output_try_help(invoked_name);
	    goto die;
	}
    }

    /*
     * Check the number of rest arguments.
     */
    if (1 < argc - optind) {
        fprintf(stderr, _("%s: too many arguments\n"), invoked_name);
        output_try_help(invoked_name);
	goto die;
    }

    /*
     * Set a book path.
     */
    if (argc == optind)
        strcpy(book_path, DEFAULT_BOOK_DIRECTORY);
    else
        strcpy(book_path, argv[optind]);

    if (is_ebnet_url(book_path)) {
	fprintf(stderr, "%s: %s\n", invoked_name,
	    eb_error_message(EB_ERR_EBNET_UNSUPPORTED));
	goto die;
    }
    canonicalize_path(book_path);

    if (PATH_MAX
	< strlen(book_path) + (1 + EB_MAX_DIRECTORY_NAME_LENGTH + 6)) {
	fprintf(stderr, _("%s: too long book directory path\n"),
	    invoked_name);
	goto die;
    }

    /*
     * Set signals.
     */
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

    /*
     * Refile a catalog.
     */
    if (refile_book(out_path, book_path, subbook_name_list,
	subbook_name_count) < 0)
	goto die;

    eb_finalize_library();

    return 0;

    /*
     * A critical error occurs...
     */
  die:
    eb_finalize_library();
    exit(1);
}


/*
 * Output help message to stdandard out.
 */
static void
output_help(void)
{
    printf(_("Usage: %s [option...] [book-directory]\n"), program_name);
    printf(_("Options:\n"));
    printf(_("  -h  --help                 display this help, then exit\n"));
    printf(_("  -o DIRECTORY  --output-directory DIRECTORY\n"));
    printf(_("                             ouput files under DIRECTORY\n"));
    printf(_("                             (default: %s)\n"),
	DEFAULT_OUTPUT_DIRECTORY);
    printf(_("  -S SUBBOOK[,SUBBOOK...]  --subbook SUBBOOK[,SUBBOOK...]\n"));
    printf(_("                             target subbook\n"));
    printf(_("                             (default: all subbooks)\n"));
    printf(_("  -v  --version              display version number, then exit\n"));
    printf(_("\nArgument:\n"));
    printf(_("  book-directory             top directory of a CD-ROM book\n"));
    printf(_("                             (default: %s)\n"),
	DEFAULT_BOOK_DIRECTORY);

    printf(_("\nReport bugs to %s.\n"), MAILING_ADDRESS);
    fflush(stdout);
}


/*
 * Read a catalog file in `in_path' and create refiled catalog file
 * in `out_path'.
 */
static int
refile_book(const char *out_path, const char *in_path,
    char subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count)
{
    char in_file_name[PATH_MAX + 1];
    char out_file_name[PATH_MAX + 1];
    char tmp_file_name[PATH_MAX + 1];
    char old_file_name[PATH_MAX + 1];
    char in_base_name[EB_MAX_FILE_NAME_LENGTH + 1];
    EB_Disc_Code disc_code;
    struct stat out_status;
    struct stat old_status;

    /*
     * Find a catalog file.
     */
    if (eb_find_file_name(in_path, "catalog", in_base_name)
	== EB_SUCCESS) {
	disc_code = EB_DISC_EB;
    } else if (eb_find_file_name(in_path, "catalogs", in_base_name)
	== EB_SUCCESS) {
	disc_code = EB_DISC_EPWING;
    } else {
	fprintf(stderr, _("%s: no catalog file: %s\n"), invoked_name, in_path);
        return -1;
    }

    /*
     * Set file names.
     */
    eb_compose_path_name(in_path, in_base_name, in_file_name);
    eb_compose_path_name(out_path, in_base_name, out_file_name);

    strcpy(old_file_name, out_file_name);
    eb_fix_path_name_suffix(old_file_name, ".old");
    strcpy(tmp_file_name, out_file_name);
    eb_fix_path_name_suffix(tmp_file_name, ".tmp");

    /*
     * Copy the original catalog file.
     */
    if (stat(old_file_name, &old_status) < 0
	&& errno == ENOENT
	&& stat(out_file_name, &out_status) == 0
	&& S_ISREG(out_status.st_mode)) {
	trap_file_name = old_file_name;
	if (copy_file(old_file_name, out_file_name) < 0)
	    return -1;
	trap_file_name = NULL;
    }

    /*
     * Refile the catalog file.
     */
    trap_file_name = tmp_file_name;
    if (refile_catalog(tmp_file_name, in_file_name, disc_code,
	subbook_name_list, subbook_name_count) < 0) {
	unlink(tmp_file_name);
	rename(old_file_name, out_file_name);
	return -1;
    }
    if (rename(tmp_file_name, out_file_name) < 0) {
	fprintf(stderr, _("%s: failed to move the file, %s: %s -> %s\n"),
	    invoked_name, strerror(errno), tmp_file_name, out_file_name);
	unlink(tmp_file_name);
	return -1;
    }

    trap_file_name = NULL;

    return 0;
}


/*
 * Read a catalog file `in_catalog_name' and create refiled catalog file
 * as `out_catalog_name'.
 */
static int
refile_catalog(const char *out_catalog_name, const char *in_catalog_name,
    EB_Disc_Code disc_code,
    char subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count)
{
    char buffer[EB_SIZE_PAGE];
    char directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
    int in_subbook_count;
    int in_file = -1;
    int out_file = -1;
    int subbbook_map_table[EB_MAX_SUBBOOKS];
    off_t out_file_offset;
    size_t catalog_size;
    int i, j;

    for (i = 0; i < EB_MAX_SUBBOOKS; i++)
	subbbook_map_table[i] = EB_SUBBOOK_INVALID;

    if (disc_code == EB_DISC_EB)
	catalog_size = EB_SIZE_EB_CATALOG;
    else
	catalog_size = EB_SIZE_EPWING_CATALOG;

    /*
     * Open input file.
     */
    in_file = open(in_catalog_name, O_RDONLY | O_BINARY);
    if (in_file < 0) {
	fprintf(stderr, _("%s: failed to open the file, %s: %s\n"),
	    invoked_name, strerror(errno), in_catalog_name);
	goto failed;
    }

    /*
     * Open output file.
     */
#ifdef O_CREAT
    out_file = open(out_catalog_name, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
	0666 ^ get_umask());
#else
    out_file = creat(out_catalog_name, 0666 ^ get_umask());
#endif
    trap_file = out_file;
    if (out_file < 0) {
	fprintf(stderr, _("%s: failed to open the file, %s: %s\n"),
	    invoked_name, strerror(errno), out_catalog_name);
	goto failed;
    }

    /*
     * Copy header.
     */
    if (read(in_file, buffer, 16) != 16) {
	fprintf(stderr, _("%s: failed to read the file, %s: %s\n"),
	    invoked_name, strerror(errno), in_catalog_name);
        goto failed;
    }
    in_subbook_count = eb_uint2(buffer);

    if (write(out_file, buffer, 16) != 16) {
	fprintf(stderr, _("%s: failed to write the file, %s: %s\n"),
	    invoked_name, strerror(errno), out_catalog_name);
        goto failed;
    }
    out_file_offset = 16;

    /*
     * Copy basic information of subbooks.
     */
    for (i = 0; i < in_subbook_count; i++) {
	/*
	 * Read subbook entry.
	 */
	if (read(in_file, buffer, catalog_size) != catalog_size) {
	    fprintf(stderr, _("%s: failed to read the file, %s: %s\n"),
		invoked_name, strerror(errno), in_catalog_name);
	    goto failed;
	}

	/*
	 * Check whether `subbook_name_list' has a directory name of
	 * this subbook.  If not, we ignore this subbook.
	 */
	if (disc_code == EB_DISC_EB) {
	    strncpy(directory_name, buffer + 2 + EB_MAX_EB_TITLE_LENGTH,
		EB_MAX_DIRECTORY_NAME_LENGTH);
	} else {
	    strncpy(directory_name, buffer + 2 + EB_MAX_EPWING_TITLE_LENGTH,
		EB_MAX_DIRECTORY_NAME_LENGTH);
	}
	directory_name[EB_MAX_DIRECTORY_NAME_LENGTH] = '\0';

	if (subbook_name_count == 0)
	    subbbook_map_table[i] = i;
	else {
	    int subbook_index;

	    subbook_index = find_subbook_name(subbook_name_list,
		subbook_name_count, directory_name);
	    if (subbook_index < 0)
		continue;
	    subbbook_map_table[i] = subbook_index;
	}

	/*
	 * Write the subbook entry.
	 */
	if (write(out_file, buffer, catalog_size) != catalog_size) {
	    fprintf(stderr, _("%s: failed to write the file, %s: %s\n"),
		invoked_name, strerror(errno), out_catalog_name);
	    goto failed;
	}

	out_file_offset += catalog_size;
    }

    /*
     * Copy extended information of subbooks.
     */
    if (disc_code == EB_DISC_EPWING) {
	for (i = 0; i < in_subbook_count; i++) {
	    if (read(in_file, buffer, catalog_size) != catalog_size) {
		fprintf(stderr, _("%s: failed to read the file, %s: %s\n"),
		    invoked_name, strerror(errno), in_catalog_name);
		goto failed;
	    }
	    if (subbbook_map_table[i] == EB_SUBBOOK_INVALID)
		continue;
	    if (write(out_file, buffer, catalog_size) != catalog_size) {
		fprintf(stderr, _("%s: failed to write the file, %s: %s\n"),
		    invoked_name, strerror(errno), out_catalog_name);
		goto failed;
	    }

	    out_file_offset += catalog_size;
	}
    }

    /*
     * Check whether all subbooks in `subbook_name_list' are found.
     */
    for (i = 0; i < subbook_name_count; i++) {
	for (j = 0; j < in_subbook_count; j++) {
	    if (subbbook_map_table[j] == i)
		break;
	}
	if (in_subbook_count <= j) {
	    fprintf(stderr, _("%s: warning: no such subbook: %s\n"),
		invoked_name, subbook_name_list[i]);
	}
    }

    /*
     * Copy rest of the catalog file.
     */
    for (;;) {
	ssize_t read_length;

	read_length = read(in_file, buffer, EB_SIZE_PAGE);
	if (read_length == 0) {
	    break;
	} else if (read_length < 0) {
	    fprintf(stderr, _("%s: failed to read the file, %s: %s\n"),
		invoked_name, strerror(errno), in_catalog_name);
	    goto failed;
	}
	if (write(out_file, buffer, read_length) != read_length) {
	    fprintf(stderr, _("%s: failed to write the file, %s: %s\n"),
		invoked_name, strerror(errno), out_catalog_name);
	    goto failed;
	}
	out_file_offset += read_length;
    }

    /*
     * Fill the current page with 0.
     */
    if (0 < out_file_offset % EB_SIZE_PAGE) {
	size_t pad_length;

	pad_length = EB_SIZE_PAGE - (out_file_offset % EB_SIZE_PAGE);
	memset(buffer, 0, EB_SIZE_PAGE);
	if (write(out_file, buffer, pad_length) != pad_length) {
	    fprintf(stderr, _("%s: failed to write the file, %s: %s\n"),
		invoked_name, strerror(errno), out_catalog_name);
	    goto failed;
	}
    }

    /*
     * Fix the number of subbook.
     */
    if (subbook_name_count == 0) {
	buffer[0] = (in_subbook_count >> 8) & 0xff;
	buffer[1] =  in_subbook_count       & 0xff;
    } else {
	buffer[0] = (subbook_name_count >> 8) & 0xff;
	buffer[1] =  subbook_name_count       & 0xff;
    }
    if (lseek(out_file, 0, SEEK_SET) < 0) {
	fprintf(stderr, _("%s: failed to seek the file, %s: %s\n"),
	    invoked_name, strerror(errno), out_catalog_name);
	goto failed;
    }
    if (write(out_file, buffer, 2) != 2) {
	fprintf(stderr, _("%s: failed to write the file, %s: %s\n"),
	    invoked_name, strerror(errno), out_catalog_name);
	goto failed;
    }

    /*
     * Close files.
     */
    close(in_file);
    close(out_file);
    trap_file = -1;

    return 0;

    /*
     * An error occurs...
     */
  failed:
    if (0 <= in_file)
	close(in_file);
    if (0 <= out_file)
	close(out_file);
    return -1;
}


/*
 * Copy a file from `in_file_name' to `out_file_name'.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
static int
copy_file(const char *out_file_name, const char *in_file_name)
{
    unsigned char buffer[EB_SIZE_PAGE];
    size_t copied_length;
    struct stat in_status;
    int in_file = -1, out_file = -1;
    ssize_t read_result;
    struct utimbuf utim;

    /*
     * Check for the input file.
     */
    if (stat(in_file_name, &in_status) != 0 || !S_ISREG(in_status.st_mode)) {
	fprintf(stderr, _("%s: no such file: %s\n"), invoked_name,
	    in_file_name);
	goto failed;
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

#ifdef O_CREAT
    out_file = open(out_file_name, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
	0666 ^ get_umask());
#else
    out_file = creat(out_file_name, 0666 ^ get_umask());
#endif
    trap_file = out_file;
    if (out_file < 0) {
	fprintf(stderr, _("%s: failed to open the file, %s: %s\n"),
	    invoked_name, strerror(errno), out_file_name);
	goto failed;
    }

    /*
     * Read data from the input file, compress the data, and then
     * write them to the output file.
     */
    copied_length = 0;
    for (;;) {
	/*
	 * Read data from `in_file', and write them to `out_file'.
	 */
	read_result = read(in_file, buffer, EB_SIZE_PAGE);
	if (read_result == 0) {
	    break;
	} else if (read_result < 0) {
	    fprintf(stderr, _("%s: failed to read from the file, %s: %s\n"),
		invoked_name, strerror(errno), in_file_name);
	    goto failed;
	}

	/*
	 * Write decoded data to `out_file'.
	 */
	if (write(out_file, buffer, read_result) != read_result) {
	    fprintf(stderr, _("%s: failed to write to the file, %s: %s\n"),
		invoked_name, strerror(errno), out_file_name);
	    goto failed;
	}
	copied_length += read_result;
    }

    /*
     * Close files.
     */
    close(in_file);
    close(out_file);
    trap_file = -1;

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


/*
 * Search `subbook_name_list[]' for `pattern'.
 * `subbook_name_count' is length of `subbook_name_list[]'.
 *
 * If found, the function returns index of the element.  Otherwise it 
 * returns -1.
 */
static int
find_subbook_name(char subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count, const char *pattern)
{
    char canonicalized_pattern[EB_MAX_FILE_NAME_LENGTH + 1];
    char *space;
    int i;

    strcpy(canonicalized_pattern, pattern);
    space = strchr(canonicalized_pattern, ' ');
    if (space != NULL)
	*space = '\0';

    for (i = 0; i < subbook_name_count; i++) {
	if (strcasecmp(subbook_name_list[i], canonicalized_pattern) == 0)
	    return i;
    }

    return -1;
}
