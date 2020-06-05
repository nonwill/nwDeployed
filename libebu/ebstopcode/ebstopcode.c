/*                                                            -*- C -*-
 * Copyright (c) 2003-2006  Motoyuki Kasahara
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
#include <sys/types.h>
#include <stdlib.h>

#ifdef ENABLE_NLS
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <libintl.h>
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

#include "ebu/eb.h"
#include "ebu/error.h"
#include "ebu/text.h"
#include "ebu/appendix.h"

#ifdef HAVE_GETOPT_LOG
#include <getopt.h>
#else
#include "getopt.h"
#endif
#include "ebutils.h"

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
 * Character type tests and conversions.
 */
#define ASCII_ISDIGIT(c) ('0' <= (c) && (c) <= '9')
#define ASCII_ISUPPER(c) ('A' <= (c) && (c) <= 'Z')
#define ASCII_ISLOWER(c) ('a' <= (c) && (c) <= 'z')
#define ASCII_ISALPHA(c) \
 (ASCII_ISUPPER(c) || ASCII_ISLOWER(c))
#define ASCII_ISALNUM(c) \
 (ASCII_ISUPPER(c) || ASCII_ISLOWER(c) || ASCII_ISDIGIT(c))
#define ASCII_ISXDIGIT(c) \
 (ASCII_ISDIGIT(c) || ('A' <= (c) && (c) <= 'F') || ('a' <= (c) && (c) <= 'f'))
#define ASCII_TOUPPER(c) (('a' <= (c) && (c) <= 'z') ? (c) - 0x20 : (c))
#define ASCII_TOLOWER(c) (('A' <= (c) && (c) <= 'Z') ? (c) + 0x20 : (c))

/*
 * Default maximum length of text.
 */
#define DEFAULT_MAX_TEXT_LENGTH		EB_SIZE_PAGE

/*
 * Default book directory.
 */
#define DEFAULT_BOOK_DIRECTORY		"."

/*
 * Dummy stop code.
 *
 * Application cannot order EB Library not to use stop code.
 * EB Library guesses stop code automatically when appendix is not
 * given by application.
 *
 * Instead, we use dummy stop code. The code is never used in text
 * of CD-ROM book.
 */
#define DUMMY_STOP_CODE0		0x1f00
#define DUMMY_STOP_CODE1		0x0000

/*
 * Command line options.
 */
static const char *short_options = "c:hl:np:v";
static struct option long_options[] = {
    {"code",          required_argument, NULL, 'c'},
    {"help",          no_argument,       NULL, 'h'},
    {"text-length",   required_argument, NULL, 'l'},
    {"no-candidates", no_argument,       NULL, 'n'},
    {"text-position", required_argument, NULL, 'p'},
    {"version",       no_argument,       NULL, 'v'},
    {NULL, 0, NULL, 0}
};

/*
 * Program name and version.
 */
static const char *program_name = "ebstopcode";
static const char *program_version = VERSION;
static const char *invoked_name;

/*
 * Unexported functions.
 */
static int parse_stop_code_argument(const char *argument,
    unsigned int *stop_code0, unsigned int *stop_code1);
static int parse_text_length_argument(const char *argument,
    ssize_t *text_length);
static int parse_text_position_argument(const char *argument,
    EB_Position *text_position);
static void output_help(void);
static int scan_subbook_text(const char *book_directory,
    const char *subbook_name, EB_Position *text_position,
    ssize_t max_text_length, int show_stop_code_flag, unsigned int stop_code0,
    unsigned int stop_code1);
static EB_Error_Code hook_stop_code(EB_Book *book, EB_Appendix *appendix,
    void *container, EB_Hook_Code code, int argc, const unsigned int *argv);


