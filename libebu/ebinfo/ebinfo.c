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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "ebu/eb.h"
#include "ebu/error.h"
#include "ebu/font.h"
#include "ebu/booklist.h"

#ifdef HAVE_GETOPT_LOG
#include <getopt.h>
#else
#include "getopt.h"
#endif
#include "ebutils.h"

#ifdef ENABLE_NLS
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <libintl.h>
#endif

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
 * Unexported functions.
 */
static void output_error_message(EB_Error_Code error_code);
static EB_Error_Code output_booklist(const char *url);
static EB_Error_Code output_information(const char *book_path, int multi_flag);
static EB_Error_Code output_multi_information(EB_Book *book);
static void output_help(void);

/*
 * Program name and version.
 */
static const char *program_name = "ebinfo";
static const char *program_version = VERSION;
static const char *invoked_name;

/*
 * Command line options.
 */
static const char *short_options = "hlmv";
static struct option long_options[] = {
  {"help",         no_argument, NULL, 'h'},
  {"book-list",    no_argument, NULL, 'l'},
  {"multi-search", no_argument, NULL, 'm'},
  {"version",      no_argument, NULL, 'v'},
  {NULL, 0, NULL, 0}
};

/*
 * Default output directory
 */
#define DEFAULT_BOOK_DIRECTORY	"."

int
main(int argc, char *argv[])
{
    EB_Error_Code error_code;
    int ch;
    char *book_path;
    int booklist_flag;
    int multi_flag;

    invoked_name = argv[0];

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
    multi_flag = 0;
    booklist_flag = 0;

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

	case 'l':
	    /*
	     * Option `-l'.  Display book list on an EBNET server.
	     */
	    booklist_flag = 1;
	    break;

	case 'm':
	    /*
	     * Option `-m'.  Also output multi-search information.
	     */
	    multi_flag = 1;
	    break;

	case 'v':
	    /*
	     * Option `-v'.  Display version number, then exit.
	     */
	    output_version(program_name, program_version);
	    exit(0);

	default:
	    output_try_help(invoked_name);
	    exit(1);
	}
    }

    /*
     * Check the number of rest arguments.
     */
    if (1 < argc - optind) {
	fprintf(stderr, _("%s: too many arguments\n"), invoked_name);
	output_try_help(invoked_name);
	exit(1);
    }

    /*
     * Output information about the book.
     */
    if (argc == optind)
	book_path = DEFAULT_BOOK_DIRECTORY;
    else
	book_path = argv[optind];

    if (booklist_flag)
	error_code = output_booklist(book_path);
    else
	error_code = output_information(book_path, multi_flag);
    if (error_code != EB_SUCCESS)
	exit(1);

    return 0;
}


/*
 * Output an error message to standard error.
 */
static void
output_error_message(EB_Error_Code error_code)
{
    fprintf(stderr, "%s: %s\n", invoked_name, eb_error_message(error_code));
    fflush(stderr);
}


/*
 * Output a list of books that an EBNET server provides.
 */
static EB_Error_Code
output_booklist(const char *url)
{
    EB_BookList booklist;
    EB_Error_Code error_code;
    int book_count;
    char *name;
    char *title;
    size_t name_length;
    int i;

    error_code = eb_initialize_library();
    if (error_code != EB_SUCCESS) {
	output_error_message(error_code);
	return error_code;
    }

    eb_initialize_booklist(&booklist);

    error_code = eb_bind_booklist(&booklist, url);
    if (error_code != EB_SUCCESS) {
	output_error_message(error_code);
	return error_code;
    }

    printf("%-20s  %s\n", _("Name"), _("Title"));

    error_code = eb_booklist_book_count(&booklist, &book_count);
    if (error_code != EB_SUCCESS) {
	output_error_message(error_code);
	return error_code;
    }

    for (i = 0; i < book_count; i++) {
	error_code = eb_booklist_book_name(&booklist, i, &name);
	if (error_code != EB_SUCCESS) {
	    output_error_message(error_code);
	    break;
	}
	error_code = eb_booklist_book_title(&booklist, i, &title);
	if (error_code != EB_SUCCESS) {
	    output_error_message(error_code);
	    break;
	}

	name_length = strlen(name);

	printf("%-20s  ", name);
	fputs_eucjp_to_locale(title, stdout);
	if (4 < name_length && strcmp(name + name_length - 4, ".app") == 0)
	    fputs(" (appendix)", stdout);
	fputc('\n', stdout);
    }

    eb_finalize_booklist(&booklist);
    eb_finalize_library();

    return EB_SUCCESS;
}


/*
 * Output information about the book at `path'.
 * If `multi_flag' is enabled, multi-search information are also output.
 */
