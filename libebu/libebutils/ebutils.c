/*                                                            -*- C -*-
 * Copyright (c) 1997-2006  Motoyuki Kasahara
 * Copyright (c) 2008-2013  Kazuhiro Ito
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
#include "ebutils.h"

/*
 * Test whether `off_t' represents a large integer.
 */
#define off_t_is_large \
	((((off_t) 1 << 41) + ((off_t) 1 << 40) + 1) % 9999991 == 7852006)

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

#if defined(DOS_FILE_PATH) && defined(HAVE_MBSTRING_H)
/* path may contain double-byte chars in SJIS. */
#include <mbstring.h>
#define strchr	_mbschr
#define strrchr	_mbsrchr
#endif

/*
 * Output ``try ...'' message to standard error.
 */
void
output_try_help(const char *invoked_name)
{
    fprintf(stderr, _("try `%s --help' for more information\n"), invoked_name);
    fflush(stderr);
}


/*
 * Output version number to stdandard out.
 */
void
output_version(const char *program_name, const char *program_version)
{
    int tmp = 0;

    printf("%s (EB Library with UTF-8 support) version %s\n", program_name, program_version);
    printf("Features:");
    if off_t_is_large {
	    printf(" LARGEFILE");
	    tmp = 1;
	}
#ifdef ENABLE_PTHREAD
    if (tmp)
	printf(", ");
    else
	printf(" ");
    printf("PTHREAD");
    tmp = 1;
#endif
#ifdef ENABLE_EBNET
    if (tmp)
	printf(", ");
    else
	printf(" ");
    printf("EBNET");
    tmp = 1;
#endif
#ifdef ENABLE_IPV6
    if (tmp)
	printf(", ");
    else
	printf(" ");
    printf("IPV6");
    tmp = 1;
#endif
#ifdef ENABLE_WINSOCK2
    if (tmp)
	printf(", ");
    else
	printf(" ");
    printf("WINSOCK2");
    tmp = 1;
#endif
#ifdef ENABLE_NLS
    if (tmp)
	printf(", ");
    else
	printf(" ");
    printf("NLS");
    tmp = 1;
#endif
#ifdef ENABLE_LIBDEFLATE
    if (tmp)
	printf(", ");
    else
	printf(" ");
    printf("LIBDEFLATE");
    tmp = 1;
#endif
    printf("\n");
    printf("Copyright (c) 1997-2006  Motoyuki Kasahara\n");
    printf("Copyright (c) 2008-2019  Kazuhiro Ito\n");
    fflush(stdout);
}


/*
 * Parse an argument to option `--subbook (-S)'.
 * If the argument is valid form, 0 is returned.
 * Otherwise -1 is returned.
 */
int
parse_subbook_name_argument(const char *invoked_name, const char *argument,
    char name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1], int *name_count)
{
    const char *argument_p = argument;
    char name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
    char *name_p;
    int i;

    while (*argument_p != '\0') {
	/*
	 * Check current `name_count'.
	 */
	if (EB_MAX_SUBBOOKS <= *name_count) {
	    fprintf(stderr, _("%s: too many subbooks\n"), invoked_name);
	    fflush(stderr);
	    return -1;
	}

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
	    fprintf(stderr, _("%s: invalid subbook name `%s...'\n"),
		invoked_name, name);
	    fflush(stderr);
	    return -1;
	}

	/*
	 * If the subbook name is not found in the subbook name list,
	 * it is added to the list.
	 */
	for (i = 0; i < *name_count; i++) {
	    if (strcmp(name, name_list[i]) == 0)
		break;
	}
	if (*name_count <= i) {
	    strcpy(name_list[i], name);
	    (*name_count)++;
	}
    }

    return 0;
}


/*
 * Find a subbook-code of the subbook whose directory name is `directory'.
 * When no sub-book is matched', EB_ERR_NO_SUCH_SUB is returned.
 */
EB_Subbook_Code
find_subbook(EB_Book *book, const char *directory,
    EB_Subbook_Code *subbook_code)
{
    EB_Error_Code error_code;
    EB_Subbook_Code subbook_list[EB_MAX_SUBBOOKS];
    char directory2[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
    int subbook_count;
    int i;

    /*
     * Find the subbook in the current book.
     */
    error_code = eb_subbook_list(book, subbook_list, &subbook_count);
    if (error_code != EB_SUCCESS) {
	*subbook_code = EB_SUBBOOK_INVALID;
	return EB_ERR_NO_SUCH_SUB;
    }
    for (i = 0; i < subbook_count; i++) {
        error_code = eb_subbook_directory2(book, subbook_list[i], directory2);
	if (error_code != EB_SUCCESS)
	    continue;
        if (strcasecmp(directory, directory2) == 0) {
            *subbook_code = subbook_list[i];
	    return EB_SUCCESS;
	}
    }

    *subbook_code = EB_SUBBOOK_INVALID;
    return EB_ERR_NO_SUCH_SUB;
}


#ifndef DOS_FILE_PATH

/*
 * Canonicalize `path' (UNIX version).
 * It eliminaes `/' at the tail of `path' unless `path' is not "/".
 */
void
canonicalize_path(char *path)
{
    char *last_slash;

    last_slash = strrchr(path, '/');
    if (last_slash == NULL || *(last_slash + 1) != '\0')
	return;

    if (last_slash != path)
        *last_slash = '\0';
}

#else /* DOS_FILE_PATH */

/*
 * Canonicalize `path' (DOS version).
 * It eliminaes `\' at the tail of `path' unless `path' is not "X:\".
 */
void
canonicalize_path(char *path)
{
    char *slash;
    char *last_backslash;

    /*
     * Replace `/' with `\\'.
     */
    slash = path;
    for (;;) {
	slash = strchr(slash, '/');
	if (slash == NULL)
	    break;
	*slash++ = '\\';
    }

    last_backslash = strrchr(path, '\\');
    if (last_backslash == NULL || *(last_backslash + 1) != '\0')
	return;

    /*
     * Eliminate `\' in the tail of the path.
     */
    if (ASCII_ISALPHA(*path) && *(path + 1) == ':') {
	if (last_backslash != path + 2)
	    *last_backslash = '\0';
    } else if (*path == '\\' && *(path + 1) == '\\') {
	if (last_backslash != path + 1)
	    *last_backslash = '\0';
    } else if (*path == '\\') {
	if (last_backslash != path)
	    *last_backslash = '\0';
    } else {
	*last_backslash = '\0';
    }
}

#endif /* DOS_FILE_PATH */