int
main(int argc, char *argv[])
{
    const char *book_directory;
    ssize_t max_text_length;
    const char *subbook_name;
    int show_stop_code_flag;
    EB_Position text_position;
    unsigned int stop_code0, stop_code1;
    int ch;

    invoked_name = argv[0];
    max_text_length = DEFAULT_MAX_TEXT_LENGTH;
    show_stop_code_flag = 1;
    stop_code0 = DUMMY_STOP_CODE0;
    stop_code1 = DUMMY_STOP_CODE1;
    text_position.page = 0;
    text_position.offset = 0;

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
     * Parse command line options.
     */
    for (;;) {
	ch = getopt_long(argc, argv, short_options, long_options, NULL);
	if (ch == -1)
	    break;

	switch (ch) {
        case 'c':
            /*
             * Option `-c'.  Specify stop code manually.
             */
	    if (parse_stop_code_argument(optarg, &stop_code0, &stop_code1) < 0)
		goto die;
	    max_text_length = 0;
            break;

	case 'h':
	    /*
	     * Option `-h'.  Display help message, then exit.
	     */
	    output_help();
	    exit(0);

        case 'l':
            /*
             * Option `-l'.  Specify maximum length of text.
             */
	    if (parse_text_length_argument(optarg, &max_text_length) < 0)
		goto die;
	    stop_code0 = DUMMY_STOP_CODE0;
	    stop_code1 = DUMMY_STOP_CODE1;
            break;

	case 'n':
	    /*
	     * Option `-n'.  Do not output stop code candidates.
	     */
	    show_stop_code_flag = 0;
	    break;

	case 'p':
	    /*
	     * Option `-p'.  Specify text position.
	     */
	    if (parse_text_position_argument(optarg, &text_position) < 0)
		goto die;
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
    if (argc - optind < 2) {
	fprintf(stderr, _("%s: too few argument\n"), invoked_name);
	output_try_help(invoked_name);
	goto die;
    }
    if (2 < argc - optind) {
	fprintf(stderr, _("%s: too many arguments\n"), invoked_name);
	output_try_help(invoked_name);
	goto die;
    }
    if (argc - optind == 2) {
	book_directory = argv[optind];
	subbook_name = argv[optind + 1];
    } else {
	book_directory = DEFAULT_BOOK_DIRECTORY;
	subbook_name = argv[optind];
    }

    /*
     * Scan stop code in text.
     */
    if (scan_subbook_text(book_directory, subbook_name, &text_position,
	max_text_length, show_stop_code_flag, stop_code0, stop_code1) < 0)
	goto die;

    return 0;

  die:
    fflush(stdout);
    exit(1);
}


/*
 * Parse stop code given as argument of the `-c' option.
 */
static int
parse_stop_code_argument(const char *argument, unsigned int *stop_code0,
    unsigned int *stop_code1)
{
    const char *p = argument;
    const char *foundp;

    /*
     * Parse stop_code0.
     */
    while (*p == ' ' || *p == '\t')
	p++;
    if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X'))
	p += 2;

    foundp = p;
    while (ASCII_ISXDIGIT(*p))
	p++;

    if (p == foundp || (*p != ' ' && *p != '\t'))
	goto failed;
    *stop_code0 = strtol(foundp, NULL, 16);
    if (*stop_code0 != 0x1f09 && *stop_code0 != 0x1f41)
	goto failed;

    /*
     * Parse stop_code1.
     */
    while (*p == ' ' || *p == '\t')
	p++;
    if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X'))
	p += 2;

    foundp = p;
    while (ASCII_ISXDIGIT(*p))
	p++;

    if (p == foundp || (*p != ' ' && *p != '\t' && *p != '\0'))
	goto failed;
    *stop_code1 = strtol(foundp, NULL, 16);
    if (0xffff < *stop_code1)
	goto failed;

    while (*p == ' ' || *p == '\t')
	p++;
    if (*p != '\0')
	goto failed;

    return 0;

    /*
     * An error occurs...
     */
  failed:
    fprintf(stderr, _("%s: invalid stop code `%s'\n"), invoked_name,
	argument);
    return -1;
}


/*
 * Parse stop code given as argument of the `-t' option.
 */
static int
parse_text_length_argument(const char *argument, ssize_t *text_length)
{
    const char *p = argument;
    const char *foundp;

    while (*p == ' ' || *p == '\t')
	p++;

    foundp = p;
    while (ASCII_ISDIGIT(*p))
	p++;
    if (p == foundp)
	goto failed;
    while (*p == ' ' || *p == '\t')
	p++;
    if (*p != '\0')
	goto failed;

    *text_length = strtol(foundp, NULL, 10);
    return 0;

    /*
     * An error occurs...
     */
  failed:
    fprintf(stderr, _("%s: invalid text length `%s'\n"), invoked_name,
	argument);
    return -1;
}


/*
 * Parse text position given as argument of the `-p' option.
 */
static int
parse_text_position_argument(const char *argument, EB_Position *text_position)
{
    const char *p = argument;
    const char *end_p;
    int page;
    int offset;

    /*
     * Parse page.
     */
    while (*p == ' ' || *p == '\t')
	p++;

    if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X'))
	p += 2;
    page = strtol(p, (char **)&end_p, 16);
    if (end_p == p || *end_p != ':')
	goto failed;
    if (page <= 0)
	goto failed;

    /*
     * Parse offset.
     */
    p = end_p + 1;

    if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X'))
	p += 2;
    offset = strtol(p, (char **)&end_p, 16);
    if (end_p == p)
	goto failed;
    while (*end_p == ' ' || *end_p == '\t')
	end_p++;
    if (*end_p != '\0')
	goto failed;
    if (offset < 0 || EB_SIZE_PAGE <= offset)
	goto failed;

    text_position->page = page;
    text_position->offset = offset;

    return 0;

    /*
     * An error occurs...
     */
  failed:
    fprintf(stderr, _("%s: invalid text position `%s'\n"), invoked_name,
	argument);
    return -1;
}


/*
 * Output help message to standard out, then exit.
 */
static void
output_help(void)
{
    printf(_("Usage: %s [option...] [book-directory] subbook\n"),
	program_name);
    printf(_("Options:\n"));
    printf(_("  -c CODE, --code CODE\n"));
    printf(_("                             set stop code manually\n"));
    printf(_("  -h  --help                 display this help, then exit\n"));
    printf(_("  -l LENGTH, --text-length LENGTH\n"));
    printf(_("                             maximum length of output text\n"));
    printf(_("                             (default: %d)\n"),
	DEFAULT_MAX_TEXT_LENGTH);
    printf(_("  -n  --no-candidates        suppress stop code candidates\n"));
    printf(_("  -p PAGE:OFFSET, --text-position PAGE:OFFSET\n"));
    printf(_("                             start position of text\n"));
    printf(_("  -v  --version              display version number, then exit\n"));
    printf(_("\nArgument:\n"));
    printf(_("  book-directory             top directory of a CD-ROM book\n"));
    printf(_("                             (default: %s)\n"),
	DEFAULT_BOOK_DIRECTORY);
    printf(_("\nReport bugs to %s.\n"), MAILING_ADDRESS);
    fflush(stdout);
}


/*
 * Scan stop code in text.
 */
static int
scan_subbook_text(const char *book_directory, const char *subbook_name,
    EB_Position *text_position, ssize_t max_text_length,
    int show_stop_code_flag, unsigned int stop_code0, unsigned int stop_code1)
{
    EB_Error_Code error_code;
    EB_Book book;
    EB_Hookset hookset;
    EB_Hook hook;
    EB_Appendix appendix;
    EB_Appendix_Subbook appendix_subbook;
    EB_Subbook_Code subbook_code;
    char text[EB_SIZE_PAGE];
    ssize_t text_length;

    /*
     * Initialize EB Library, `book', `appendix' and `hookset'.
     */
    eb_initialize_library();
    eb_initialize_book(&book);
    eb_initialize_appendix(&appendix);
    eb_initialize_hookset(&hookset);

    /*
     * Bind `book'.
     */
    error_code = eb_bind(&book, book_directory);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, _("%s: failed to bind the book, %s: %s\n"),
            program_name, eb_error_message(error_code), book_directory);
        goto die;
    }

    /*
     * Get a subbook code from the subbook name.
     */
    error_code = find_subbook(&book, subbook_name, &subbook_code);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: %s: %s\n",
            program_name, eb_error_message(error_code), subbook_name);
        goto die;
    }

    /*
     * Set current subbook.
     */
    if (eb_set_subbook(&book, subbook_code) < 0) {
        fprintf(stderr, _("%s: failed to set the current subbook, %s\n"),
            program_name, eb_error_message(error_code));
        goto die;
    }

    /*
     * Set stop-code manually.
     * (we hack `appendix' directly.)
     */
    appendix.code               = subbook_code;
    appendix.subbook_current    = &appendix_subbook;
    appendix_subbook.code       = subbook_code;
    appendix_subbook.stop_code0 = stop_code0;
    appendix_subbook.stop_code1 = stop_code1;

    /*
     * Set text hooks (for 0x1f41 and 0x1f09).
     */
    if (show_stop_code_flag) {
	hook.code = EB_HOOK_BEGIN_KEYWORD;
	hook.function = hook_stop_code;
	eb_set_hook(&hookset, &hook);

	hook.code = EB_HOOK_SET_INDENT;
	hook.function = hook_stop_code;
	eb_set_hook(&hookset, &hook);
    }

    /*
     * Get a position where the text data starts, if text_position
     * is {page=0, offset=0}.
     */
    if (text_position->page == 0 && text_position->offset == 0) {
	error_code = eb_text(&book, text_position);
	if (error_code != EB_SUCCESS) {
	    fprintf(stderr, _("%s: failed to get text information, %s\n"),
		program_name, eb_error_message(error_code));
	    goto die;
	}
    }

    /*
     * Read text.
     */
    error_code = eb_seek_text(&book, text_position);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: %s\n",
	    program_name, eb_error_message(error_code));
        goto die;
    }

    text_length = 0;
    while (max_text_length == 0 || text_length < max_text_length) {
	ssize_t result_length;
	size_t read_length;

	if (max_text_length == 0
	    || sizeof(text) - 1 <= max_text_length - text_length) {
	    read_length = sizeof(text) - 1;
	} else {
	    read_length = max_text_length - text_length;
	}
	error_code = eb_read_text(&book, &appendix, &hookset, NULL,
	    read_length, text, &result_length);
	if (error_code != EB_SUCCESS) {
            fprintf(stderr, _("%s: failed to read text, %s\n"),
                program_name, eb_error_message(error_code));
            goto die;
        }
	if (result_length <= 0)
	    break;
	fputs_eucjp_to_locale(text, stdout);
	text_length += result_length;
    }

    /*
     * Finalize `hookset', `appendix', `book' and EB Library.
     */
    eb_finalize_hookset(&hookset);
    eb_finalize_appendix(&appendix);
    eb_finalize_book(&book);
    eb_finalize_library();
    return 0;

    /*
     * An error occurs...
     */
  die:
    eb_finalize_hookset(&hookset);
    eb_finalize_appendix(&appendix);
    eb_finalize_book(&book);
    eb_finalize_library();
    return -1;
}


/*
 * Hook function for EB_HOOK_BEGIN_KEYWORD and EB_HOK_SET_INDENT.
 */
static EB_Error_Code
hook_stop_code(EB_Book *book, EB_Appendix *appendix, void *container,
    EB_Hook_Code code, int argc, const unsigned int *argv)
{
    char string[EB_SIZE_PAGE];

    if (0 < book->text_context.printable_count) {
	sprintf(string, "\n=== stop-code?: 0x%04x 0x%04x ===\n",
	    argv[0], argv[1]);
	eb_write_text_string(book, string);
    }

    return EB_SUCCESS;
}
