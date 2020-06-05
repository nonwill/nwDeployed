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

#if defined(DOS_FILE_PATH) && defined(HAVE_MBSTRING_H)
/* a path may contain double-byte chars in SJIS. */
#include <mbstring.h>
#define strchr  _mbschr
#define strrchr _mbsrchr
#endif

/*
 * Program name and version.
 */
const char *program_name = "ebzip";
const char *program_version = VERSION;
const char *invoked_name;

/*
 * Command line options.
 */
static const char *short_options = "fhikl:no:qs:S:tT:uvw:zr:";
static struct option long_options[] = {
    {"force-overwrite",   no_argument,       NULL, 'f'},
    {"help",              no_argument,       NULL, 'h'},
    {"information",       no_argument,       NULL, 'i'},
    {"keep",              no_argument,       NULL, 'k'},
    {"level",             required_argument, NULL, 'l'},
    {"no-overwrite",      no_argument,       NULL, 'n'},
    {"output-directory",  required_argument, NULL, 'o'},
    {"quiet",             no_argument,       NULL, 'q'},
    {"silent",            no_argument,       NULL, 'q'},
    {"skip-content",      required_argument, NULL, 's'},
    {"subbook",           required_argument, NULL, 'S'},
    {"test",              no_argument,       NULL, 't'},
    {"uncompress",        no_argument,       NULL, 'u'},
    {"version",           no_argument,       NULL, 'v'},
    {"overwrite",         required_argument, NULL, 'w'},
    {"compress",          no_argument,       NULL, 'z'},
    {"slice-number",      required_argument, NULL, 'r'},
    {NULL, 0, NULL, 0}
};

/*
 * Zip level.
 */
int ebzip_level = EBZIP_DEFAULT_LEVEL;

/*
 * Keep mode flag.
 */
int ebzip_keep_flag = EBZIP_DEFAULT_KEEP;

/*
 * Quiet mode flag.
 */
int ebzip_quiet_flag = EBZIP_DEFAULT_QUIET;

/*
 * Test mode flag.
 */
int ebzip_test_flag = EBZIP_DEFAULT_TEST;

/*
 * Overwrite mode.
 */
int ebzip_overwrite_mode = EBZIP_DEFAULT_OVERWRITE;

/*
 * Target contents.
 */
int ebzip_skip_flag_font    = EBZIP_DEFAULT_SKIP_FONT;
int ebzip_skip_flag_graphic = EBZIP_DEFAULT_SKIP_GRAPHIC;
int ebzip_skip_flag_movie   = EBZIP_DEFAULT_SKIP_MOVIE;
int ebzip_skip_flag_sound   = EBZIP_DEFAULT_SKIP_SOUND;

/*
 * Number of slices to load at a time.
 */
int ebzip_slice_number = EBZIP_DEFAULT_SLICE_NUMBER;

/*
 * List of files to be unlinked.
 */
String_List unlinking_files;

/*
 * Operation modes.
 */
#define EBZIP_ACTION_ZIP		0
#define EBZIP_ACTION_UNZIP		1
#define EBZIP_ACTION_INFO		2

static int action_mode = EBZIP_ACTION_ZIP;

/*
 * A list of subbook names to be compressed/uncompressed.
 */
static char
subbook_name_list[EB_MAX_SUBBOOKS][EB_MAX_DIRECTORY_NAME_LENGTH + 1];
static int subbook_name_count = 0;

/*
 * Unexported functions.
 */
static int parse_zip_level(const char *argument, int *zip_level);
static int parse_slice_number(const char *argument, int *slice_number);
static int parse_skip_content_argument(const char *argument);
static void output_help(void);