static EB_Error_Code
output_information(const char *book_path, int multi_flag)
{
    EB_Book book;
    EB_Error_Code return_code = EB_SUCCESS;
    EB_Error_Code error_code;
    EB_Disc_Code disc_code;
    EB_Character_Code character_code;
    EB_Subbook_Code subbook_list[EB_MAX_SUBBOOKS];
    EB_Font_Code font_list[EB_MAX_FONTS];
    char title[EB_MAX_TITLE_LENGTH + 1];
    char directory[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
    int font_height, font_start, font_end;
    int subbook_count;
    int font_count;
    int i, j;

    /*
     * Start to use a book.
     */
    error_code = eb_initialize_library();
    if (error_code != EB_SUCCESS) {
	output_error_message(error_code);
	return_code = error_code;
	goto failed;
    }
    eb_initialize_book(&book);
    error_code = eb_bind(&book, book_path);
    if (error_code != EB_SUCCESS) {
	output_error_message(error_code);
	return_code = error_code;
	goto failed;
    }

    /*
     * Output disc type.
     */
    error_code = eb_disc_type(&book, &disc_code);
    if (error_code != EB_SUCCESS) {
	output_error_message(error_code);
	return_code = error_code;
	goto failed;
    }
    printf(_("disc type: "));
    if (disc_code == EB_DISC_EB)
	printf("EB/EBG/EBXA/EBXA-C/S-EBXA\n");
    else
	printf("EPWING\n");

    /*
     * Output character code.
     */
    error_code = eb_character_code(&book, &character_code);
    if (error_code != EB_SUCCESS) {
	output_error_message(error_code);
	return_code = error_code;
	goto failed;
    }
    printf(_("character code: "));
    switch (character_code) {
    case EB_CHARCODE_ISO8859_1:
	printf("ISO 8859-1\n");
	break;
    case EB_CHARCODE_JISX0208:
	printf("JIS X 0208\n");
	break;
    case EB_CHARCODE_JISX0208_GB2312:
	printf("JIS X 0208 + GB 2312\n");
	break;
    case EB_CHARCODE_UTF8:
	printf("UTF-8\n");
	break;
    default:
	printf(_("unknown\n"));
	break;
    }

    /*
     * Output the number of subbooks in the book.
     */
    error_code = eb_subbook_list(&book, subbook_list, &subbook_count);
    if (error_code != EB_SUCCESS) {
	output_error_message(error_code);
	return_code = error_code;
	goto failed;
    }
    printf(_("the number of subbooks: %d\n\n"), subbook_count);

    /*
     * Output information about each subbook.
     */
    for (i = 0; i < subbook_count; i++) {
	printf(_("subbook %d:\n"), i + 1);

	/*
	 * Output a title of the subbook.
	 */
	error_code = eb_subbook_title2(&book, subbook_list[i], title);
	if (error_code != EB_SUCCESS) {
	    return_code = error_code;
	    continue;
	}
	fputs(_("  title: "), stdout);
	fputs_eucjp_to_locale(title, stdout);
	fputc('\n', stdout);

	/*
	 * Output a directory name of the subbook.
	 */
	error_code = eb_subbook_directory2(&book, subbook_list[i], directory);
	if (error_code != EB_SUCCESS) {
	    return_code = error_code;
	    continue;
	}
	printf(_("  directory: %s\n"), directory);

	/*
	 * Set the current subbook to `i'.
	 */
	error_code = eb_set_subbook(&book, subbook_list[i]);
	if (error_code != EB_SUCCESS) {
	    output_error_message(error_code);
	    return_code = error_code;
	    continue;
	}

	/*
	 * Output supported methods.
	 */
	printf(_("  search methods: "));
	if (eb_have_word_search(&book))
	    fputs(_("word "), stdout);
	if (eb_have_endword_search(&book))
	    fputs(_("endword "), stdout);
	if (eb_have_keyword_search(&book))
	    fputs(_("keyword "), stdout);
	if (eb_have_cross_search(&book))
	    fputs(_("cross "), stdout);
	if (eb_have_multi_search(&book))
	    fputs(_("multi "), stdout);
	if (eb_have_menu(&book))
	    fputs(_("menu "), stdout);
	if (eb_have_image_menu(&book))
	    fputs(_("image-menu "), stdout);
	if (eb_have_copyright(&book))
	    fputs(_("copyright "), stdout);
	fputc('\n', stdout);

	/*
	 * Output a font list.
	 */
	fputs(_("  font sizes: "), stdout);
	error_code = eb_font_list(&book, font_list, &font_count);
	if (error_code != EB_SUCCESS) {
	    fputc('\n', stdout);
	    output_error_message(error_code);
	    return_code = error_code;
	} else {
	    for (j = 0; j < font_count; j++) {
		error_code = eb_font_height2(font_list[j], &font_height);
		if (error_code == EB_SUCCESS)
		    printf("%d ", font_height);
		else {
		    output_error_message(error_code);
		    return_code = error_code;
		}
	    }
	    fputc('\n', stdout);
	}

	/*
	 * Output character range of the narrow font.
	 */
        fputs(_("  narrow font characters: "), stdout);
	if (eb_have_narrow_font(&book)) {
	    do {
		error_code = eb_set_font(&book, font_list[0]);
		if (error_code != EB_SUCCESS)
		    break;
		error_code = eb_narrow_font_start(&book, &font_start);
		if (error_code != EB_SUCCESS)
		    break;
		error_code = eb_narrow_font_end(&book, &font_end);
		if (error_code != EB_SUCCESS)
		    break;
	    } while (0);

	    if (error_code == EB_SUCCESS)
		printf("0x%04x -- 0x%04x\n", font_start, font_end);
	    else {
		fputc('\n', stdout);
		output_error_message(error_code);
		return_code = error_code;
	    }
	} else {
	    fputc('\n', stdout);
	}

	/*
	 * Output character range of the wide font.
	 */
	printf(_("  wide font characters: "));
	if (eb_have_wide_font(&book)) {
	    do {
		error_code = eb_set_font(&book, font_list[0]);
		if (error_code != EB_SUCCESS)
		    break;
		error_code = eb_wide_font_start(&book, &font_start);
		if (error_code != EB_SUCCESS)
		    break;
		error_code = eb_wide_font_end(&book, &font_end);
		if (error_code != EB_SUCCESS)
		    break;
	    } while (0);

	    if (error_code == EB_SUCCESS)
		printf("0x%04x -- 0x%04x\n", font_start, font_end);
	    else {
		fputc('\n', stdout);
		output_error_message(error_code);
		return_code = error_code;
	    }
	} else {
	    fputc('\n', stdout);
	}

	if (multi_flag) {
	    error_code = output_multi_information(&book);
	    if (error_code != EB_SUCCESS)
		return_code = error_code;
	}
	fputc('\n', stdout);
    }
    fflush(stdout);

    /*
     * End to use the book.
     */
    eb_finalize_book(&book);
    eb_finalize_library();

    return return_code;

    /*
     * An error occurs...
     */
  failed:
    fflush(stdout);
    fflush(stderr);
    eb_finalize_book(&book);
    eb_finalize_library();

    return return_code;
}

/*
 * Output information about multi searches.
 */
static EB_Error_Code
output_multi_information(EB_Book *book)
{
    EB_Error_Code return_code = EB_SUCCESS;
    EB_Error_Code error_code;
    EB_Multi_Search_Code multi_list[EB_MAX_MULTI_SEARCHES];
    int multi_count;
    int entry_count;
    char search_title[EB_MAX_MULTI_TITLE_LENGTH + 1];
    char entry_label[EB_MAX_MULTI_LABEL_LENGTH + 1];
    int i, j;

    /*
     * Get a list of mutli search codes.
     */
    error_code = eb_multi_search_list(book, multi_list, &multi_count);
    if (error_code != EB_SUCCESS) {
	output_error_message(error_code);
	return_code = error_code;
	multi_count = 0;
    }

    /*
     * Output information.
     */
    for (i = 0; i < multi_count; i++) {
	printf(_("  multi search %d:\n"), i + 1);
	error_code = eb_multi_entry_count(book, multi_list[i], &entry_count);
	if (error_code != EB_SUCCESS) {
	    output_error_message(error_code);
	    return_code = error_code;
	    continue;
	}

	error_code = eb_multi_title(book, multi_list[i], search_title);
	if (error_code != EB_SUCCESS) {
	    output_error_message(error_code);
	    return_code = error_code;
	    continue;
	}
	fputs(_("    title: "), stdout);
	fputs_eucjp_to_locale(search_title, stdout);
	fputc('\n', stdout);

	for (j = 0; j < entry_count; j++) {
	    error_code = eb_multi_entry_label(book, multi_list[i], j,
		entry_label);
	    if (error_code != EB_SUCCESS) {
		output_error_message(error_code);
		return_code = error_code;
		continue;
	    }

	    printf(_("    label %d: "), j + 1);
	    fputs_eucjp_to_locale(entry_label, stdout);
	    fputc('\n', stdout);

	    fputs(_("      candidates: "), stdout);
	    if (eb_multi_entry_have_candidates(book, multi_list[i], j))
		fputs(_("exist\n"), stdout);
	    else
		fputs(_("not-exist\n"), stdout);
	}
    }

    fflush(stdout);

    return return_code;
}


/*
 * Output help message to stdandard out.
 */
static void
output_help(void)
{
    printf(_("Usage: %s [option...] [book-directory]\n"),
	program_name);
    printf(_("Options:\n"));
    printf(_("  -h  --help                 display this help, then exit\n"));
    printf(_("  -l  --book-list            output a list of books on an EBENT server\n"));
    printf(_("  -m  --multi-search         also output multi-search information\n"));
    printf(_("  -v  --version              display version number, then exit\n"));
    printf(_("\nArgument:\n"));
    printf(_("  book-directory             top directory of a CD-ROM book\n"));
    printf(_("                             (default: %s)\n"),
	DEFAULT_BOOK_DIRECTORY);
    printf(_("\nReport bugs to %s.\n"), MAILING_ADDRESS);
    fflush(stdout);
}