int
main(int argc, char *argv[])
{
    EB_Error_Code error_code;
    char out_top_path[PATH_MAX + 1];
    char book_path[PATH_MAX + 1];
    int ch;
    char *last_slash, *last_backslash;
    char *invoked_base_name;

    invoked_name = argv[0];
    strcpy(out_top_path, EBZIP_DEFAULT_OUTPUT_DIRECTORY);

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
     * Determine the default action.
     */
    last_slash = strrchr(argv[0], '/');
#ifndef DOS_FILE_PATH
    last_backslash = NULL;
#else
    last_backslash = strrchr(argv[0], '\\');
#endif

    if (last_slash == NULL && last_backslash == NULL)
	invoked_base_name = argv[0];
    else if (last_slash == NULL)
	invoked_base_name = last_backslash + 1;
    else if (last_backslash == NULL)
	invoked_base_name = last_slash + 1;
    else if (last_slash < last_backslash)
	invoked_base_name = last_backslash + 1;
    else
	invoked_base_name = last_slash + 1;

#ifndef EXEEXT_EXE
    if (strcmp(invoked_base_name, "ebunzip") == 0
	|| strcmp(invoked_base_name, "ebuunzip") == 0)
	action_mode = EBZIP_ACTION_UNZIP;
    else if (strcmp(invoked_base_name, "ebzipinfo") == 0
	     || strcmp(invoked_base_name, "ebuzipinfo") == 0)
	action_mode = EBZIP_ACTION_INFO;
#else /* EXEEXT_EXE */
    if (strcasecmp(invoked_base_name, "ebunzip") == 0
	|| strcasecmp(invoked_base_name, "ebunzip.exe") == 0
	|| strcasecmp(invoked_base_name, "ebuunzip") == 0
	|| strcasecmp(invoked_base_name, "ebuunzip.exe") == 0) {
	action_mode = EBZIP_ACTION_UNZIP;
    } else if (strcasecmp(invoked_base_name, "ebzipinfo") == 0
	|| strcasecmp(invoked_base_name, "ebzipinfo.exe") == 0
	|| strcasecmp(invoked_base_name, "ebuzipinfo") == 0
	|| strcasecmp(invoked_base_name, "ebuzipinfo.exe") == 0) {
	action_mode = EBZIP_ACTION_INFO;
    }
#endif /* EXEEXT_EXE */

    /*
     * Set overwrite mode.
     */
    if (!isatty(0))
	ebzip_overwrite_mode = EBZIP_OVERWRITE_NO;

    /*
     * Initialize list of files to be unlinked.
     */

    string_list_initialize(&unlinking_files);

    /*
     * Initialize EB Library.
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
        case 'f':
            /*
             * Obsolete option `-f'.  Set `force' to the overwrite flag.
             */
	    ebzip_overwrite_mode = EBZIP_OVERWRITE_FORCE;
	    break;

        case 'h':
            /*
             * Option `-h'.  Display help message, then exit.
             */
            output_help();
            exit(0);

        case 'i':
	    /*
	     * Option `-i'.  Information mode.
	     */
	    action_mode = EBZIP_ACTION_INFO;
	    break;

        case 'k':
            /*
             * Option `-k'.  Keep (don't delete) input files.
             */
	    ebzip_keep_flag = 1;
	    break;

        case 'l':
            /*
             * Option `-l'.  Specify compression level.
             */
	    if (parse_zip_level(optarg, &ebzip_level) < 0)
		exit(1);
	    break;

        case 'n':
            /*
             * Obsolete option `-n'.  Set `no' to the overwrite flag.
             */
	    ebzip_overwrite_mode = EBZIP_OVERWRITE_NO;
	    break;

        case 'o':
            /*
             * Option `-o'.  Output files under DIRECOTRY.
	     * The length of the file name
	     *    "<out_top_path>/subdir/subsubdir/file.ebz;1"
	     * must not exceed PATH_MAX.
             */
            if (PATH_MAX < strlen(optarg)) {
                fprintf(stderr, _("%s: too long output directory path\n"),
                    invoked_name);
                exit(1);
            }
            strcpy(out_top_path, optarg);
	    canonicalize_path(out_top_path);
	    if (PATH_MAX < strlen(out_top_path) + 1
		+ EB_MAX_DIRECTORY_NAME_LENGTH + 1
		+ EB_MAX_DIRECTORY_NAME_LENGTH + 1
		+ EB_MAX_FILE_NAME_LENGTH) {
		fprintf(stderr, _("%s: too long output directory path\n"),
		    invoked_name);
		goto die;
	    }
	    break;

        case 'q':
            /*
             * Option `-q'.  Set quiet flag.
             */
	    ebzip_quiet_flag = 1;
	    break;

        case 's':
            /*
             * Option `-s'.  Specify content type to be skipped.
             */
            if (parse_skip_content_argument(optarg) < 0)
                exit(1);
	    break;

        case 'S':
            /*
             * Option `-S'.  Specify target subbooks.
             */
            if (parse_subbook_name_argument(invoked_name, optarg,
		subbook_name_list, &subbook_name_count) < 0)
                exit(1);
            break;

        case 't':
            /*
             * Option `-t'.  Set test mode.
             */
	    ebzip_test_flag = 1;
	    break;

        case 'u':
            /*
             * Option `-u'.  Decompression mode.
             */
	    action_mode = EBZIP_ACTION_UNZIP;
	    break;

        case 'v':
            /*
             * Option `-v'.  Display version number, then exit.
             */
            output_version(program_name, program_version);
            exit(0);

	case 'w':
            /*
             * Option `-w'.  Set overwrite mode.
             */
	    if (strcasecmp(optarg, "confirm") == 0)
		ebzip_overwrite_mode = EBZIP_OVERWRITE_CONFIRM;
	    else if (strcasecmp(optarg, "force") == 0)
		ebzip_overwrite_mode = EBZIP_OVERWRITE_FORCE;
	    else if (strcasecmp(optarg, "no") == 0)
		ebzip_overwrite_mode = EBZIP_OVERWRITE_NO;
	    else {
		fprintf(stderr, _("%s: invalid overwrite mode: %s\n"),
		    invoked_name, optarg);
		output_try_help(invoked_name);
		goto die;
	    }
	    break;
		
        case 'z':
            /*
             * Option `-z'.  Compression mode.
             */
	    action_mode = EBZIP_ACTION_ZIP;
	    break;

        case 'r':
            /*
             * Option `-r'.  Specify a number of slices to load at a time.
             */
	    if (parse_slice_number(optarg, &ebzip_slice_number) < 0)
		exit(1);
	    break;

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
        strcpy(book_path, EBZIP_DEFAULT_BOOK_DIRECTORY);
    else
        strcpy(book_path, argv[optind]);

    if (is_ebnet_url(book_path)) {
        fprintf(stderr, "%s: %s\n", invoked_name,
	    eb_error_message(EB_ERR_EBNET_UNSUPPORTED));
	goto die;
    }
    canonicalize_path(book_path);

    /*
     * Compress the book.
     */
    switch (action_mode) {
    case EBZIP_ACTION_ZIP:
	if (ebzip_zip_book(out_top_path, book_path, subbook_name_list,
	    subbook_name_count) < 0) {
	    goto die;
	}
	break;
    case EBZIP_ACTION_UNZIP:
	if (ebzip_unzip_book(out_top_path, book_path, subbook_name_list,
	    subbook_name_count) < 0) {
	    goto die;
	}
	break;
    case EBZIP_ACTION_INFO:
	if (ebzip_zipinfo_book(book_path, subbook_name_list,
	    subbook_name_count) < 0) {
	    goto die;
	}
	break;
    }

    eb_finalize_library();
    unlink_files();
    string_list_finalize(&unlinking_files);

    return 0;

    /*
     * A critical error occurs...
     */
  die:
    eb_finalize_library();
    exit(1);
}


/*
 * Parse an argument to option `--level (-l)'.
 * If the argument is valid form, 0 is returned.
 * Otherwise -1 is returned.
 */
static int
parse_zip_level(const char *argument, int *zip_level)
{
    char *end_p;
    int level;

    level = (int)strtol(argument, &end_p, 10);
    if (!ASCII_ISDIGIT(*argument) || *end_p != '\0'
	|| level < 0 || ZIO_MAX_EBZIP_LEVEL < level) {
	fprintf(stderr, _("%s: invalid compression level `%s'\n"),
	    invoked_name, argument);
	fflush(stderr);
	return -1;
    }

    *zip_level = level;

    return 0;
}


/*
 * Parse an argument to option `--slice-number (-r)'.
 * If the argument is valid form, 0 is returned.
 * Otherwise -1 is returned.
 */
static int
parse_slice_number(const char *argument, int *slice_number)
{
    char *end_p;
    int level;

    level = (int)strtol(argument, &end_p, 10);
    if (!ASCII_ISDIGIT(*argument) || *end_p != '\0'
	|| level < 1 || EBZIP_MAX_SLICE_NUMBER < level) {
      fprintf(stderr, _("%s: invalid slice number `%s'\n"),
	  invoked_name, argument);
      fflush(stderr);
      return -1;
    }

    *slice_number = level;

    return 0;
}


/*
 * Parse an argument to option `--skip-content (-S)'.
 * If the argument is valid form, 0 is returned.
 * Otherwise -1 is returned.
 */
static int
parse_skip_content_argument(const char *argument)
{
    const char *argument_p = argument;
    char name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
    char *name_p;
    int i;

    while (*argument_p != '\0') {
	/*
	 * Take a next element in the argument.
	 */
	i = 0;
	name_p = name;
	while (*argument_p != ',' && *argument_p != '\0'
	    && i < EB_MAX_DIRECTORY_NAME_LENGTH) {
		*name_p = ASCII_TOLOWER(*argument_p);
	    i++;
	    name_p++;
	    argument_p++;
	}
	*name_p = '\0';
	if (*argument_p == ',')
	    argument_p++;
	else if (*argument_p != '\0') {
	    fprintf(stderr, _("%s: invalid content name `%s'\n"),
		invoked_name, name);
	    fflush(stderr);
	    return -1;
	}

	/*
	 * If the font name is not found in `font_list', it is added to
	 * the list.
	 */
	if (strcasecmp(name, "font") == 0) {
	    ebzip_skip_flag_font = 1;
	} else if (strcasecmp(name, "sound") == 0) {
	    ebzip_skip_flag_sound = 1;
	} else if (strcasecmp(name, "graphic") == 0) {
	    ebzip_skip_flag_graphic = 1;
	} else if (strcasecmp(name, "movie") == 0) {
	    ebzip_skip_flag_movie = 1;
	} else {
	    fprintf(stderr, _("%s: invalid content name `%s'\n"),
		invoked_name, name);
	    fflush(stderr);
	    return -1;
	}
    }

    return 0;
}


/*
 * Output help message to stdandard out.
 */
static void
output_help(void)
{
    printf(_("Usage: %s [option...] [book-directory]\n"), program_name);
    printf(_("Options:\n"));
    printf(_("  -f  --force-overwrite      set overwrite mode to `force'\n"));
    printf(_("                             (same as `--overwrite force')\n"));
    printf(_("  -h  --help                 display this help, then exit\n"));
    printf(_("  -i  --information          list information of compressed files\n"));
    printf(_("  -k  --keep                 don't delete original files\n"));
    printf(_("  -l INTEGER  --level INTEGER\n"));
    printf(_("                             compression level; 0..%d\n"),
	ZIO_MAX_EBZIP_LEVEL);
    printf(_("                             (default: %d)\n"),
	EBZIP_DEFAULT_LEVEL);
    printf(_("  -r INTEGER  --slice-number INTEGER\n"));
    printf(_("                             set a number of slices to load at a time; 1..%d\n"),
	EBZIP_MAX_SLICE_NUMBER);
    printf(_("                             (default: %d)\n"),
	EBZIP_DEFAULT_SLICE_NUMBER);
    printf(_("  -n  --no-overwrite         set overwrite mode to `no'\n"));
    printf(_("                             (same as `--overwrite no')\n"));
    printf(_("  -o DIRECTORY  --output-directory DIRECTORY\n"));
    printf(_("                             ouput files under DIRECTORY\n"));
    printf(_("                             (default: %s)\n"),
	EBZIP_DEFAULT_OUTPUT_DIRECTORY);
    printf(_("  -q  --quiet  --silence     suppress all warnings\n"));
    printf(_("  -s TYPE[,TYPE]  --skip-content TYPE[,TYPE...]\n"));
    printf(_("                             skip content; font, graphic, sound or movie\n"));
    printf(_("                             (default: none is skipped)\n"));
    printf(_("  -S SUBBOOK[,SUBBOOK...]  --subbook SUBBOOK[,SUBBOOK...]\n"));
    printf(_("                             target subbook\n"));
    printf(_("                             (default: all subbooks)\n"));
    printf(_("  -t  --test                 only check for input files\n"));
    printf(_("  -u  --uncompress           uncompress files\n"));
    printf(_("  -v  --version              display version number, then exit\n"));
    printf(_("  -w MODE  --overwrite MODE  set overwrite mode of output files;\n"));
    printf(_("                             confirm, force or no\n"));
    printf(_("                             (default: confirm)\n"));
    printf(_("  -z  --compress             compress files\n"));
    printf(_("\nArgument:\n"));
    printf(_("  book-directory             top directory of a CD-ROM book\n"));
    printf(_("                             (default: %s)\n"),
	EBZIP_DEFAULT_BOOK_DIRECTORY);

    printf(_("\nDefault action:\n"));
    printf(_("  When invoked as `ebuunzip', uncompression is the default action.\n"));
    printf(_("  When invoked as `ebuzipinfo', listing information is the default action.\n"));
    printf(_("  Otherwise, compression is the default action.\n"));
    printf(_("\nReport bugs to %s.\n"), MAILING_ADDRESS);
    fflush(stdout);
}
